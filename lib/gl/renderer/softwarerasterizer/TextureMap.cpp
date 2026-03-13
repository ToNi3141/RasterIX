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

#include "TextureMap.hpp"

namespace rr::softwarerasterizer
{

Vec4iColorRGBA TextureMap::getTexel(const int32_t s, const int32_t t) const
{
    if (!m_enable)
    {
        // Return homogeneous vector (0, 0, 0, 1) in S8.8 format
        return Vec4iColorRGBA { Vec4iColorRGBA::Zero, Vec4iColorRGBA::Zero, Vec4iColorRGBA::Zero, Vec4iColorRGBA::FracMax };
    }

    // TODO: Mipmapping (m_enableMinFilter)
    if (m_enableMagFilter)
    {
        return getFilteredTexel(s - m_halfTexelSizeW, t - m_halfTexelSizeH);
    }
    else
    {
        return getUnfilteredTexel(s, t);
    }
}

Vec4iColorRGBA TextureMap::getUnfilteredTexel(const int32_t s, const int32_t t) const
{
    const int32_t cS = clampTexCoord(s, m_wrapModeS);
    const int32_t cT = clampTexCoord(t, m_wrapModeT);

    const auto [uS, uT] = texCoordToTexel(cS, cT);
    const uint32_t addr = getTexelAddrFromInt(uS, uT);
    return m_deserialize(readTexelAtAddr(addr));
}

Vec4iColorRGBA TextureMap::getFilteredTexel(const int32_t s, const int32_t t) const
{
    const int32_t cS = clampTexCoord(s, m_wrapModeS);
    const int32_t cT = clampTexCoord(t, m_wrapModeT);

    // Convert to texel coordinates once
    const int32_t sTexel = cS * m_textureSizeW;
    const int32_t tTexel = cT * m_textureSizeH;
    const int32_t sInt = sTexel >> SHIFT_15; // Integer part in S16.15
    const int32_t tInt = tTexel >> SHIFT_15; // Integer part in S16.15

    // Calculate texel coordinates for all 4 corners
    uint32_t uS0, uT0, uS1, uT1;
    if (m_wrapModeS == TextureWrapMode::CLAMP_TO_EDGE)
    {
        uS0 = static_cast<uint32_t>(std::clamp(sInt, ZERO_15, m_textureMaskW));
        uT0 = static_cast<uint32_t>(std::clamp(tInt, ZERO_15, m_textureMaskH));
        uS1 = static_cast<uint32_t>(std::clamp(sInt + 1, ZERO_15, m_textureMaskW));
        uT1 = static_cast<uint32_t>(std::clamp(tInt + 1, ZERO_15, m_textureMaskH));
    }
    else // REPEAT - use bitmask
    {
        uS0 = static_cast<uint32_t>(sInt) & m_textureMaskW;
        uT0 = static_cast<uint32_t>(tInt) & m_textureMaskH;
        uS1 = static_cast<uint32_t>(sInt + 1) & m_textureMaskW;
        uT1 = static_cast<uint32_t>(tInt + 1) & m_textureMaskH;
    }

    // Read all 4 texels
    const uint16_t texel00 = readTexelAtAddr(getTexelAddrFromInt(uS0, uT0));
    const uint16_t texel01 = readTexelAtAddr(getTexelAddrFromInt(uS0, uT1));
    const uint16_t texel10 = readTexelAtAddr(getTexelAddrFromInt(uS1, uT0));
    const uint16_t texel11 = readTexelAtAddr(getTexelAddrFromInt(uS1, uT1));

    // Deserialize using function pointer (no switch in hot path)
    const Vec4iColorRGBA c00 = m_deserialize(texel00);
    const Vec4iColorRGBA c01 = m_deserialize(texel01);
    const Vec4iColorRGBA c10 = m_deserialize(texel10);
    const Vec4iColorRGBA c11 = m_deserialize(texel11);

    // Bilinear interpolation factors in S16.15
    const int32_t factorS_15 = sTexel & (static_cast<uint32_t>(ONE_15 - 1) | 0x80000000); // Fractional part in S16.15
    const int32_t factorT_15 = tTexel & (static_cast<uint32_t>(ONE_15 - 1) | 0x80000000); // Fractional part in S16.15

    // Convert factors from S16.15 to S8.8 for Vec4iColorRGBA::interpolate
    const int16_t factorS = static_cast<int16_t>(factorS_15 >> (SHIFT_15 - Vec4iColorRGBA::Shift));
    const int16_t factorT = static_cast<int16_t>(factorT_15 >> (SHIFT_15 - Vec4iColorRGBA::Shift));

    const Vec4iColorRGBA c0 = Vec4iColorRGBA::interpolate(c00, c01, factorT);
    const Vec4iColorRGBA c1 = Vec4iColorRGBA::interpolate(c10, c11, factorT);
    return Vec4iColorRGBA::interpolate(c0, c1, factorS);
}

} // namespace rr::softwarerasterizer
