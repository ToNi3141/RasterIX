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
    return softwarerasterizerhelpers::deserializeTexel(readTexel(cS, cT), m_pixelFormat);
}

Vec4 TextureMap::getFilteredTexel(const float s, const float t) const
{
    const float cS = clampTexCoord(s, m_wrapModeS);
    const float cT = clampTexCoord(t, m_wrapModeT);
    const float cSInc = cS + m_textureSizeOOW;
    const float cTInc = cT + m_textureSizeOOH;

    const uint16_t texel00 = readTexel(cS, cT);
    const uint16_t texel01 = readTexel(cS, cTInc);
    const uint16_t texel10 = readTexel(cSInc, cT);
    const uint16_t texel11 = readTexel(cSInc, cTInc);

    const Vec4 c00 = softwarerasterizerhelpers::deserializeTexel(texel00, m_pixelFormat);
    const Vec4 c01 = softwarerasterizerhelpers::deserializeTexel(texel01, m_pixelFormat);
    const Vec4 c10 = softwarerasterizerhelpers::deserializeTexel(texel10, m_pixelFormat);
    const Vec4 c11 = softwarerasterizerhelpers::deserializeTexel(texel11, m_pixelFormat);

    // TODO Clamping of the texel quad?

    const float factorS = ((cS * m_textureSizeW) - static_cast<int32_t>(cS * m_textureSizeW));
    const float factorT = ((cT * m_textureSizeH) - static_cast<int32_t>(cT * m_textureSizeH));

    const Vec4 c0 = rr::interpolate(c00, c01, factorT);
    const Vec4 c1 = rr::interpolate(c10, c11, factorT);
    const Vec4 c = rr::interpolate(c0, c1, factorS);
    return c;
}

float TextureMap::clampTexCoord(const float coord, const TextureWrapMode wrapMode) const
{
    if (wrapMode == TextureWrapMode::CLAMP_TO_EDGE)
    {
        if (coord < 0.0f)
        {
            return 0.0f;
        }
        else if (coord > 1.0f)
        {
            return 1.0f;
        }
    }
    const int32_t cInt = static_cast<int32_t>(coord);
    const float cFrac = coord - static_cast<float>(cInt);
    return (cFrac < 0.0f) ? (1.0f + cFrac) : cFrac;
}

uint32_t TextureMap::getTexelAddr(const float s, const float t) const
{
    const uint32_t uS = static_cast<uint32_t>(s * m_textureSizeW) % static_cast<uint32_t>(m_textureSizeW);
    const uint32_t uT = static_cast<uint32_t>(t * m_textureSizeH) % static_cast<uint32_t>(m_textureSizeH);
    const uint32_t index = uT * m_textureSizeW + uS;
    const uint32_t addr = index * 2;
    const uint32_t texelAddress = translateAddress(addr);
    return texelAddress;
}

} // namespace rr::softwarerasterizer
