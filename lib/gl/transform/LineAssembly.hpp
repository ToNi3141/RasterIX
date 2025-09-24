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

#ifndef LINE_ASSEMBLY_HPP
#define LINE_ASSEMBLY_HPP

#include "VertexParameter.hpp"
#include "math/Vec.hpp"
#include <array>
#include <tcb/span.hpp>

namespace rr::lineassembly
{

struct LineAssemblyData
{
    float lineWidth { 1.0f };
};

class LineAssemblyCalc
{
public:
    using Triangles = std::array<VertexParameter, 6>;

    LineAssemblyCalc(const LineAssemblyData& data, const float& viewPortWidth, const float& viewPortHeight)
        : m_data { data }
        , m_viewPortWidth { viewPortWidth }
        , m_viewPortHeight { viewPortHeight }
    {
    }

    // Creates lines from projected triangles in NDC
    Triangles createLine(const VertexParameter& vp0, const VertexParameter& vp1) const;

private:
    const LineAssemblyData& m_data;
    const float& m_viewPortWidth;
    const float& m_viewPortHeight;
};

class LineAssemblySetter
{
public:
    LineAssemblySetter(LineAssemblyData& data)
        : m_data { data }
    {
    }

    void setLineWidth(const float lineWidth) { m_data.lineWidth = lineWidth; }

private:
    LineAssemblyData& m_data;
};

} // namespace rr::lineassembly

#endif // LINE_ASSEMBLY_HPP
