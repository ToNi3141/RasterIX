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

#include "Enums.hpp"
#include "renderer/registers/TmuTextureReg.hpp"
#include <array>
#include <memory>

namespace rr
{
struct TextureObject
{
    using PixelsType = std::shared_ptr<uint16_t>;
    static constexpr std::size_t MAX_LOD { 8 };

    DevicePixelFormat getDevicePixelFormat() const
    {
        DevicePixelFormat format {};
        switch (internalPixelFormat)
        {
        case InternalPixelFormat::ALPHA:
            format = DevicePixelFormat::RGBA4444;
            break;
        case InternalPixelFormat::LUMINANCE:
            format = DevicePixelFormat::RGB565;
            break;
        case InternalPixelFormat::INTENSITY:
            format = DevicePixelFormat::RGBA4444;
            break;
        case InternalPixelFormat::LUMINANCE_ALPHA:
            format = DevicePixelFormat::RGBA4444;
            break;
        case InternalPixelFormat::RGB:
            format = DevicePixelFormat::RGB565;
            break;
        case InternalPixelFormat::RGBA:
            format = DevicePixelFormat::RGBA4444;
            break;
        case InternalPixelFormat::RGBA1:
            format = DevicePixelFormat::RGBA5551;
            break;
        default:
            break;
        }
        return format;
    }

    PixelsType pixels {}; ///< The texture in the format defined by DevicePixelFormat
    std::size_t sizeInBytes {}; // The size of the pixels pointer in bytes
    std::size_t width {}; ///< The width of the texture
    std::size_t height {}; ///< The height of the texture
    InternalPixelFormat internalPixelFormat {}; ///< The intended pixel format which is converted to a type of DevicePixelFormat
};

using TextureObjectMipmap = std::array<TextureObject, TextureObject::MAX_LOD + 1>;
} // namespace rr
#endif // TEXTURE_OBJECT_HPP
