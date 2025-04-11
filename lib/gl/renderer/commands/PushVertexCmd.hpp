// RRX
// https://github.com/ToNi3141/RRX
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

#ifndef _PUSH_VERTEX_CMD_HPP_
#define _PUSH_VERTEX_CMD_HPP_

#include "RenderConfigs.hpp"
#include "math/Vec.hpp"
#include "transform/Types.hpp"
#include <array>
#include <cstdint>
#include <tcb/span.hpp>
#include <type_traits>
#include <typeinfo>

namespace rr
{

class PushVertexCmd
{
    static constexpr uint32_t PUSH_VERTEX { 0xD000'0000 };
    static constexpr uint32_t OP_MASK { 0xF000'0000 };

public:
    struct Vertex
    {
#pragma pack(push, 4)
        VertexParameter vertex;
#pragma pack(pop)
        Vertex& operator=(const Vertex&) = default;
    };

    PushVertexCmd() = default;
    PushVertexCmd(const VertexParameter& vertex)
    {
        m_desc[0].vertex = vertex;
    }

    using PayloadType = std::array<Vertex, 1>;
    const PayloadType& payload() const { return m_desc; }
    using CommandType = uint32_t;
    static constexpr CommandType command() { return PUSH_VERTEX | (displaylist::DisplayList::template sizeOf<Vertex>()); }

    static std::size_t getNumberOfElementsInPayloadByCommand(const uint32_t) { return std::tuple_size<PayloadType> {}; }
    static bool isThis(const CommandType cmd) { return (cmd & OP_MASK) == PUSH_VERTEX; }

    PushVertexCmd& operator=(const PushVertexCmd&) = default;

private:
    PayloadType m_desc;
};

} // namespace rr

#endif // _PUSH_VERTEX_CMD_HPP_
