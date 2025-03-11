// Rasterix
// https://github.com/ToNi3141/Rasterix
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

#ifndef _TMU_TEXTURE_REG_
#define _TMU_TEXTURE_REG_

#include "Enums.hpp"
#include <cmath>
#include <cstdint>
#include <functional>

namespace rr
{
class TmuTextureReg
{
public:
    static_assert(static_cast<uint32_t>(TextureWrapMode::REPEAT) == 0);
    static_assert(static_cast<uint32_t>(TextureWrapMode::CLAMP_TO_EDGE) == 1);

    static_assert(static_cast<uint32_t>(PixelFormat::RGBA4444) == 0);
    static_assert(static_cast<uint32_t>(PixelFormat::RGBA5551) == 1);
    static_assert(static_cast<uint32_t>(PixelFormat::RGB565) == 2);

    TmuTextureReg() = default;
    void setTextureWidth(const uint16_t val) { m_regVal.fields.texWidth = static_cast<uint32_t>(log2f(static_cast<float>(val))); }
    void setTextureHeight(const uint16_t val) { m_regVal.fields.texHeight = static_cast<uint32_t>(log2f(static_cast<float>(val))); }
    void setWarpModeS(const TextureWrapMode val) { m_regVal.fields.wrapModeS = static_cast<uint32_t>(val); }
    void setWarpModeT(const TextureWrapMode val) { m_regVal.fields.wrapModeT = static_cast<uint32_t>(val); }
    void setEnableMagFilter(const bool val) { m_regVal.fields.enableMagFilter = val; }
    void setEnableMinFilter(const bool val) { m_regVal.fields.enableMinFilter = val; }
    void setPixelFormat(const PixelFormat val) { m_regVal.fields.pixelFormat = static_cast<uint32_t>(val); }

    uint16_t getTextureWidth() const { return powf(2.0f, m_regVal.fields.texWidth); }
    uint16_t getTextureHeight() const { return powf(2.0f, m_regVal.fields.texHeight); }
    TextureWrapMode getWrapModeS() const { return static_cast<TextureWrapMode>(m_regVal.fields.wrapModeS); }
    TextureWrapMode getWrapModeT() const { return static_cast<TextureWrapMode>(m_regVal.fields.wrapModeT); }
    bool getEnableMagFilter() const { return m_regVal.fields.enableMagFilter; }
    bool getEnableMinFilter() const { return m_regVal.fields.enableMinFilter; }
    PixelFormat getPixelFormat() const { return static_cast<PixelFormat>(m_regVal.fields.pixelFormat); }

    void setTmu(const std::size_t tmu) { m_offset = tmu * TMU_OFFSET; }
    uint32_t serialize() const { return m_regVal.data; }
    uint32_t getAddr() const { return 0xC + m_offset; }

private:
    static constexpr std::size_t TMU_OFFSET { 3 };
    union RegVal
    {
#pragma pack(push, 1)
        struct RegContent
        {
            RegContent()
                : texWidth { 0 }
                , texHeight { 0 }
                , wrapModeS { static_cast<uint32_t>(TextureWrapMode::REPEAT) }
                , wrapModeT { static_cast<uint32_t>(TextureWrapMode::REPEAT) }
                , enableMagFilter { false }
                , enableMinFilter { true }
                , pixelFormat { static_cast<uint32_t>(PixelFormat::RGBA4444) }
            {
            }

            uint32_t texWidth : 4;
            uint32_t texHeight : 4;
            uint32_t wrapModeS : 1;
            uint32_t wrapModeT : 1;
            uint32_t enableMagFilter : 1;
            uint32_t enableMinFilter : 1;
            uint32_t pixelFormat : 4;
        } fields {};
        uint32_t data;
#pragma pack(pop)
    } m_regVal;
    std::size_t m_offset { 0 };
};
} // namespace rr

#endif // _TMU_TEXTURE_REG_
