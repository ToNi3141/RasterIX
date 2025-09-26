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
#include "TransformingVertexParameter.hpp"
#include "ViewPort.hpp"

namespace rr::primitiveassembler
{

struct PrimitiveAssemblerData
{
    DrawMode mode { DrawMode::TRIANGLES };
    std::size_t primitiveCount {};
};

class PrimitiveAssemblerCalc
{
public:
    using Primitive = tcb::span<const TransformingVertexParameter>;

    PrimitiveAssemblerCalc(const viewport::ViewPortData& viewPortData, const PrimitiveAssemblerData& primitiveAssemblerData);
    void init();

    Primitive getPrimitive();
    void removePrimitive() { m_queue.removeElements(m_decrement); }

    TransformingVertexParameter& createParameter() { return m_queue.create_back(); }
    void pushParameter(const TransformingVertexParameter& param) { m_queue.push_back(param); };

    bool hasTriangles() const { return m_queue.size() >= 3; }

private:
    enum class PrimitiveType
    {
        Triangle,
        Line,
        Point
    };

    void clear();
    void updateMode();
    PrimitiveAssemblerCalc::Primitive constructTriangle();
    PrimitiveAssemblerCalc::Primitive constructLine();
    PrimitiveAssemblerCalc::Primitive constructPoint();

    FixedSizeQueue<TransformingVertexParameter, 3> m_queue {};

    std::size_t m_count { 0 };
    TransformingVertexParameter m_pTmp {};

    std::size_t m_decrement { 0 };

    const viewport::ViewPortData& m_viewPortData;
    const PrimitiveAssemblerData& m_primitiveAssemblerData;
    PrimitiveType m_primitiveType { PrimitiveType::Triangle };
    std::array<TransformingVertexParameter, 3> m_primitiveBuffer;
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

private:
    PrimitiveAssemblerData& m_data;
};

} // namespace rr::primitiveassembler
#endif // PRIMITIVE_ASSEMBLER_HPP
