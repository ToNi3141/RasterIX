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

#ifndef _DSE_BUSCONNECTOR_HPP_
#define _DSE_BUSCONNECTOR_HPP_

#include "DmaStreamEngineCommands.hpp"
#include "IBusConnector.hpp"
#include "RenderConfigs.hpp"
#include "renderer/IDevice.hpp"

namespace rr::DSEC
{

class DmaStreamEngine : public IDevice
{
public:
    DmaStreamEngine(IBusConnector& busConnector)
        : m_busConnector { busConnector }
    {
    }

    void deinit() { }

    void streamDisplayList(const uint8_t index, uint32_t size) override
    {
        blockUntilDeviceIsIdle();
        size = fillWhenDataIsTooSmall(index, size);
        const uint32_t commandSize = addDseStreamCommand(index, size);
        m_busConnector.writeData(index, size + commandSize);
    }

    bool writeToDeviceMemory(tcb::span<const uint8_t> data, const uint32_t addr) override
    {
        blockUntilDeviceIsIdle();
        const uint32_t commandSize = addDseStoreCommand(
            (std::max)(static_cast<uint32_t>(data.size()), DEVICE_MIN_TRANSFER_SIZE),
            addr + RenderConfig::GRAM_MEMORY_LOC);
        addDseStorePayload(commandSize, data);
        m_busConnector.writeData(getLoadStoreBufferIndex(), commandSize + data.size());
        return true;
    }

    bool readFromDeviceMemory(tcb::span<uint8_t> data, const uint32_t addr) override
    {
        // TODO: Handle transfers, which are bigger than the displaylist
        blockUntilDeviceIsIdle();
        const uint32_t commandSize = addDseLoadCommand(
            (std::max)(static_cast<uint32_t>(data.size()), DEVICE_MIN_TRANSFER_SIZE),
            addr + RenderConfig::GRAM_MEMORY_LOC);
        m_busConnector.writeData(getLoadStoreBufferIndex(), commandSize);

        m_busConnector.readData(getLoadStoreBufferIndex(), data.size());
        tcb::span<uint8_t> loadedData = m_busConnector.requestBuffer(getLoadStoreBufferIndex()).subspan(0, data.size());
        std::copy(loadedData.begin(), loadedData.end(), data.begin());
        return true;
    }

    void blockUntilDeviceIsIdle() override
    {
        m_busConnector.blockUntilWriteComplete();
    }

    tcb::span<uint8_t> requestDisplayListBuffer(const uint8_t index) override
    {
        tcb::span<uint8_t> s = m_busConnector.requestBuffer(index);
        return s.last(s.size() - sizeof(Command));
    }

    uint8_t getDisplayListBufferCount() const override
    {
        return m_busConnector.getBufferCount() - 1;
    }

private:
    uint32_t getLoadStoreBufferIndex() const
    {
        return m_busConnector.getBufferCount() - 1;
    }

    uint32_t addDseStreamCommand(const uint8_t index, const uint32_t size)
    {
        return addDseCommand(index, OP_STREAM, size, 0);
    }

    uint32_t addDseStoreCommand(const uint32_t size, const uint32_t addr)
    {
        return addDseCommand(getLoadStoreBufferIndex(), OP_STORE, size, addr);
    }

    uint32_t addDseLoadCommand(const uint32_t size, const uint32_t addr)
    {
        return addDseCommand(getLoadStoreBufferIndex(), OP_LOAD, size, addr);
    }

    uint32_t addDseStorePayload(const std::size_t offset, const tcb::span<const uint8_t> payload)
    {
        tcb::span<uint8_t> s = m_busConnector.requestBuffer(getLoadStoreBufferIndex());
        std::copy(payload.begin(), payload.end(), s.last(s.size() - offset).begin());
        return (std::max)(static_cast<uint32_t>(payload.size()), DEVICE_MIN_TRANSFER_SIZE);
    }

    uint32_t addDseCommand(
        const uint8_t index,
        const uint32_t op,
        const uint32_t size,
        const uint32_t addr)
    {
        tcb::span<uint8_t> s = m_busConnector.requestBuffer(index);
        Command* c = reinterpret_cast<Command*>(s.first<sizeof(Command)>().data());
        c->op = op | (IMM_MASK & size);
        c->addr = addr;
        return sizeof(Command);
    }

    uint32_t fillWhenDataIsTooSmall(const uint8_t index, const uint32_t size)
    {
        if (size < DEVICE_MIN_TRANSFER_SIZE)
        {
            tcb::span<uint8_t> buffer = requestDisplayListBuffer(index).subspan(size, DEVICE_MIN_TRANSFER_SIZE - size);
            std::fill(buffer.begin(), buffer.end(), 0);
        }
        return (std::max)(size, DEVICE_MIN_TRANSFER_SIZE);
    }

    IBusConnector& m_busConnector;
};

} // namespace rr::DSEC

#endif // _DSE_BUSCONNECTOR_HPP_
