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

#include "PointAssembly.hpp"
#include <algorithm>
#include <cmath>

namespace rr::pointassembly
{

PointAssemblyCalc::Triangles PointAssemblyCalc::createPoint(const TransformingVertexParameter& vp) const
{
    // Center of the point in NDC
    const Vec4& center = vp.vertex;
    const Vec3& pda = m_data.pointDistanceAttenuation;
    const float size = std::clamp(
        m_data.pointSize / std::sqrt(pda[0] + (pda[1] * vp.vertex[3]) + (pda[2] * vp.vertex[3] * vp.vertex[3])),
        m_data.pointSizeMin,
        m_data.pointSizeMax);

    if (size < m_data.pointFadeThresholdSize)
    {
        return {};
    };

    // Convert size from pixels to NDC
    const float rcpViewportScaleX = 2.0f / m_viewPortWidth;
    const float rcpViewportScaleY = 2.0f / m_viewPortHeight;
    const float halfSizeX = 0.5f * size * center[3] * rcpViewportScaleX;
    const float halfSizeY = 0.5f * size * center[3] * rcpViewportScaleY;

    // Triangle strip vertices for a quad (6 vertices: 0-1-2, 2-3-0)
    Triangles strip { vp };

    // Bottom-left
    strip[0] = { center, vp.colorFront, vp.colorBack, vp.tex };
    strip[0].vertex[0] += -halfSizeX;
    strip[0].vertex[1] += -halfSizeY;

    // Bottom-right
    strip[1] = { center, vp.colorFront, vp.colorBack, vp.tex };
    strip[1].vertex[0] += halfSizeX;
    strip[1].vertex[1] += -halfSizeY;

    // Top-right
    strip[2] = { center, vp.colorFront, vp.colorBack, vp.tex };
    strip[2].vertex[0] += halfSizeX;
    strip[2].vertex[1] += halfSizeY;

    // Top-left
    strip[3] = { center, vp.colorFront, vp.colorBack, vp.tex };
    strip[3].vertex[0] += -halfSizeX;
    strip[3].vertex[1] += halfSizeY;

    if (m_data.texCoordReplace && m_data.enablePointSprite)
    {
        for (std::size_t i = 0; i < RenderConfig::TMU_COUNT; i++)
        {
            // Bottom-left
            strip[0].tex[i] = Vec4 { 0.0f, 0.0f, 0.0f, 1.0f };
            // Bottom-right
            strip[1].tex[i] = Vec4 { 1.0f, 0.0f, 0.0f, 1.0f };
            // Top-right
            strip[2].tex[i] = Vec4 { 1.0f, 1.0f, 0.0f, 1.0f };
            // Top-left
            strip[3].tex[i] = Vec4 { 0.0f, 1.0f, 0.0f, 1.0f };
        }
    }

    // Repeat bottom-left for strip
    strip[4] = strip[0];

    // Repeat top-right for strip
    strip[5] = strip[2];

    return strip;
}

} // namespace rr::pointassembly
