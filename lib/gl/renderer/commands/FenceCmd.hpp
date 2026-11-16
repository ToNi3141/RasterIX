// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2026 ToNi3141

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

#ifndef _FENCE_CMD_HPP_
#define _FENCE_CMD_HPP_

#include "Op.hpp"
#include "renderer/displaylist/DisplayList.hpp"
#include <array>
#include <cstdint>
#include <tcb/span.hpp>

namespace rr
{

class FenceCmd
{
public:
    using PayloadType = tcb::span<const uint8_t>;
    using CommandType = uint32_t;
    using SerializedPayloadType = uint8_t;
    FenceCmd() = default;
    FenceCmd(const CommandType, const PayloadType&, const bool) { }

    const PayloadType& payload() const { return m_payload; }
    CommandType command() const { return op::FENCE; }
    static constexpr std::size_t getOpSize() { return displaylist::DisplayList::template sizeOf<CommandType>(); }
    static constexpr std::size_t getCommandMaxSize() { return getOpSize() + getMaxPayloadSize(); }
    static constexpr std::size_t getCommandSize() { return getCommandMaxSize(); }
    static constexpr std::size_t getPayloadSize() { return getMaxPayloadSize(); }
    static constexpr std::size_t getNumberOfPayloadElements() { return 0; }
    static constexpr std::size_t getMaxPayloadSize() { return 0; }
    bool serializePayload(SerializedPayloadType&, const std::size_t) const { return true; }
    static std::size_t getNumberOfElementsInPayloadByCommand(const uint32_t) { return 0; }
    static bool isThis(const CommandType cmd) { return (cmd & op::MASK) == op::FENCE; }

private:
    PayloadType m_payload {};
};

} // namespace rr

#endif // _FENCE_CMD_HPP_
