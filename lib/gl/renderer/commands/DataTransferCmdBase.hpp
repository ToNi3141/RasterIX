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

#ifndef _DATA_TO_TRANSFER_CMD_BASE_HPP_
#define _DATA_TO_TRANSFER_CMD_BASE_HPP_

#include "Op.hpp"
#include "RenderConfigs.hpp"
#include "math/Vec.hpp"
#include <array>
#include <cstdint>
#include <tcb/span.hpp>
#include <type_traits>
#include <typeinfo>

namespace rr
{

template <typename TDataToTransfer, std::size_t TOP>
class DataTransferCmdBase
{
public:
    using PayloadType = tcb::span<const TDataToTransfer>;
    using CommandType = uint32_t;

    DataTransferCmdBase() = default;
    DataTransferCmdBase(const TDataToTransfer& data)
    {
        m_buffer[0] = data;
        m_desc = { m_buffer };
    }

    DataTransferCmdBase(const CommandType, const PayloadType& payload, const bool)
    {
        m_desc = payload;
    }

    PayloadType payload() const { return m_desc; }
    static constexpr CommandType command() { return TOP | (displaylist::DisplayList::template sizeOf<TDataToTransfer>()); }

    static std::size_t getNumberOfElementsInPayloadByCommand(const uint32_t) { return std::tuple_size<PayloadBuffer> {}; }
    static bool isThis(const CommandType cmd) { return (cmd & op::MASK) == TOP; }

    DataTransferCmdBase& operator=(const DataTransferCmdBase&) = default;

private:
    using PayloadBuffer = std::array<TDataToTransfer, 1>;
    PayloadBuffer m_buffer;
    PayloadType m_desc;
};

} // namespace rr

#endif // _DATA_TO_TRANSFER_CMD_BASE_HPP_
