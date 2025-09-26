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

#include "Clipper.hpp"
#include "ClippingHelper.hpp"

namespace rr
{

bool Clipper::hasOutCode(const Vec4& v, const OutCode oc)
{
    switch (oc)
    {
    case OutCode::OC_LEFT:
        return v[0] < -v[3];
    case OutCode::OC_RIGHT:
        return v[0] > v[3];
    case OutCode::OC_BOTTOM:
        return v[1] < -v[3];
    case OutCode::OC_TOP:
        return v[1] > v[3];
    case OutCode::OC_NEAR:
        return v[2] < -v[3];
    case OutCode::OC_FAR:
        return v[2] > v[3];
    default:
        return false;
    }
}

float Clipper::lerpAmt(OutCode plane, const Vec4& v0, const Vec4& v1)
{
    // For a better explanation see https://chaosinmotion.com/2016/05/22/3d-clipping-in-homogeneous-coordinates/
    // and https://github.com/w3woody/arduboy/blob/master/Demo3D/pipeline.cpp
    float zDot0 = 0.0f;
    float zDot1 = 0.0f;

    switch (plane)
    {
    case OutCode::OC_RIGHT: // v.dot(1,0,0,-1)
        zDot0 = v0[0] - v0[3];
        zDot1 = v1[0] - v1[3];
        break;
    case OutCode::OC_LEFT: // v.dot(1,0,0,1)
        zDot0 = v0[0] + v0[3];
        zDot1 = v1[0] + v1[3];
        break;
    case OutCode::OC_TOP: // v.dot(0,1,0,-1)
        zDot0 = v0[1] - v0[3];
        zDot1 = v1[1] - v1[3];
        break;
    case OutCode::OC_BOTTOM: // v.dot(0,1,0,1)
        zDot0 = v0[1] + v0[3];
        zDot1 = v1[1] + v1[3];
        break;
    case OutCode::OC_NEAR: // v.dot(0,0,1,1)
        zDot0 = v0[2] + v0[3];
        zDot1 = v1[2] + v1[3];
        break;
    case OutCode::OC_FAR: // v.dot(0,0,1,-1)
    default:
        zDot0 = v0[2] - v0[3];
        zDot1 = v1[2] - v1[3];
        break;
    }
    return zDot0 / (zDot0 - zDot1);
}

tcb::span<TransformingVertexParameter> Clipper::clip(ClipList& __restrict list, ClipList& __restrict listBuffer)
{
    ClipList* listIn = &list;
    ClipList* listOut = &listBuffer;

    std::size_t numberOfVerts = 3; // Initial the list contains 3 vertices
    std::size_t numberOfVertsCurrentPlane = 0;

    for (auto oc : { OutCode::OC_NEAR, OutCode::OC_FAR, OutCode::OC_LEFT, OutCode::OC_RIGHT, OutCode::OC_TOP, OutCode::OC_BOTTOM })
    {
        // Optimization hint: If no vertex has an outcode given from oc, then it can just be
        // ignored. We just have to avoid that the swapping of the buffers is executed.
        // Then we can skip the unneeded copying of data.
        numberOfVertsCurrentPlane = clipAgainstPlane(*listOut, oc, *listIn, numberOfVerts);
        if (numberOfVertsCurrentPlane > 0)
        {
            // Swap buffers
            std::swap(listIn, listOut);

            // Safe the new number of planes
            numberOfVerts = numberOfVertsCurrentPlane;
        }
    }

    // Assume in this trivial case, that we have clipped a triangle, which was already
    // complete outside. So this triangle shouldn't result in a bigger triangle
    if (outCode((*listIn)[0].vertex) & outCode((*listIn)[1].vertex) & outCode((*listIn)[2].vertex))
        return {};

    return { listIn->data(), numberOfVerts };
}

std::size_t Clipper::clipAgainstPlane(ClipList& __restrict listOut, const OutCode clipPlane, const ClipList& listIn, const std::size_t listSize)
{
    // Start Clipping
    std::size_t i = 0;

    for (int32_t vert = 0; vert < static_cast<int32_t>(listSize); vert++)
    {
        if (hasOutCode(listIn[vert].vertex, clipPlane))
        {
            // std::size_t vertMod = (vert - 1) % listSize;
            const std::size_t vertPrev = (vert - 1) < 0 ? listSize - 1 : static_cast<std::size_t>(vert - 1);
            if (!hasOutCode(listIn[vertPrev].vertex, clipPlane))
            {
                listOut[i] = clippinghelper::ClippingHelper::lerp(lerpAmt(clipPlane, listIn[vert].vertex, listIn[vertPrev].vertex), listIn[vert], listIn[vertPrev]);
                i++;
            }

            // vertMod = (vert + 1) % listSize;
            const std::size_t vertNext = ((vert + 1) >= static_cast<int32_t>(listSize)) ? 0 : static_cast<std::size_t>(vert + 1);
            if (!hasOutCode(listIn[vertNext].vertex, clipPlane))
            {
                listOut[i] = clippinghelper::ClippingHelper::lerp(lerpAmt(clipPlane, listIn[vert].vertex, listIn[vertNext].vertex), listIn[vert], listIn[vertNext]);
                i++;
            }
        }
        else
        {
            listOut[i] = listIn[vert];
            i++;
        }
    }

    return i;
}

} // namespace rr