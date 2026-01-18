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

#ifndef _TRIANGLE_STREAM_CMD_HPP_
#define _TRIANGLE_STREAM_CMD_HPP_

#include "math/Vec.hpp"
#include "renderer/Rasterizer.hpp"
#include "renderer/Triangle.hpp"
#include "renderer/commands/TriangleStreamTypes.hpp"
#include "renderer/displaylist/DisplayList.hpp"
#include <array>
#include <cstdint>
#include <tcb/span.hpp>
#include <type_traits>
#include <typeinfo>

namespace rr
{

class TriangleStreamCmd
{
public:
    // Both, the float and fix point variant expecting the triangle parameters as float.
    // Therefore: Set the interpolation by default to float.
    static constexpr bool ENABLE_FLOAT_INTERPOLATION { true };
    using TrDesc = typename std::conditional<ENABLE_FLOAT_INTERPOLATION, TriangleStreamTypes::TriangleDesc, TriangleStreamTypes::TriangleDescX>::type;
    using PayloadType = tcb::span<const TrDesc>;
    using CommandType = uint32_t;

    TriangleStreamCmd() = default;
    TriangleStreamCmd(const Rasterizer& rasterizer, const TransformedTriangle& triangle)
    {
        m_visible = rasterizer.rasterize(m_buffer[0], triangle);
        m_desc = { m_buffer };
    }

    TriangleStreamCmd(const CommandType, const PayloadType& payload, const bool)
    {
        m_desc = payload;
    }

    TriangleStreamCmd(const TriangleStreamCmd& c) { operator=(c); }

    bool isInBounds(const std::size_t lineStart, const std::size_t lineEnd) const
    {
        return Rasterizer::checkIfTriangleIsInBounds(m_desc[0].param, lineStart, lineEnd);
    }

    TriangleStreamCmd getIncremented(const std::size_t lineStart, const std::size_t lineEnd)
    {
        TriangleStreamCmd cmd = *this;
        Rasterizer::increment(cmd.m_buffer[0], lineStart, lineEnd);
        return cmd;
    }

    bool isVisible() const { return m_visible; };

    PayloadType payload() const { return m_desc; }
    static constexpr CommandType command() { return op::TRIANGLE_STREAM | (displaylist::DisplayList::template sizeOf<TrDesc>()); }

    TriangleStreamCmd& operator=(const TriangleStreamCmd& rhs)
    {
        std::copy(rhs.m_desc.begin(), rhs.m_desc.end(), m_buffer.begin());
        m_desc = { m_buffer };
        m_visible = rhs.m_visible;
        return *this;
    }

    static std::size_t getNumberOfElementsInPayloadByCommand(const uint32_t) { return std::tuple_size<PayloadBuffer> {}; }
    static bool isThis(const CommandType cmd) { return (cmd & op::MASK) == op::TRIANGLE_STREAM; }

private:
    using PayloadBuffer = std::array<TrDesc, 1>;
    PayloadBuffer m_buffer {};
    PayloadType m_desc {};
    bool m_visible { false };
};

} // namespace rr

#endif // _TRIANGLE_STREAM_CMD_HPP_
