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

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "IThreadRunner.hpp"
#include "Rasterizer.hpp"
#include "Renderer.hpp"
#include "TextureMemoryManager.hpp"
#include "displaylist/DisplayList.hpp"
#include "displaylist/DisplayListAssembler.hpp"
#include "displaylist/DisplayListDispatcher.hpp"
#include "displaylist/DisplayListDoubleBuffer.hpp"
#include "math/Vec.hpp"
#include "renderer/IDevice.hpp"
#include <algorithm>
#include <array>
#include <limits>
#include <optional>
#include <stdint.h>
#include <string.h>

#include "RenderConfigs.hpp"
#include "commands/DrawNewElementCmd.hpp"
#include "commands/FogLutStreamCmd.hpp"
#include "commands/FramebufferCmd.hpp"
#include "commands/NopCmd.hpp"
#include "commands/PushVertexCmd.hpp"
#include "commands/SetElementGlobalCtxCmd.hpp"
#include "commands/SetElementLocalCtxCmd.hpp"
#include "commands/SetLightingCtxCmd.hpp"
#include "commands/TextureStreamCmd.hpp"
#include "commands/TriangleStreamCmd.hpp"
#include "commands/WriteRegisterCmd.hpp"
#include "registers/BaseColorReg.hpp"
#include "registers/ColorBufferAddrReg.hpp"
#include "registers/ColorBufferClearColorReg.hpp"
#include "registers/DepthBufferAddrReg.hpp"
#include "registers/DepthBufferClearDepthReg.hpp"
#include "registers/FeatureEnableReg.hpp"
#include "registers/FogColorReg.hpp"
#include "registers/FragmentPipelineReg.hpp"
#include "registers/RenderResolutionReg.hpp"
#include "registers/ScissorEndReg.hpp"
#include "registers/ScissorStartReg.hpp"
#include "registers/StencilBufferAddrReg.hpp"
#include "registers/StencilReg.hpp"
#include "registers/TexEnvColorReg.hpp"
#include "registers/TexEnvReg.hpp"
#include "registers/TmuTextureReg.hpp"
#include "registers/YOffsetReg.hpp"
#include "transform/ElementGlobalData.hpp"
#include "transform/ElementLocalData.hpp"
#include "transform/VertexTransforming.hpp"

namespace rr
{

class Renderer
{
public:
    Renderer(IDevice& device);

    void deinit();

    /// @brief Sets a new element global context
    /// @note The element global context contains things which do not change often.
    ///     Examples are the viewport, culling configs, and so on.
    /// @param ctx The new global context
    void setElementGlobalContext(const transform::ElementGlobalData& ctx);

    /// @brief Sets a new element local context
    /// @note The element local context contains things which change from element to element.
    ///     Examples are transformation matrices (one can assume that every new element gets its own transformation)
    ///     or data for the primitive assembler, like the number of vertices the element contains.
    /// @param ctx The new element local context
    void setElementLocalContext(const transform::ElementLocalData& ctx);

    /// @brief Sets a new lighting context
    /// @note The lighting context is a bit unusual here. It would be reasonable to put it into the global context.
    ///     The reason why it has its own command is because it has a large memory footprint. The finer granularity
    ///     reduces the memory consumption in the display list.
    /// @param ctx The new lighting context.
    void setLightingContext(const lighting::LightingData& ctx);

    /// @brief Pushes a vertex into the renderer
    /// @param vertex The new vertex
    /// @return true when the vertex was accepted. False could be a out of memory error.
    bool pushVertex(const VertexParameter& vertex) { return pushVertexImpl(vertex); }

    /// @brief Resets the primitive assembler and other modules to draw a new element
    ///     Must be called before a new element is drawn
    void drawNewElement();

    /// @brief Starts the rendering process by uploading textures and the displaylist and also swapping
    /// the framebuffers
    void swapDisplayList();

    /// @brief Creates a new texture
    /// @return pair with the first value to indicate if the operation succeeded (true) and the second value with the id
    std::pair<bool, uint16_t> createTexture() { return m_textureManager.createTexture(); }

    /// @brief Creates a new texture for a given texture id
    /// @param texId the name of the new texture
    /// @return pair with the first value to indicate if the operation succeeded (true) and the second value with the id
    bool createTextureWithName(const uint16_t texId) { return m_textureManager.createTextureWithName(texId); }

    /// @brief This will update the texture data of the texture with the given id
    /// @param texId The texture id which texture has to be updated
    /// @param textureObject The object which contains the texture and all its meta data
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool updateTexture(const uint16_t texId, const TextureObjectMipmap& textureObject) { return m_textureManager.updateTexture(texId, textureObject); }

    /// @brief Returns a texture associated to the texId
    /// @param texId The texture id of the texture to get the data from
    /// @return The texture object
    TextureObjectMipmap getTexture(const uint16_t texId) { return m_textureManager.getTexture(texId); }

    /// @brief Queries if the current texture id is a valid texture
    /// @param texId Texture id to query
    /// @return true if the current texture id is mapped to a valid texture
    bool isTextureValid(const uint16_t texId) const { return m_textureManager.textureValid(texId); }

    /// @brief Activates a texture which then is used for rendering
    /// @param tmu The used TMU
    /// @param texId The id of the texture to use
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool useTexture(const std::size_t tmu, const uint16_t texId);

    /// @brief Deletes a texture
    /// @param texId The id of the texture to delete
    /// @return true if succeeded
    bool deleteTexture(const uint16_t texId) { return m_textureManager.deleteTexture(texId); }

    /// @brief The wrapping mode of the texture in s direction
    /// @param tmu The used TMU
    /// @param texId The texture from where to change the parameter
    /// @param mode The new mode
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setTextureWrapModeS(const std::size_t tmu, const uint16_t texId, TextureWrapMode mode);

    /// @brief The wrapping mode of the texture in t direction
    /// @param tmu The used TMU
    /// @param texId The texture from where to change the parameter
    /// @param mode The new mode
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setTextureWrapModeT(const std::size_t tmu, const uint16_t texId, TextureWrapMode mode);

    /// @brief Enables the texture filtering for magnification
    /// @param tmu The used TMU
    /// @param texId The texture from where to change the parameter
    /// @param filter True to enable the filter
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool enableTextureMagFiltering(const std::size_t tmu, const uint16_t texId, bool filter);

    /// @brief Enables the texture filtering for minification (mipmapping)
    /// @param tmu The used TMU
    /// @param texId The texture from where to change the parameter
    /// @param filter True to enable the filter
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool enableTextureMinFiltering(const std::size_t tmu, const uint16_t texId, bool filter);

    /// @brief Sets the resolution of the renderer
    /// @param x X is the width of the produced image
    /// @param y Y is the height of the produced image
    /// @return true if success
    bool setRenderResolution(const std::size_t x, const std::size_t y);

    /// @brief Sets the scissor box parameter
    /// @param x X coordinate of the box
    /// @param y Y coordinate of the box
    /// @param width Width of the box
    /// @param height Height of the box
    /// @return true if success
    bool setScissorBox(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height);

    /// @brief Sets the fog LUT. Note that the fog uses the w value as index (the distance from eye to the polygon znear..zfar)
    /// and not z (clamped value of 0.0..1.0)
    /// @param fogLut the fog lookup table
    /// The fog LUT uses a exponential distribution of w, means fogLut[0] = f(1), fogLut[1] = f(2), fogLut[2] = f(4), fogLut[3] = f(8).
    /// The fog values between start and end must not exceed 1.0f
    /// @param start the start value of the fog
    /// @param end the end value of the fog
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setFogLut(const std::array<float, 33>& fogLut, float start, float end) { return addCommand(FogLutStreamCmd { fogLut, start, end }); }

    /// @brief Will clear a buffer
    /// @param frameBuffer Will clear the frame buffer
    /// @param zBuffer Will clear the z buffer
    /// @param stencilBuffer Will clear the stencil buffer
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool clear(const bool colorBuffer, const bool depthBuffer, const bool stencilBuffer);

    /// @brief Enables vsync when swapping the color buffer
    /// @param enable true to enable vsync
    void setEnableVSync(const bool enable) { m_enableVSync = enable; }

    /// @brief Sets the config for the stencil buffer like the clear value or the tests
    /// @param stencilConf the used stencil buffer config
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setStencilBufferConfig(const StencilReg& stencilConf) { return writeReg(stencilConf); }

    /// @brief Sets the clear color (see clear()) of the color buffer
    /// @param color the clear clear color
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setClearColor(const ColorBufferClearColorReg& color) { return writeReg(color); }

    /// @brief Sets the clear depth value (see clear()) of the depth buffer
    /// @param depth the depth value
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setClearDepth(const DepthBufferClearDepthReg& depth) { return writeReg(depth); }

    /// @brief Sets the fragment pipe line config like the blend equation, color and depth masks and so on
    /// @param pipelineConf the pipeline config
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setFragmentPipelineConfig(const FragmentPipelineReg& pipelineConf) { return writeReg(pipelineConf); }

    /// @brief Sets the config of the texture combiners.
    ///     Note: The number of the TMU is configured in this config
    /// @param texEnvConfig the texture combiner config
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setTexEnv(const TexEnvReg& texEnvConfig) { return writeReg(texEnvConfig); }

    /// @brief Sets the texture environment color.
    ///     Note: The number of the TMU is configured in this config
    /// @param color the texture environment color
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setTexEnvColor(const TexEnvColorReg& color) { return writeReg(color); }

    /// @brief Sets the fog color
    /// @param color the fog color
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setFogColor(const FogColorReg& color) { return writeReg(color); }

    /// @brief Enables features of the renderer like fogging, blending, depth tests and so on
    /// @param featureEnable contains a config of which features are enabled
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setFeatureEnableConfig(const FeatureEnableReg& featureEnable);

private:
    using DisplayListAssemblerType = displaylist::DisplayListAssembler<RenderConfig::TMU_COUNT, displaylist::DisplayList>;
    using TextureManagerType = TextureMemoryManager<RenderConfig>;
    using DisplayListDoubleBufferType = displaylist::DisplayListDoubleBuffer<DisplayListAssemblerType>;

    /// @brief Will render a triangle which is constructed with the given parameters
    /// @return true if the triangle was rendered, otherwise the display list was full and the triangle can't be added
    bool drawTriangle(const TransformedTriangle& triangle);

    /// @brief Uploads the display list to the hardware
    void uploadDisplayList();

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

    void clearDisplayListAssembler()
    {
        m_displayListBuffer.getBack().clearAssembler();
    }

    void switchDisplayLists()
    {
        m_displayListBuffer.swap();
    }

    // Inlining this function enables the return code optimization from the start of the chain to the transformation
    bool pushVertexImpl(const VertexParameter& vertex)
    {
        if constexpr (RenderConfig::THREADED_RASTERIZATION)
        {
            if (!addCommand(PushVertexCmd { vertex }))
            {
                SPDLOG_CRITICAL("Cannot push vertex into queue. This may brake the rendering.");
                return false;
            }
            return true;
        }
        else
        {
            return m_vertexTransform.pushVertex(vertex);
        }
    }

    bool setDepthBufferAddress(const uint32_t addr) { return writeReg(DepthBufferAddrReg { addr }); }
    bool setStencilBufferAddress(const uint32_t addr) { return writeReg(StencilBufferAddrReg { addr }); }
    bool writeToTextureConfig(const std::size_t tmu, TmuTextureReg tmuConfig);
    bool setColorBufferAddress(const uint32_t addr);
    void uploadTextures();
    void swapFramebuffer();
    void intermediateUpload();
    void initDisplayLists();
    void addCommitFramebufferCommand();
    void addColorBufferAddressOfTheScreen() { writeReg(ColorBufferAddrReg { m_colorBufferAddr }); }
    void swapScreenToNewColorBuffer();
    // In a single list case, this is always zero. It is required for the threaded renderer and the multi list support
    void setYOffset() { writeReg(YOffsetReg { 0, 0 }); }
    void loadFramebuffer();

    uint32_t m_colorBufferAddr {};
    bool m_selectedColorBuffer { true };
    bool m_enableVSync { RenderConfig::ENABLE_VSYNC };

    std::size_t m_resolutionX { 640 };
    std::size_t m_resolutionY { 480 };

    IDevice& m_device;
    TextureManagerType m_textureManager;
    Rasterizer m_rasterizer { !RenderConfig::USE_FLOAT_INTERPOLATION };

    const std::function<bool(const TransformedTriangle&)> m_drawTriangleLambda = [this](const TransformedTriangle& triangle)
    { return drawTriangle(triangle); };
    const std::function<bool(const StencilReg&)> m_setStencilBufferConfigLambda = [this](const StencilReg& stencilConf)
    { return setStencilBufferConfig(stencilConf); };

    vertextransforming::VertexTransformingData m_vertexCtx {};
    vertextransforming::VertexTransformingCalc<decltype(m_drawTriangleLambda), decltype(m_setStencilBufferConfigLambda)> m_vertexTransform {
        m_vertexCtx,
        m_drawTriangleLambda,
        m_setStencilBufferConfigLambda,
    };

    // Instantiation of the displaylist assemblers
    std::array<DisplayListAssemblerType, 2> m_displayListAssembler {};
    DisplayListDoubleBufferType m_displayListBuffer { m_displayListAssembler[0], m_displayListAssembler[1] };
};

} // namespace rr
#endif // RENDERER_HPP
