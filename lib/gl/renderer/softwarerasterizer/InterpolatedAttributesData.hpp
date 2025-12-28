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

#ifndef _INTERPOLATED_ATTRIBUTES_DATA_HPP_
#define _INTERPOLATED_ATTRIBUTES_DATA_HPP_

#include "RenderConfigs.hpp"
#include <cstdint>

namespace rr::softwarerasterizer
{
struct InterpolatedAttributesData
{
    struct Texture
    {
        float s; // S16.15
        float t; // S16.15
        float q; // S16.15
    };
    std::array<Texture, RenderConfig::TMU_COUNT> tex;
    std::array<Texture, RenderConfig::TMU_COUNT> texMipmap;
    float depthW;
    float depthZ; // Q16.16
    float colorR; // Qn.0
    float colorG; // Qn.0
    float colorB; // Qn.0
    float colorA; // Qn.0
};

} // namespace rr::softwarerasterizer

#endif // _INTERPOLATED_ATTRIBUTES_DATA_HPP_
