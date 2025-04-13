// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

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

#ifndef TEXTURE_OBJECT_HPP
#define TEXTURE_OBJECT_HPP

#include "renderer/registers/TmuTextureReg.hpp"
#include <array>
#include <memory>

namespace rr
{
struct TextureObject
{
    static constexpr std::size_t MAX_LOD { 8 };
    enum class IntendedInternalPixelFormat
    {
        ALPHA,
        LUMINANCE,
        INTENSITY,
        LUMINANCE_ALPHA,
        RGB,
        RGBA,
        RGBA1,
    };

    static uint16_t convertColor(
        const IntendedInternalPixelFormat ipf,
        const uint8_t r,
        const uint8_t g,
        const uint8_t b,
        const uint8_t a)
    {
        uint16_t color {};
        switch (ipf)
        {
        case IntendedInternalPixelFormat::ALPHA: // RGBA4444
            color = static_cast<uint16_t>(a >> 4);
            break;
        case IntendedInternalPixelFormat::LUMINANCE: // RGB565
            color |= static_cast<uint16_t>(r >> 3) << 11;
            color |= static_cast<uint16_t>(r >> 2) << 5;
            color |= static_cast<uint16_t>(r >> 3) << 0;
            break;
        case IntendedInternalPixelFormat::INTENSITY: // RGBA4444
            color |= static_cast<uint16_t>(r >> 4) << 12;
            color |= static_cast<uint16_t>(r >> 4) << 8;
            color |= static_cast<uint16_t>(r >> 4) << 4;
            color |= static_cast<uint16_t>(r >> 4) << 0;
            break;
        case IntendedInternalPixelFormat::LUMINANCE_ALPHA: // RGBA4444
            color |= static_cast<uint16_t>(r >> 4) << 12;
            color |= static_cast<uint16_t>(r >> 4) << 8;
            color |= static_cast<uint16_t>(r >> 4) << 4;
            color |= static_cast<uint16_t>(a >> 4) << 0;
            break;
        case IntendedInternalPixelFormat::RGB: // RGB565
            color |= static_cast<uint16_t>(r >> 3) << 11;
            color |= static_cast<uint16_t>(g >> 2) << 5;
            color |= static_cast<uint16_t>(b >> 3) << 0;
            break;
        case IntendedInternalPixelFormat::RGBA: // RGBA4444
            color |= static_cast<uint16_t>(r >> 4) << 12;
            color |= static_cast<uint16_t>(g >> 4) << 8;
            color |= static_cast<uint16_t>(b >> 4) << 4;
            color |= static_cast<uint16_t>(a >> 4) << 0;
            break;
        case IntendedInternalPixelFormat::RGBA1: // RGBA5551
            color |= static_cast<uint16_t>(r >> 3) << 11;
            color |= static_cast<uint16_t>(g >> 3) << 6;
            color |= static_cast<uint16_t>(b >> 3) << 1;
            color |= static_cast<uint16_t>(a >> 7) << 0;
            break;
        default:
            break;
        }
        return color;
    }

    PixelFormat getPixelFormat() const
    {
        PixelFormat format {};
        switch (intendedPixelFormat)
        {
        case IntendedInternalPixelFormat::ALPHA:
            format = PixelFormat::RGBA4444;
            break;
        case IntendedInternalPixelFormat::LUMINANCE:
            format = PixelFormat::RGB565;
            break;
        case IntendedInternalPixelFormat::INTENSITY:
            format = PixelFormat::RGBA4444;
            break;
        case IntendedInternalPixelFormat::LUMINANCE_ALPHA:
            format = PixelFormat::RGBA4444;
            break;
        case IntendedInternalPixelFormat::RGB:
            format = PixelFormat::RGB565;
            break;
        case IntendedInternalPixelFormat::RGBA:
            format = PixelFormat::RGBA4444;
            break;
        case IntendedInternalPixelFormat::RGBA1:
            format = PixelFormat::RGBA5551;
            break;
        default:
            break;
        }
        return format;
    }

    std::shared_ptr<const uint16_t> pixels {}; ///< The texture in the format defined by PixelFormat
    std::size_t width {}; ///< The width of the texture
    std::size_t height {}; ///< The height of the texture
    IntendedInternalPixelFormat intendedPixelFormat {}; ///< The intended pixel format which is converted to a type of PixelFormat
};

using TextureObjectMipmap = std::array<TextureObject, TextureObject::MAX_LOD + 1>;
} // namespace rr
#endif // TEXTURE_OBJECT_HPP
