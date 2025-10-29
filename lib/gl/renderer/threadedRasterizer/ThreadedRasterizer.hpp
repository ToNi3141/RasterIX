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

#include "renderer/commands/CommandVariant.hpp"

#include "renderer/Rasterizer.hpp"
#include "renderer/displaylist/DisplayListDisassembler.hpp"
#include "renderer/registers/BaseColorReg.hpp"
#include "renderer/registers/RegisterVariant.hpp"

#include "renderer/threadedRasterizer/DeviceUploadList.hpp"

#include <spdlog/spdlog.h>

namespace rr
{

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
        m_device.blockUntilDeviceIsIdle();
    }

    void streamDisplayList(const uint8_t index, const uint32_t size) override
    {
        const std::function<void()> compute = [this, index, size]()
        {
            displaylist::DisplayList srcList {};
            srcList.setBuffer(requestDisplayListBuffer(index));
            srcList.resetGet();
            srcList.setCurrentSize(size);
            displaylist::DisplayListDisassembler disassembler { srcList };

            if constexpr (!DisplayListDispatcherType::singleList())
            {
                // Write the initial state to the display list. Only necessary when multilists are used
                // to reset the state to the end of the last frame.
                writeRenderStateToDisplayList();
            }

            while (disassembler.hasNextCommand())
            {
                const bool ret = std::visit([this](const auto& cmd)
                    { return handleCommand(cmd); },
                    disassembler.getNextCommand());
                if (!ret)
                {
                    SPDLOG_ERROR("Failed to handle command in display list. This might cause the renderer to crash ...");
                }
            }
            swapAndUploadDisplayLists();
        };
        m_workerThread.wait();
        m_workerThread.run(compute);
    }

    bool writeToDeviceMemory(tcb::span<const uint8_t> data, const uint32_t addr) override
    {
        m_workerThread.wait();
        if (!m_textureUploadList.addPage(data, addr))
        {
            SPDLOG_ERROR("Failed to add page to texture upload list");
            return false;
        }
        return true;
    }

    bool readFromDeviceMemory(tcb::span<uint8_t> data, const uint32_t addr) override
    {
        m_workerThread.wait();
        m_uploadThread.wait();
        return m_device.readFromDeviceMemory(data, addr);
    }

    void blockUntilDeviceIsIdle() override
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

    struct RenderState
    {
        std::array<RegisterVariant, std::variant_size_v<RegisterVariant>> registers {};
        FogLutStreamCmd fogLut {};
        std::array<TextureStreamCmd, RenderConfig::TMU_COUNT> textureStream {};
    };

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
        const std::function<void()> uploader = [this]()
        {
            m_displayListBuffer.getFront().displayListLooper(
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
            m_device.blockUntilDeviceIsIdle();
        };
        m_uploadThread.run(uploader);
    }

    template <typename TArg>
    bool writeReg(const TArg& regVal)
    {
        return addCommand(WriteRegisterCmd { regVal });
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

    void addLineDepthBufferAddresses()
    {
        addCommandWithFactory(
            [this](const std::size_t i, const std::size_t lines, const std::size_t resX, const std::size_t resY)
            {
                const uint32_t screenSize = static_cast<uint32_t>(resY) * resX * 2;
                const uint32_t addr = m_depthBufferAddr + (screenSize * (lines - i - 1));
                return WriteRegisterCmd { DepthBufferAddrReg { addr } };
            });
    }

    void addLineStencilBufferAddresses()
    {
        addCommandWithFactory(
            [this](const std::size_t i, const std::size_t lines, const std::size_t resX, const std::size_t resY)
            {
                const uint32_t screenSize = static_cast<uint32_t>(resY) * resX * 2;
                const uint32_t addr = m_stencilBufferAddr + (screenSize * (lines - i - 1));
                return WriteRegisterCmd { StencilBufferAddrReg { addr } };
            });
    }

    bool handleCommand(const FramebufferCmd& cmd)
    {
        if (cmd.getSwapFramebuffer())
        {
            addLastCommand(WriteRegisterCmd { ColorBufferAddrReg { m_colorBufferAddr } });
            return addLastCommandWithFactory(
                [&cmd](const std::size_t, const std::size_t displayLines, const std::size_t resX, const std::size_t resY)
                {
                    const std::size_t screenSize = resX * resY * displayLines;
                    FramebufferCmd c { cmd.getSelectColorBuffer(), cmd.getSelectDepthBuffer(), cmd.getSelectStencilBuffer(), screenSize };
                    c.swapFramebuffer();
                    return c;
                });
        }

        addLineColorBufferAddresses();
        addLineDepthBufferAddresses();
        addLineStencilBufferAddresses();
        // Clear
        if (cmd.getEnableMemset())
        {
            return addCommandWithFactory_if(
                [&cmd](const std::size_t i, const std::size_t, const std::size_t resX, const std::size_t resY)
                {
                    const std::size_t screenSize = resX * resY;
                    FramebufferCmd c { cmd.getSelectColorBuffer(), cmd.getSelectDepthBuffer(), cmd.getSelectStencilBuffer(), screenSize };
                    c.enableMemset();
                    return c;
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
                    const std::size_t screenSize = resX * resY;
                    FramebufferCmd c { cmd.getSelectColorBuffer(), cmd.getSelectDepthBuffer(), cmd.getSelectStencilBuffer(), screenSize };
                    c.commitFramebuffer();
                    return c;
                });
        }
        // Load
        if (cmd.getLoadFramebuffer())
        {
            return addCommandWithFactory(
                [&cmd](const std::size_t, const std::size_t, const std::size_t resX, const std::size_t resY)
                {
                    const std::size_t screenSize = resX * resY;
                    FramebufferCmd c { cmd.getSelectColorBuffer(), cmd.getSelectDepthBuffer(), cmd.getSelectStencilBuffer(), screenSize };
                    c.loadFramebuffer();
                    return c;
                });
        }
        SPDLOG_CRITICAL("FramebufferCmd was not correctly handled and is ignored. This might cause the renderer to crash ...");
        return true;
    }

    bool handleCommand(const FogLutStreamCmd& cmd)
    {
        if constexpr (!DisplayListDispatcherType::singleList())
        {
            m_renderState.fogLut = cmd;
        }
        return addCommand(cmd);
    }

    bool handleCommand(const TriangleStreamCmd& cmd)
    {
        SPDLOG_CRITICAL("TriangleStreamCmd not allowed in ThreadedRasterizer. This might cause the renderer to crash ...");
        return true;
    }

    bool handleCommand(const PushVertexCmd& cmd)
    {
        return m_vertexTransform.pushVertex(cmd.payload()[0]);
    }

    bool handleCommand(const SetElementGlobalCtxCmd& cmd)
    {
        m_vertexCtx.setElementGlobalData(cmd.payload()[0]);
        return true;
    }

    bool handleCommand(const SetElementLocalCtxCmd& cmd)
    {
        m_vertexCtx.setElementLocalData(cmd.payload()[0]);
        return true;
    }

    bool handleCommand(const SetLightingCtxCmd& cmd)
    {
        m_vertexCtx.lighting = cmd.payload()[0];
        return true;
    }

    bool handleCommand(const DrawNewElementCmd& cmd)
    {
        m_vertexTransform.init();
        return true;
    }

    bool handleCommand(const NopCmd& cmd)
    {
        return addCommand(cmd);
    }

    bool handleCommand(const TextureStreamCmd& cmd)
    {
        if constexpr (!DisplayListDispatcherType::singleList())
        {
            m_renderState.textureStream[cmd.getTmu()] = cmd;
        }
        return addCommand(cmd);
    }

    bool handleCommand(const WriteRegisterCmd& cmd)
    {
        if constexpr (!DisplayListDispatcherType::singleList())
        {
            m_renderState.registers[cmd.getRegister().index()] = cmd.getRegister();
        }
        return std::visit(
            [this](const auto& reg)
            {
                return handleRegister(reg);
            },
            cmd.getRegister());
    }

    bool handleRegister(const ColorBufferAddrReg& reg)
    {
        m_colorBufferAddr = reg.getValue();
        return true; // The color buffer address will be set when a framebuffer command is handled
    }

    bool handleRegister(const ColorBufferClearColorReg& reg)
    {
        return writeReg(reg);
    }

    bool handleRegister(const DepthBufferAddrReg& reg)
    {
        m_depthBufferAddr = reg.getValue();
        return writeReg(reg);
    }

    bool handleRegister(const DepthBufferClearDepthReg& reg)
    {
        return writeReg(reg);
    }

    bool handleRegister(const FeatureEnableReg& reg)
    {
        m_rasterizer.enableScissor(reg.getEnableScissor());
        m_rasterizer.enableTmu(0, reg.getEnableTmu(0));
        m_rasterizer.enableTmu(1, reg.getEnableTmu(1));
        m_scissorEnabled = reg.getEnableScissor();
        return writeReg(reg);
    }

    bool handleRegister(const FogColorReg& reg)
    {
        return writeReg(reg);
    }

    bool handleRegister(const FragmentPipelineReg& reg)
    {
        return writeReg(reg);
    }

    bool handleRegister(RenderResolutionReg reg)
    {
        if (!m_displayListBuffer.getBack().setResolution(reg.getX(), reg.getY())
            || !m_displayListBuffer.getFront().setResolution(reg.getX(), reg.getY()))
        {
            SPDLOG_ERROR("Invalid resolution set in RenderResolutionReg: {}x{}",
                reg.getX(), reg.getY());
            return false;
        }
        reg.setY(m_displayListBuffer.getBack().getYLineResolution());
        return writeReg(reg);
    }

    bool handleRegister(const ScissorEndReg& reg)
    {
        m_rasterizer.setScissorEnd(reg.getX(), reg.getY());
        m_scissorYEnd = reg.getY();
        return writeReg(reg);
    }

    bool handleRegister(const ScissorStartReg& reg)
    {
        m_rasterizer.setScissorStart(reg.getX(), reg.getY());
        m_scissorYStart = reg.getY();
        return writeReg(reg);
    }

    bool handleRegister(const StencilBufferAddrReg& reg)
    {
        m_stencilBufferAddr = reg.getValue();
        return writeReg(reg);
    }

    bool handleRegister(const StencilReg& reg)
    {
        return writeReg(reg);
    }

    bool handleRegister(const TexEnvColorReg& reg)
    {
        return writeReg(reg);
    }

    bool handleRegister(const TexEnvReg& reg)
    {
        return writeReg(reg);
    }

    bool handleRegister(const TmuTextureReg& reg)
    {
        return writeReg(reg);
    }

    bool handleRegister(const YOffsetReg&)
    {
        return addCommandWithFactory(
            [](const std::size_t i, const std::size_t, const std::size_t, const std::size_t resY)
            {
                const uint16_t yOffset = i * resY;
                return WriteRegisterCmd { YOffsetReg { 0, yOffset } };
            });
    }

    bool handleRegister(const std::monostate&)
    {
        return true;
    }

    void swapAndUploadDisplayLists()
    {
        m_uploadThread.wait();
        switchDisplayLists();
        textureUpload();
        uploadDisplayList();
    }

    void intermediateUpload()
    {
        if (m_displayListBuffer.getBack().singleList())
        {
            swapAndUploadDisplayLists();
        }
        else
        {
            SPDLOG_CRITICAL("Intermediate upload called, but display list is not single list. This might cause the renderer to crash ...");
        }
    }

    bool setStencilBufferConfig(const StencilReg& stencilConf)
    {
        return m_displayListBuffer.getBack().addCommand(WriteRegisterCmd { stencilConf });
    }

    void textureUpload()
    {
        if (m_textureUploadList.empty())
        {
            return;
        }

        while (!m_textureUploadList.atEnd())
        {
            const auto* page = m_textureUploadList.getNextPage();
            if (page)
            {
                if (!m_device.writeToDeviceMemory(page->data, page->addr))
                {
                    SPDLOG_ERROR("Failed to write texture data to device memory");
                }
            }
        }
        m_textureUploadList.clear();
        m_device.blockUntilDeviceIsIdle();
    }

    void writeRenderStateToDisplayList()
    {
        for (const auto& r : m_renderState.registers)
        {
            std::visit(
                [this](const auto& reg)
                {
                    handleRegister(reg);
                },
                r);
        }
        handleCommand(m_renderState.fogLut);
        for (const auto& ts : m_renderState.textureStream)
        {
            handleCommand(ts);
        }
    }

    using ConcreteDisplayListAssembler = displaylist::DisplayListAssembler<RenderConfig::TMU_COUNT, displaylist::DisplayList, false>;
    static constexpr std::size_t NUMBER_OF_DISPLAY_LISTS { 2 };

    IDevice& m_device;
    IThreadRunner& m_uploadThread;
    IThreadRunner& m_workerThread;
    std::array<DisplayListAssemblerArrayType, 2> m_displayListAssembler {};
    std::array<DisplayListDispatcherType, 2> m_displayListDispatcher { m_displayListAssembler[0], m_displayListAssembler[1] };
    DisplayListDoubleBufferType m_displayListBuffer { m_displayListDispatcher[0], m_displayListDispatcher[1] };
    DeviceUploadList<RenderConfig::THREADED_RASTERIZATION_DISPLAY_LIST_BUFFER_SIZE, RenderConfig::TEXTURE_PAGE_SIZE> m_textureUploadList {};

    std::array<std::array<uint8_t, RenderConfig::THREADED_RASTERIZATION_DISPLAY_LIST_BUFFER_SIZE>, NUMBER_OF_DISPLAY_LISTS> m_buffer;

    Rasterizer m_rasterizer { !RenderConfig::USE_FLOAT_INTERPOLATION };

    const std::function<bool(const TransformedTriangle&)> m_drawTriangleLambda = [this](const TransformedTriangle& triangle)
    { return addTriangleCmd(triangle); };
    const std::function<bool(const StencilReg&)> m_setStencilBufferConfigLambda = [this](const StencilReg& stencilConf)
    { return setStencilBufferConfig(stencilConf); };

    vertextransforming::VertexTransformingData m_vertexCtx {};
    vertextransforming::VertexTransformingCalc<decltype(m_drawTriangleLambda), decltype(m_setStencilBufferConfigLambda)> m_vertexTransform {
        m_vertexCtx,
        m_drawTriangleLambda,
        m_setStencilBufferConfigLambda,
    };

    RenderState m_renderState {};
    uint32_t m_colorBufferAddr {};
    uint32_t m_depthBufferAddr {};
    uint32_t m_stencilBufferAddr {};
    bool m_scissorEnabled { false };
    int32_t m_scissorYStart { 0 };
    int32_t m_scissorYEnd { 0 };
};

} // namespace rr

#endif // _THREADED_RASTERIZER_HPP_
