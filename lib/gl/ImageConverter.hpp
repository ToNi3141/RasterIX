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

#ifndef GL_IMAGE_CONVERTER_HPP_
#define GL_IMAGE_CONVERTER_HPP_

#include "Enums.hpp"
#include "RIXGL.hpp"
#include "gl.h"
#include "pixelpipeline/Texture.hpp"
#include <algorithm>
#include <cstring>
#include <spdlog/spdlog.h>

namespace rr
{

class ImageConverter
{
public:
    void convertUnpack(
        std::shared_ptr<uint16_t> texelsDevice,
        const InternalPixelFormat ipf,
        const std::size_t rowLength,
        const GLint xoffset,
        const GLint yoffset,
        const GLsizei width,
        const GLsizei height,
        const GLenum format,
        const GLenum type,
        const uint8_t* texelsClient)
    {
        const uint8_t* texelsClientRow = texelsClient;
        for (std::size_t row = yoffset; row < static_cast<std::size_t>(height + yoffset); row++)
        {
            std::size_t currentRow { 0 };
            for (std::size_t column = xoffset; column < static_cast<std::size_t>(width + xoffset); column++)
            {
                const std::size_t texPos { (row * rowLength) + column };
                currentRow += convertTexel(texelsDevice.get()[texPos], ipf, format, type, texelsClientRow + currentRow);
            }
            texelsClientRow = texelsClientRow + alignRow(currentRow, m_unpackAlignment);
        }
    }

    void convertPackRgb565ToRGBA8888(
        uint8_t* pixelsClient,
        const GLsizei width,
        const GLsizei height,
        const uint16_t* pixelsDevice)
    {
        uint8_t* pixelsClientRow = pixelsClient;
        for (std::size_t row = 0; row < static_cast<std::size_t>(height); row++)
        {
            std::size_t currentRow { 0 };
            for (std::size_t column = 0; column < static_cast<std::size_t>(width); column++)
            {
                const std::size_t pixelPos { (row * width) + column };
                convertRgbPixelRgba(pixelsClientRow + currentRow, pixelsDevice[pixelPos]);
                currentRow += 4;
            }
            pixelsClientRow = pixelsClientRow + alignRow(currentRow, m_packAlignment);
        }
    }

    static GLenum convertInternalPixelFormat(InternalPixelFormat& conf, const GLint internalFormat)
    {
        switch (internalFormat)
        {
        case 1:
        case GL_COMPRESSED_ALPHA:
        case GL_ALPHA8:
        case GL_ALPHA12:
        case GL_ALPHA16:
            conf = InternalPixelFormat::ALPHA;
            break;
        case GL_COMPRESSED_LUMINANCE:
        case GL_LUMINANCE:
        case GL_LUMINANCE4:
        case GL_LUMINANCE8:
        case GL_LUMINANCE12:
        case GL_LUMINANCE16:
            conf = InternalPixelFormat::LUMINANCE;
            break;
        case GL_COMPRESSED_INTENSITY:
        case GL_INTENSITY:
        case GL_INTENSITY4:
        case GL_INTENSITY8:
        case GL_INTENSITY12:
        case GL_INTENSITY16:
            conf = InternalPixelFormat::INTENSITY;
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
            conf = InternalPixelFormat::LUMINANCE_ALPHA;
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
            conf = InternalPixelFormat::RGB;
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
            conf = InternalPixelFormat::RGBA;
            break;
        case GL_RGB5_A1:
            conf = InternalPixelFormat::RGBA1;
            break;
        case GL_DEPTH_COMPONENT:
            SPDLOG_WARN("glTexImage2D internal format GL_DEPTH_COMPONENT not supported");
            conf = InternalPixelFormat::RGBA;
            break;
        default:
            SPDLOG_ERROR("glTexImage2D invalid internalformat");
            conf = InternalPixelFormat::RGBA;
            return GL_INVALID_ENUM;
        }
        return GL_NO_ERROR;
    }

    void setUnpackAlignment(const std::size_t unpackAlignment) { m_unpackAlignment = unpackAlignment; }
    void setPackAlignment(const std::size_t packAlignment) { m_packAlignment = packAlignment; }

private:
    static uint16_t convertRGBA8888ToDevicePixelFormat(
        const InternalPixelFormat ipf,
        const uint8_t r,
        const uint8_t g,
        const uint8_t b,
        const uint8_t a)
    {
        uint16_t color {};
        switch (ipf)
        {
        case InternalPixelFormat::ALPHA: // RGBA4444
            color = static_cast<uint16_t>(a >> 4);
            break;
        case InternalPixelFormat::LUMINANCE: // RGB565
            color |= static_cast<uint16_t>(r >> 3) << 11;
            color |= static_cast<uint16_t>(r >> 2) << 5;
            color |= static_cast<uint16_t>(r >> 3) << 0;
            break;
        case InternalPixelFormat::INTENSITY: // RGBA4444
            color |= static_cast<uint16_t>(r >> 4) << 12;
            color |= static_cast<uint16_t>(r >> 4) << 8;
            color |= static_cast<uint16_t>(r >> 4) << 4;
            color |= static_cast<uint16_t>(r >> 4) << 0;
            break;
        case InternalPixelFormat::LUMINANCE_ALPHA: // RGBA4444
            color |= static_cast<uint16_t>(r >> 4) << 12;
            color |= static_cast<uint16_t>(r >> 4) << 8;
            color |= static_cast<uint16_t>(r >> 4) << 4;
            color |= static_cast<uint16_t>(a >> 4) << 0;
            break;
        case InternalPixelFormat::RGB: // RGB565
            color |= static_cast<uint16_t>(r >> 3) << 11;
            color |= static_cast<uint16_t>(g >> 2) << 5;
            color |= static_cast<uint16_t>(b >> 3) << 0;
            break;
        case InternalPixelFormat::RGBA: // RGBA4444
            color |= static_cast<uint16_t>(r >> 4) << 12;
            color |= static_cast<uint16_t>(g >> 4) << 8;
            color |= static_cast<uint16_t>(b >> 4) << 4;
            color |= static_cast<uint16_t>(a >> 4) << 0;
            break;
        case InternalPixelFormat::RGBA1: // RGBA5551
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

    static std::size_t alignRow(const std::size_t row, const std::size_t alignment)
    {
        std::size_t currentRow = row;
        if (currentRow % alignment != 0)
        {
            currentRow += alignment - (currentRow % alignment);
        }
        return currentRow;
    }

    void convertRgbPixelRgba(uint8_t* clientPixels, const uint16_t devicePixels)
    {
        // Extract RGB565 components
        uint8_t r5 = (devicePixels >> 11) & 0x1F;
        uint8_t g6 = (devicePixels >> 5) & 0x3F;
        uint8_t b5 = devicePixels & 0x1F;

        // Expand to 8 bits and repeat low bits
        clientPixels[0] = (r5 << 3) | (r5 >> 2); // Red: 5 bits to 8 bits
        clientPixels[1] = (g6 << 2) | (g6 >> 4); // Green: 6 bits to 8 bits
        clientPixels[2] = (b5 << 3) | (b5 >> 2); // Blue: 5 bits to 8 bits
        clientPixels[3] = 0xff;
    }

    std::size_t convertTexel(
        uint16_t& deviceTexel,
        const InternalPixelFormat ipf,
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

    static std::size_t convertRgbTexel(uint16_t& texel, const InternalPixelFormat ipf, const GLenum type, const uint8_t* pixels)
    {
        switch (type)
        {
        case GL_UNSIGNED_SHORT_5_6_5:
        {
            const uint16_t color = *reinterpret_cast<const uint16_t*>(pixels);
            texel = convertRGBA8888ToDevicePixelFormat(
                ipf,
                convertColorComponentToUint8<11, 5, 0x1f>(color),
                convertColorComponentToUint8<5, 6, 0x3f>(color),
                convertColorComponentToUint8<0, 5, 0x1f>(color),
                0xff);
            return 2;
        }
        case GL_UNSIGNED_BYTE:
            texel = convertRGBA8888ToDevicePixelFormat(
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

    static std::size_t convertRgbaTexel(uint16_t& texel, const InternalPixelFormat ipf, const GLenum type, const uint8_t* pixels)
    {
        switch (type)
        {
        case GL_UNSIGNED_SHORT_5_5_5_1:
        {
            const uint16_t color = *reinterpret_cast<const uint16_t*>(pixels);
            texel = convertRGBA8888ToDevicePixelFormat(
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
            texel = convertRGBA8888ToDevicePixelFormat(
                ipf,
                convertColorComponentToUint8<12, 4, 0xf>(color),
                convertColorComponentToUint8<8, 4, 0xf>(color),
                convertColorComponentToUint8<4, 4, 0xf>(color),
                convertColorComponentToUint8<0, 4, 0xf>(color));
        }
            return 2;
        case GL_UNSIGNED_BYTE:
            texel = convertRGBA8888ToDevicePixelFormat(
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

    static std::size_t convertBgraTexel(uint16_t& texel, const InternalPixelFormat ipf, const GLenum type, const uint8_t* pixels)
    {
        switch (type)
        {
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
        {
            const uint16_t color = *reinterpret_cast<const uint16_t*>(pixels);
            texel = convertRGBA8888ToDevicePixelFormat(
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
            texel = convertRGBA8888ToDevicePixelFormat(
                ipf,
                convertColorComponentToUint8<8, 4, 0xf>(color),
                convertColorComponentToUint8<4, 4, 0xf>(color),
                convertColorComponentToUint8<0, 4, 0xf>(color),
                convertColorComponentToUint8<12, 4, 0xf>(color));
            return 2;
        }
        case GL_UNSIGNED_BYTE:
            texel = convertRGBA8888ToDevicePixelFormat(
                ipf,
                pixels[2],
                pixels[1],
                pixels[0],
                pixels[3]);
            return 4;
        case GL_UNSIGNED_INT_8_8_8_8_REV:
            texel = convertRGBA8888ToDevicePixelFormat(
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
    std::size_t m_packAlignment { 4 };
};
} // namespace rr

#endif // GL_IMAGE_CONVERTER_HPP_
