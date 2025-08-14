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

#include "LineAssembly.hpp"

namespace rr::lineassembly
{

LineAssemblyCalc::Triangles LineAssemblyCalc::createLine(const VertexParameter& vp0, const VertexParameter& vp1) const
{
    // Copied from swGL and adapted.

    // Get the reciprocal viewport scaling factor
    const float rcpViewportScaleX = 2.0f / m_viewPortWidth;
    const float rcpViewportScaleY = 2.0f / m_viewPortHeight;

    const Vec4& v0 = vp0.vertex;
    const Vec4& v1 = vp1.vertex;

    // Calculate the lines normal n = normalize(-dx, dy)
    float nx = -((v1[1] / v1[3]) - (v0[1] / v0[3]));
    float ny = ((v1[0] / v1[3]) - (v0[0] / v0[3]));
    float rcpLength = 1.0f / std::sqrt(nx * nx + ny * ny);
    nx *= rcpLength;
    ny *= rcpLength;

    // Scale normal according to the width of the line
    float halfLineWidth = m_data.lineWidth * 0.5f;
    nx *= halfLineWidth;
    ny *= halfLineWidth;

    // Now create the vertices that define two triangles which are used to draw the line
    Vec4 nv0 = v0;
    Vec4 nv1 = v0;
    Vec4 nv2 = v1;
    Vec4 nv3 = v1;

    nv0[0] += (nx * v0[3]) * rcpViewportScaleX;
    nv0[1] += (ny * v0[3]) * rcpViewportScaleY;
    nv1[0] += (-nx * v0[3]) * rcpViewportScaleX;
    nv1[1] += (-ny * v0[3]) * rcpViewportScaleY;

    nv2[0] += (nx * v1[3]) * rcpViewportScaleX;
    nv2[1] += (ny * v1[3]) * rcpViewportScaleY;
    nv3[0] += (-nx * v1[3]) * rcpViewportScaleX;
    nv3[1] += (-ny * v1[3]) * rcpViewportScaleY;

    const Vec4& c0 = vp0.color;
    const Vec4& c1 = vp1.color;
    const std::array<Vec4, RenderConfig::TMU_COUNT>& tc0 = vp0.tex;
    const std::array<Vec4, RenderConfig::TMU_COUNT>& tc1 = vp1.tex;
    Triangles triangles;
    triangles[0] = { nv0, c0, { 0.0f, 0.0f, 0.0f }, tc0 };
    triangles[1] = { nv1, c0, { 0.0f, 0.0f, 0.0f }, tc0 };
    triangles[2] = { nv2, c1, { 0.0f, 0.0f, 0.0f }, tc1 };
    triangles[3] = { nv2, c1, { 0.0f, 0.0f, 0.0f }, tc1 };
    triangles[4] = { nv1, c0, { 0.0f, 0.0f, 0.0f }, tc0 };
    triangles[5] = { nv3, c1, { 0.0f, 0.0f, 0.0f }, tc1 };
    return triangles;
}

} // namespace rr::lineassembly