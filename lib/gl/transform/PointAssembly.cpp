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
#include <cmath>

namespace rr::pointassembly
{

PointAssemblyCalc::Triangles PointAssemblyCalc::createPoint(const VertexParameter& vp) const
{
    // Center of the point in NDC
    const Vec4& center = vp.vertex;
    float size = m_data.pointSize;

    // Convert size from pixels to NDC
    float rcpViewportScaleX = 2.0f / m_viewPortWidth;
    float rcpViewportScaleY = 2.0f / m_viewPortHeight;
    float halfSizeX = 0.5f * size * center[3] * rcpViewportScaleX;
    float halfSizeY = 0.5f * size * center[3] * rcpViewportScaleY;

    // Triangle strip vertices for a quad (6 vertices: 0-1-2, 2-3-0)
    Triangles strip { vp };

    // Bottom-left
    strip[0] = { center, vp.color, vp.normal, vp.tex };
    strip[0].vertex[0] += -halfSizeX;
    strip[0].vertex[1] += -halfSizeY;

    // Bottom-right
    strip[1] = { center, vp.color, vp.normal, vp.tex };
    strip[1].vertex[0] += halfSizeX;
    strip[1].vertex[1] += -halfSizeY;

    // Top-right
    strip[2] = { center, vp.color, vp.normal, vp.tex };
    strip[2].vertex[0] += halfSizeX;
    strip[2].vertex[1] += halfSizeY;

    // Top-left
    strip[3] = { center, vp.color, vp.normal, vp.tex };
    strip[3].vertex[0] += -halfSizeX;
    strip[3].vertex[1] += halfSizeY;

    // Repeat bottom-left for strip
    strip[4] = strip[0];

    // Repeat top-right for strip
    strip[5] = strip[2];

    return strip;
}

} // namespace rr::pointassembly
