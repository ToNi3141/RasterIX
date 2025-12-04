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

#ifndef PLANE_CLIPPER_HPP
#define PLANE_CLIPPER_HPP

#include "ClippingHelper.hpp"
#include "math/Mat44.hpp"
#include "math/Vec.hpp"
#include "transform/TransformingVertexParameter.hpp"
#include <array>
#include <tcb/span.hpp>

namespace rr::planeclipper
{

struct PlaneClipperData
{
    Vec4 equation {};
    bool enable { false };
};

class PlaneClipperCalc
{
public:
    // Each clipping plane can potentially introduce one more vertex. A triangle contains 3 vertexes, plus 6 possible planes, results in 9 vertexes.
    using ClipList = std::array<TransformingVertexParameter, 4>;

    PlaneClipperCalc(const PlaneClipperData& data)
        : m_data { data }
    {
    }

    bool enabled() { return m_data.enable; }
    tcb::span<TransformingVertexParameter> clipTriangle(ClipList& __restrict list, ClipList& __restrict listBuffer);
    tcb::span<TransformingVertexParameter> clipLine(ClipList& __restrict list, ClipList& __restrict listBuffer);
    tcb::span<TransformingVertexParameter> clipPoint(ClipList& __restrict list, ClipList& __restrict listBuffer);

private:
    inline static float isInPlane(const float e) { return e > 0.0f; }
    inline static float planeEquationLerpAmount(const float e0, const float e1) { return e0 / (e0 - e1); }
    inline static float planeEquation(const Vec4& v, const Vec4& e) { return v.dot(e); }
    tcb::span<TransformingVertexParameter> clipTriangleAgainstPlane(ClipList& __restrict listOut, const ClipList& listIn);
    tcb::span<TransformingVertexParameter> clipLineAgainstPlane(ClipList& __restrict listOut, const ClipList& listIn);
    tcb::span<TransformingVertexParameter> clipPointAgainstPlane(ClipList& __restrict listOut, const ClipList& listIn);

    const PlaneClipperData& m_data;
};

class PlaneClipperSetter
{
public:
    PlaneClipperSetter(PlaneClipperData& data, const Mat44& modelViewMat)
        : m_modelViewMat { modelViewMat }
        , m_data { data }
    {
    }

    void setEquation(const Vec4& equation)
    {
        Mat44 mat = m_modelViewMat;
        mat.invert();
        m_data.equation = mat.transform(equation);
    }

    void setEnable(const bool enable) { m_data.enable = enable; }

    const Vec4& getEquation() const { return m_data.equation; }
    bool getEnable() const { return m_data.enable; }

    static constexpr std::size_t getMaxClipPlanes() { return 1; }

private:
    const Mat44& m_modelViewMat;
    PlaneClipperData& m_data;
};

} // namespace rr::planeclipper

#endif // PLANE_CLIPPER_HPP
