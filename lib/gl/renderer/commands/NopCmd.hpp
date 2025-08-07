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

#ifndef _NOP_CMD_HPP_
#define _NOP_CMD_HPP_

#include "Op.hpp"
#include <array>
#include <cstdint>
#include <tcb/span.hpp>

namespace rr
{

class NopCmd
{
public:
    using PayloadType = tcb::span<const uint8_t>;
    using CommandType = uint32_t;
    NopCmd() = default;
    NopCmd(const CommandType, const PayloadType&, const bool) { }

    const PayloadType& payload() const { return m_payload; }
    CommandType command() const { return op::NOP; }
    static std::size_t getNumberOfElementsInPayloadByCommand(const uint32_t) { return 0; }
    static bool isThis(const CommandType cmd) { return (cmd & op::MASK) == op::NOP; }

private:
    PayloadType m_payload {};
};

} // namespace rr

#endif // _NOP_CMD_HPP_
