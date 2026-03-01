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
#include "math/Veci.hpp"
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

    Vec4iColorRGBA getTexel(const int32_t s, const int32_t t) const;

    bool isEnabled() const
    {
        return m_enable;
    }

    void setPages(const tcb::span<const uint32_t> pages)
    {
        std::copy(pages.begin(), pages.end(), m_pages.begin());
    }

    void setTextureSize(const uint16_t w, const uint16_t h)
    {
        static constexpr int32_t ONE_30 = 1 << 30; // S1.30 fixed-point one for precomputing reciprocals
        static constexpr int32_t HALF_30 = ONE_30 >> 1; // S1.30 fixed-point half (0.5) for precomputing half-texel sizes
        m_textureSizeW = w;
        m_textureSizeH = h;
        m_textureSizeOOW = ONE_30 / w; // S1.30 / S16.15 = S16.15
        m_textureSizeOOH = ONE_30 / h; // S1.30 / S16.15 = S16.15
        m_halfTexelSizeW = (HALF_30 / m_textureSizeW) >> SHIFT_15; // S1.30 / Sx.0 = Sx.30 >> 15 = Sx.15
        m_halfTexelSizeH = (HALF_30 / m_textureSizeH) >> SHIFT_15; // S1.30 / Sx.0 = Sx.30 >> 15 = Sx.15
        // Precompute integer sizes and masks for power-of-2 textures
        m_textureSizeWInt = static_cast<int32_t>(w);
        m_textureSizeHInt = static_cast<int32_t>(h);
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
        m_deserialize = softwarerasterizerhelpers::getDeserializeTexelFn(format);
    }

    void setEnable(const bool enable)
    {
        m_enable = enable;
    }

private:
    static constexpr uint32_t SHIFT_15 = 15; // Number of fractional bits in S16.15 fixed-point format
    static constexpr int32_t ONE_15 = 1 << SHIFT_15; // S16.15 fixed-point one
    static constexpr int32_t HALF_15 = ONE_15 >> 1; // S16.15 fixed-point half
    static constexpr int32_t ZERO_15 = 0; // S16.15 fixed-point zero

    Vec4iColorRGBA getUnfilteredTexel(const int32_t s, const int32_t t) const;
    Vec4iColorRGBA getFilteredTexel(const int32_t s, const int32_t t) const;

    uint16_t readTexelAtAddr(const uint32_t texelAddress) const
    {
        return *reinterpret_cast<const uint16_t*>(&m_gram[texelAddress]);
    }

    int32_t clampTexCoord(const int32_t coord, const TextureWrapMode wrapMode) const
    {
        if (wrapMode == TextureWrapMode::CLAMP_TO_EDGE)
        {
            if (coord < ZERO_15)
            {
                return ZERO_15;
            }
            else if (coord > ONE_15)
            {
                return ONE_15;
            }
            return coord;
        }
        // REPEAT mode
        const int32_t cFrac = coord & ((ONE_15 - 1) | 0x80000000); // Fractional part in S16.15
        return (coord < ZERO_15) ? (ONE_15 + cFrac) : cFrac;
    }

    // Convert normalized coordinates to integer texel coordinates with wrapping
    std::pair<uint32_t, uint32_t> texCoordToTexel(const int32_t s, const int32_t t) const
    {
        const int32_t sInt = (s * m_textureSizeW) >> SHIFT_15;
        const int32_t tInt = (t * m_textureSizeH) >> SHIFT_15;

        if (m_wrapModeS == TextureWrapMode::CLAMP_TO_EDGE)
        {
            return { static_cast<uint32_t>(std::clamp(sInt, 0, m_textureMaskW)),
                static_cast<uint32_t>(std::clamp(tInt, 0, m_textureMaskH)) };
        }
        // REPEAT - use bitmask for power-of-2 textures
        return { static_cast<uint32_t>(sInt & m_textureMaskW),
            static_cast<uint32_t>(tInt & m_textureMaskH) };
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

    int32_t m_textureSizeW { 0 };
    int32_t m_textureSizeH { 0 };
    int32_t m_textureSizeOOW { 0 };
    int32_t m_textureSizeOOH { 0 };
    int32_t m_halfTexelSizeW { 0 };
    int32_t m_halfTexelSizeH { 0 };

    int32_t m_textureSizeWInt { 0 };
    int32_t m_textureSizeHInt { 0 };
    int32_t m_textureMaskW { 0 };
    int32_t m_textureMaskH { 0 };

    TextureWrapMode m_wrapModeS { TextureWrapMode::REPEAT };
    TextureWrapMode m_wrapModeT { TextureWrapMode::REPEAT };

    bool m_enableMagFilter { false };
    bool m_enableMinFilter { false };

    DevicePixelFormat m_pixelFormat { DevicePixelFormat::RGBA4444 };
    softwarerasterizerhelpers::DeserializeTexelFn m_deserialize {
        &softwarerasterizerhelpers::deserializeTexelRGBA4444
    };

    bool m_enable { false };
};

} // namespace rr::softwarerasterizer

#endif // _TEXTURE_MAP_HPP_
