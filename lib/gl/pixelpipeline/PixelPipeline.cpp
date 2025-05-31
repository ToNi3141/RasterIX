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

#include "PixelPipeline.hpp"

namespace rr
{
PixelPipeline::PixelPipeline(IDevice& device)
    : m_renderer { device }
{
}

bool PixelPipeline::updatePipeline()
{
    bool ret { true };

    ret = ret && m_featureEnable.update();
    ret = ret && m_fragmentPipeline.update();
    ret = ret && m_fog.updateFogLut();
    ret = ret && m_texture.updateTexture();

    return ret;
}

bool PixelPipeline::setClearColor(const Vec4& color)
{
    return m_renderer.setClearColor({
        Vec4i {
            static_cast<uint8_t>(color[0] * 255.0f),
            static_cast<uint8_t>(color[1] * 255.0f),
            static_cast<uint8_t>(color[2] * 255.0f),
            static_cast<uint8_t>(color[3] * 255.0f),
        },
    });
}

bool PixelPipeline::setClearDepth(const float depth)
{
    // Convert from signed float (0.0 .. 1.0) to unsigned fix (0 .. 65535)
    const uint16_t depthx = depth * 65535;
    return m_renderer.setClearDepth({ depthx });
}

bool PixelPipeline::clearFramebuffer(const bool frameBuffer, const bool zBuffer, const bool stencilBuffer)
{
    return m_renderer.clear(frameBuffer, zBuffer, stencilBuffer);
}

} // namespace rr
