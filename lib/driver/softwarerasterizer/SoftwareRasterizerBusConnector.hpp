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
template <uint32_t NUMBER_OF_DISPLAY_LISTS = 2, uint32_t DISPLAY_LIST_SIZE = 32 * 1024 * 1024>
class SoftwareRasterizerBusConnector : public GenericMemoryBusConnector<NUMBER_OF_DISPLAY_LISTS, DISPLAY_LIST_SIZE>
{
public:
    virtual ~SoftwareRasterizerBusConnector() = default;

    SoftwareRasterizerBusConnector(tcb::span<uint16_t> framebuffer, const uint16_t resolutionW = 128, const uint16_t resolutionH = 128)
        : m_resolutionW { resolutionW }
        , m_resolutionH { resolutionH }
        , m_framebuffer { framebuffer }
    {
    }

    virtual void writeData(const uint8_t index, const uint32_t size, const uint32_t offset) override
    {
        tcb::span<uint8_t> buffer8 = this->requestWriteBuffer(index);
        tcb::span<uint16_t> buffer16 = tcb::span<uint16_t>(reinterpret_cast<uint16_t*>(buffer8.subspan(offset).data()), size / sizeof(uint16_t));

        std::size_t fbSize = std::min(buffer16.size(), m_framebuffer.size());
        for (std::size_t i = 0; i < fbSize; i++)
        {
            m_framebuffer[i] = buffer16[i];
        }
    }

    virtual void readData(const uint8_t index, const uint32_t size) override
    {
    }

    virtual void blockUntilTransferIsComplete() override
    {
    }

private:
    const uint16_t m_resolutionW = 128;
    const uint16_t m_resolutionH = 128;
    tcb::span<uint16_t> m_framebuffer {};
};

} // namespace rr
#endif // SOFTWARERASTERIZERBUSCONNECTOR_H
