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

#ifndef CLIPPING_HELPER_HPP
#define CLIPPING_HELPER_HPP

#include "RenderConfigs.hpp"
#include "VertexParameter.hpp"
#include "math/Vec.hpp"
#include <array>

namespace rr::clippinghelper
{

class ClippingHelper
{
public:
    inline static VertexParameter lerp(const float lerpw, const VertexParameter& curr, const VertexParameter& next)
    {
        VertexParameter out;
        out.vertex = lerpVert(curr.vertex, next.vertex, lerpw);
        out.color = lerpVert(curr.color, next.color, lerpw);
        out.tex = lerpTexCoord(curr.tex, next.tex, lerpw);
        return out;
    }

private:
    inline static Vec4 lerpVert(const Vec4& v0, const Vec4& v1, const float amt)
    {
        Vec4 vOut;
        vOut[3] = ((v0[3] - v1[3]) * (1 - amt)) + v1[3];
        vOut[2] = ((v0[2] - v1[2]) * (1 - amt)) + v1[2];
        vOut[1] = ((v0[1] - v1[1]) * (1 - amt)) + v1[1];
        vOut[0] = ((v0[0] - v1[0]) * (1 - amt)) + v1[0];
        return vOut;
    }

    inline static std::array<Vec4, RenderConfig::TMU_COUNT> lerpTexCoord(const std::array<Vec4, RenderConfig::TMU_COUNT>& v0, const std::array<Vec4, RenderConfig::TMU_COUNT>& v1, const float amt)
    {
        std::array<Vec4, RenderConfig::TMU_COUNT> vOut;
        for (std::size_t i = 0; i < vOut.size(); i++)
        {
            vOut[i] = lerpVert(v0[i], v1[i], amt);
        }
        return vOut;
    }
};

} // namespace clippinghelper

#endif // CLIPPING_HELPER_HPP