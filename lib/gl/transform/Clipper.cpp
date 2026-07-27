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

tcb::span<TransformingVertexParameter> Clipper::clip(ClipList& __restrict list, ClipList& __restrict listBuffer)
{
    ClipList* listIn = &list;
    ClipList* listOut = &listBuffer;

    std::size_t numberOfVerts = 3; // Initial the list contains 3 vertices

    const OutCode oc0 = outCode((*listIn)[0].vertex);
    const OutCode oc1 = outCode((*listIn)[1].vertex);
    const OutCode oc2 = outCode((*listIn)[2].vertex);

    for (auto oc : {
             OutCode::OC_NEAR,
             OutCode::OC_FAR,
             OutCode::OC_LEFT,
             OutCode::OC_RIGHT,
             OutCode::OC_TOP,
             OutCode::OC_BOTTOM })
    {
        // Check if the triangle clips one of the planes. If not, we can skip the plane
        const bool skipPlane = ((oc0 | oc1 | oc2) & oc) == 0;
        if (skipPlane)
        {
            continue;
        }

        // Save the new number of planes
        numberOfVerts = clipAgainstPlane(*listOut, oc, *listIn, numberOfVerts);

        // Swap buffers
        std::swap(listIn, listOut);
    }

    // Assume in this trivial case, that we have clipped a triangle, which was already
    // complete outside. So this triangle shouldn't result in a bigger triangle
    if (outCode((*listIn)[0].vertex) & outCode((*listIn)[1].vertex) & outCode((*listIn)[2].vertex))
        return {};

    return { listIn->data(), numberOfVerts };
}

std::size_t Clipper::clipAgainstPlane(ClipList& __restrict listOut, const OutCode clipPlane, const ClipList& listIn, const std::size_t listSize)
{
    std::size_t outputSize = 0;

    std::size_t previousIndex = listSize - 1;
    float previousDistance = planeDistance(listIn[previousIndex].vertex, clipPlane);
    bool previousInside = previousDistance >= 0.0f;

    for (std::size_t currentIndex = 0; currentIndex < listSize; currentIndex++)
    {
        const float currentDistance = planeDistance(listIn[currentIndex].vertex, clipPlane);
        const bool currentInside = currentDistance >= 0.0f;

        if (currentInside != previousInside)
        {
            const float amount = currentDistance / (currentDistance - previousDistance);
            listOut[outputSize] = clippinghelper::ClippingHelper::lerp(
                amount,
                listIn[currentIndex],
                listIn[previousIndex]);
            outputSize++;
        }

        if (currentInside)
        {
            listOut[outputSize] = listIn[currentIndex];
            outputSize++;
        }

        previousIndex = currentIndex;
        previousDistance = currentDistance;
        previousInside = currentInside;
    }

    return outputSize;
}

} // namespace rr