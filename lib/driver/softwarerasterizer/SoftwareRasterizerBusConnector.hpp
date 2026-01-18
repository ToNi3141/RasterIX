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

#ifndef SOFTWARERASTERIZERBUSCONNECTOR_H
#define SOFTWARERASTERIZERBUSCONNECTOR_H

#include "GenericMemoryBusConnector.hpp"
#include <tcb/span.hpp>

namespace rr
{
template <uint32_t NUMBER_OF_DISPLAY_LISTS = 2, uint32_t DISPLAY_LIST_SIZE = 32 * 1024 * 1024, bool RGB565 = false>
class SoftwareRasterizerBusConnector : public GenericMemoryBusConnector<NUMBER_OF_DISPLAY_LISTS, DISPLAY_LIST_SIZE>
{
public:
    virtual ~SoftwareRasterizerBusConnector() = default;

    SoftwareRasterizerBusConnector(tcb::span<uint8_t> framebuffer)
        : m_framebuffer { framebuffer }
    {
    }

    virtual void writeData(const uint8_t index, const uint32_t size, const uint32_t offset) override
    {
        tcb::span<uint8_t> deviceMemory = this->requestWriteBuffer(index);
        tcb::span<const uint8_t> devicefb = tcb::span<const uint8_t> { reinterpret_cast<const uint8_t*>(deviceMemory.data() + offset), size };

        if constexpr (RGB565)
        {
            copyRgb565ToFramebuffer(devicefb);
        }
        else
        {
            copyRgba8888ToFramebuffer(devicefb);
        }
    }

    virtual void readData(const uint8_t index, const uint32_t size) override
    {
    }

    virtual void blockUntilTransferIsComplete() override
    {
    }

private:
    void copyRgb565ToFramebuffer(const tcb::span<uint8_t>& devicefb)
    {
        std::size_t fbSize = std::min(devicefb.size(), m_framebuffer.size());
        for (std::size_t i = 0; i < fbSize; i++)
        {
            m_framebuffer[i] = devicefb[i];
        }
    }

    void copyRgba8888ToFramebuffer(const tcb::span<const uint8_t>& devicefb)
    {
        std::size_t fbSize = std::min(devicefb.size(), m_framebuffer.size());
        tcb::span<const uint16_t> fbAs16Bit { reinterpret_cast<const uint16_t*>(devicefb.data()), devicefb.size() / 2 };
        for (std::size_t i = 0; i < fbAs16Bit.size(); i++)
        {
            const uint32_t r = (fbAs16Bit[i] >> 11) & 0x1F;
            const uint32_t g = (fbAs16Bit[i] >> 5) & 0x3F;
            const uint32_t b = (fbAs16Bit[i] >> 0) & 0x1F;
            m_framebuffer[i * 4 + 0] = (r << 3) | (r >> 2);
            m_framebuffer[i * 4 + 1] = (g << 2) | (g >> 4);
            m_framebuffer[i * 4 + 2] = (b << 3) | (b >> 2);
            m_framebuffer[i * 4 + 3] = 0xFF;
        }
    }

    tcb::span<uint8_t> m_framebuffer {};
};

} // namespace rr
#endif // SOFTWARERASTERIZERBUSCONNECTOR_H
