// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2025 ToNi3141

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef POINT_ASSEMBLY_HPP
#define POINT_ASSEMBLY_HPP

#include "Types.hpp"
#include "math/Vec.hpp"
#include <array>
#include <tcb/span.hpp>

namespace rr::pointassembly
{

struct PointAssemblyData
{
    float pointSize { 1.0f };
};

class PointAssemblyCalc
{
public:
    using Triangles = std::array<VertexParameter, 6>;

    PointAssemblyCalc(const PointAssemblyData& data, const float& viewPortWidth, const float& viewPortHeight)
        : m_data { data }
        , m_viewPortWidth { viewPortWidth }
        , m_viewPortHeight { viewPortHeight }
    {
    }

    // Creates a square for a point in NDC
    Triangles createPoint(const VertexParameter& vp) const;

private:
    const PointAssemblyData& m_data;
    const float& m_viewPortWidth;
    const float& m_viewPortHeight;
};

class PointAssemblySetter
{
public:
    PointAssemblySetter(PointAssemblyData& data)
        : m_data { data }
    {
    }

    void setPointSize(const float pointSize) { m_data.pointSize = pointSize; }

private:
    PointAssemblyData& m_data;
};

} // namespace rr::pointassembly

#endif // POINT_ASSEMBLY_HPP
