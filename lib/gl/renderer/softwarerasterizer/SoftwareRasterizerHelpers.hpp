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

static Vec4ui8 deserializeTexelInt(const uint16_t texel, const DevicePixelFormat format)
{
    Vec4ui8 color;
    switch (format)
    {
    case DevicePixelFormat::RGBA4444:
        color[0] = ((texel >> 12) & 0x0F) << 4;
        color[1] = ((texel >> 8) & 0x0F) << 4;
        color[2] = ((texel >> 4) & 0x0F) << 4;
        color[3] = ((texel >> 0) & 0x0F) << 4;
        break;
    case DevicePixelFormat::RGBA5551:
        color[0] = ((texel >> 11) & 0x1F) << 3;
        color[1] = ((texel >> 6) & 0x1F) << 3;
        color[2] = ((texel >> 1) & 0x1F) << 3;
        color[3] = ((texel >> 0) & 0x01) ? 255 : 0;
        break;
    case DevicePixelFormat::RGB565:
        color[0] = ((texel >> 11) & 0x1F) << 3;
        color[1] = ((texel >> 5) & 0x3F) << 2;
        color[2] = ((texel >> 0) & 0x1F) << 3;
        color[3] = 255;
        break;
    }
    return color;
}

static uint16_t convertColorMask(const bool r, const bool g, const bool b, const bool a, const DevicePixelFormat format)
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

static uint16_t convertDepthMask(const bool depthMask)
{
    return depthMask ? 0xFFFF : 0x0000;
}

static Vec4 deserializeTexel(const uint16_t texel, const DevicePixelFormat format)
{
    constexpr float inv255 = 1.0f / 255.0f;
    Vec4ui8 colorUi8 = deserializeTexelInt(texel, format);
    Vec4 color;
    color[0] = colorUi8[0] * inv255;
    color[1] = colorUi8[1] * inv255;
    color[2] = colorUi8[2] * inv255;
    color[3] = colorUi8[3] * inv255;
    return color;
}

static uint16_t serializeToRgb565(const Vec4 color)
{
    const uint16_t r = (static_cast<uint16_t>(color[0] * 255.0f) >> 3) << 11;
    const uint16_t g = (static_cast<uint16_t>(color[1] * 255.0f) >> 2) << 5;
    const uint16_t b = (static_cast<uint16_t>(color[2] * 255.0f) >> 3) << 0;
    return r | g | b;
}

static Vec4 deserializeFromRgb565(const uint16_t color)
{
    constexpr float inv255 = 1.0f / 255.0f;
    const uint8_t r = ((color >> 11) & 0x1F) << 3;
    const uint8_t g = ((color >> 5) & 0x3F) << 2;
    const uint8_t b = ((color >> 0) & 0x1F) << 3;
    return Vec4 { r * inv255, g * inv255, b * inv255, 1.0f };
}

static float deserializeDepth(const uint16_t depth)
{
    return static_cast<float>(depth) / 65535.0f;
}

static uint16_t serializeDepth(const float depth)
{
    return static_cast<uint16_t>(depth * 65535.0f);
}

} // namespace rr::softwarerasterizer::softwarerasterizerhelpers

#endif // _SOFTWARE_RASTERIZER_HELPERS_HPP_
