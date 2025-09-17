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

#include "PrimitiveAssembler.hpp"

namespace rr::primitiveassembler
{

PrimitiveAssemblerCalc::PrimitiveAssemblerCalc(const viewport::ViewPortData& viewPortData, const PrimitiveAssemblerData& primitiveAssemblerData)
    : m_viewPortData { viewPortData }
    , m_primitiveAssemblerData { primitiveAssemblerData }
{
    init();
}

void PrimitiveAssemblerCalc::init()
{
    updateMode();
    clear();
}

PrimitiveAssemblerCalc::Primitive PrimitiveAssemblerCalc::getPrimitive()
{
    switch (m_primitiveType)
    {
    case PrimitiveType::Triangle:
        return constructTriangle();
    case PrimitiveType::Line:
        return constructLine();
    case PrimitiveType::Point:
        return constructPoint();
    default:
        return {};
    }
}

PrimitiveAssemblerCalc::Primitive PrimitiveAssemblerCalc::constructTriangle()
{
    if (m_queue.size() < 3)
    {
        return {};
    }

    switch (m_primitiveAssemblerData.mode)
    {
    case DrawMode::TRIANGLES:
        m_primitiveBuffer = { m_queue[0], m_queue[1], m_queue[2] };
        m_decrement = 3;
        break;
    case DrawMode::POLYGON:
    case DrawMode::TRIANGLE_FAN:
        if (m_count == 0)
        {
            m_pTmp = m_queue[0];
        }
        m_primitiveBuffer = { m_pTmp, m_queue[1], m_queue[2] };
        m_decrement = 1;
        break;
    case DrawMode::TRIANGLE_STRIP:
        if (m_count & 0x1)
        {
            m_primitiveBuffer = { m_queue[1], m_queue[0], m_queue[2] };
        }
        else
        {
            m_primitiveBuffer = { m_queue[0], m_queue[1], m_queue[2] };
        }
        m_decrement = 1;
        break;
    case DrawMode::QUADS:
        if (m_count & 0x1)
        {
            m_primitiveBuffer = { m_pTmp, m_queue[1], m_queue[2] };
            m_decrement = 3;
        }
        else
        {
            m_pTmp = m_queue[0];
            m_primitiveBuffer = { m_pTmp, m_queue[1], m_queue[2] };

            m_decrement = 1;
        }
        break;
    case DrawMode::QUAD_STRIP:
        if (m_count & 0x1)
        {
            m_primitiveBuffer = { m_queue[0], m_queue[2], m_queue[1] };
        }
        else
        {
            m_primitiveBuffer = { m_queue[0], m_queue[1], m_queue[2] };
        }
        m_decrement = 1;
        break;
    default:
        return {};
    }

    m_count++;
    return { m_primitiveBuffer.data(), 3 };
}

PrimitiveAssemblerCalc::Primitive PrimitiveAssemblerCalc::constructLine()
{
    if (m_queue.size() < 2)
    {
        return {};
    }

    const std::size_t last = (m_count == (m_primitiveAssemblerData.primitiveCount - 1));

    switch (m_primitiveAssemblerData.mode)
    {
    case DrawMode::LINES:
        m_primitiveBuffer[0] = m_queue[0];
        m_primitiveBuffer[1] = m_queue[1];
        m_decrement = 2;
        break;
    case DrawMode::LINE_LOOP:
        if (m_count == 0)
        {
            m_pTmp = m_queue[0];
        }
        if (last)
        {
            m_primitiveBuffer[0] = m_queue[0];
            m_primitiveBuffer[1] = m_pTmp;
        }
        else
        {
            m_primitiveBuffer[0] = m_queue[0];
            m_primitiveBuffer[1] = m_queue[1];
        }
        m_decrement = 1;
        break;
    case DrawMode::LINE_STRIP:
        m_primitiveBuffer[0] = m_queue[0];
        m_primitiveBuffer[1] = m_queue[1];
        m_decrement = 1;
        break;
    default:
        return {};
    }

    m_count++;
    return { m_primitiveBuffer.data(), 2 };
}

PrimitiveAssemblerCalc::Primitive PrimitiveAssemblerCalc::constructPoint()
{
    if (m_queue.size() < 1)
    {
        return {};
    }

    m_primitiveBuffer[0] = m_queue[0];
    m_decrement = 1;

    return { m_primitiveBuffer.data(), 1 };
}

void PrimitiveAssemblerCalc::updateMode()
{
    switch (m_primitiveAssemblerData.mode)
    {
    case DrawMode::LINES:
    case DrawMode::LINE_LOOP:
    case DrawMode::LINE_STRIP:
        m_primitiveType = PrimitiveType::Line;
        break;
    case DrawMode::POINTS:
        m_primitiveType = PrimitiveType::Point;
        break;
    default:
        m_primitiveType = PrimitiveType::Triangle;
        break;
    }
}

void PrimitiveAssemblerCalc::clear()
{
    m_queue.clear();
    m_count = 0;
}

} // namespace rr::primitiveassembler
