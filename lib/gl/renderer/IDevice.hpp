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

#ifndef _IDEVICE_HPP_
#define _IDEVICE_HPP_

#include <cstdint>
#include <tcb/span.hpp>

namespace rr
{

class IDevice
{
public:
    virtual ~IDevice() = default;
    virtual void streamDisplayList(const uint8_t index, const uint32_t size) = 0;
    virtual void writeToDeviceMemory(tcb::span<const uint8_t> data, const uint32_t addr) = 0;
    virtual void waitTillDeviceIsIdle() = 0;
    virtual tcb::span<uint8_t> requestDisplayListBuffer(const uint8_t index) = 0;
    virtual uint8_t getDisplayListBufferCount() const = 0;
};

} // namespace rr

#endif // _IDEVICE_HPP_
