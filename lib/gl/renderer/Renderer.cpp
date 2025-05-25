// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2023 ToNi3141

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
#include "Renderer.hpp"

namespace rr
{
Renderer::Renderer(IDevice& device, IThreadRunner& runner)
    : m_device { device }
    , m_displayListUploaderThread { runner }
{
    m_displayListBuffer.getBack().clearDisplayListAssembler();
    m_displayListBuffer.getFront().clearDisplayListAssembler();

    initDisplayLists();

    setYOffset();

    setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_1);
    setDepthBufferAddress(RenderConfig::DEPTH_BUFFER_LOC);
    setStencilBufferAddress(RenderConfig::STENCIL_BUFFER_LOC);

    setRenderResolution(RenderConfig::MAX_DISPLAY_WIDTH, RenderConfig::MAX_DISPLAY_HEIGHT);

    // Set initial values
    setTexEnvColor({ 0, Vec4i { 0, 0, 0, 0 } });
    setClearColor({ Vec4i { 0, 0, 0, 0 } });
    setClearDepth({ 65535 });
    setFogColor({ Vec4i { 255, 255, 255, 255 } });
    std::array<float, 33> fogLut {};
    std::fill(fogLut.begin(), fogLut.end(), 1.0f);
    setFogLut(fogLut, 0.0f, (std::numeric_limits<float>::max)()); // Windows defines macros with max ... parenthesis are a work around against build errors.
}

Renderer::~Renderer()
{
    clearDisplayListAssembler();
    setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_0);
    swapScreenToNewColorBuffer();
    switchDisplayLists();
    uploadDisplayList();
    m_displayListUploaderThread.wait();
}

bool Renderer::drawTriangle(const TransformedTriangle& triangle)
{
    if constexpr (RenderConfig::THREADED_RASTERIZATION)
    {
        RegularTriangleCmd triangleCmd { triangle };
        return addTriangleCmd(triangleCmd);
    }
    else
    {
        TriangleStreamCmd triangleCmd { m_rasterizer, triangle };
        return addTriangleCmd(triangleCmd);
    }
}

void Renderer::setVertexContext(const vertextransforming::VertexTransformingData& ctx)
{
    if constexpr (!RenderConfig::THREADED_RASTERIZATION || (RenderConfig::getDisplayLines() > 1))
    {
        new (&m_vertexTransform) vertextransforming::VertexTransformingCalc<decltype(drawTriangleLambda), decltype(setStencilBufferConfigLambda)> {
            ctx,
            drawTriangleLambda,
            setStencilBufferConfigLambda,
        };
    }

    if constexpr (RenderConfig::THREADED_RASTERIZATION && (RenderConfig::getDisplayLines() == 1))
    {
        if (!addCommand(SetVertexCtxCmd { ctx }))
        {
            SPDLOG_CRITICAL("Cannot push vertex context into queue. This may brake the rendering.");
        }
    }
}

void Renderer::initDisplayLists()
{
    for (std::size_t i = 0, buffId = 0; i < m_displayListAssembler[0].size(); i++)
    {
        m_displayListAssembler[0][i].setBuffer(m_device.requestDisplayListBuffer(buffId), buffId);
        buffId++;
        m_displayListAssembler[1][i].setBuffer(m_device.requestDisplayListBuffer(buffId), buffId);
        buffId++;
    }
}

void Renderer::intermediateUpload()
{
    // It can only work for single lists. Loading of partial framebuffers in the rixif config
    // is not supported which is a requirement to get it to work.
    if (m_displayListBuffer.getBack().singleList())
    {
        switchDisplayLists();
        uploadTextures();
        clearDisplayListAssembler();
        uploadDisplayList();
    }
}

void Renderer::swapDisplayList()
{
    addLineColorBufferAddresses();
    addCommitFramebufferCommand();
    addColorBufferAddressOfTheScreen();
    swapScreenToNewColorBuffer();
    switchDisplayLists();
    uploadTextures();
    clearDisplayListAssembler();
    setYOffset();
    swapFramebuffer();
}

void Renderer::addLineColorBufferAddresses()
{
    addCommandWithFactory(
        [this](const std::size_t i, const std::size_t lines, const std::size_t resX, const std::size_t resY)
        {
            const uint32_t screenSize = static_cast<uint32_t>(resY) * resX * 2;
            const uint32_t addr = m_colorBufferAddr + (screenSize * (lines - i - 1));
            return WriteRegisterCmd { ColorBufferAddrReg { addr } };
        });
}

void Renderer::addCommitFramebufferCommand()
{
    addCommandWithFactory(
        [](const std::size_t, const std::size_t, const std::size_t resX, const std::size_t resY)
        {
            // The EF config requires a NopCmd or another command like a commit (which is in this config a Nop)
            // to flush the pipeline. This is the easiest way to solve WAR conflicts.
            // This command is required for the IF config.
            const uint32_t screenSize = resY * resX;
            FramebufferCmd cmd { true, true, true, screenSize };
            cmd.commitFramebuffer();
            return cmd;
        });
}

void Renderer::addColorBufferAddressOfTheScreen()
{
    // The last list is responsible for the overall system state
    addLastCommand(WriteRegisterCmd { ColorBufferAddrReg { m_colorBufferAddr } });
}

void Renderer::swapScreenToNewColorBuffer()
{
    addLastCommandWithFactory(
        [this](const std::size_t, const std::size_t, const std::size_t resX, const std::size_t resY)
        {
            const std::size_t screenSize = resY * resX;
            FramebufferCmd cmd { false, false, false, screenSize };
            cmd.selectColorBuffer();
            cmd.swapFramebuffer();
            if (m_enableVSync)
            {
                cmd.enableVSync();
            }
            return cmd;
        });
}

void Renderer::setYOffset()
{
    addCommandWithFactory(
        [](const std::size_t i, const std::size_t, const std::size_t, const std::size_t resY)
        {
            const uint16_t yOffset = i * resY;
            return WriteRegisterCmd<YOffsetReg> { YOffsetReg { 0, yOffset } };
        });
}

void Renderer::uploadDisplayList()
{
    const std::function<bool()> uploader = [this]()
    {
        return m_displayListBuffer.getFront().displayListLooper(
            [this](
                DisplayListDispatcherType& dispatcher,
                const std::size_t i,
                const std::size_t,
                const std::size_t,
                const std::size_t)
            {
                while (!m_device.clearToSend())
                    ;
                m_device.streamDisplayList(
                    dispatcher.getDisplayListBufferId(i),
                    dispatcher.getDisplayListSize(i));
                return true;
            });
    };
    m_displayListUploaderThread.run(uploader);
}

bool Renderer::clear(const bool colorBuffer, const bool depthBuffer, const bool stencilBuffer)
{
    return addCommandWithFactory_if(
        [&](const std::size_t, const std::size_t, const std::size_t x, const std::size_t y)
        {
            FramebufferCmd cmd { colorBuffer, depthBuffer, stencilBuffer, x * y };
            cmd.enableMemset();
            return cmd;
        },
        [&](const std::size_t i, const std::size_t, const std::size_t x, const std::size_t y)
        {
            FramebufferCmd cmd { colorBuffer, depthBuffer, stencilBuffer, x * y };
            cmd.enableMemset();
            if (m_scissorEnabled)
            {
                const std::size_t currentScreenPositionStart = i * y;
                const std::size_t currentScreenPositionEnd = (i + 1) * y;
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

bool Renderer::useTexture(const std::size_t tmu, const uint16_t texId)
{
    if (!m_textureManager.textureValid(texId))
    {
        return false;
    }
    bool ret { true };

    const tcb::span<const std::size_t> pages = m_textureManager.getPages(texId);
    ret = ret && addCommand(TextureStreamCmd { tmu, pages });

    TmuTextureReg reg = m_textureManager.getTmuConfig(texId);
    reg.setTmu(tmu);
    ret = ret && addCommand(WriteRegisterCmd { reg });

    return ret;
}

bool Renderer::setFeatureEnableConfig(const FeatureEnableReg& featureEnable)
{
    m_scissorEnabled = featureEnable.getEnableScissor();
    m_rasterizer.enableScissor(featureEnable.getEnableScissor());
    m_rasterizer.enableTmu(0, featureEnable.getEnableTmu(0));
    if constexpr (RenderConfig::TMU_COUNT == 2)
        m_rasterizer.enableTmu(1, featureEnable.getEnableTmu(1));
    static_assert(RenderConfig::TMU_COUNT <= 2, "Adapt following code when the TMU count has changed");
    return writeReg(featureEnable);
}

bool Renderer::setScissorBox(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height)
{
    bool ret = true;

    ScissorStartReg regStart;
    ScissorEndReg regEnd;
    regStart.setX(x);
    regStart.setY(y);
    regEnd.setX(x + width);
    regEnd.setY(y + height);

    ret = ret && writeReg(regStart);
    ret = ret && writeReg(regEnd);

    m_scissorYStart = y;
    m_scissorYEnd = y + height;

    m_rasterizer.setScissorBox(x, y, x + width, y + height);

    return ret;
}

bool Renderer::setTextureWrapModeS(const std::size_t tmu, const uint16_t texId, TextureWrapMode mode)
{
    m_textureManager.setTextureWrapModeS(texId, mode);
    return writeToTextureConfig(tmu, texId, m_textureManager.getTmuConfig(texId));
}

bool Renderer::setTextureWrapModeT(const std::size_t tmu, const uint16_t texId, TextureWrapMode mode)
{
    m_textureManager.setTextureWrapModeT(texId, mode);
    return writeToTextureConfig(tmu, texId, m_textureManager.getTmuConfig(texId));
}

bool Renderer::enableTextureMagFiltering(const std::size_t tmu, const uint16_t texId, bool filter)
{
    m_textureManager.enableTextureMagFiltering(texId, filter);
    return writeToTextureConfig(tmu, texId, m_textureManager.getTmuConfig(texId));
}

bool Renderer::enableTextureMinFiltering(const std::size_t tmu, const uint16_t texId, bool filter)
{
    m_textureManager.enableTextureMinFiltering(texId, filter);
    return writeToTextureConfig(tmu, texId, m_textureManager.getTmuConfig(texId));
}

bool Renderer::setRenderResolution(const std::size_t x, const std::size_t y)
{
    // The resolution must be set on both displaylists
    if (!m_displayListBuffer.getBack().setResolution(x, y)
        || !m_displayListBuffer.getFront().setResolution(x, y))
    {
        return false;
    }

    RenderResolutionReg reg;
    reg.setX(x);
    reg.setY(m_displayListBuffer.getBack().getYLineResolution());
    return writeReg(reg);
}

bool Renderer::writeToTextureConfig(const std::size_t tmu, const uint16_t texId, TmuTextureReg tmuConfig)
{
    tmuConfig.setTmu(tmu);
    return writeReg(tmuConfig);
}

bool Renderer::setColorBufferAddress(const uint32_t addr)
{
    m_colorBufferAddr = addr;
    return writeReg(ColorBufferAddrReg { addr });
}

void Renderer::uploadTextures()
{
    m_displayListUploaderThread.wait();
    m_textureManager.uploadTextures(
        [&](uint32_t gramAddr, const tcb::span<const uint8_t> data)
        {
            while (!m_device.clearToSend())
                ;
            m_device.writeToDeviceMemory(data, gramAddr);
            return true;
        });
}

void Renderer::swapFramebuffer()
{
    if (m_selectedColorBuffer)
    {
        setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_2);
    }
    else
    {
        setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_1);
    }
    m_selectedColorBuffer = !m_selectedColorBuffer;
}

} // namespace rr
