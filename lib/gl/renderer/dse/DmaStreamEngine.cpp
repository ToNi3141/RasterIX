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

#include "DmaStreamEngine.hpp"
#include "RenderConfigs.hpp"
#include <algorithm>

namespace rr::dsec
{

void DmaStreamEngine::streamDisplayList(const uint8_t index, uint32_t size)
{
    blockUntilDeviceIsIdle();
    size = fillWhenDataIsTooSmall(index, size);
    const uint32_t commandSize = addDseStreamCommand(index, size);
    m_busConnector.writeData(index, size + commandSize);
}

bool DmaStreamEngine::writeToDeviceMemory(tcb::span<const uint8_t> data, const uint32_t addr)
{
    blockUntilDeviceIsIdle();
    const uint32_t commandSize = addDseStoreCommand(
        (std::max)(static_cast<uint32_t>(data.size()), DEVICE_MIN_TRANSFER_SIZE),
        addr + RenderConfig::GRAM_MEMORY_LOC);
    addDseStorePayload(commandSize, data);
    m_busConnector.writeData(getStoreBufferIndex(), commandSize + data.size());
    return true;
}

bool DmaStreamEngine::readFromDeviceMemory(tcb::span<uint8_t> data, const uint32_t addr)
{
    if (hasLoadBuffer())
    {
        blockUntilDeviceIsIdle();
        const std::size_t alignedSize = ((data.size() + DEVICE_MIN_TRANSFER_SIZE - 1) / DEVICE_MIN_TRANSFER_SIZE) * DEVICE_MIN_TRANSFER_SIZE;
        const std::size_t loadBufferSize = m_busConnector.requestReadBuffer(getLoadBufferIndex()).size();
        const std::size_t chunkSize = (loadBufferSize / DEVICE_MIN_TRANSFER_SIZE) * DEVICE_MIN_TRANSFER_SIZE;
        for (std::size_t i = 0; i < alignedSize; i += (std::min)(chunkSize, alignedSize - i))
        {
            loadChunk(
                data.subspan(i, (std::min)(data.size() - i, chunkSize)),
                (std::min)(alignedSize - i, chunkSize),
                addr + RenderConfig::GRAM_MEMORY_LOC + i);
        }
    }
    return true;
}

uint32_t DmaStreamEngine::addDseStorePayload(const std::size_t offset, const tcb::span<const uint8_t> payload)
{
    tcb::span<uint8_t> s = m_busConnector.requestWriteBuffer(getStoreBufferIndex());
    std::copy(payload.begin(), payload.end(), s.last(s.size() - offset).begin());
    return (std::max)(static_cast<uint32_t>(payload.size()), DEVICE_MIN_TRANSFER_SIZE);
}

uint32_t DmaStreamEngine::addDseCommand(
    const uint8_t index,
    const uint32_t op,
    const uint32_t size,
    const uint32_t addr)
{
    tcb::span<uint8_t> s = m_busConnector.requestWriteBuffer(index);
    Command* c = reinterpret_cast<Command*>(s.first<sizeof(Command)>().data());
    c->op = op | (IMM_MASK & size);
    c->addr = addr;
    return sizeof(Command);
}

uint32_t DmaStreamEngine::fillWhenDataIsTooSmall(const uint8_t index, const uint32_t size)
{
    if (size < DEVICE_MIN_TRANSFER_SIZE)
    {
        tcb::span<uint8_t> buffer = requestDisplayListBuffer(index).subspan(size, DEVICE_MIN_TRANSFER_SIZE - size);
        std::fill(buffer.begin(), buffer.end(), 0);
    }
    return (std::max)(size, DEVICE_MIN_TRANSFER_SIZE);
}

void DmaStreamEngine::loadChunk(const tcb::span<uint8_t> data, const std::size_t alignedSize, const uint32_t addr)
{
    const uint32_t commandSize = addDseLoadCommand(alignedSize, addr);
    m_busConnector.writeData(getStoreBufferIndex(), commandSize);
    blockUntilDeviceIsIdle();

    m_busConnector.readData(getLoadBufferIndex(), alignedSize);
    blockUntilDeviceIsIdle();

    tcb::span<uint8_t> loadedData = m_busConnector.requestReadBuffer(getLoadBufferIndex()).subspan(0, data.size());
    std::copy(loadedData.begin(), loadedData.end(), data.begin());
}

} // namespace rr::dsec
