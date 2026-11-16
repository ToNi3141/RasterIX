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
    using SerializedPayloadType = TrDesc;

    TriangleStreamCmd() = default;
    TriangleStreamCmd(const Rasterizer& rasterizer, const TransformedTriangle& triangle)
        : m_rasterizer { &rasterizer }
        , m_triangle { &triangle }
    {
    }

    TriangleStreamCmd(const CommandType, const PayloadType& payload, const bool)
    {
        m_desc = payload;
    }

    TriangleStreamCmd(const TriangleStreamCmd& c) { operator=(c); }

    bool isInBounds(const std::size_t lineStart, const std::size_t lineEnd) const
    {
        ensureRasterized();
        return m_visible && Rasterizer::checkIfTriangleIsInBounds(m_desc[0].param, lineStart, lineEnd);
    }

    TriangleStreamCmd getIncremented(const std::size_t lineStart, const std::size_t lineEnd)
    {
        ensureRasterized();
        TriangleStreamCmd cmd = *this;
        Rasterizer::increment(cmd.m_buffer[0], lineStart, lineEnd);
        return cmd;
    }

    PayloadType payload() const
    {
        ensureRasterized();
        return m_desc;
    }
    static constexpr CommandType command() { return op::TRIANGLE_STREAM | (displaylist::DisplayList::template sizeOf<TrDesc>()); }
    static constexpr std::size_t getOpSize() { return displaylist::DisplayList::template sizeOf<CommandType>(); }
    static constexpr std::size_t getCommandMaxSize() { return getOpSize() + getMaxPayloadSize(); }
    static constexpr std::size_t getCommandSize() { return getCommandMaxSize(); }
    static constexpr std::size_t getPayloadSize() { return displaylist::DisplayList::template sizeOf<TrDesc>(); }
    static constexpr std::size_t getNumberOfPayloadElements() { return 1; }
    static constexpr std::size_t getMaxPayloadSize() { return displaylist::DisplayList::template sizeOf<TrDesc>(); }

    // Returns false if the triangle is invisible. The caller must then drop the slot.
    bool serializePayload(SerializedPayloadType& buffer, const std::size_t) const
    {
        if (m_triangle != nullptr)
        {
            return m_rasterizer->rasterize(buffer, *m_triangle);
        }
        if (!m_visible)
        {
            return false;
        }
        buffer = m_desc[0];
        return true;
    }

    TriangleStreamCmd& operator=(const TriangleStreamCmd& rhs)
    {
        std::copy(rhs.m_desc.begin(), rhs.m_desc.end(), m_buffer.begin());
        m_desc = rhs.m_desc.empty() ? PayloadType {} : PayloadType { m_buffer };
        m_visible = rhs.m_visible;
        m_rasterizer = rhs.m_rasterizer;
        m_triangle = rhs.m_triangle;
        return *this;
    }

    static std::size_t getNumberOfElementsInPayloadByCommand(const uint32_t) { return std::tuple_size<PayloadBuffer> {}; }
    static bool isThis(const CommandType cmd) { return (cmd & op::MASK) == op::TRIANGLE_STREAM; }

private:
    void ensureRasterized() const
    {
        if (m_triangle == nullptr)
        {
            return;
        }
        m_visible = m_rasterizer->rasterize(m_buffer[0], *m_triangle);
        m_desc = { m_buffer };
        m_triangle = nullptr;
    }

    using PayloadBuffer = std::array<TrDesc, 1>;
    mutable PayloadBuffer m_buffer;
    mutable PayloadType m_desc {};
    mutable bool m_visible { true };
    const Rasterizer* m_rasterizer { nullptr };
    mutable const TransformedTriangle* m_triangle { nullptr };
};

} // namespace rr

#endif // _TRIANGLE_STREAM_CMD_HPP_
