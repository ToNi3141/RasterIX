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

#ifndef CLIPPER_HPP
#define CLIPPER_HPP

#include "Types.hpp"
#include "math/Vec.hpp"
#include <array>
#include <tcb/span.hpp>

namespace rr
{

class Clipper
{
public:
    // Each clipping plane can potentially introduce one more vertex. A triangle contains 3 vertexes, plus 6 possible planes, results in 9 vertexes.
    using ClipList = std::array<VertexParameter, 9>;

    static tcb::span<VertexParameter> clip(ClipList& __restrict list, ClipList& __restrict listBuffer);

    static bool isOutside(const Vec4& v0, const Vec4& v1, const Vec4& v2)
    {
        const OutCode oc0 = outCode(v0);
        const OutCode oc1 = outCode(v1);
        const OutCode oc2 = outCode(v2);

        return oc0 & oc1 & oc2;
    }

    static bool isInside(const Vec4& v0, const Vec4& v1, const Vec4& v2)
    {
        const OutCode oc0 = outCode(v0);
        const OutCode oc1 = outCode(v1);
        const OutCode oc2 = outCode(v2);

        return (oc0 | oc1 | oc2) == OutCode::OC_NONE;
    }

private:
    enum OutCode
    {
        OC_NONE = 0x00,
        OC_NEAR = 0x01,
        OC_FAR = 0x02,
        OC_TOP = 0x04,
        OC_BOTTOM = 0x08,
        OC_LEFT = 0x10,
        OC_RIGHT = 0x20
    };

    inline static float lerpAmt(OutCode plane, const Vec4& v0, const Vec4& v1);
    inline static bool hasOutCode(const Vec4& v, const OutCode oc);

    static std::size_t clipAgainstPlane(ClipList& __restrict listOut,
        const OutCode clipPlane,
        const ClipList& listIn,
        const std::size_t listSize);

    static OutCode outCode(const Vec4& v)
    {
        OutCode c = OutCode::OC_NONE;
        const float w = v[3];

        if (v[0] < -w)
            c |= OutCode::OC_LEFT;
        if (v[0] > w)
            c |= OutCode::OC_RIGHT;
        if (v[1] < -w)
            c |= OutCode::OC_BOTTOM;
        if (v[1] > w)
            c |= OutCode::OC_TOP;
        if (v[2] < -w)
            c |= OutCode::OC_NEAR;
        if (v[2] > w)
            c |= OutCode::OC_FAR;

        return c;
    }

    friend Clipper::OutCode operator|=(Clipper::OutCode& lhs, Clipper::OutCode rhs);
};

inline Clipper::OutCode operator|=(Clipper::OutCode& lhs, Clipper::OutCode rhs)
{
    return lhs = static_cast<Clipper::OutCode>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

} // namespace rr

#endif // CLIPPER_HPP
