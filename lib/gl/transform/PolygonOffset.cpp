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

#include "PolygonOffset.hpp"

namespace rr::polygonoffset
{

float PolygonOffsetCalc::dZw(const Vec4& v0, const Vec4& v1, const Vec4& v2)
{
    return (v0[0] * (v1[1] - v2[1])) + (v1[0] * (v2[1] - v0[1])) + (v2[0] * (v0[1] - v1[1]));
}

float PolygonOffsetCalc::dXw(const Vec4& v0, const Vec4& v1, const Vec4& v2)
{
    return (v0[2] * (v1[1] - v2[1])) + (v1[2] * (v2[1] - v0[1])) + (v2[2] * (v0[1] - v1[1]));
}

float PolygonOffsetCalc::dYw(const Vec4& v0, const Vec4& v1, const Vec4& v2)
{
    return (v0[2] * (v2[0] - v1[0])) + (v1[2] * (v0[0] - v2[0])) + (v2[2] * (v1[0] - v0[0]));
}

float PolygonOffsetCalc::approximatedM(const Vec4& v0, const Vec4& v1, const Vec4& v2)
{
    const float D = dZw(v0, v1, v2);
    const float dx = dXw(v0, v1, v2);
    const float dy = dYw(v0, v1, v2);
    const float dzdx = std::abs(dx / D);
    const float dzdy = std::abs(dy / D);
    return std::max(dzdx, dzdy);
}

float PolygonOffsetCalc::calculateO(const Vec4& v0, const Vec4& v1, const Vec4& v2) const
{
    const float m = approximatedM(v0, v1, v2);
    return (m * m_data.factor) + (m_r * m_data.units);
}

} // namespace rr::polygonoffset