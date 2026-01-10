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
        return getFilteredTexel(s, t);
    }
    else
    {
        return softwarerasterizerhelpers::deserializeTexel(readTexel(s, t), m_pixelFormat);
    }
}

Vec4 TextureMap::getFilteredTexel(const float s, const float t) const
{
    const uint16_t texel00 = readTexel(s, t);
    const uint16_t texel01 = readTexel(s, t + 1.0f / m_textureSizeW);
    const uint16_t texel10 = readTexel(s + 1.0f / m_textureSizeH, t);
    const uint16_t texel11 = readTexel(s + 1.0f / m_textureSizeH, t + 1.0f / m_textureSizeW);

    const Vec4 c00 = softwarerasterizerhelpers::deserializeTexel(texel00, m_pixelFormat);
    const Vec4 c01 = softwarerasterizerhelpers::deserializeTexel(texel01, m_pixelFormat);
    const Vec4 c10 = softwarerasterizerhelpers::deserializeTexel(texel10, m_pixelFormat);
    const Vec4 c11 = softwarerasterizerhelpers::deserializeTexel(texel11, m_pixelFormat);

    const float factorS = (s * m_textureSizeW) - static_cast<uint32_t>(s * m_textureSizeW);
    const float factorT = (t * m_textureSizeH) - static_cast<uint32_t>(t * m_textureSizeH);

    const Vec4 c0 = rr::interpolate(c00, c01, factorT);
    const Vec4 c1 = rr::interpolate(c10, c11, factorT);
    const Vec4 c = rr::interpolate(c0, c1, factorS);
    return c;
}

uint32_t TextureMap::clampTexCoord(const float coord, const float size, const TextureWrapMode wrapMode) const
{
    if (wrapMode == TextureWrapMode::CLAMP_TO_EDGE)
    {
        if (coord >= size)
        {
            return static_cast<uint32_t>(size) - 1;
        }
        if (coord < 0.0f)
        {
            return 0;
        }
    }
    return static_cast<uint32_t>(coord);
}

uint32_t TextureMap::getTexelAddr(const float s, const float t) const
{
    const uint32_t uS = clampTexCoord(s * m_textureSizeW, m_textureSizeW, m_wrapModeS);
    const uint32_t uT = clampTexCoord(t * m_textureSizeH, m_textureSizeH, m_wrapModeT);

    const uint32_t index = uT * m_textureSizeW + uS;
    const uint32_t addr = index * 2;
    const uint32_t texelAddress = translateAddress(addr);
    return texelAddress;
}

} // namespace rr::softwarerasterizer
