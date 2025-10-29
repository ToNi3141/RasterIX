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

    setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_1);
    setDepthBufferAddress(RenderConfig::DEPTH_BUFFER_LOC);
    setStencilBufferAddress(RenderConfig::STENCIL_BUFFER_LOC);

    // Set initial values
    writeReg(RenderResolutionReg { RenderConfig::MAX_DISPLAY_WIDTH, RenderConfig::MAX_DISPLAY_HEIGHT });
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

void Renderer::setElementGlobalContext(const transform::ElementGlobalData& ctx)
{
    if constexpr (RenderConfig::THREADED_RASTERIZATION)
    {
        if (!addCommand(SetElementGlobalCtxCmd { ctx }))
        {
            SPDLOG_CRITICAL("Cannot push element global context into queue. This may brake the rendering.");
        }
    }
    else
    {
        m_vertexCtx.setElementGlobalData(ctx);
    }
}

void Renderer::setElementLocalContext(const transform::ElementLocalData& ctx)
{
    if constexpr (RenderConfig::THREADED_RASTERIZATION)
    {
        if (!addCommand(SetElementLocalCtxCmd { ctx }))
        {
            SPDLOG_CRITICAL("Cannot push element local context into queue. This may brake the rendering.");
        }
    }
    else
    {
        m_vertexCtx.setElementLocalData(ctx);
    }
}

void Renderer::setLightingContext(const lighting::LightingData& ctx)
{
    if constexpr (RenderConfig::THREADED_RASTERIZATION)
    {
        if (!addCommand(SetLightingCtxCmd { ctx }))
        {
            SPDLOG_CRITICAL("Cannot push lighting context into queue. This may brake the rendering.");
        }
    }
    else
    {
        m_vertexCtx.lighting = ctx;
    }
}

void Renderer::drawNewElement()
{
    if constexpr (RenderConfig::THREADED_RASTERIZATION)
    {
        addCommand(DrawNewElementCmd {});
    }
    else
    {
        m_vertexTransform.init();
    }
}

void Renderer::initDisplayLists()
{
    m_displayListAssembler[0].setBuffer(m_device.requestDisplayListBuffer(0), 0);
    m_displayListAssembler[1].setBuffer(m_device.requestDisplayListBuffer(1), 1);
}

void Renderer::intermediateUpload()
{
    // Add a raw commit framebuffer command to write the current frame into the framebuffer
    FramebufferCmd cmd { true, true, false, m_resolutionX * m_resolutionY };
    cmd.commitFramebuffer();
    m_displayListBuffer.getBack().addCommand(cmd);

    // Upload and clear display list
    initAndUploadDisplayList();

    // Now load the framebuffer again with the new display list
    setYOffset();
    loadFramebuffer();
}

void Renderer::swapDisplayList()
{
    endFrame(true);
    initAndUploadDisplayList();
    initNewFrame(true);
}

void Renderer::endFrame(const bool swapScreen)
{
    // Finish frame
    addCommitFramebufferCommand();
    if (swapScreen)
    {
        swapScreenToNewColorBuffer();
    }
}

void Renderer::initAndUploadDisplayList()
{
    // Upload new constructed displaylist
    uploadTextures();
    uploadDisplayList();

    // Swap to a new display list
    switchDisplayLists();
    clearDisplayListAssembler();
}

void Renderer::initNewFrame(const bool swapScreen)
{
    // Prepare the new display list
    setYOffset();
    if (swapScreen)
    {
        swapFramebuffer();
    }
    loadFramebuffer();
}

void Renderer::loadFramebuffer()
{
    // Loads the framebuffer into the internal framebuffer when using the IF config.
    // This is required to enable framebuffer effects, such as redrawing to an uncleared framebuffer.
    // This command is ignored in the EF config.
    FramebufferCmd cmd { true, true, false, m_resolutionX * m_resolutionY };
    cmd.loadFramebuffer();
    addCommand(cmd);
}

void Renderer::addCommitFramebufferCommand()
{
    // The EF config requires a NopCmd or another command like a commit (which is in this config a Nop)
    // to flush the pipeline. This is the easiest way to solve WAR conflicts.
    // This command is required for the IF config.
    const uint32_t screenSize = m_resolutionX * m_resolutionY;
    FramebufferCmd cmd { true, true, false, screenSize };
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

    // End current frame, and render it. Initialize the new display list and add as the first
    // command the new resolution. The initNewFrame contains commands which are based on the
    // current resolution
    endFrame(false);
    initAndUploadDisplayList();
    writeReg(reg);
    initNewFrame(false);

    return true;
}

bool Renderer::writeToTextureConfig(const std::size_t tmu, TmuTextureReg tmuConfig)
{
    tmuConfig.setTmu(tmu);
    return writeReg(tmuConfig);
}

bool Renderer::setColorBufferAddress(const uint32_t addr)
{
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

bool Renderer::readFromDeviceMemory(const tcb::span<uint8_t> data, const uint32_t deviceAddr)
{
    return m_device.readFromDeviceMemory(data, deviceAddr);
}

bool Renderer::readBackColorBuffer(const tcb::span<uint8_t> buffer)
{
    endFrame(false);
    initAndUploadDisplayList();
    initNewFrame(false);
    return readFromDeviceMemory(buffer, getCurrentColorBufferAddr(true));
}

bool Renderer::readFrontColorBuffer(const tcb::span<uint8_t> buffer)
{
    return readFromDeviceMemory(buffer, getCurrentColorBufferAddr(false));
}

void Renderer::swapFramebuffer()
{
    m_selectedColorBuffer = !m_selectedColorBuffer;
    setColorBufferAddress(getCurrentColorBufferAddr(true));
}

uint32_t Renderer::getCurrentColorBufferAddr(const bool back) const
{
    if (m_selectedColorBuffer && back)
    {
        return RenderConfig::COLOR_BUFFER_LOC_2;
    }
    else
    {
        return RenderConfig::COLOR_BUFFER_LOC_1;
    }
}

} // namespace rr
