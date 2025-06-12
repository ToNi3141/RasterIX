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

    /// @brief Streams a display list to the device.
    ///
    /// @param index The index of the display list in the device's memory.
    /// @param size The size of the display list in bytes.
    virtual void streamDisplayList(const uint8_t index, const uint32_t size) = 0;

    /// @brief Writes data to a specific address in the device's memory.
    ///
    /// @param data The data to write.
    /// @param addr The address to write to.
    virtual void writeToDeviceMemory(tcb::span<const uint8_t> data, const uint32_t addr) = 0;

    /// @brief Waits until the device is idle and ready for new commands.
    ///     When this method returns, the buffer used in streamDisplayList can be safely reused.
    ///     Same is true for the buffer in writeToDeviceMemory.
    virtual void blockUntilDeviceIsIdle() = 0;

    /// @brief Requests a buffer to write display lists into.
    ///
    /// @param index The index of the buffer to request.
    virtual tcb::span<uint8_t> requestDisplayListBuffer(const uint8_t index) = 0;

    /// @brief Gets the number of display list buffers available on this device.
    virtual uint8_t getDisplayListBufferCount() const = 0;
};

} // namespace rr

#endif // _IDEVICE_HPP_
