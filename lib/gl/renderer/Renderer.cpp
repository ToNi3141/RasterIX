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
Renderer::Renderer(IDevice& device)
    : m_device { device }
{
    m_displayListBuffer.getBack().clearAssembler();
    m_displayListBuffer.getFront().clearAssembler();

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

void Renderer::deinit()
{
    clearDisplayListAssembler();
    setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_0);
    swapScreenToNewColorBuffer();
    uploadDisplayList();
    switchDisplayLists();
}

bool Renderer::drawTriangle(const TransformedTriangle& triangle)
{
    TriangleStreamCmd triangleCmd { m_rasterizer, triangle };
    if (!triangleCmd.isVisible())
    {
        return true;
    }
    return addCommand(triangleCmd);
}

void Renderer::setVertexContext(const vertextransforming::VertexTransformingData& ctx)
{
    if constexpr (RenderConfig::THREADED_RASTERIZATION)
    {
        if (!addCommand(SetVertexCtxCmd { ctx }))
        {
            SPDLOG_CRITICAL("Cannot push vertex context into queue. This may brake the rendering.");
        }
    }
    else
    {
        new (&m_vertexTransform) vertextransforming::VertexTransformingCalc<decltype(drawTriangleLambda), decltype(setStencilBufferConfigLambda)> {
            ctx,
            drawTriangleLambda,
            setStencilBufferConfigLambda,
        };
    }
}

void Renderer::initDisplayLists()
{
    m_displayListAssembler[0].setBuffer(m_device.requestDisplayListBuffer(0), 0);
    m_displayListAssembler[1].setBuffer(m_device.requestDisplayListBuffer(1), 1);
}

void Renderer::intermediateUpload()
{
    uploadTextures();
    uploadDisplayList();
    switchDisplayLists();
    clearDisplayListAssembler();
}

void Renderer::swapDisplayList()
{
    addCommitFramebufferCommand();
    addColorBufferAddressOfTheScreen();
    swapScreenToNewColorBuffer();
    uploadTextures();
    uploadDisplayList();
    switchDisplayLists();
    clearDisplayListAssembler();
    loadFramebuffer();
    setYOffset();
    swapFramebuffer();
}

void Renderer::loadFramebuffer()
{
    // Loads the framebuffer into the internal framebuffer in the IF config.
    // This is required to have some framebuffer effects like to redraw to an uncleared framebuffer.
    // This command is ignored in the EF config.
    FramebufferCmd cmd { true, true, true, m_resolutionX * m_resolutionY };
    cmd.loadFramebuffer();
    addCommand(cmd);
}

void Renderer::addCommitFramebufferCommand()
{
    // The EF config requires a NopCmd or another command like a commit (which is in this config a Nop)
    // to flush the pipeline. This is the easiest way to solve WAR conflicts.
    // This command is required for the IF config.
    const uint32_t screenSize = m_resolutionX * m_resolutionY;
    FramebufferCmd cmd { true, true, true, screenSize };
    cmd.commitFramebuffer();
    addCommand(cmd);
}

void Renderer::swapScreenToNewColorBuffer()
{
    const std::size_t screenSize = m_resolutionX * m_resolutionY;
    FramebufferCmd cmd { false, false, false, screenSize };
    cmd.selectColorBuffer();
    cmd.swapFramebuffer();
    if (m_enableVSync)
    {
        cmd.enableVSync();
    }
    addCommand(cmd);
}

void Renderer::uploadDisplayList()
{
    m_device.streamDisplayList(
        m_displayListBuffer.getBack().getDisplayListBufferId(),
        m_displayListBuffer.getBack().getDisplayListSize());
}

bool Renderer::clear(const bool colorBuffer, const bool depthBuffer, const bool stencilBuffer)
{
    FramebufferCmd cmd { colorBuffer, depthBuffer, stencilBuffer, m_resolutionX * m_resolutionY };
    cmd.enableMemset();
    return addCommand(cmd);
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
    ret = ret && writeReg(reg);

    return ret;
}

bool Renderer::setFeatureEnableConfig(const FeatureEnableReg& featureEnable)
{
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

    m_rasterizer.setScissorBox(x, y, x + width, y + height);

    return ret;
}

bool Renderer::setTextureWrapModeS(const std::size_t tmu, const uint16_t texId, TextureWrapMode mode)
{
    m_textureManager.setTextureWrapModeS(texId, mode);
    return writeToTextureConfig(tmu, m_textureManager.getTmuConfig(texId));
}

bool Renderer::setTextureWrapModeT(const std::size_t tmu, const uint16_t texId, TextureWrapMode mode)
{
    m_textureManager.setTextureWrapModeT(texId, mode);
    return writeToTextureConfig(tmu, m_textureManager.getTmuConfig(texId));
}

bool Renderer::enableTextureMagFiltering(const std::size_t tmu, const uint16_t texId, bool filter)
{
    m_textureManager.enableTextureMagFiltering(texId, filter);
    return writeToTextureConfig(tmu, m_textureManager.getTmuConfig(texId));
}

bool Renderer::enableTextureMinFiltering(const std::size_t tmu, const uint16_t texId, bool filter)
{
    m_textureManager.enableTextureMinFiltering(texId, filter);
    return writeToTextureConfig(tmu, m_textureManager.getTmuConfig(texId));
}

bool Renderer::setRenderResolution(const std::size_t x, const std::size_t y)
{
    m_resolutionX = x;
    m_resolutionY = y;

    RenderResolutionReg reg;
    reg.setX(x);
    reg.setY(y);
    return writeReg(reg);
}

bool Renderer::writeToTextureConfig(const std::size_t tmu, TmuTextureReg tmuConfig)
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
    m_textureManager.uploadTextures(
        [&](uint32_t gramAddr, const tcb::span<const uint8_t> data)
        {
            return m_device.writeToDeviceMemory(data, gramAddr);
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
