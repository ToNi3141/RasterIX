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

[[maybe_unused]] static Vec4 deserializeTexelFloatRGBA4444(const uint16_t texel)
{
    constexpr float inv255 = 1.0f / 255.0f;
    const float r = static_cast<float>(((texel >> 12) & 0x0F) << 4) * inv255;
    const float g = static_cast<float>(((texel >> 8) & 0x0F) << 4) * inv255;
    const float b = static_cast<float>(((texel >> 4) & 0x0F) << 4) * inv255;
    const float a = static_cast<float>(((texel >> 0) & 0x0F) << 4) * inv255;
    return Vec4 { r, g, b, a };
}

[[maybe_unused]] static Vec4 deserializeTexelFloatRGBA5551(const uint16_t texel)
{
    constexpr float inv255 = 1.0f / 255.0f;
    const float r = static_cast<float>(((texel >> 11) & 0x1F) << 3) * inv255;
    const float g = static_cast<float>(((texel >> 6) & 0x1F) << 3) * inv255;
    const float b = static_cast<float>(((texel >> 1) & 0x1F) << 3) * inv255;
    const float a = ((texel >> 0) & 0x01) ? 1.0f : 0.0f;
    return Vec4 { r, g, b, a };
}

[[maybe_unused]] static Vec4 deserializeTexelFloatRGB565(const uint16_t texel)
{
    constexpr float inv255 = 1.0f / 255.0f;
    const float r = static_cast<float>(((texel >> 11) & 0x1F) << 3) * inv255;
    const float g = static_cast<float>(((texel >> 5) & 0x3F) << 2) * inv255;
    const float b = static_cast<float>(((texel >> 0) & 0x1F) << 3) * inv255;
    return Vec4 { r, g, b, 1.0f };
}

// Function pointer type for float texel deserialization
using DeserializeTexelFloatFn = Vec4 (*)(uint16_t);

[[maybe_unused]] static DeserializeTexelFloatFn getDeserializeTexelFloatFn(const DevicePixelFormat format)
{
    switch (format)
    {
    case DevicePixelFormat::RGBA4444:
        return &deserializeTexelFloatRGBA4444;
    case DevicePixelFormat::RGBA5551:
        return &deserializeTexelFloatRGBA5551;
    case DevicePixelFormat::RGB565:
        return &deserializeTexelFloatRGB565;
    default:
        return &deserializeTexelFloatRGBA4444;
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

[[maybe_unused]] static uint16_t serializeToRgb565(const Vec4 color)
{
    const uint16_t r = (static_cast<uint16_t>(color[0] * 255.0f) >> 3) << 11;
    const uint16_t g = (static_cast<uint16_t>(color[1] * 255.0f) >> 2) << 5;
    const uint16_t b = (static_cast<uint16_t>(color[2] * 255.0f) >> 3) << 0;
    return r | g | b;
}

[[maybe_unused]] static Vec4 deserializeFromRgb565(const uint16_t color)
{
    constexpr float inv255 = 1.0f / 255.0f;
    const uint8_t r = ((color >> 11) & 0x1F) << 3;
    const uint8_t g = ((color >> 5) & 0x3F) << 2;
    const uint8_t b = ((color >> 0) & 0x1F) << 3;
    return Vec4 { r * inv255, g * inv255, b * inv255, 1.0f };
}

[[maybe_unused]] static float deserializeDepth(const uint16_t depth)
{
    return static_cast<float>(depth) / 65535.0f;
}

[[maybe_unused]] static uint16_t serializeDepth(const float depth)
{
    return static_cast<uint16_t>(depth * 65535.0f);
}

} // namespace rr::softwarerasterizer::softwarerasterizerhelpers

#endif // _SOFTWARE_RASTERIZER_HELPERS_HPP_
