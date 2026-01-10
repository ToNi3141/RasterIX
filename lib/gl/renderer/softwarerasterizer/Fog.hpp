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

#ifndef _FOG_HPP_
#define _FOG_HPP_

#include "Enums.hpp"
#include "math/Vec.hpp"
#include <array>
#include <cstdint>

namespace rr::softwarerasterizer
{

class Fog
{
public:
    struct FogLutEntry
    {
        float m {};
        float b {};
    };
    using FogLut = std::array<FogLutEntry, 32>;

    Vec4 calculateFog(const float w, const Vec4& color) const;

    void setFogLut(const FogLut& lut, const float lowerBound, const float upperBound)
    {
        m_lowerBound = lowerBound;
        m_upperBound = upperBound;
        m_fogLut = lut;
    }

    void setFogColor(const Vec4& color)
    {
        m_fogColor = color;
    }

    void setEnable(bool enable)
    {
        m_enable = enable;
    }

private:
    float computeFogFactor(const float w) const;

    FogLut m_fogLut {};
    float m_lowerBound { 1.0f };
    float m_upperBound { 1000.0f };
    Vec4 m_fogColor {};
    bool m_enable { false };
};

} // namespace rr::softwarerasterizer

#endif // _FOG_HPP_
