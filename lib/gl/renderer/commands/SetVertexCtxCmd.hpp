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

#ifndef _SET_VERTEX_CTX_CMD_HPP_
#define _SET_VERTEX_CTX_CMD_HPP_

#include "Op.hpp"
#include "RenderConfigs.hpp"
#include "math/Vec.hpp"
#include "transform/VertexTransforming.hpp"
#include <array>
#include <cstdint>
#include <tcb/span.hpp>
#include <type_traits>
#include <typeinfo>

namespace rr
{

class SetVertexCtxCmd
{
public:
    struct VertexCtx
    {
#pragma pack(push, 4)
        vertextransforming::VertexTransformingData ctx;
#pragma pack(pop)
        VertexCtx& operator=(const VertexCtx&) = default;
    };
    using PayloadType = tcb::span<const VertexCtx>;
    using CommandType = uint32_t;

    SetVertexCtxCmd() = default;
    SetVertexCtxCmd(const vertextransforming::VertexTransformingData& ctx)
    {
        m_buffer[0].ctx = ctx;
        m_desc = { m_buffer };
    }

    SetVertexCtxCmd(const CommandType, const PayloadType& payload, const bool)
    {
        m_desc = payload;
    }

    const PayloadType& payload() const { return m_desc; }
    static constexpr CommandType command() { return op::SET_VERTEX_CTX | (displaylist::DisplayList::template sizeOf<VertexCtx>()); }

    static std::size_t getNumberOfElementsInPayloadByCommand(const uint32_t) { return std::tuple_size<PayloadBuffer> {}; }
    static bool isThis(const CommandType cmd) { return (cmd & op::MASK) == op::SET_VERTEX_CTX; }

    SetVertexCtxCmd& operator=(const SetVertexCtxCmd&) = default;

private:
    using PayloadBuffer = std::array<VertexCtx, 1>;
    PayloadBuffer m_buffer;
    PayloadType m_desc;
};

} // namespace rr

#endif // _SET_VERTEX_CTX_CMD_HPP_
