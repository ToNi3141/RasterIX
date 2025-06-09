// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

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

#ifndef PRIMITIVE_ASSEMBLER_HPP
#define PRIMITIVE_ASSEMBLER_HPP

#include <tcb/span.hpp>

#include "Enums.hpp"
#include "FixedSizeQueue.hpp"
#include "Types.hpp"
#include "ViewPort.hpp"

namespace rr::primitiveassembler
{

struct PrimitiveAssemblerData
{
    DrawMode mode { DrawMode::TRIANGLES };
    std::size_t primitiveCount {};
    float lineWidth { 1.0f };
};

class PrimitiveAssemblerCalc
{
public:
    using Triangle = std::array<std::reference_wrapper<const VertexParameter>, 3>;

    PrimitiveAssemblerCalc(const viewport::ViewPortData& viewPortData, const PrimitiveAssemblerData& primitiveAssemblerData)
        : m_viewPortData { viewPortData }
        , m_primitiveAssemblerData { primitiveAssemblerData }
    {
        updateMode();
        clear();
    }

    tcb::span<const Triangle> getPrimitive()
    {
        if (m_line)
        {
            return constructLine();
        }
        return constructTriangle();
    }
    void removePrimitive() { m_queue.removeElements(m_decrement); }

    VertexParameter& createParameter() { return m_queue.create_back(); }
    void pushParameter(const VertexParameter& param) { m_queue.push_back(param); };

    bool hasTriangles() const { return m_queue.size() >= 3; }

private:
    void clear();
    void updateMode();
    tcb::span<const Triangle> constructTriangle();
    tcb::span<const Triangle> constructLine();
    tcb::span<const Triangle> drawLine(const Vec4& v0,
        const Vec4& v1,
        const std::array<Vec4, RenderConfig::TMU_COUNT>& tc0,
        const std::array<Vec4, RenderConfig::TMU_COUNT>& tc1,
        const Vec4& c0,
        const Vec4& c1);

    FixedSizeQueue<VertexParameter, 3> m_queue {};

    std::size_t m_count { 0 };
    VertexParameter m_pTmp {};

    std::size_t m_decrement { 0 };

    const viewport::ViewPortData& m_viewPortData;
    const PrimitiveAssemblerData& m_primitiveAssemblerData;
    bool m_line { false };
    std::array<VertexParameter, 6> m_vertexParameters;
    std::array<PrimitiveAssemblerCalc::Triangle, 2> m_triangleBuffer { { { { m_pTmp, m_pTmp, m_pTmp } }, { { m_pTmp, m_pTmp, m_pTmp } } } };
};

class PrimitiveAssemblerSetter
{
public:
    PrimitiveAssemblerSetter(PrimitiveAssemblerData& primitiveAssemblerData)
        : m_data { primitiveAssemblerData }
    {
    }

    void setExpectedPrimitiveCount(const std::size_t count) { m_data.primitiveCount = count; }
    void setDrawMode(const DrawMode mode) { m_data.mode = mode; };
    void setLineWidth(const float width) { m_data.lineWidth = width; }

private:
    PrimitiveAssemblerData& m_data;
};

} // namespace rr::primitiveassembler
#endif // PRIMITIVE_ASSEMBLER_HPP
