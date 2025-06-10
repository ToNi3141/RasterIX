// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2025 ToNi3141

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef _THREADED_RASTERIZER_HPP_
#define _THREADED_RASTERIZER_HPP_

#include "renderer/IDevice.hpp"
#include "renderer/displaylist/DisplayList.hpp"
#include "renderer/displaylist/DisplayListAssembler.hpp"
#include "renderer/displaylist/DisplayListDoubleBuffer.hpp"
#include "renderer/displaylist/RIXDisplayListAssembler.hpp"
#include <cstdint>
#include <tcb/span.hpp>

#include "renderer/commands/FogLutStreamCmd.hpp"
#include "renderer/commands/FramebufferCmd.hpp"
#include "renderer/commands/NopCmd.hpp"
#include "renderer/commands/PushVertexCmd.hpp"
#include "renderer/commands/RegularTriangleCmd.hpp"
#include "renderer/commands/SetVertexCtxCmd.hpp"
#include "renderer/commands/TextureStreamCmd.hpp"
#include "renderer/commands/TriangleStreamCmd.hpp"
#include "renderer/commands/WriteRegisterCmd.hpp"

#include "renderer/Rasterizer.hpp"
#include "renderer/registers/BaseColorReg.hpp"

#include "renderer/registers/FeatureEnableReg.hpp"
#include "renderer/registers/ScissorEndReg.hpp"
#include "renderer/registers/ScissorStartReg.hpp"

#include <spdlog/spdlog.h>

namespace rr
{

template <std::size_t BUFFER_COUNT, std::size_t BUFFER_SIZE>
class ThreadedRasterizer : public IDevice
{
public:
    ThreadedRasterizer(IDevice& device, IThreadRunner& uploadThread, IThreadRunner& workerThread)
        : m_device { device }
        , m_uploadThread { uploadThread }
        , m_workerThread { workerThread }
    {
        initDisplayLists();
        SPDLOG_INFO("Treaded rasterization enabled");
    }

    void deinit()
    {
        m_workerThread.wait();
        m_uploadThread.wait();
        m_device.waitTillDeviceIsIdle();
    }

    void streamDisplayList(const uint8_t index, const uint32_t size) override
    {
        const std::function<bool()> compute = [this, index, size]()
        {
            displaylist::DisplayList srcList {};
            srcList.setBuffer(requestDisplayListBuffer(index));
            srcList.resetGet();
            srcList.setCurrentSize(size);

            while (!srcList.atEnd())
            {
                if (!decodeAndCopyCommand(srcList))
                {
                    SPDLOG_CRITICAL("Decoding of displaylist failed.");
                }
            }
            swapAndUploadDisplayLists();
            return true;
        };
        m_workerThread.wait();
        m_workerThread.run(compute);
    }

    void writeToDeviceMemory(tcb::span<const uint8_t> data, const uint32_t addr) override
    {
        m_workerThread.wait();
        m_uploadThread.wait();
        m_device.waitTillDeviceIsIdle();
        m_device.writeToDeviceMemory(data, addr);
    }

    void waitTillDeviceIsIdle() override
    {
        m_workerThread.wait();
    }

    tcb::span<uint8_t> requestDisplayListBuffer(const uint8_t index) override
    {
        return { m_buffer[index] };
    }

    uint8_t getDisplayListBufferCount() const override
    {
        return m_buffer.size();
    }

private:
    using DisplayListAssemblerType = displaylist::DisplayListAssembler<RenderConfig::TMU_COUNT, displaylist::DisplayList>;
    using DisplayListAssemblerArrayType = std::array<DisplayListAssemblerType, RenderConfig::getDisplayLines()>;
    using DisplayListDispatcherType = displaylist::DisplayListDispatcher<RenderConfig, DisplayListAssemblerArrayType>;
    using DisplayListDoubleBufferType = displaylist::DisplayListDoubleBuffer<DisplayListDispatcherType>;

    void initDisplayLists()
    {
        for (std::size_t i = 0, buffId = 0; i < m_displayListAssembler[0].size(); i++)
        {
            m_displayListAssembler[0][i].setBuffer(m_device.requestDisplayListBuffer(buffId), buffId);
            buffId++;
            m_displayListAssembler[1][i].setBuffer(m_device.requestDisplayListBuffer(buffId), buffId);
            buffId++;
        }
        m_displayListBuffer.getFront().clearDisplayListAssembler();
        m_displayListBuffer.getBack().clearDisplayListAssembler();
    }

    void uploadDisplayList()
    {
        const std::function<bool()> uploader = [this]()
        {
            const bool ret =  m_displayListBuffer.getFront().displayListLooper(
                [this](
                    DisplayListDispatcherType& dispatcher,
                    const std::size_t i,
                    const std::size_t,
                    const std::size_t,
                    const std::size_t)
                {
                    if (dispatcher.getDisplayListSize(i) > 0)
                    {
                        m_device.streamDisplayList(
                            dispatcher.getDisplayListBufferId(i),
                            dispatcher.getDisplayListSize(i));
                    }
                    return true;
                });
            m_device.waitTillDeviceIsIdle();
            return ret;
        };
        m_uploadThread.run(uploader);
    }

    bool setVertexCtx(displaylist::DisplayList& src)
    {
        using PayloadType = typename std::remove_const<typename std::remove_reference<decltype(SetVertexCtxCmd {}.payload()[0])>::type>::type;
        src.getNext<typename SetVertexCtxCmd::CommandType>();
        const PayloadType* t = src.getNext<PayloadType>();

        new (&m_vertexTransform) vertextransforming::VertexTransformingCalc<decltype(drawTriangleLambda), decltype(setStencilBufferConfigLambda)> {
            t->ctx,
            drawTriangleLambda,
            setStencilBufferConfigLambda,
        };

        return true;
    }

    bool pushVertex(displaylist::DisplayList& src)
    {
        using PayloadType = typename std::remove_const<typename std::remove_reference<decltype(PushVertexCmd {}.payload()[0])>::type>::type;
        src.getNext<typename PushVertexCmd::CommandType>();
        return m_vertexTransform.pushVertex(src.getNext<PayloadType>()->vertex);
    }

    template <typename TArg>
    bool writeReg(const TArg& regVal)
    {
        return addCommand(WriteRegisterCmd { regVal });
    }

    template <typename TCmd>
    bool copyCmd(displaylist::DisplayList& src)
    {
        using PayloadType = typename std::remove_const<typename std::remove_reference<decltype(TCmd {}.payload()[0])>::type>::type;
        const typename TCmd::CommandType* op = src.getNext<typename TCmd::CommandType>();
        const PayloadType* pl = nullptr;
        const std::size_t numberOfElements = TCmd::getNumberOfElementsInPayloadByCommand(*op);
        if (numberOfElements > 0)
        {
            pl = src.getNext<PayloadType>(); // store start addr
            // push display list to the end of the payload
            for (std::size_t i = 0; i < numberOfElements - 1; i++)
            {
                src.getNext<PayloadType>();
            }
        }
        // The third argument exists because sometimes (TextureStreamCmd) the constructors are ambiguous.
        // The bool enforces the correct constructor
        TCmd cmd { *op, { pl, numberOfElements }, true };
        return addCommand(cmd);
    }
    template <typename Command>
    bool addCommand(const Command& cmd)
    {
        bool ret = m_displayListBuffer.getBack().addCommand(cmd);
        if (!ret)
        {
            intermediateUpload();
            ret = m_displayListBuffer.getBack().addCommand(cmd);
        }
        return ret;
    }

    template <typename Command>
    bool addLastCommand(const Command& cmd)
    {
        return m_displayListBuffer.getBack().addLastCommand(cmd);
    }

    template <typename Factory>
    bool addLastCommandWithFactory(const Factory& commandFactory)
    {
        return m_displayListBuffer.getBack().addLastCommandWithFactory_if(commandFactory,
            [](std::size_t, std::size_t, std::size_t, std::size_t)
            { return true; });
    }

    template <typename Factory>
    bool addCommandWithFactory(const Factory& commandFactory)
    {
        return addCommandWithFactory_if(commandFactory,
            [](std::size_t, std::size_t, std::size_t, std::size_t)
            { return true; });
    }

    template <typename Factory, typename Pred>
    bool addCommandWithFactory_if(const Factory& commandFactory, const Pred& pred)
    {
        return m_displayListBuffer.getBack().addCommandWithFactory_if(commandFactory, pred);
    }

    template <typename Function>
    bool displayListLooper(const Function& func)
    {
        return m_displayListBuffer.getBack().displayListLooper(func);
    }

    void switchDisplayLists()
    {
        m_uploadThread.wait();
        m_displayListBuffer.swap();
        m_displayListBuffer.getBack().clearDisplayListAssembler();
    }

    template <typename TriangleCmd>
    bool addMultiListTriangle(TriangleCmd& triangleCmd)
    {
        const auto factory = [&triangleCmd](DisplayListDispatcherType& dispatcher, const std::size_t i, const std::size_t, const std::size_t, const std::size_t resY)
        {
            const std::size_t currentScreenPositionStart = i * resY;
            const std::size_t currentScreenPositionEnd = currentScreenPositionStart + resY;
            if (triangleCmd.isInBounds(currentScreenPositionStart, currentScreenPositionEnd))
            {
                // The floating point rasterizer can automatically increment all attributes
                if constexpr (RenderConfig::USE_FLOAT_INTERPOLATION)
                {
                    return dispatcher.addCommand(i, triangleCmd);
                }
                else
                {
                    return dispatcher.addCommand(i, triangleCmd.getIncremented(currentScreenPositionStart, currentScreenPositionEnd));
                }
            }
            return true;
        };
        return displayListLooper(factory);
    }

    bool addTriangleCmd(const TransformedTriangle& triangle)
    {
        TriangleStreamCmd triangleCmd { m_rasterizer, triangle };
        if (!triangleCmd.isVisible())
        {
            return true;
        }

        if constexpr (DisplayListDispatcherType::singleList())
        {
            return addCommand(triangleCmd);
        }
        else
        {
            return addMultiListTriangle(triangleCmd);
        }
        return true;
    }

    bool handleFramebufferCmd(displaylist::DisplayList& src)
    {
        const FramebufferCmd::CommandType* op = src.getNext<typename FramebufferCmd::CommandType>();
        FramebufferCmd cmd { *op, {}, true };
        if (cmd.getSwapFramebuffer())
        {
            addLastCommand(WriteRegisterCmd { ColorBufferAddrReg { m_colorBufferAddr } });
            return addLastCommandWithFactory(
                [&cmd](const std::size_t, const std::size_t displayLines, const std::size_t resX, const std::size_t resY)
                {
                    const std::size_t screenSize = resX * resY * displayLines;
                    cmd.setFramebufferSizeInPixel(screenSize);
                    return cmd;
                });
        }

        addLineColorBufferAddresses();
        // Clear
        if (cmd.getEnableMemset())
        {
            return addCommandWithFactory_if(
                [&cmd](const std::size_t, const std::size_t, const std::size_t resX, const std::size_t resY)
                {
                    const uint32_t screenSize = resX * resY;
                    cmd.setFramebufferSizeInPixel(screenSize);
                    return cmd;
                },
                [this](const std::size_t i, const std::size_t, const std::size_t, const std::size_t resY)
                {
                    if (m_scissorEnabled)
                    {
                        const std::size_t currentScreenPositionStart = i * resY;
                        const std::size_t currentScreenPositionEnd = (i + 1) * resY;
                        if ((static_cast<int32_t>(currentScreenPositionEnd) >= m_scissorYStart)
                            && (static_cast<int32_t>(currentScreenPositionStart) < m_scissorYEnd))
                        {
                            return true;
                        }
                    }
                    else
                    {
                        return true;
                    }
                    return false;
                });
        }
        // Commit
        if (cmd.getCommitFramebuffer())
        {
            return addCommandWithFactory(
                [&cmd](const std::size_t, const std::size_t, const std::size_t resX, const std::size_t resY)
                {
                    // The EF config requires a NopCmd or another command like a commit (which is in this config a Nop)
                    // to flush the pipeline. This is the easiest way to solve WAR conflicts.
                    // This command is required for the IF config.
                    const uint32_t screenSize = resX * resY;
                    cmd.setFramebufferSizeInPixel(screenSize);
                    return cmd;
                });
        }
        SPDLOG_CRITICAL("FramebufferCmd was not correctly handled and is ignored. This might cause the renderer to crash ...");
        return true;
    }

    void addLineColorBufferAddresses()
    {
        addCommandWithFactory(
            [this](const std::size_t i, const std::size_t lines, const std::size_t resX, const std::size_t resY)
            {
                const uint32_t screenSize = static_cast<uint32_t>(resY) * resX * 2;
                const uint32_t addr = m_colorBufferAddr + (screenSize * (lines - i - 1));
                return WriteRegisterCmd { ColorBufferAddrReg { addr } };
            });
    }

    bool handleWriteRegisterCmd(displaylist::DisplayList& src)
    {
        const uint32_t op = *(src.lookAhead<uint32_t>(1));
        const uint32_t regData = *(src.lookAhead<uint32_t>(2));
        switch (WriteRegisterCmd<BaseColorReg>::getRegAddr(op))
        {
        case FeatureEnableReg::getAddr():
        {
            FeatureEnableReg reg {};
            reg.deserialize(regData);
            m_rasterizer.enableScissor(reg.getEnableScissor());
            m_rasterizer.enableTmu(0, reg.getEnableTmu(0));
            m_rasterizer.enableTmu(1, reg.getEnableTmu(1));
            m_scissorEnabled = reg.getEnableScissor();
            return copyCmd<WriteRegisterCmd<FeatureEnableReg>>(src);
        }
        break;
        case ScissorStartReg::getAddr():
        {
            ScissorStartReg reg {};
            reg.deserialize(regData);
            m_rasterizer.setScissorStart(reg.getX(), reg.getY());
            m_scissorYStart = reg.getY();
            return copyCmd<WriteRegisterCmd<ScissorStartReg>>(src);
        }
        break;
        case ScissorEndReg::getAddr():
        {
            ScissorEndReg reg {};
            reg.deserialize(regData);
            m_rasterizer.setScissorEnd(reg.getX(), reg.getY());
            m_scissorYEnd = reg.getY();
            return copyCmd<WriteRegisterCmd<ScissorEndReg>>(src);
        }
        break;
        case ColorBufferAddrReg::getAddr():
        {
            ColorBufferAddrReg reg {};
            reg.deserialize(regData);
            m_colorBufferAddr = reg.getValue();
            return copyCmd<WriteRegisterCmd<ColorBufferAddrReg>>(src);
        }
        break;
        case YOffsetReg::getAddr():
        {
            src.getNext<uint32_t>(); // op
            src.getNext<uint32_t>(); // payload
            return addCommandWithFactory(
                [](const std::size_t i, const std::size_t, const std::size_t, const std::size_t resY)
                {
                    const uint16_t yOffset = i * resY;
                    return WriteRegisterCmd<YOffsetReg> { YOffsetReg { 0, yOffset } };
                });
        }
        break;
        case RenderResolutionReg::getAddr():
        {
            RenderResolutionReg reg {};
            reg.deserialize(regData);
            if (!m_displayListBuffer.getBack().setResolution(reg.getX(), reg.getY())
                || !m_displayListBuffer.getFront().setResolution(reg.getX(), reg.getY()))
            {
                return false;
            }
            reg.setY(m_displayListBuffer.getBack().getYLineResolution());
            src.getNext<uint32_t>(); // op
            src.getNext<uint32_t>(); // payload
            return writeReg(reg);
        }
        break;
        default:
            return copyCmd<WriteRegisterCmd<BaseColorReg>>(src);
        }
        return false;
    }

    bool decodeAndCopyCommand(displaylist::DisplayList& srcList)
    {
        const uint32_t op = *(srcList.lookAhead<uint32_t>());
        bool ret = true;

        if (PushVertexCmd::isThis(op))
        {
            ret = pushVertex(srcList);
        }
        else if (SetVertexCtxCmd::isThis(op))
        {
            ret = setVertexCtx(srcList);
        }
        else if (WriteRegisterCmd<BaseColorReg>::isThis(op))
        {
            ret = handleWriteRegisterCmd(srcList);
        }
        else if (NopCmd::isThis(op))
        {
            ret = copyCmd<NopCmd>(srcList);
        }
        else if (TextureStreamCmd::isThis(op))
        {
            ret = copyCmd<TextureStreamCmd>(srcList);
        }
        else if (FramebufferCmd::isThis(op))
        {
            ret = handleFramebufferCmd(srcList);
        }
        else if (FogLutStreamCmd::isThis(op))
        {
            ret = copyCmd<FogLutStreamCmd>(srcList);
        }
        else if (RegularTriangleCmd::isThis(op))
        {
            SPDLOG_CRITICAL("RegularTriangleCmd not allowed in ThreadedRasterizer. This might cause the renderer to crash ...");
            ret = false;
        }
        else if (TriangleStreamCmd::isThis(op))
        {
            SPDLOG_CRITICAL("TriangleStreamCmd not allowed in ThreadedRasterizer. This might cause the renderer to crash ...");
            ret = false;
        }
        else
        {
            SPDLOG_CRITICAL("Unknown command (0x{:X})found. This might cause the renderer to crash ...", op);
            ret = false;
        }
        return ret;
    }

    void swapAndUploadDisplayLists()
    {
        switchDisplayLists();
        uploadDisplayList();
    }

    void intermediateUpload()
    {
        if (m_displayListBuffer.getBack().singleList())
        {
            swapAndUploadDisplayLists();
        }
    }

    bool setStencilBufferConfig(const StencilReg& stencilConf)
    {
        return m_displayListBuffer.getBack().addCommand(WriteRegisterCmd<StencilReg> { stencilConf });
    }

    using ConcreteDisplayListAssembler = displaylist::DisplayListAssembler<RenderConfig::TMU_COUNT, displaylist::DisplayList, false>;

    IDevice& m_device;
    IThreadRunner& m_uploadThread;
    IThreadRunner& m_workerThread;
    std::array<DisplayListAssemblerArrayType, 2> m_displayListAssembler {};
    std::array<DisplayListDispatcherType, 2> m_displayListDispatcher { m_displayListAssembler[0], m_displayListAssembler[1] };
    DisplayListDoubleBufferType m_displayListBuffer { m_displayListDispatcher[0], m_displayListDispatcher[1] };

    std::array<std::array<uint8_t, BUFFER_SIZE>, BUFFER_COUNT> m_buffer;

    Rasterizer m_rasterizer { !RenderConfig::USE_FLOAT_INTERPOLATION };

    const std::function<bool(const TransformedTriangle&)> drawTriangleLambda = [this](const TransformedTriangle& triangle)
    { return addTriangleCmd(triangle); };
    const std::function<bool(const StencilReg&)> setStencilBufferConfigLambda = [this](const StencilReg& stencilConf)
    { return setStencilBufferConfig(stencilConf); };

    vertextransforming::VertexTransformingCalc<decltype(drawTriangleLambda), decltype(setStencilBufferConfigLambda)> m_vertexTransform {
        {},
        drawTriangleLambda,
        setStencilBufferConfigLambda,
    };

    uint32_t m_colorBufferAddr {};
    bool m_scissorEnabled { false };
    int32_t m_scissorYStart { 0 };
    int32_t m_scissorYEnd { 0 };
};

} // namespace rr

#endif // _THREADED_RASTERIZER_HPP_
