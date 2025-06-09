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

VecInt Rasterizer::edgeFunctionFixPoint(const Vec2i& a, const Vec2i& b, const Vec2i& c)
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

    VecInt area = edgeFunctionFixPoint(v0, v1, v2); // Sn.4
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
    wi[0] = edgeFunctionFixPoint(v1, v2, p);
    wi[1] = edgeFunctionFixPoint(v2, v0, p);
    wi[2] = edgeFunctionFixPoint(v0, v1, p);
    wi *= sign;
    Vec2i pw = { bbStartX + EDGE_FUNC_ONE_P_ZERO, bbStartY };
    wIncX[0] = edgeFunctionFixPoint(v1, v2, pw);
    wIncX[1] = edgeFunctionFixPoint(v2, v0, pw);
    wIncX[2] = edgeFunctionFixPoint(v0, v1, pw);
    wIncX *= sign;
    wIncX -= wi;
    Vec2i ph = { bbStartX, bbStartY + EDGE_FUNC_ONE_P_ZERO };
    wIncY[0] = edgeFunctionFixPoint(v1, v2, ph);
    wIncY[1] = edgeFunctionFixPoint(v2, v0, ph);
    wIncY[2] = edgeFunctionFixPoint(v0, v1, ph);
    wIncY *= sign;
    wIncY -= wi;

    float areaInv = 1.0f / area;

    Vec3 wNorm = { static_cast<float>(wi[0]), static_cast<float>(wi[1]), static_cast<float>(wi[2]) };
    wNorm.mul(areaInv);

    Vec3 wIncXNorm = { static_cast<float>(wIncX[0]), static_cast<float>(wIncX[1]), static_cast<float>(wIncX[2]) };
    wIncXNorm.mul(areaInv);

    Vec3 wIncYNorm = { static_cast<float>(wIncY[0]), static_cast<float>(wIncY[1]), static_cast<float>(wIncY[2]) };
    wIncYNorm.mul(areaInv);

    Vec3 w = { triangle.vertex0[3], triangle.vertex1[3], triangle.vertex2[3] };
    // Avoid that the w gets too small/big by normalizing it
    if (m_enableScaling)
    {
        w.normalize();
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

            t.texStq = (tex0 * wNorm[0])
                + (tex1 * wNorm[1])
                + (tex2 * wNorm[2]);

            t.texStqXInc = (tex0 * wIncXNorm[0])
                + (tex1 * wIncXNorm[1])
                + (tex2 * wIncXNorm[2]);

            t.texStqYInc = (tex0 * wIncYNorm[0])
                + (tex1 * wIncYNorm[1])
                + (tex2 * wIncYNorm[2]);
        }
    }

    // Depth
    const Vec2 zw0 { triangle.vertex0[2], triangle.vertex0[3] };
    const Vec2 zw1 { triangle.vertex1[2], triangle.vertex1[3] };
    const Vec2 zw2 { triangle.vertex2[2], triangle.vertex2[3] };

    params.depthZw = (zw0 * wNorm[0])
        + (zw1 * wNorm[1])
        + (zw2 * wNorm[2]);

    params.depthZwXInc = (zw0 * wIncXNorm[0])
        + (zw1 * wIncXNorm[1])
        + (zw2 * wIncXNorm[2]);

    params.depthZwYInc = (zw0 * wIncYNorm[0])
        + (zw1 * wIncYNorm[1])
        + (zw2 * wIncYNorm[2]);

    // Color
    params.color = (triangle.color0 * wNorm[0])
        + (triangle.color1 * wNorm[1])
        + (triangle.color2 * wNorm[2]);

    params.colorXInc = (triangle.color0 * wIncXNorm[0])
        + (triangle.color1 * wIncXNorm[1])
        + (triangle.color2 * wIncXNorm[2]);

    params.colorYInc = (triangle.color0 * wIncYNorm[0])
        + (triangle.color1 * wIncYNorm[1])
        + (triangle.color2 * wIncYNorm[2]);

    return true;
}

float Rasterizer::edgeFunctionFloat(const Vec4& a, const Vec4& b, const Vec4& c)
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