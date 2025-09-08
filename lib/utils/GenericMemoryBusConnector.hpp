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

#ifndef GENERICMEMORYBUSCONNECTOR_HPP
#define GENERICMEMORYBUSCONNECTOR_HPP

#include "IBusConnector.hpp"
#include <array>
#include <cstdint>
#include <tcb/span.hpp>

namespace rr
{
template <uint32_t NUMBER_OF_DISPLAY_LISTS, uint32_t DISPLAY_LIST_SIZE>
class GenericMemoryBusConnector : public IBusConnector
{
public:
    virtual ~GenericMemoryBusConnector() = default;

    virtual tcb::span<uint8_t> requestWriteBuffer(const uint8_t index) override { return { m_dlMemTx[index] }; }
    virtual tcb::span<uint8_t> requestReadBuffer(const uint8_t index) override { return { m_dlMemRx[index] }; }
    virtual uint8_t getWriteBufferCount() const override { return m_dlMemTx.size(); }
    virtual uint8_t getReadBufferCount() const override { return m_dlMemRx.size(); }

protected:
    std::array<std::array<uint8_t, DISPLAY_LIST_SIZE>, 1> m_dlMemRx;
    std::array<std::array<uint8_t, DISPLAY_LIST_SIZE>, NUMBER_OF_DISPLAY_LISTS> m_dlMemTx;
};

} // namespace rr
#endif // #ifndef GENERICMEMORYBUSCONNECTOR_HPP
