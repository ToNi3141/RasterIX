// RasterIX
// https://github.com/ToNi3141/RasterIX
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

#ifndef _TRIANGLE_STREAM_TYPES_CMD_HPP_
#define _TRIANGLE_STREAM_TYPES_CMD_HPP_

#include "RenderConfigs.hpp"
#include "math/Vec.hpp"
#include "math/Veci.hpp"
#include <cstdint>

namespace rr
{

namespace TriangleStreamTypes
{

    struct Texture
    {
        Vec3 texStq;
        Vec3 texStqXInc;
        Vec3 texStqYInc;
    };

    struct StaticParams
    {
        uint32_t reserved;
        uint16_t bbStartX;
        uint16_t bbStartY;
        uint16_t bbEndX;
        uint16_t bbEndY;
        Vec3i wInit;
        Vec3i wXInc;
        Vec3i wYInc;
        Vec4 color;
        Vec4 colorXInc;
        Vec4 colorYInc;
        Vec2 depthZw;
        Vec2 depthZwXInc;
        Vec2 depthZwYInc;
    };

    struct TextureX
    {
        Vec3i texStq;
        Vec3i texStqXInc;
        Vec3i texStqYInc;

        void operator=(const Texture& t)
        {
            texStq.fromVec<Vec3, 28>(t.texStq);
            texStqXInc.fromVec<Vec3, 28>(t.texStqXInc);
            texStqYInc.fromVec<Vec3, 28>(t.texStqYInc);
        }
    };

    struct StaticParamsX
    {
        uint32_t reserved;
        uint16_t bbStartX;
        uint16_t bbStartY;
        uint16_t bbEndX;
        uint16_t bbEndY;
        Vec3i wInit;
        Vec3i wXInc;
        Vec3i wYInc;
        Vec4i color;
        Vec4i colorXInc;
        Vec4i colorYInc;
        Vec2i depthZw;
        Vec2i depthZwXInc;
        Vec2i depthZwYInc;

        void operator=(const StaticParams& t)
        {
            bbStartX = t.bbStartX;
            bbStartY = t.bbStartY;
            bbEndX = t.bbEndX;
            bbEndY = t.bbEndY;
            wInit = t.wInit;
            wXInc = t.wXInc;
            wYInc = t.wYInc;
            color.fromVec<Vec4, 24>(t.color);
            colorXInc.fromVec<Vec4, 24>(t.colorXInc);
            colorYInc.fromVec<Vec4, 24>(t.colorYInc);
            depthZw.fromVec<Vec2, 30>(t.depthZw);
            depthZwXInc.fromVec<Vec2, 30>(t.depthZwXInc);
            depthZwYInc.fromVec<Vec2, 30>(t.depthZwYInc);
        }
    };

    struct TriangleDesc
    {
#pragma pack(push, 4)
        StaticParams param;
        std::array<Texture, RenderConfig::TMU_COUNT> texture;
#pragma pack(pop)
    };

    struct TriangleDescX
    {
#pragma pack(push, 4)
        StaticParamsX param;
        std::array<TextureX, RenderConfig::TMU_COUNT> texture;

        TriangleDescX() = default;

        TriangleDescX(const TriangleDesc& t)
        {
            *this = t;
        }

        void operator=(const TriangleDesc& t)
        {
            param = t.param;
            for (std::size_t i = 0; i < RenderConfig::TMU_COUNT; i++)
            {
                texture[i] = t.texture[i];
            }
        };
#pragma pack(pop)
    };

} // namespace TriangleStreamTypes

} // namespace rr

#endif // _TRIANGLE_STREAM_TYPES_CMD_HPP_
