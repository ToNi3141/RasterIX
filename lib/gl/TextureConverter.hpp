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

#ifndef GL_TEXTURE_CONVERTER_HPP_
#define GL_TEXTURE_CONVERTER_HPP_

#include "gl.h"
#include "pixelpipeline/Texture.hpp"
#include <algorithm>
#include <cstring>
#include <spdlog/spdlog.h>

namespace rr
{

class TextureConverter
{
public:
    void convert(
        std::shared_ptr<uint16_t> texelsDevice,
        const TextureObject::IntendedInternalPixelFormat ipf,
        const std::size_t rowLength,
        const GLint xoffset,
        const GLint yoffset,
        const GLsizei width,
        const GLsizei height,
        const GLenum format,
        const GLenum type,
        const uint8_t* texelsClient)
    {
        std::size_t i = 0;
        const uint8_t* texelsClientRow = texelsClient;
        // TODO: Also use GL_UNPACK_ROW_LENGTH configured via glPixelStorei
        for (std::size_t row = yoffset; row < static_cast<std::size_t>(height + yoffset); row++)
        {
            std::size_t currentRow { 0 };
            for (std::size_t column = xoffset; column < static_cast<std::size_t>(width + xoffset); column++)
            {
                const std::size_t texPos { (row * rowLength) + column };
                currentRow += convertTexel(texelsDevice.get()[texPos], ipf, format, type, texelsClientRow + currentRow);
            }
            // Align row
            if (currentRow % m_unpackAlignment != 0)
            {
                currentRow += m_unpackAlignment - (currentRow % m_unpackAlignment);
            }
            texelsClientRow = texelsClientRow + currentRow;
        }
    }

    static GLenum convertToIntendedPixelFormat(TextureObject::IntendedInternalPixelFormat& conf, const GLint internalFormat)
    {
        switch (internalFormat)
        {
        case 1:
        case GL_COMPRESSED_ALPHA:
        case GL_ALPHA8:
        case GL_ALPHA12:
        case GL_ALPHA16:
            conf = TextureObject::IntendedInternalPixelFormat::ALPHA;
            break;
        case GL_COMPRESSED_LUMINANCE:
        case GL_LUMINANCE:
        case GL_LUMINANCE4:
        case GL_LUMINANCE8:
        case GL_LUMINANCE12:
        case GL_LUMINANCE16:
            conf = TextureObject::IntendedInternalPixelFormat::LUMINANCE;
            break;
        case GL_COMPRESSED_INTENSITY:
        case GL_INTENSITY:
        case GL_INTENSITY4:
        case GL_INTENSITY8:
        case GL_INTENSITY12:
        case GL_INTENSITY16:
            conf = TextureObject::IntendedInternalPixelFormat::INTENSITY;
            break;
        case 2:
        case GL_COMPRESSED_LUMINANCE_ALPHA:
        case GL_LUMINANCE_ALPHA:
        case GL_LUMINANCE4_ALPHA4:
        case GL_LUMINANCE6_ALPHA2:
        case GL_LUMINANCE8_ALPHA8:
        case GL_LUMINANCE12_ALPHA4:
        case GL_LUMINANCE12_ALPHA12:
        case GL_LUMINANCE16_ALPHA16:
            conf = TextureObject::IntendedInternalPixelFormat::LUMINANCE_ALPHA;
            break;
        case 3:
        case GL_COMPRESSED_RGB:
        case GL_R3_G3_B2:
        case GL_RGB:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16:
            conf = TextureObject::IntendedInternalPixelFormat::RGB;
            break;
        case 4:
        case GL_COMPRESSED_RGBA:
        case GL_RGBA:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGBA8:
        case GL_RGB10_A2:
        case GL_RGBA12:
        case GL_RGBA16:
            conf = TextureObject::IntendedInternalPixelFormat::RGBA;
            break;
        case GL_RGB5_A1:
            conf = TextureObject::IntendedInternalPixelFormat::RGBA1;
            break;
        case GL_DEPTH_COMPONENT:
            SPDLOG_WARN("glTexImage2D internal format GL_DEPTH_COMPONENT not supported");
            conf = TextureObject::IntendedInternalPixelFormat::RGBA;
            break;
        default:
            SPDLOG_ERROR("glTexImage2D invalid internalformat");
            conf = TextureObject::IntendedInternalPixelFormat::RGBA;
            return GL_INVALID_ENUM;
        }
        return GL_NO_ERROR;
    }

    void setUnpackAlignment(const std::size_t unpackAlignment) { m_unpackAlignment = unpackAlignment; }

private:
    std::size_t convertTexel(
        uint16_t& deviceTexel,
        const TextureObject::IntendedInternalPixelFormat ipf,
        const GLenum format,
        const GLenum type,
        const uint8_t* clientTexel)
    {
        switch (format)
        {
        case GL_RGB:
            return convertRgbTexel(deviceTexel, ipf, type, clientTexel);
        case GL_RGBA:
            return convertRgbaTexel(deviceTexel, ipf, type, clientTexel);
        case GL_ALPHA:
        case GL_RED:
        case GL_GREEN:
        case GL_BLUE:
        case GL_BGR:
        case GL_BGRA:
            return convertBgraTexel(deviceTexel, ipf, type, clientTexel);
        case GL_LUMINANCE:
        case GL_LUMINANCE_ALPHA:
            SPDLOG_WARN("glTexSubImage2D unsupported format");
            return 1; // Avoiding to get stuck
        default:
            SPDLOG_WARN("glTexSubImage2D invalid format");
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
            return 1; // Avoiding to get stuck
        }
        return 1; // Avoiding to get stuck
    }

    template <uint8_t ColorPos, uint8_t ComponentSize, uint8_t Mask>
    static uint8_t convertColorComponentToUint8(const uint16_t color)
    {
        static constexpr uint8_t ComponentShift = 8 - ComponentSize;
        static constexpr uint8_t ComponentShiftFill = ComponentSize - ComponentShift;
        return (((color >> ColorPos) & Mask) << ComponentShift) | (((color >> ColorPos) & Mask) >> ComponentShiftFill);
    }

    static std::size_t convertRgbTexel(uint16_t& texel, const TextureObject::IntendedInternalPixelFormat ipf, const GLenum type, const uint8_t* pixels)
    {
        switch (type)
        {
        case GL_UNSIGNED_SHORT_5_6_5:
        {
            const uint16_t color = *reinterpret_cast<const uint16_t*>(pixels);
            texel = TextureObject::convertColor(
                ipf,
                convertColorComponentToUint8<11, 5, 0x1f>(color),
                convertColorComponentToUint8<5, 6, 0x3f>(color),
                convertColorComponentToUint8<0, 5, 0x1f>(color),
                0xff);
            return 2;
        }
        case GL_UNSIGNED_BYTE:
            texel = TextureObject::convertColor(
                ipf,
                pixels[0],
                pixels[1],
                pixels[2],
                0xff);
            return 3;
        case GL_BYTE:
        case GL_BITMAP:
        case GL_UNSIGNED_SHORT:
        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FLOAT:
        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
            SPDLOG_WARN("glTexSubImage2D unsupported type 0x{:X}", type);
            return 0;
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            SPDLOG_WARN("glTexSubImage2D invalid operation");
            RIXGL::getInstance().setError(GL_INVALID_OPERATION);
            return 0;
        default:
            SPDLOG_WARN("glTexSubImage2D invalid type");
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
            return 0;
        }
        return 0;
    }

    static std::size_t convertRgbaTexel(uint16_t& texel, const TextureObject::IntendedInternalPixelFormat ipf, const GLenum type, const uint8_t* pixels)
    {
        switch (type)
        {
        case GL_UNSIGNED_SHORT_5_5_5_1:
        {
            const uint16_t color = *reinterpret_cast<const uint16_t*>(pixels);
            texel = TextureObject::convertColor(
                ipf,
                convertColorComponentToUint8<11, 5, 0x1f>(color),
                convertColorComponentToUint8<6, 5, 0x1f>(color),
                convertColorComponentToUint8<1, 5, 0x1f>(color),
                (color & 0x1) ? 0xff : 0);
        }
            return 2;
        case GL_UNSIGNED_SHORT_4_4_4_4:
        {
            const uint16_t color = *reinterpret_cast<const uint16_t*>(pixels);
            texel = TextureObject::convertColor(
                ipf,
                convertColorComponentToUint8<12, 4, 0xf>(color),
                convertColorComponentToUint8<8, 4, 0xf>(color),
                convertColorComponentToUint8<4, 4, 0xf>(color),
                convertColorComponentToUint8<0, 4, 0xf>(color));
        }
            return 2;
        case GL_UNSIGNED_BYTE:
            texel = TextureObject::convertColor(
                ipf,
                pixels[0],
                pixels[1],
                pixels[2],
                pixels[3]);
            return 4;
        case GL_BYTE:
        case GL_BITMAP:
        case GL_UNSIGNED_SHORT:
        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FLOAT:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            SPDLOG_WARN("glTexSubImage2D unsupported type 0x{:X}", type);
            return 0;
        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
            SPDLOG_WARN("glTexSubImage2D invalid operation");
            RIXGL::getInstance().setError(GL_INVALID_OPERATION);
            return 0;
        default:
            SPDLOG_WARN("glTexSubImage2D invalid type");
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
            return 0;
        }
        return 0;
    }

    static std::size_t convertBgraTexel(uint16_t& texel, const TextureObject::IntendedInternalPixelFormat ipf, const GLenum type, const uint8_t* pixels)
    {
        switch (type)
        {
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
        {
            const uint16_t color = *reinterpret_cast<const uint16_t*>(pixels);
            texel = TextureObject::convertColor(
                ipf,
                convertColorComponentToUint8<10, 5, 0x1f>(color),
                convertColorComponentToUint8<5, 5, 0x1f>(color),
                convertColorComponentToUint8<0, 5, 0x1f>(color),
                ((color >> 15) & 0x1) ? 0xff : 0);
            return 2;
        }
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        {
            const uint16_t color = *reinterpret_cast<const uint16_t*>(pixels);
            texel = TextureObject::convertColor(
                ipf,
                convertColorComponentToUint8<8, 4, 0xf>(color),
                convertColorComponentToUint8<4, 4, 0xf>(color),
                convertColorComponentToUint8<0, 4, 0xf>(color),
                convertColorComponentToUint8<12, 4, 0xf>(color));
            return 2;
        }
        case GL_UNSIGNED_BYTE:
            texel = TextureObject::convertColor(
                ipf,
                pixels[2],
                pixels[1],
                pixels[0],
                pixels[3]);
            return 4;
        case GL_UNSIGNED_INT_8_8_8_8_REV:
            texel = TextureObject::convertColor(
                ipf,
                pixels[2],
                pixels[1],
                pixels[0],
                pixels[3]);
            return 4;
        case GL_BYTE:
        case GL_BITMAP:
        case GL_UNSIGNED_SHORT:
        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FLOAT:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            SPDLOG_WARN("glTexSubImage2D unsupported type 0x{:X}", type);
            return 0;
        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
            SPDLOG_WARN("glTexSubImage2D invalid operation");
            RIXGL::getInstance().setError(GL_INVALID_OPERATION);
            return 0;
        default:
            SPDLOG_WARN("glTexSubImage2D invalid type");
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
            return 0;
        }
        return 0;
    }

    std::size_t m_unpackAlignment { 4 };
};
} // namespace rr

#endif // GL_TEXTURE_CONVERTER_HPP_
