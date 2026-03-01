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
#include "math/Veci.hpp"
#include <array>
#include <cstdint>
#include <cstring>

namespace rr::softwarerasterizer
{

class Fog
{
public:
    struct FogLutEntry
    {
        int32_t m {}; // Qx.14
        int32_t b {}; // Qx.22
    };
    using FogLut = std::array<FogLutEntry, 32>;

    Vec4iColorRGBA calculateFog(const float w, const Vec4iColorRGBA& color) const
    {
        if (!m_enable)
        {
            return color;
        }

        const Vec4iColorRGBA::Type factor = computeFogFactor(w);

        Vec4iColorRGBA foggedColor = Vec4iColorRGBA::interpolate(m_fogColor, color, std::clamp(factor, Vec4iColorRGBA::Zero, Vec4iColorRGBA::FracMax));
        foggedColor.clamp(Vec4iColorRGBA::Zero, Vec4iColorRGBA::FracMax);
        foggedColor[3] = color[3]; // Preserve alpha

        return foggedColor;
    }

    void setFogLut(const FogLut& lut, const float lowerBound, const float upperBound)
    {
        std::memcpy(&m_lowerBound, &lowerBound, sizeof(m_lowerBound));
        std::memcpy(&m_upperBound, &upperBound, sizeof(m_upperBound));
        m_fogLut = lut;
    }

    void setFogColor(const Vec4iColorRGBA& color)
    {
        m_fogColor = color;
    }

    void setEnable(bool enable)
    {
        m_enable = enable;
    }

private:
    static constexpr int32_t LUT_INTERPOLATION_STEPS = 8;

    static std::pair<int32_t, int32_t> getExpAndMantissa(const float w)
    {
        uint32_t wBits;
        std::memcpy(&wBits, &w, sizeof(wBits));

        const int32_t exponent = static_cast<int32_t>((wBits >> 23) & 0xFF) - 127; // Remove bias
        const int32_t mantissa = wBits & 0x7FFFFF; // 23 bits
        return { exponent, mantissa };
    }

    Vec4iColorRGBA::Type computeFogFactor(const float w) const
    {
        uint32_t wBits;
        std::memcpy(&wBits, &w, sizeof(wBits));
        if (wBits <= m_lowerBound)
        {
            return Vec4iColorRGBA::FracMax;
        }
        if (wBits >= m_upperBound)
        {
            return Vec4iColorRGBA::Zero;
        }

        const auto [exponent, mantissa] = getExpAndMantissa(w);

        // Use exponent as LUT index (clamped to valid range)
        const std::size_t index = static_cast<std::size_t>(std::clamp(exponent, 0, static_cast<int32_t>(m_fogLut.size() - 1)));
        const FogLutEntry& entry = m_fogLut[index];

        // xs: upper 8 bits of mantissa as interpolation factor (0 - 255, representing 0.0 - 1.0)
        const int32_t xs = static_cast<int32_t>(mantissa >> (23 - LUT_INTERPOLATION_STEPS)); // Sx.8
        const int32_t fx = entry.m * xs + entry.b; // Sx.22
        const int32_t fx_scaled = fx >> 14; // S1.8

        return static_cast<Vec4iColorRGBA::Type>(fx_scaled);
    }

    FogLut m_fogLut {};
    uint32_t m_lowerBound { 0x3F800000 }; // 1.0f
    uint32_t m_upperBound { 0x447A0000 }; // 1000.0f
    Vec4iColorRGBA m_fogColor {};
    bool m_enable { false };
};

} // namespace rr::softwarerasterizer

#endif // _FOG_HPP_
