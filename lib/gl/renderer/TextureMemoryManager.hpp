// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2023 ToNi3141

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

#ifndef TEXTUREMEMORYMANAGER_HPP
#define TEXTUREMEMORYMANAGER_HPP
#include "TextureObject.hpp"
#include "registers/TmuTextureReg.hpp"
#include <array>
#include <cmath>
#include <cstring>
#include <functional>
#include <optional>
#include <spdlog/spdlog.h>
#include <tcb/span.hpp>

namespace rr
{
template <class RenderConfig>
class TextureMemoryManager
{
public:
    static constexpr std::size_t TEXTURE_PAGE_SIZE { RenderConfig::TEXTURE_PAGE_SIZE };
    static constexpr std::size_t MAX_PAGES_PER_TEXTURE { static_cast<std::size_t>((static_cast<float>(RenderConfig::MAX_TEXTURE_SIZE * RenderConfig::MAX_TEXTURE_SIZE * 2.0f * 1.33f) / static_cast<float>(RenderConfig::TEXTURE_PAGE_SIZE)) + 1.0f) };

    TextureMemoryManager()
    {
    }

    std::pair<bool, uint16_t> createTexture()
    {
        for (std::size_t i = 1; i < RenderConfig::NUMBER_OF_TEXTURES; i++)
        {
            if (!m_textureLut[i])
            {
                return { createTextureWithName(i), static_cast<uint16_t>(i) };
            }
        }
        return { false, 0 };
    }

    bool createTextureWithName(const uint16_t texId)
    {
        m_textureLut[texId] = allocTexture();
        if (m_textureLut[texId])
        {
            m_textureEntryFlags[*m_textureLut[texId]].requiresUpload = false;
            m_textureEntryFlags[*m_textureLut[texId]].requiresDelete = false;
            setTextureWrapModeS(texId, TextureWrapMode::REPEAT);
            setTextureWrapModeT(texId, TextureWrapMode::REPEAT);
            enableTextureMagFiltering(texId, true);
            return true;
        }
        return false;
    }

    bool updateTexture(const uint16_t texId, const TextureObject& textureObject)
    {
        if (!m_textureLut[texId])
        {
            SPDLOG_ERROR("updateTexture with invalid texID called");
            return false;
        }
        bool ret = true;
        std::size_t textureSlot = *m_textureLut[texId];
        const std::size_t textureSlotOld = *m_textureLut[texId];

        if (m_textureEntryFlags[textureSlot].requiresUpload)
        {
            m_textureEntryFlags[textureSlot].requiresDelete = true;
            std::optional<std::size_t> newTextureSlot = allocTexture();

            if (!newTextureSlot)
            {
                SPDLOG_ERROR("Was not able to allocate new texture");
                return false;
            }

            textureSlot = *newTextureSlot;
            m_textureLut[texId] = newTextureSlot;
            SPDLOG_DEBUG("Use new texture slot {} for texId {}", *newTextureSlot, texId);
        }
        else
        {
            // If it is already uploaded, then it is safe to delete the pages to free them for the reallocation.
            deallocPages(m_textures[textureSlot]);
        }
        m_textures[textureSlot].textures = textureObject;

        m_textureEntryFlags[textureSlot].requiresUpload = true;
        m_textureEntryFlags[textureSlot].requiresDelete = false;

        m_textures[textureSlot].tmuConfig.setWarpModeS(m_textures[textureSlotOld].tmuConfig.getWrapModeS());
        m_textures[textureSlot].tmuConfig.setWarpModeT(m_textures[textureSlotOld].tmuConfig.getWrapModeT());
        m_textures[textureSlot].tmuConfig.setEnableMagFilter(m_textures[textureSlotOld].tmuConfig.getEnableMagFilter());
        m_textures[textureSlot].tmuConfig.setEnableMinFilter(m_textures[textureSlotOld].tmuConfig.getEnableMinFilter());

        m_textures[textureSlot].tmuConfig.setPixelFormat(textureObject.getDevicePixelFormat(0));
        m_textures[textureSlot].tmuConfig.setTextureWidth(textureObject.getWidth(0));
        m_textures[textureSlot].tmuConfig.setTextureHeight(textureObject.getHeight(0));

        // Allocate memory pages
        const std::size_t textureSize = m_textures[textureSlot].getTextureSize();
        const std::size_t texturePages = (textureSize / TEXTURE_PAGE_SIZE) + ((textureSize % TEXTURE_PAGE_SIZE) ? 1 : 0);
        SPDLOG_DEBUG("Use number of pages: {}", texturePages);
        ret = allocPages(m_textures[textureSlot], texturePages);
        if (!ret)
        {
            SPDLOG_ERROR("Ran out of memory during page allocation");
            deallocPages(m_textures[textureSlot]);
        }

        m_textureUpdateRequired = true;

        return ret;
    }

    void setTextureWrapModeS(const uint16_t texId, TextureWrapMode mode)
    {
        if (!m_textureLut[texId])
        {
            SPDLOG_ERROR("setTextureWrapModeS with invalid texID called");
            return;
        }
        Texture& tex = m_textures[*m_textureLut[texId]];
        tex.tmuConfig.setWarpModeS(mode);
    }

    TextureWrapMode getTextureWrapModeS(const uint16_t texId) const
    {
        if (!m_textureLut[texId])
        {
            SPDLOG_ERROR("getTextureWrapModeS with invalid texID called");
            return TextureWrapMode::REPEAT;
        }
        const Texture& tex = m_textures[*m_textureLut[texId]];
        return tex.tmuConfig.getWrapModeS();
    }

    void setTextureWrapModeT(const uint16_t texId, TextureWrapMode mode)
    {
        if (!m_textureLut[texId])
        {
            SPDLOG_ERROR("setTextureWrapModeT with invalid texID called");
            return;
        }
        Texture& tex = m_textures[*m_textureLut[texId]];
        tex.tmuConfig.setWarpModeT(mode);
    }

    TextureWrapMode getTextureWrapModeT(const uint16_t texId) const
    {
        if (!m_textureLut[texId])
        {
            SPDLOG_ERROR("getTextureWrapModeT with invalid texID called");
            return TextureWrapMode::REPEAT;
        }
        const Texture& tex = m_textures[*m_textureLut[texId]];
        return tex.tmuConfig.getWrapModeT();
    }

    void enableTextureMagFiltering(const uint16_t texId, bool filter)
    {
        if (!m_textureLut[texId])
        {
            SPDLOG_ERROR("enableTextureMagFiltering with invalid texID called");
            return;
        }
        Texture& tex = m_textures[*m_textureLut[texId]];
        tex.tmuConfig.setEnableMagFilter(filter);
    }

    bool textureMagFilteringEnabled(const uint16_t texId) const
    {
        if (!m_textureLut[texId])
        {
            SPDLOG_ERROR("getTextureMagFilteringEnabled with invalid texID called");
            return false;
        }
        const Texture& tex = m_textures[*m_textureLut[texId]];
        return tex.tmuConfig.getEnableMagFilter();
    }

    void enableTextureMinFiltering(const uint16_t texId, bool filter)
    {
        if (!m_textureLut[texId])
        {
            SPDLOG_ERROR("enableTextureMinFiltering with invalid texID called");
            return;
        }
        Texture& tex = m_textures[*m_textureLut[texId]];
        tex.tmuConfig.setEnableMinFilter(filter);
    }

    bool textureMinFilteringEnabled(const uint16_t texId) const
    {
        if (!m_textureLut[texId])
        {
            SPDLOG_ERROR("getTextureMinFilteringEnabled with invalid texID called");
            return false;
        }
        const Texture& tex = m_textures[*m_textureLut[texId]];
        return tex.tmuConfig.getEnableMinFilter();
    }

    bool textureValid(const uint16_t texId) const
    {
        if (!m_textureLut[texId])
        {
            return false;
        }
        const TextureEntry& tex = m_textureEntryFlags[*m_textureLut[texId]];
        return (texId != 0) && tex.inUse;
    }

    TmuTextureReg getTmuConfig(const uint16_t texId) const
    {
        if (!m_textureLut[texId])
        {
            SPDLOG_ERROR("getTmuConfig with invalid texID called");
            return {};
        }
        const Texture& tex = m_textures[*m_textureLut[texId]];
        return tex.tmuConfig;
    }

    tcb::span<const std::size_t> getPages(const uint16_t texId) const
    {
        if (textureValid(texId))
        {
            const Texture& tex = m_textures[*m_textureLut[texId]];
            return { tex.pageTable.data(), tex.pages };
        }
        return {};
    }

    TextureObject getTexture(const uint16_t texId)
    {
        if (!m_textureLut[texId])
        {
            SPDLOG_ERROR("getTexture with invalid texID called");
            return {};
        }
        const std::size_t textureSlot = *m_textureLut[texId];
        if (!m_textureEntryFlags[textureSlot].inUse)
        {
            return {};
        }
        return m_textures[textureSlot].textures;
    }

    bool deleteTexture(const uint16_t texId)
    {
        if (!m_textureLut[texId])
        {
            SPDLOG_ERROR("deleteTexture with invalid texID called");
            return false;
        }
        const std::size_t texLutId = *m_textureLut[texId];
        m_textureLut[texId] = std::nullopt;
        m_textureEntryFlags[texLutId].requiresDelete = true;
        m_textureUpdateRequired = true;
        return true;
    }

    bool uploadTextures(const std::function<bool(uint32_t gramAddr, const tcb::span<const uint8_t> data)> uploader)
    {
        if (!m_textureUpdateRequired)
            return true;
        m_textureUpdateRequired = false;

        // Upload textures
        for (std::size_t i = 0; i < RenderConfig::NUMBER_OF_TEXTURES; i++)
        {
            Texture& texture = m_textures[i];
            TextureEntry& textureEntry = m_textureEntryFlags[i];
            if (textureEntry.requiresUpload)
            {
                bool ret { true };
                std::array<uint8_t, TEXTURE_PAGE_SIZE> buffer;
                std::size_t j = 0;
                for (tcb::span<const uint8_t> b = texture.getPageData(j, buffer); !b.empty(); b = texture.getPageData(++j, buffer))
                {
                    ret = ret && uploader(static_cast<std::size_t>(texture.pageTable[j]) * TEXTURE_PAGE_SIZE, { buffer });
                }
                textureEntry.requiresUpload = !ret;
            }

            if (textureEntry.requiresDelete)
            {
                textureEntry.requiresDelete = false;
                textureEntry.inUse = false;
                texture.textures = {};
                deallocPages(texture);
            }
        }

        return true;
    }

private:
    struct PageEntry
    {
        bool inUse { false };
    };

    struct TextureEntry
    {
        bool inUse { false };
        bool requiresUpload { false };
        bool requiresDelete { false };
    };

    struct Texture
    {
        std::array<std::size_t, MAX_PAGES_PER_TEXTURE> pageTable {};
        std::size_t pages { 0 };
        TextureObject textures {};
        TmuTextureReg tmuConfig {};

        std::size_t getTextureSize() const
        {
            std::size_t counter = 0;
            const std::size_t levels = textures.getLevels();
            for (std::size_t i = 0; i < levels; i++)
            {
                counter += textures.getSizeInBytes(i);
            }
            return counter;
        }

        tcb::span<const uint8_t> getPageData(std::size_t page, const tcb::span<uint8_t>& buffer)
        {
            const uint32_t addr = page * buffer.size();
            uint32_t level = 0;
            uint32_t mipMapAddr = 0;
            const std::size_t levels = textures.getLevels();
            for (level = 0; level < levels; level++)
            {
                if (addr < (mipMapAddr + textures.getSizeInBytes(level)))
                {
                    mipMapAddr = addr - mipMapAddr;
                    break;
                }
                mipMapAddr += textures.getSizeInBytes(level);
            }

            tcb::span<const uint8_t> ret {};
            std::size_t bufferSize = 0;
            for (; level < levels; level++)
            {
                const std::size_t texSize = textures.getSizeInBytes(level);
                const uint8_t* pixels = std::reinterpret_pointer_cast<const uint8_t, const uint16_t>(textures.getPixels(level)).get() + mipMapAddr;
                const std::size_t dataSize = (std::min)(texSize - static_cast<std::size_t>(mipMapAddr), buffer.size() - bufferSize);
                std::memcpy(buffer.data() + bufferSize, pixels, dataSize);
                bufferSize += dataSize;
                if (bufferSize == buffer.size())
                {
                    break;
                }
                mipMapAddr = 0; // When arriving at this point, then the current mipmap level is completely in the buffer and the next mipmap level is selected.
            }
            if (bufferSize != 0)
            {
                ret = { buffer.data(), bufferSize };
            }
            return ret;
        }
    };

    bool allocPages(Texture& tex, const std::size_t numberOfPages)
    {
        std::size_t cp = 0;
        if (numberOfPages == 0)
        {
            SPDLOG_ERROR("Called allocPages with numberOfPages == 0");
            return false;
        }
        for (std::size_t p = 0; p < m_pageTable.size(); p++)
        {
            if (m_pageTable[p].inUse == false)
            {
                tex.pageTable[cp] = p;
                m_pageTable[p].inUse = true;
                SPDLOG_DEBUG("Use page: {}", p);
                cp++;
                tex.pages = cp;
                if (cp == numberOfPages)
                {
                    break;
                }
                if (cp >= tex.pageTable.size())
                {
                    SPDLOG_ERROR("Texture specific page table overflown");
                    return false;
                }
            }
        }
        if (cp != numberOfPages)
        {
            SPDLOG_ERROR("Not enough pages available for texture");
            return false;
        }
        return true;
    }

    void deallocPages(Texture& tex)
    {
        for (std::size_t j = 0; j < tex.pages; j++)
        {
            m_pageTable[tex.pageTable[j]].inUse = false;
        }
        tex.pages = 0;
    }

    std::optional<std::size_t> allocTexture()
    {
        for (std::size_t i = 1; i < RenderConfig::NUMBER_OF_TEXTURES; i++)
        {
            if (m_textureEntryFlags[i].inUse == false)
            {
                m_textureEntryFlags[i].inUse = true;
                SPDLOG_DEBUG("Allocating texture {}", i);
                return std::make_optional(i);
            }
        }
        SPDLOG_ERROR("Ran out of memory during texture allocation");
        return std::nullopt;
    }

    // Texture memory allocator
    std::array<Texture, RenderConfig::NUMBER_OF_TEXTURES> m_textures;
    std::array<TextureEntry, RenderConfig::NUMBER_OF_TEXTURES> m_textureEntryFlags {};
    std::array<std::optional<std::size_t>, RenderConfig::NUMBER_OF_TEXTURES> m_textureLut {};
    std::array<PageEntry, RenderConfig::NUMBER_OF_TEXTURE_PAGES> m_pageTable {};

    bool m_textureUpdateRequired { false };
};

} // namespace rr
#endif
