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

#include "Rasterizer.hpp"
#include <cstring>

#include <algorithm> // std::max

namespace rr
{
namespace
{
    // Linear interpolation of an attribute a over the triangle with barycentric weights w0..w2:
    //   value = a0*w0 + a1*w1 + a2*w2
    //   xInc  = a0*wx0 + a1*wx1 + a2*wx2   (same for y)
    //
    // The three edge functions always sum to the triangle area (they are affine in p and equal to the area at all
    // three vertices). Normalized by the area this gives:
    //   w0 + w1 + w2 = 1        ->  w2 = 1 - w0 - w1
    // A constant sum does not change when moving one pixel, so the steps cancel:
    //   wx0 + wx1 + wx2 = 0     ->  wx2 = -wx0 - wx1   (same for y)
    //
    // Inserting w2 and multiplying out:
    //   value = a0*w0 + a1*w1 + a2*(1 - w0 - w1)
    //         = a2 + (a0 - a2)*w0 + (a1 - a2)*w1
    //   xInc  = a0*wx0 + a1*wx1 + a2*(-wx0 - wx1)
    //         = (a0 - a2)*wx0 + (a1 - a2)*wx1
    //
    // With d0 = a0 - a2 and d1 = a1 - a2 this needs 6 instead of 9 multiplications per component, and only two
    // weights have to be calculated.
    template <std::size_t N>
    inline void interpolate(
        const Vec<N>& a0, const Vec<N>& a1, const Vec<N>& a2,
        const Vec2& w, const Vec2& wXInc, const Vec2& wYInc,
        Vec<N>& value, Vec<N>& xInc, Vec<N>& yInc)
    {
        const Vec<N> d0 = a0 - a2;
        const Vec<N> d1 = a1 - a2;
        value = a2 + (d0 * w[0]) + (d1 * w[1]);
        xInc = (d0 * wXInc[0]) + (d1 * wXInc[1]);
        yInc = (d0 * wYInc[0]) + (d1 * wYInc[1]);
    }

} // namespace

bool Rasterizer::increment(TriangleStreamTypes::TriangleDesc& desc,
    const std::size_t lineStart,
    const std::size_t lineEnd)
{
    TriangleStreamTypes::StaticParams& params = desc.param;
    if ((lineStart == 0) && (params.bbStartY < lineEnd))
    {
        // If the triangle completely in the current line, then nothing has to be done here.
        return true;
    }
    else
    {
        // Check if the triangle is in the current area by checking if the end position is below the start line
        // and if the start of the triangle is within this area
        if ((params.bbEndY >= lineStart) && (params.bbStartY < lineEnd))
        {
            // The triangle is within the current display area
            // Check if the triangle started in the previous area. If so, we have to move the interpolation factors
            // to the current area
            if (params.bbStartY < lineStart)
            {
                const int32_t bbDiff = lineStart - params.bbStartY;

                const auto wInitTmp = params.wInit;
                params.wInit = params.wYInc;
                params.wInit *= bbDiff;
                params.wInit += wInitTmp;

                params.depthZw += params.depthZwYInc * bbDiff;

                const auto colorTmp = params.color;
                params.color = params.colorYInc;
                params.color *= bbDiff;
                params.color += colorTmp;

                for (std::size_t i = 0; i < desc.texture.size(); i++)
                {
                    const auto texStqTmp = desc.texture[i].texStq;
                    desc.texture[i].texStq = desc.texture[i].texStqYInc;
                    desc.texture[i].texStq *= bbDiff;
                    desc.texture[i].texStq += texStqTmp;
                }
            }

            return true;
        }
    }
    return false;
}

VecInt Rasterizer::edgeFunctionX(const Vec2i& a, const Vec2i& b, const Vec2i& c)
{
    VecInt val1 = (c[0] - a[0]) * (b[1] - a[1]);
    VecInt val2 = (c[1] - a[1]) * (b[0] - a[0]);
    VecInt ges = val1 - val2;
    return ges;
}

bool Rasterizer::rasterize(TriangleStreamTypes::TriangleDesc& __restrict desc,
    const TransformedTriangle& triangle) const
{
    TriangleStreamTypes::StaticParams& params = desc.param;
    Vec2i v0 = Vec2i::createFromVec<std::array<float, 2>, EDGE_FUNC_SIZE>({ triangle.vertex0[0], triangle.vertex0[1] });
    Vec2i v1 = Vec2i::createFromVec<std::array<float, 2>, EDGE_FUNC_SIZE>({ triangle.vertex1[0], triangle.vertex1[1] });
    Vec2i v2 = Vec2i::createFromVec<std::array<float, 2>, EDGE_FUNC_SIZE>({ triangle.vertex2[0], triangle.vertex2[1] });

    VecInt area = edgeFunctionX(v0, v1, v2); // Sn.4
    VecInt sign = -1; // 1 backface culling; -1 frontface culling
    sign = (area <= 0) ? -1 : 1; // No culling
    area *= sign;
    if (area <= 0x0)
    {
        return false;
    }

    // Initialize Bounding box
    // Get the bounding box
    int32_t bbStartX = std::min(std::min(v0[0], v1[0]), v2[0]);
    int32_t bbStartY = std::min(std::min(v0[1], v1[1]), v2[1]);
    int32_t bbEndX = std::max(std::max(v0[0], v1[0]), v2[0]);
    int32_t bbEndY = std::max(std::max(v0[1], v1[1]), v2[1]);

    // Convert to integer values
    bbStartX = bbStartX + EDGE_FUNC_ZERO_P_FIVE;
    bbStartY = bbStartY + EDGE_FUNC_ZERO_P_FIVE;
    bbEndX = bbEndX + EDGE_FUNC_ONE_P_ZERO + EDGE_FUNC_ZERO_P_FIVE;
    bbEndY = bbEndY + EDGE_FUNC_ONE_P_ZERO + EDGE_FUNC_ZERO_P_FIVE;

    params.bbStartX = bbStartX >> EDGE_FUNC_SIZE;
    params.bbStartY = bbStartY >> EDGE_FUNC_SIZE;
    params.bbEndX = bbEndX >> EDGE_FUNC_SIZE;
    params.bbEndY = bbEndY >> EDGE_FUNC_SIZE;

    if (m_enableScissor)
    {
        bbStartX = std::max(bbStartX, m_scissorStartX);
        bbStartY = std::max(bbStartY, m_scissorStartY);
        bbEndX = std::min(bbEndX, m_scissorEndX);
        bbEndY = std::min(bbEndY, m_scissorEndY);

        if (bbStartX >= bbEndX)
        {
            return false;
        }
        if (bbStartY >= bbEndY)
        {
            return false;
        }
    }

    //    // Clamp against the view port
    //    // Should not be needed when the clipping is enabled
    //         bbStartX = std::max(bbStartX, (int32_t)0);
    //         bbStartY = std::max(bbStartY, (int32_t)0);
    //         bbEndX = std::min(bbEndX + 1, 480); // Increase the size at the end of the bounding box a bit. It can happen otherwise that triangles is discarded because it was too small
    //         bbEndY = std::min(bbEndY + 1, 320);
    //     // Check if the bounding box has at least a width of one. Otherwise the hardware will stuck.
    //        if ((bbEndX - bbStartX) == 0)
    //            return false;

    bbStartX = params.bbStartX << EDGE_FUNC_SIZE;
    bbStartY = params.bbStartY << EDGE_FUNC_SIZE;

    // Interpolate triangle
    Vec2i p = { bbStartX, bbStartY };
    Vec3i& wi = params.wInit; // Sn.4
    Vec3i& wIncX = params.wXInc;
    Vec3i& wIncY = params.wYInc;
    wi[0] = edgeFunctionX(v1, v2, p);
    wi[1] = edgeFunctionX(v2, v0, p);
    wi[2] = edgeFunctionX(v0, v1, p);
    wi *= sign;
    // Calculate the increments of the edge functions for the x and y direction.
    // Optimized version of a
    // Vec2i pw = { bbStartX + EDGE_FUNC_ONE_P_ZERO, bbStartY };
    // wIncX[0] = edgeFunctionX(v1, v2, pw);
    wIncX[0] = (v2[1] - v1[1]) * EDGE_FUNC_ONE_P_ZERO;
    wIncX[1] = (v0[1] - v2[1]) * EDGE_FUNC_ONE_P_ZERO;
    wIncX[2] = (v1[1] - v0[1]) * EDGE_FUNC_ONE_P_ZERO;
    wIncX *= sign;
    wIncY[0] = (v1[0] - v2[0]) * EDGE_FUNC_ONE_P_ZERO;
    wIncY[1] = (v2[0] - v0[0]) * EDGE_FUNC_ONE_P_ZERO;
    wIncY[2] = (v0[0] - v1[0]) * EDGE_FUNC_ONE_P_ZERO;
    wIncY *= sign;

    const float areaInv = 1.0f / area;

    Vec2 wNorm = { static_cast<float>(wi[0]), static_cast<float>(wi[1]) };
    wNorm.mul(areaInv);

    Vec2 wIncXNorm = { static_cast<float>(wIncX[0]), static_cast<float>(wIncX[1]) };
    wIncXNorm.mul(areaInv);

    Vec2 wIncYNorm = { static_cast<float>(wIncY[0]), static_cast<float>(wIncY[1]) };
    wIncYNorm.mul(areaInv);

    Vec3 w = { triangle.vertex0[3], triangle.vertex1[3], triangle.vertex2[3] };
    // Avoid that the w gets too small/big by normalizing it
    if (m_enableScaling)
    {
        w = w.normalize();
    }

    // Texture
    for (std::size_t i = 0; i < desc.texture.size(); i++)
    {
        if (m_tmuEnable[i])
        {
            Vec3 tex0 { triangle.texture0[i][0], triangle.texture0[i][1], triangle.texture0[i][3] };
            Vec3 tex1 { triangle.texture1[i][0], triangle.texture1[i][1], triangle.texture1[i][3] };
            Vec3 tex2 { triangle.texture2[i][0], triangle.texture2[i][1], triangle.texture2[i][3] };

            // Avoid overflowing the integer part by adding an offset
            if (m_enableScaling)
            {
                const float minS = std::min(tex0[0], std::min(tex1[0], tex2[0]));
                const float minT = std::min(tex0[1], std::min(tex1[1], tex2[1]));
                const float maxS = std::max(tex0[0], std::max(tex1[0], tex2[0]));
                const float maxT = std::max(tex0[1], std::max(tex1[1], tex2[1]));

                if (minS < -4.0f)
                {
                    const float minSG = static_cast<int32_t>(minS);
                    tex0[0] -= minSG;
                    tex1[0] -= minSG;
                    tex2[0] -= minSG;
                }
                if (minT < -4.0f)
                {
                    const float minTG = static_cast<int32_t>(minT);
                    tex0[1] -= minTG;
                    tex1[1] -= minTG;
                    tex2[1] -= minTG;
                }
                if (maxS > 4.0f)
                {
                    const float maxSG = static_cast<int32_t>(maxS);
                    tex0[0] -= maxSG;
                    tex1[0] -= maxSG;
                    tex2[0] -= maxSG;
                }
                if (maxT > 4.0f)
                {
                    const float maxTG = static_cast<int32_t>(maxT);
                    tex0[1] -= maxTG;
                    tex1[1] -= maxTG;
                    tex2[1] -= maxTG;
                }
            }

            // Perspective correction
            tex0.mul(w[0]);
            tex1.mul(w[1]);
            tex2.mul(w[2]);

            TriangleStreamTypes::Texture& t = desc.texture[i];

            interpolate(tex0, tex1, tex2, wNorm, wIncXNorm, wIncYNorm, t.texStq, t.texStqXInc, t.texStqYInc);
        }
    }

    // Depth
    const Vec2 zw0 { triangle.vertex0[2], triangle.vertex0[3] };
    const Vec2 zw1 { triangle.vertex1[2], triangle.vertex1[3] };
    const Vec2 zw2 { triangle.vertex2[2], triangle.vertex2[3] };

    interpolate(zw0, zw1, zw2, wNorm, wIncXNorm, wIncYNorm, params.depthZw, params.depthZwXInc, params.depthZwYInc);

    // Color
    interpolate(triangle.color0, triangle.color1, triangle.color2, wNorm, wIncXNorm, wIncYNorm, params.color, params.colorXInc, params.colorYInc);

    return true;
}

float Rasterizer::edgeFunction(const Vec4& a, const Vec4& b, const Vec4& c)
{
    float val1 = (c[0] - a[0]) * (b[1] - a[1]);
    float val2 = (c[1] - a[1]) * (b[0] - a[0]);
    float ges = val1 - val2;
    return ges;
}

void Rasterizer::setScissorBox(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height)
{
    m_scissorStartX = x << EDGE_FUNC_SIZE;
    m_scissorStartY = y << EDGE_FUNC_SIZE;
    m_scissorEndX = (width << EDGE_FUNC_SIZE);
    m_scissorEndY = (height << EDGE_FUNC_SIZE);
}

} // namespace rr