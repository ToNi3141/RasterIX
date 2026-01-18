// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

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

#ifndef IBUSCONNECTOR_HPP
#define IBUSCONNECTOR_HPP
#include <stdint.h>
#include <tcb/span.hpp>

namespace rr
{
class IBusConnector
{
public:
    virtual ~IBusConnector() = default;

    /// @brief Uploads a chunk of data
    /// @param index The index of the buffer to upload
    /// @param size How many bytes of this buffer to upload
    /// @param offset The offset in the buffer to start uploading from
    /// @note: A new transfer is started when the previous one is finished.
    ///     As long as the previous one is ongoing, this function blocks.
    virtual void writeData(const uint8_t index, const uint32_t size, const uint32_t offset = 0) = 0;

    /// @brief Downloads a chunk of data
    /// @param index The index of the buffer to download
    /// @param size How many bytes of this buffer to download
    /// @note: This function blocks until the read operation is done.
    virtual void readData(const uint8_t index, const uint32_t size) = 0;

    /// @brief Blocks until a new read()/write() can be called without blocking.
    ///     This also implies that the currently transferred buffer is free for reuse.
    virtual void blockUntilTransferIsComplete() = 0;

    /// @brief Requests a buffer which supports the requirements for the given device (for instance DMA capabilities).
    /// @param index The index of the requested buffer
    /// @return Returns the requested buffer, or an empty span if no buffer is available for the given index
    virtual tcb::span<uint8_t> requestWriteBuffer(const uint8_t index) = 0;

    /// @brief Requests a buffer which supports the requirements for the given device (for instance DMA capabilities).
    /// @param index The index of the requested buffer
    /// @return Returns the requested buffer, or an empty span if no buffer is available for the given index
    virtual tcb::span<uint8_t> requestReadBuffer(const uint8_t index) = 0;

    /// @brief Returns the number of buffers available to request
    /// @return The number of buffers which can be requested
    virtual uint8_t getWriteBufferCount() const = 0;

    /// @brief Returns the number of buffers available to request
    /// @return The number of buffers which can be requested
    virtual uint8_t getReadBufferCount() const = 0;
};

} // namespace rr
#endif // IBUSCONNECTOR_HPP
