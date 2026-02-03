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
#include <utility>

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

    bool isEnabled() const
    {
        return m_enable;
    }

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
        // Precompute integer sizes and masks for power-of-2 textures
        m_textureSizeWInt = static_cast<uint32_t>(w);
        m_textureSizeHInt = static_cast<uint32_t>(h);
        m_textureMaskW = m_textureSizeWInt - 1;
        m_textureMaskH = m_textureSizeHInt - 1;
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
        // Set function pointer based on pixel format using shared helper
        m_deserialize = softwarerasterizerhelpers::getDeserializeTexelFloatFn(format);
    }

    void setEnable(const bool enable)
    {
        m_enable = enable;
    }

private:
    Vec4 getUnfilteredTexel(const float s, const float t) const;
    Vec4 getFilteredTexel(const float s, const float t) const;

    uint16_t readTexelAtAddr(const uint32_t texelAddress) const
    {
        return *reinterpret_cast<const uint16_t*>(&m_gram[texelAddress]);
    }

    float clampTexCoord(const float coord, const TextureWrapMode wrapMode) const
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
            return coord;
        }
        // REPEAT mode
        const int32_t cInt = static_cast<int32_t>(coord);
        const float cFrac = coord - static_cast<float>(cInt);
        return (cFrac < 0.0f) ? (1.0f + cFrac) : cFrac;
    }

    // Convert normalized coordinates to integer texel coordinates with wrapping
    std::pair<uint32_t, uint32_t> texCoordToTexel(const float s, const float t) const
    {
        const int32_t sInt = static_cast<int32_t>(s * m_textureSizeW);
        const int32_t tInt = static_cast<int32_t>(t * m_textureSizeH);

        if (m_wrapModeS == TextureWrapMode::CLAMP_TO_EDGE)
        {
            return { static_cast<uint32_t>(std::clamp(sInt, 0, static_cast<int32_t>(m_textureMaskW))),
                static_cast<uint32_t>(std::clamp(tInt, 0, static_cast<int32_t>(m_textureMaskH))) };
        }
        // REPEAT - use bitmask for power-of-2 textures
        return { static_cast<uint32_t>(sInt) & m_textureMaskW,
            static_cast<uint32_t>(tInt) & m_textureMaskH };
    }

    // Calculate texel address from integer coordinates
    uint32_t getTexelAddrFromInt(const uint32_t uS, const uint32_t uT) const
    {
        const uint32_t index = uT * m_textureSizeWInt + uS;
        const uint32_t addr = index * 2; // 2 bytes per texel
        return translateAddress(addr);
    }

    // Compute log2 at compile time for page size
    static constexpr uint32_t log2PageSize()
    {
        static_assert((RenderConfig::TEXTURE_PAGE_SIZE & (RenderConfig::TEXTURE_PAGE_SIZE - 1)) == 0,
            "TEXTURE_PAGE_SIZE must be a power of 2");
        uint32_t size = RenderConfig::TEXTURE_PAGE_SIZE;
        uint32_t shift = 0;
        while (size > 1)
        {
            size >>= 1;
            shift++;
        }
        return shift;
    }

    uint32_t translateAddress(const uint32_t addr) const
    {
        // Use bit operations instead of division/modulo (TEXTURE_PAGE_SIZE is power of 2)
        constexpr uint32_t PAGE_SHIFT = log2PageSize();
        constexpr uint32_t PAGE_MASK = RenderConfig::TEXTURE_PAGE_SIZE - 1;

        const uint32_t pageNr = addr >> PAGE_SHIFT;
        const uint32_t offset = addr & PAGE_MASK;
        const uint32_t pageOffset = m_pages[pageNr];
        return pageOffset + offset;
    }

    tcb::span<const uint8_t> m_gram {};
    std::array<uint32_t, RenderConfig::getMaxTexturePages()> m_pages;

    float m_textureSizeW { 0.0f };
    float m_textureSizeH { 0.0f };
    float m_textureSizeOOW { 0.0f };
    float m_textureSizeOOH { 0.0f };
    float m_halfTexelSizeW { 0.0f };
    float m_halfTexelSizeH { 0.0f };

    uint32_t m_textureSizeWInt { 0 };
    uint32_t m_textureSizeHInt { 0 };
    uint32_t m_textureMaskW { 0 };
    uint32_t m_textureMaskH { 0 };

    TextureWrapMode m_wrapModeS { TextureWrapMode::REPEAT };
    TextureWrapMode m_wrapModeT { TextureWrapMode::REPEAT };

    bool m_enableMagFilter { false };
    bool m_enableMinFilter { false };

    DevicePixelFormat m_pixelFormat { DevicePixelFormat::RGBA4444 };
    softwarerasterizerhelpers::DeserializeTexelFloatFn m_deserialize {
        &softwarerasterizerhelpers::deserializeTexelFloatRGBA4444
    };

    bool m_enable { false };
};

} // namespace rr::softwarerasterizer

#endif // _TEXTURE_MAP_HPP_
