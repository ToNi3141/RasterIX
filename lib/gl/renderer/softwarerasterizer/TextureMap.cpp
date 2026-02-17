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

Vec4 TextureMap::getTexel(const float s, const float t) const
{
    if (!m_enable)
    {
        return Vec4::createHomogeneous();
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

Vec4 TextureMap::getUnfilteredTexel(const float s, const float t) const
{
    const float cS = clampTexCoord(s, m_wrapModeS);
    const float cT = clampTexCoord(t, m_wrapModeT);

    const auto [uS, uT] = texCoordToTexel(cS, cT);
    const uint32_t addr = getTexelAddrFromInt(uS, uT);
    return m_deserialize(readTexelAtAddr(addr));
}

Vec4 TextureMap::getFilteredTexel(const float s, const float t) const
{
    const float cS = clampTexCoord(s, m_wrapModeS);
    const float cT = clampTexCoord(t, m_wrapModeT);

    // Convert to texel coordinates once
    const float sTexel = cS * m_textureSizeW;
    const float tTexel = cT * m_textureSizeH;
    const int32_t sInt = static_cast<int32_t>(sTexel);
    const int32_t tInt = static_cast<int32_t>(tTexel);

    // Calculate texel coordinates for all 4 corners
    uint32_t uS0, uT0, uS1, uT1;
    if (m_wrapModeS == TextureWrapMode::CLAMP_TO_EDGE)
    {
        uS0 = static_cast<uint32_t>(std::clamp(sInt, static_cast<int32_t>(0), static_cast<int32_t>(m_textureMaskW)));
        uT0 = static_cast<uint32_t>(std::clamp(tInt, static_cast<int32_t>(0), static_cast<int32_t>(m_textureMaskH)));
        uS1 = static_cast<uint32_t>(std::clamp(sInt + 1, static_cast<int32_t>(0), static_cast<int32_t>(m_textureMaskW)));
        uT1 = static_cast<uint32_t>(std::clamp(tInt + 1, static_cast<int32_t>(0), static_cast<int32_t>(m_textureMaskH)));
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
    const Vec4 c00 = m_deserialize(texel00);
    const Vec4 c01 = m_deserialize(texel01);
    const Vec4 c10 = m_deserialize(texel10);
    const Vec4 c11 = m_deserialize(texel11);

    // Bilinear interpolation factors
    const float factorS = sTexel - static_cast<float>(sInt);
    const float factorT = tTexel - static_cast<float>(tInt);

    const Vec4 c0 = rr::interpolate(c00, c01, factorT);
    const Vec4 c1 = rr::interpolate(c10, c11, factorT);
    return rr::interpolate(c0, c1, factorS);
}

} // namespace rr::softwarerasterizer
