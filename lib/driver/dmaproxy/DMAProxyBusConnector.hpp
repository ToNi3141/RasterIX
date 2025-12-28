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

#ifndef DMAPROXYBUSCONNECTOR_HPP
#define DMAPROXYBUSCONNECTOR_HPP

#include "IBusConnector.hpp"
#include <atomic>

struct channel_buffer;
namespace rr
{
class DMAProxyBusConnector : public IBusConnector
{
public:
    virtual ~DMAProxyBusConnector();

    DMAProxyBusConnector();

    virtual void writeData(const uint8_t index, const uint32_t size, const uint32_t offset) override;
    virtual void readData(const uint8_t index, const uint32_t size) override;
    virtual void blockUntilTransferIsComplete() override;
    virtual tcb::span<uint8_t> requestWriteBuffer(const uint8_t index) override;
    virtual tcb::span<uint8_t> requestReadBuffer(const uint8_t index) override;
    virtual uint8_t getWriteBufferCount() const override;
    virtual uint8_t getReadBufferCount() const override;

private:
    struct Channel
    {
        struct channel_buffer* buf_ptr;
        int fd;
    };

    void waitForDma();
    void openChannel(Channel& channel, const char* channelName[]);

    static const std::size_t INVALID_BUFFER;
    Channel m_txChannel;
    Channel m_rxChannel;

    std::size_t m_busyBufferId { INVALID_BUFFER };
    Channel m_busyChannel {};
};

} // namespace rr
#endif // #ifndef DMAPROXYBUSCONNECTOR_HPP