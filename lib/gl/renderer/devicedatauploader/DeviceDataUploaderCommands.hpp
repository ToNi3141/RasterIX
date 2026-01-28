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

#ifndef _FRAME_TRANSFER_ENGINE_COMMANDS_HPP_
#define _FRAME_TRANSFER_ENGINE_COMMANDS_HPP_

#include <cstdint>
#include <tcb/span.hpp>

namespace rr::devicedatauploader
{

using SCT = uint32_t;

// OPs for the Frame Transfer Engine
// This software component communicates with the RTL module FrameStreamingCore.
// Refer FrameStreamingCore.v for more details. In short the anatomy of a command:
// Standard operation
// Beat 1:
// +----------+---------+-----------------------------+
// | 2'hx out | 2'hx in | 28'h stream length in bytes |
// +----------+---------+-----------------------------+
// Beat 2:
// +--------------------------------------------------+
// | 32'h addr (only evaluated when mem_axi is used)  |
// +--------------------------------------------------+
// Beat 3 .. n:
// +--------------------------------------------------+
// | STREAM_WIDTH'h payload                           |
// +--------------------------------------------------+

static constexpr SCT OP_MASK { 0xF000'0000 };
static constexpr SCT IMM_MASK { ~OP_MASK };
static constexpr SCT OP_NOP { 0x0000'0000 };
static constexpr SCT OP_STORE { 0xD000'0000 };
static constexpr SCT OP_LOAD { 0x7000'0000 };
static constexpr SCT OP_STREAM { 0x9000'0000 };

static constexpr uint32_t DEVICE_MIN_TRANSFER_SIZE { 64 }; // The FTE only supports 16 * 4 byte transfers

struct Command
{
#pragma pack(push, 4)
    SCT op;
    uint32_t addr;
#pragma pack(pop)
};

} // namespace rr::devicedatauploader

#endif // _FRAME_TRANSFER_ENGINE_COMMANDS_HPP_
