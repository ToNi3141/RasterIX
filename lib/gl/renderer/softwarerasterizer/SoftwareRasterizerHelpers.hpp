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

#ifndef _SOFTWARE_RASTERIZER_HELPERS_HPP_
#define _SOFTWARE_RASTERIZER_HELPERS_HPP_

#include "Enums.hpp"
#include "math/Vec.hpp"
#include "math/Veci.hpp"
#include <cstdint>

namespace rr::softwarerasterizer::softwarerasterizerhelpers
{

[[maybe_unused]] static Vec4iColorRGBA deserializeTexelRGBA4444(const uint16_t texel)
{
    const int16_t r = ((texel >> 12) & 0x0F);
    const int16_t g = ((texel >> 8) & 0x0F);
    const int16_t b = ((texel >> 4) & 0x0F);
    const int16_t a = ((texel >> 0) & 0x0F);
    const int16_t r8 = (r << 4) | r;
    const int16_t g8 = (g << 4) | g;
    const int16_t b8 = (b << 4) | b;
    const int16_t a8 = (a << 4) | a;
    return Vec4iColorRGBA { r8, g8, b8, a8 };
}

[[maybe_unused]] static Vec4iColorRGBA deserializeTexelRGBA5551(const uint16_t texel)
{
    const int16_t r = ((texel >> 11) & 0x1F);
    const int16_t g = ((texel >> 6) & 0x1F);
    const int16_t b = ((texel >> 1) & 0x1F);
    const int16_t a = ((texel >> 0) & 0x01);
    const int16_t r8 = (r << 3) | (r >> 2);
    const int16_t g8 = (g << 3) | (g >> 2);
    const int16_t b8 = (b << 3) | (b >> 2);
    const int16_t a8 = a ? Vec4iColorRGBA::FracMax : Vec4iColorRGBA::Zero;
    return Vec4iColorRGBA { r8, g8, b8, a8 };
}

[[maybe_unused]] static Vec4iColorRGBA deserializeTexelRGB565(const uint16_t texel)
{
    const int16_t r = ((texel >> 11) & 0x1F);
    const int16_t g = ((texel >> 5) & 0x3F);
    const int16_t b = ((texel >> 0) & 0x1F);
    const int16_t r8 = (r << 3) | (r >> 2);
    const int16_t g8 = (g << 2) | (g >> 4);
    const int16_t b8 = (b << 3) | (b >> 2);
    return Vec4iColorRGBA { r8, g8, b8, Vec4iColorRGBA::FracMax };
}

// Function pointer type for float texel deserialization
using DeserializeTexelFn = Vec4iColorRGBA (*)(uint16_t);
[[maybe_unused]] static DeserializeTexelFn getDeserializeTexelFn(const DevicePixelFormat format)
{
    switch (format)
    {
    case DevicePixelFormat::RGBA4444:
        return &deserializeTexelRGBA4444;
    case DevicePixelFormat::RGBA5551:
        return &deserializeTexelRGBA5551;
    case DevicePixelFormat::RGB565:
        return &deserializeTexelRGB565;
    default:
        return &deserializeTexelRGBA4444;
    }
}

[[maybe_unused]] static uint16_t convertColorMask(const bool r, const bool g, const bool b, const bool a, const DevicePixelFormat format)
{
    uint16_t mask = 0;
    switch (format)
    {
    case DevicePixelFormat::RGBA4444:
        if (r)
            mask |= 0xF000;
        if (g)
            mask |= 0x0F00;
        if (b)
            mask |= 0x00F0;
        if (a)
            mask |= 0x000F;
        break;
    case DevicePixelFormat::RGBA5551:
        if (r)
            mask |= 0xF800;
        if (g)
            mask |= 0x07C0;
        if (b)
            mask |= 0x003E;
        if (a)
            mask |= 0x0001;
        break;
    case DevicePixelFormat::RGB565:
        if (r)
            mask |= 0xF800;
        if (g)
            mask |= 0x07E0;
        if (b)
            mask |= 0x001F;
        break;
    }
    return mask;
}

[[maybe_unused]] static uint16_t convertDepthMask(const bool depthMask)
{
    return depthMask ? 0xFFFF : 0x0000;
}

[[maybe_unused]] static uint16_t serializeToRgb565(const Vec4iColorRGBA& color)
{
    const uint16_t r = (static_cast<uint16_t>(color[0]) >> 3) << 11;
    const uint16_t g = (static_cast<uint16_t>(color[1]) >> 2) << 5;
    const uint16_t b = (static_cast<uint16_t>(color[2]) >> 3) << 0;
    return r | g | b;
}

[[maybe_unused]] static Vec4iColorRGBA deserializeFromRgb565(const uint16_t color)
{
    return deserializeTexelRGB565(color);
}

[[maybe_unused]] static float deserializeDepth(const uint16_t depth)
{
    return static_cast<float>(depth) / 65535.0f;
}

[[maybe_unused]] static uint16_t serializeDepth(const float depth)
{
    return static_cast<uint16_t>(depth * 65535.0f);
}

[[maybe_unused]] static uint16_t serializeDepthInt(const int32_t depth)
{
    return static_cast<uint16_t>(depth);
}

} // namespace rr::softwarerasterizer::softwarerasterizerhelpers

#endif // _SOFTWARE_RASTERIZER_HELPERS_HPP_
