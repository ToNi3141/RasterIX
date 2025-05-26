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

#ifndef PIXELPIPELINE_HPP_
#define PIXELPIPELINE_HPP_

#include "FeatureEnable.hpp"
#include "Fogging.hpp"
#include "FragmentPipeline.hpp"
#include "IThreadRunner.hpp"
#include "Texture.hpp"
#include "math/Vec.hpp"
#include "renderer/IDevice.hpp"
#include "renderer/Renderer.hpp"
#include <optional>

namespace rr
{
class PixelPipeline
{
public:
    PixelPipeline(IDevice& device, IThreadRunner& runner);

    // Drawing
    void setVertexContext(const vertextransforming::VertexTransformingData& ctx) { m_renderer.setVertexContext(ctx); }
    bool pushVertex(const VertexParameter& vertex) { return m_renderer.pushVertex(vertex); }

    // Switch and updating of display lists
    void swapDisplayList() { m_renderer.swapDisplayList(); }
    void uploadDisplayList() { m_renderer.uploadDisplayList(); }

    // General configs
    bool setRenderResolution(const std::size_t x, const std::size_t y) { return m_renderer.setRenderResolution(x, y); }
    bool setScissorBox(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height)
    {
        return m_renderer.setScissorBox(x, y, width, height);
    }
    void enableVSync(const bool enable) { m_renderer.setEnableVSync(enable); }

    // Framebuffer
    bool clearFramebuffer(const bool frameBuffer, const bool zBuffer, const bool stencilBuffer);
    bool setClearColor(const Vec4& color);
    bool setClearDepth(const float depth);

    // Pixel pipeline configs
    Fogging& fog() { return m_fog; }
    Texture& texture() { return m_texture; }
    FragmentPipeline& fragmentPipeline() { return m_fragmentPipeline; }
    FeatureEnable& featureEnable() { return m_featureEnable; }

    // Methods for the vertex pipeline
    bool updatePipeline();
    bool setStencilBufferConfig(const StencilReg& stencilConf) { return m_renderer.setStencilBufferConfig(stencilConf); }

private:
    Renderer m_renderer;

    Fogging m_fog { m_renderer };
    Texture m_texture { m_renderer };
    FragmentPipeline m_fragmentPipeline { m_renderer };
    FeatureEnable m_featureEnable { m_renderer, m_texture };
};

} // namespace rr
#endif // PIXELPIPELINE_HPP_
