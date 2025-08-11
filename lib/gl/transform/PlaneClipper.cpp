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

#include "PlaneClipper.hpp"

namespace rr::planeclipper
{

tcb::span<VertexParameter> PlaneClipperCalc::clip(ClipList& __restrict list, ClipList& __restrict listBuffer)
{
    const std::size_t numberOfVerts = clipAgainstPlane(listBuffer, list, 3);
    return { listBuffer.data(), numberOfVerts };
}

std::size_t PlaneClipperCalc::clipAgainstPlane(ClipList& __restrict listOut, const ClipList& listIn, const std::size_t listSize)
{
    // Start Clipping
    std::size_t i = 0;
    bool flagIfSomeVertexIsInside = false;

    for (int32_t vert = 0; vert < static_cast<int32_t>(listSize); vert++)
    {
        const float a = planeEquation(listIn[vert].vertex, m_data.equation);
        if (!isInPlane(a))
        {
            if (i == listOut.size())
            {
                return 0;
            }

            // std::size_t vertPrev = (vert - 1) % listSize;
            const std::size_t vertPrev = (vert - 1) < 0 ? listSize - 1 : static_cast<std::size_t>(vert - 1);
            const float b = planeEquation(listIn[vertPrev].vertex, m_data.equation);
            if (isInPlane(b))
            {
                listOut[i] = clippinghelper::ClippingHelper::lerp(planeEquationLerpAmount(a, b), listIn[vert], listIn[vertPrev]);
                i++;
            }

            // vertNext = (vert + 1) % listSize;
            const std::size_t vertNext = ((vert + 1) >= static_cast<int32_t>(listSize)) ? 0 : static_cast<std::size_t>(vert + 1);
            const float c = planeEquation(listIn[vertNext].vertex, m_data.equation);
            if (isInPlane(c))
            {
                listOut[i] = clippinghelper::ClippingHelper::lerp(planeEquationLerpAmount(a, c), listIn[vert], listIn[vertNext]);
                i++;
            }
        }
        else
        {
            listOut[i] = listIn[vert];
            i++;
            // It's clipped against _one_ infinite plane. That means, at least one vertex needs to be inside and not clipped
            // then we can assume that something of the triangle is inside. In case that all vertecies are outside,
            // then there is no chance that parts of the triangle are still intersect. That is completely different to the
            // frustum clipping where a triangle can intersect the frustum and still has all vertecies outside.
            flagIfSomeVertexIsInside = true;
        }
    }

    if (flagIfSomeVertexIsInside)
    {
        return i;
    }
    return 0;
}

} // namespace rr::planeclipper
