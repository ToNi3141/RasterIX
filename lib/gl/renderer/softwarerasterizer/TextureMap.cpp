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

Vec4 TextureMap::getTexel(const float sf, const float tf) const
{
    if (!m_enable)
    {
        return Vec4::createHomogeneous();
    }

    const int32_t s = static_cast<int32_t>(sf * static_cast<float>(1 << 15));
    const int32_t t = static_cast<int32_t>(tf * static_cast<float>(1 << 15));

    // TODO: Mipmapping (m_enableMinFilter)
    if (m_enableMagFilter)
    {
        const Vec4i16 texel = getFilteredTexel(s - m_halfTexelSizeW, t - m_halfTexelSizeH);
        return Vec4 {
            static_cast<float>(texel[0]) / 255.0f,
            static_cast<float>(texel[1]) / 255.0f,
            static_cast<float>(texel[2]) / 255.0f,
            static_cast<float>(texel[3]) / 255.0f
        };
    }
    else
    {
        const Vec4i16 texel = getUnfilteredTexel(s, t);
        return Vec4 {
            static_cast<float>(texel[0]) / 255.0f,
            static_cast<float>(texel[1]) / 255.0f,
            static_cast<float>(texel[2]) / 255.0f,
            static_cast<float>(texel[3]) / 255.0f
        };
    }
}

Vec4i16 TextureMap::getUnfilteredTexel(const int32_t s, const int32_t t) const
{
    const int32_t cS = clampTexCoord(s, m_wrapModeS);
    const int32_t cT = clampTexCoord(t, m_wrapModeT);

    const auto [uS, uT] = texCoordToTexel(cS, cT);
    const uint32_t addr = getTexelAddrFromInt(uS, uT);
    return m_deserialize(readTexelAtAddr(addr));
}

Vec4i16 TextureMap::getFilteredTexel(const int32_t s, const int32_t t) const
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
    const Vec4i16 c00 = m_deserialize(texel00);
    const Vec4i16 c01 = m_deserialize(texel01);
    const Vec4i16 c10 = m_deserialize(texel10);
    const Vec4i16 c11 = m_deserialize(texel11);

    // Bilinear interpolation factors in S16.15
    const int32_t factorS_15 = sTexel & (ONE_15 - 1);
    const int32_t factorT_15 = tTexel & (ONE_15 - 1);

    // Convert factors from S16.15 to S8.8 for Vec4i16::interpolate
    const int16_t factorS = static_cast<int16_t>(factorS_15 >> (SHIFT_15 - Vec4i16::Shift));
    const int16_t factorT = static_cast<int16_t>(factorT_15 >> (SHIFT_15 - Vec4i16::Shift));

    const Vec4i16 c0 = Vec4i16::interpolate(c00, c01, factorT);
    const Vec4i16 c1 = Vec4i16::interpolate(c10, c11, factorT);
    return Vec4i16::interpolate(c0, c1, factorS);
}

} // namespace rr::softwarerasterizer
