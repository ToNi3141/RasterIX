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
#include <algorithm>
#include <array>
#include <cmath>
#include <memory>

namespace rr
{
struct TextureObject
{
    using PixelsType = std::shared_ptr<uint16_t>;
    static constexpr std::size_t MAX_LOD { 9 };

    DevicePixelFormat getDevicePixelFormat([[maybe_unused]] const std::size_t level) const
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

    std::size_t getWidth(const std::size_t level) const
    {
        return std::max(std::size_t { 1 }, width >> level);
    }

    std::size_t getHeight(const std::size_t level) const
    {
        return std::max(std::size_t { 1 }, height >> level);
    }

    std::size_t getSizeInBytes(const std::size_t level) const
    {
        if (pixels[level])
        {
            return getWidth(level) * getHeight(level) * sizeof(TextureObject::PixelsType::element_type);
        }
        return std::size_t { 0 };
    }

    void setWidth(const std::size_t level, const std::size_t w)
    {
        if (level != 0)
        {
            return;
        }
        width = w;
    }

    void setHeight(const std::size_t level, const std::size_t h)
    {
        if (level != 0)
        {
            return;
        }
        height = h;
    }

    InternalPixelFormat getInternalPixelFormat([[maybe_unused]] const std::size_t level) const
    {
        return internalPixelFormat;
    }

    void setInternalPixelFormat(const std::size_t level, const InternalPixelFormat ipf)
    {
        if (level != 0)
        {
            return;
        }
        internalPixelFormat = ipf;
    }

    PixelsType getPixels(const std::size_t level)
    {
        return pixels[level];
    }

    void setPixels(const std::size_t level, PixelsType p)
    {
        pixels[level] = p;
    }

    std::size_t getLevels() const
    {
        const std::size_t maxPixel = std::max(getWidth(0), getHeight(0));
        const float level = std::log2(static_cast<float>(maxPixel));
        return static_cast<std::size_t>(level);
    }

    std::size_t getMaxLevels() const
    {
        return TextureObject::MAX_LOD;
    }

private:
    /// @brief The texture in the format defined by DevicePixelFormat. Do not reuse this pointer.
    /// The memory allocated in this pointer might be already queued for texture upload.
    /// It is safe to read from this pointer, but not safe to write to it.
    std::array<PixelsType, TextureObject::MAX_LOD> pixels {};
    std::size_t width {}; ///< The width of the texture
    std::size_t height {}; ///< The height of the texture
    InternalPixelFormat internalPixelFormat {}; ///< The intended pixel format which is converted to a type of DevicePixelFormat
};
} // namespace rr
#endif // TEXTURE_OBJECT_HPP
