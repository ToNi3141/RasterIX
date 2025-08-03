// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

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

#ifndef VERTEXPIPELINE_HPP
#define VERTEXPIPELINE_HPP

#include "RenderObj.hpp"
#include "math/Mat44.hpp"
#include "math/Vec.hpp"
#include "pixelpipeline/PixelPipeline.hpp"
#include "transform/Clipper.hpp"
#include "transform/Culling.hpp"
#include "transform/Lighting.hpp"
#include "transform/MatrixStore.hpp"
#include "transform/PrimitiveAssembler.hpp"
#include "transform/Stencil.hpp"
#include "transform/TexGen.hpp"
#include "transform/VertexTransforming.hpp"
#include "transform/ViewPort.hpp"
#include <cstdint>

namespace rr
{
class VertexPipeline
{
public:
    VertexPipeline(PixelPipeline& renderer);

    void deinit() { }

    // Drawing
    bool drawObj(const RenderObj& obj);

    // Misc
    void activateTmu(const std::size_t tmu)
    {
        m_tmu = tmu;
        m_matrixStore.setTmu(tmu);
    }

    // Switching and uploading of display lists
    void swapDisplayList() { m_renderer.swapDisplayList(); }

    // General configs
    bool setRenderResolution(const std::size_t x, const std::size_t y) { return m_renderer.setRenderResolution(x, y); }
    bool setScissorBox(const int32_t x,
        const int32_t y,
        const uint32_t width,
        const uint32_t height)
    {
        return m_renderer.setScissorBox(x, y, width, height);
    }
    void setEnableNormalizing(const bool enable) { m_vertexCtx.elementGlobalData.normalizeLightNormal = enable; }
    void enableVSync(const bool enable) { m_renderer.enableVSync(enable); }

    // Framebuffer
    bool clearFramebuffer(const bool frameBuffer, const bool zBuffer, const bool stencilBuffer);
    bool setClearColor(const Vec4& color) { return m_renderer.setClearColor(color); };
    bool setClearDepth(const float depth) { return m_renderer.setClearDepth(depth); };

    // Pixel pipeline configs
    Fogging& fog() { return m_renderer.fog(); }
    Texture& texture() { return m_renderer.texture(); }
    FragmentPipeline& fragmentPipeline() { return m_renderer.fragmentPipeline(); }
    FeatureEnable& featureEnable() { return m_renderer.featureEnable(); }

    // Vertex pipeline configs
    stencil::StencilSetter& stencil() { return m_stencil; }
    lighting::LightingSetter& getLighting() { return m_lighting; }
    texgen::TexGenSetter& getTexGen() { return m_texGen[m_tmu]; }
    viewport::ViewPortSetter& getViewPort() { return m_viewPort; }
    matrixstore::MatrixStore& getMatrixStore() { return m_matrixStore; }
    culling::CullingSetter& getCulling() { return m_culling; }
    primitiveassembler::PrimitiveAssemblerSetter& getPrimitiveAssembler() { return m_primitiveAssembler; }

private:
    VertexParameter fetch(const RenderObj& obj, std::size_t i);
    bool updatePipeline();
    void updateGlobalElementContext();

    vertextransforming::VertexTransformingData m_vertexCtx {};
    vertextransforming::VertexTransformingData::ElementGlobalData m_elementGlobalCtxTransferred {};

    // Current active TMU
    std::size_t m_tmu {};

    PixelPipeline& m_renderer;
    stencil::StencilSetter m_stencil { [this](const StencilReg& reg)
        { return m_renderer.setStencilBufferConfig(reg); },
        m_vertexCtx.elementGlobalData.stencil };
    lighting::LightingSetter m_lighting { m_vertexCtx.lighting };
    viewport::ViewPortSetter m_viewPort { m_vertexCtx.elementGlobalData.viewPort };
    matrixstore::MatrixStore m_matrixStore { m_vertexCtx.elementLocalData.transformMatrices };
    culling::CullingSetter m_culling { m_vertexCtx.elementGlobalData.culling };
    std::array<texgen::TexGenSetter, RenderConfig::TMU_COUNT> m_texGen {};
    primitiveassembler::PrimitiveAssemblerSetter m_primitiveAssembler { m_vertexCtx.elementLocalData.primitiveAssembler };
};

} // namespace rr
#endif // VERTEXPIPELINE_HPP
