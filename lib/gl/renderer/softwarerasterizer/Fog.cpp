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

#include "Fog.hpp"
#include <algorithm>
#include <cmath>

namespace rr::softwarerasterizer
{

Vec4 Fog::calculateFog(const float w, const Vec4& color) const
{
    if (!m_enable)
    {
        return color;
    }

    const float factor = computeFogFactor(w);
    Vec4 foggedColor = interpolate(m_fogColor, color, std::clamp(factor, 0.0f, 1.0f));
    foggedColor.clamp(0.0f, 1.0f);
    foggedColor[3] = color[3]; // Preserve alpha
    return foggedColor;
}

float Fog::computeFogFactor(const float w) const
{
    if (w <= m_lowerBound)
    {
        return 1.0f;
    }
    if (w >= m_upperBound)
    {
        return 0.0f;
    }

    const float exp = std::log2(w);
    float wInt;
    const float wFrac = std::modf(exp, &wInt);
    std::size_t index = static_cast<std::size_t>(wInt);
    index = std::clamp(index, static_cast<std::size_t>(0), m_fogLut.size() - 1);
    const FogLutEntry& entry = m_fogLut[index];
    const float f = (entry.m * wFrac) + entry.b;
    return f;
}

} // namespace rr::softwarerasterizer
