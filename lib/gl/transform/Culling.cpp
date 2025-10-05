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

#include "Culling.hpp"

namespace rr::culling
{

void CullingSetter::setCullMode(const Face mode)
{
    m_data.cullMode = mode;
}

void CullingSetter::enableCulling(const bool enable)
{
    m_data.enableCulling = enable;
}

void CullingSetter::setFrontFace(const Orientation orientation)
{
    m_data.frontFace = orientation;
}

bool CullingSetter::isCullingEnabled() const
{
    return m_data.enableCulling;
}

Face CullingSetter::getCullMode() const
{
    return m_data.cullMode;
}

Orientation CullingSetter::getFrontFace() const
{
    return m_data.frontFace;
}

CullingCalc::CullingCalc(const CullingData& cullingData)
    : m_data { cullingData }
{
}

bool CullingCalc::cull(const Vec4& v0, const Vec4& v1, const Vec4& v2) const
{
    if (m_data.enableCulling)
    {
        const bool frontFace = isFrontFace(v0, v1, v2);
        return (m_data.cullMode == Face::FRONT_AND_BACK)
            || (frontFace && (m_data.cullMode == Face::FRONT))
            || (!frontFace && (m_data.cullMode == Face::BACK));
    }
    return false;
}

bool CullingCalc::isFrontFace(const Vec4& v0, const Vec4& v1, const Vec4& v2) const
{
    const float edgeVal { Rasterizer::edgeFunctionFloat(v0, v1, v2) };
    if (m_data.frontFace == Orientation::CCW)
    {
        return edgeVal < 0.0f;
    }
    return edgeVal > 0.0f;
}

} // namespace rr::culling
