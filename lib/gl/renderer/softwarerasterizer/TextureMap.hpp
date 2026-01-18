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

#ifndef _TEXTURE_MAP_HPP_
#define _TEXTURE_MAP_HPP_

#include "Enums.hpp"
#include "RenderConfigs.hpp"
#include "SoftwareRasterizerHelpers.hpp"
#include <array>
#include <cstdint>
#include <tcb/span.hpp>

namespace rr::softwarerasterizer
{

class TextureMap
{
public:
    void setGRAM(const tcb::span<const uint8_t> gram)
    {
        m_gram = gram;
    }

    Vec4 getTexel(const float s, const float t) const;

    void setPages(const tcb::span<const uint32_t> pages)
    {
        std::copy(pages.begin(), pages.end(), m_pages.begin());
    }

    void setTextureSize(const float w, const float h)
    {
        m_textureSizeW = w;
        m_textureSizeH = h;
        m_textureSizeOOW = 1.0f / w;
        m_textureSizeOOH = 1.0f / h;
        m_halfTexelSizeW = 0.5f / w;
        m_halfTexelSizeH = 0.5f / h;
    }

    void setWrapMode(const TextureWrapMode s, const TextureWrapMode t)
    {
        m_wrapModeS = s;
        m_wrapModeT = t;
    }

    void setEnableMagFilter(const bool enable)
    {
        m_enableMagFilter = enable;
    }

    void setEnableMinFilter(const bool enable)
    {
        m_enableMinFilter = enable;
    }

    void setPixelFormat(const DevicePixelFormat format)
    {
        m_pixelFormat = format;
    }

    void setEnable(const bool enable)
    {
        m_enable = enable;
    }

private:
    Vec4 getUnfilteredTexel(const float s, const float t) const;
    Vec4 getFilteredTexel(const float s, const float t) const;

    uint16_t readTexel(const float s, const float t) const
    {
        const uint32_t texelAddress = getTexelAddr(s, t);
        return *reinterpret_cast<const uint16_t*>(&m_gram[texelAddress]);
    }

    float clampTexCoord(const float coord, const TextureWrapMode wrapMode) const;

    uint32_t getTexelAddr(const float s, const float t) const;

    uint32_t translateAddress(const uint32_t addr) const
    {
        const uint32_t pageNr = addr / RenderConfig::TEXTURE_PAGE_SIZE;
        const uint32_t offset = addr % RenderConfig::TEXTURE_PAGE_SIZE;
        const uint32_t pageOffset = m_pages[pageNr];
        const uint32_t texelAddress = pageOffset + offset;
        return texelAddress;
    }

    tcb::span<const uint8_t> m_gram {};
    std::array<uint32_t, RenderConfig::getMaxTexturePages()> m_pages;

    float m_textureSizeW { 0.0f };
    float m_textureSizeH { 0.0f };
    float m_textureSizeOOW { 0.0f };
    float m_textureSizeOOH { 0.0f };
    float m_halfTexelSizeW { 0.0f };
    float m_halfTexelSizeH { 0.0f };

    TextureWrapMode m_wrapModeS { TextureWrapMode::REPEAT };
    TextureWrapMode m_wrapModeT { TextureWrapMode::REPEAT };

    bool m_enableMagFilter { false };
    bool m_enableMinFilter { false };

    DevicePixelFormat m_pixelFormat { DevicePixelFormat::RGBA4444 };

    bool m_enable { false };
};

} // namespace rr::softwarerasterizer

#endif // _TEXTURE_MAP_HPP_
