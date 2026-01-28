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

#ifndef _DEVICEDATAUPLOADER_HPP_
#define _DEVICEDATAUPLOADER_HPP_

#include "DeviceDataUploaderCommands.hpp"
#include "IBusConnector.hpp"
#include "renderer/IDevice.hpp"

namespace rr::devicedatauploader
{

/// @brief Software-side implementation of the Frame Transfer Protocol.
/// @details This class communicates with the RTL module FrameStreamingCore (FrameStreamingCore.v).
///          It handles streaming of display lists, and reading/writing device memory.
class DeviceDataUploader : public IDevice
{
public:
    DeviceDataUploader(IBusConnector& busConnector)
        : m_busConnector { busConnector }
    {
    }

    void deinit() { }

    void streamDisplayList(const uint8_t index, uint32_t size) override;
    bool writeToDeviceMemory(tcb::span<const uint8_t> data, const uint32_t addr) override;
    bool readFromDeviceMemory(tcb::span<uint8_t> data, const uint32_t addr) override;

    void blockUntilDeviceIsIdle() override { m_busConnector.blockUntilTransferIsComplete(); }

    tcb::span<uint8_t> requestDisplayListBuffer(const uint8_t index) override
    {
        tcb::span<uint8_t> s = m_busConnector.requestWriteBuffer(index);
        return s.last(s.size() - sizeof(Command));
    }

    uint8_t getDisplayListBufferCount() const override
    {
        return m_busConnector.getWriteBufferCount() - 1;
    }

private:
    bool hasLoadBuffer() const
    {
        return m_busConnector.getReadBufferCount() != 0;
    }

    uint32_t getLoadBufferIndex() const
    {
        return m_busConnector.getReadBufferCount() - 1;
    }

    uint32_t getStoreBufferIndex() const
    {
        return m_busConnector.getWriteBufferCount() - 1;
    }

    uint32_t addDduStreamCommand(const uint8_t index, const uint32_t size)
    {
        return addDduCommand(index, OP_STREAM, size, 0);
    }

    uint32_t addDduStoreCommand(const uint32_t size, const uint32_t addr)
    {
        return addDduCommand(getStoreBufferIndex(), OP_STORE, size, addr);
    }

    uint32_t addDduLoadCommand(const uint32_t size, const uint32_t addr) { return addDduCommand(getStoreBufferIndex(), OP_LOAD, size, addr); }
    uint32_t addDduStorePayload(const std::size_t offset, const tcb::span<const uint8_t> payload);
    uint32_t addDduCommand(
        const uint8_t index,
        const uint32_t op,
        const uint32_t size,
        const uint32_t addr);
    uint32_t fillWhenDataIsTooSmall(const uint8_t index, const uint32_t size);
    void loadChunk(const tcb::span<uint8_t> data, const std::size_t alignedSize, const uint32_t addr);

    IBusConnector& m_busConnector;
};

} // namespace rr::devicedatauploader

#endif // _DEVICEDATAUPLOADER_HPP_
