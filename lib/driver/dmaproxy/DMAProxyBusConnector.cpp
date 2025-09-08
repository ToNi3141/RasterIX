#include "DMAProxyBusConnector.hpp"
#include "kernel/dma-proxy/files/include/dma-proxy.h"

#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

namespace rr
{

const std::size_t DMAProxyBusConnector::INVALID_BUFFER { BUFFER_SIZE + 1 };

DMAProxyBusConnector::DMAProxyBusConnector()
{
    const char* tx_channel_names[] = { "dma_proxy_tx" };
    const char* rx_channel_names[] = { "dma_proxy_rx" };

    openChannel(m_txChannel, tx_channel_names);
    openChannel(m_rxChannel, rx_channel_names);
}

void DMAProxyBusConnector::openChannel(Channel& channel, const char* channelName[])
{
    char channel_name[64] = "/dev/";
    strcat(channel_name, channelName[0]);
    channel.fd = open(channel_name, O_RDWR);
    if (channel.fd < 1)
    {
        SPDLOG_ERROR("Unable to open DMA proxy device file: {}", channel_name);
        exit(EXIT_FAILURE);
    }
    channel.buf_ptr = reinterpret_cast<struct channel_buffer*>(mmap(NULL, sizeof(channel_buffer) * BUFFER_COUNT,
        PROT_READ | PROT_WRITE, MAP_SHARED, channel.fd, 0));
    if (channel.buf_ptr == MAP_FAILED)
    {
        SPDLOG_ERROR("Failed to mmap channel");
        exit(EXIT_FAILURE);
    }
}

DMAProxyBusConnector::~DMAProxyBusConnector()
{
    blockUntilTransferIsComplete();
}

void DMAProxyBusConnector::writeData(const uint8_t index, const uint32_t size)
{
    if (index >= TX_BUFFER_COUNT)
    {
        SPDLOG_ERROR("Index {} out of bounds.", index);
        return;
    }
    blockUntilTransferIsComplete();
    int buffer_id = index;
    m_txChannel.buf_ptr[buffer_id].length = size;
    ioctl(m_txChannel.fd, START_XFER, &buffer_id);
    if (m_txChannel.buf_ptr[buffer_id].status != channel_buffer::proxy_status::PROXY_NO_ERROR)
    {
        SPDLOG_ERROR("Proxy tx transfer error");
    }
    m_busyBufferId = index;
    m_busyChannel = m_txChannel;
}

void DMAProxyBusConnector::readData(const uint8_t index, const uint32_t size)
{
    if (index >= RX_BUFFER_COUNT)
    {
        SPDLOG_ERROR("Index {} out of bounds.", index);
        return;
    }
    blockUntilTransferIsComplete();
    int buffer_id = index;
    m_rxChannel.buf_ptr[buffer_id].length = size;
    ioctl(m_rxChannel.fd, START_XFER, &buffer_id);
    if (m_rxChannel.buf_ptr[buffer_id].status != channel_buffer::proxy_status::PROXY_NO_ERROR)
    {
        SPDLOG_ERROR("Proxy rx transfer error 0x{:X}", m_rxChannel.buf_ptr[buffer_id].status);
        return;
    }
    blockUntilTransferIsComplete();
    m_busyBufferId = index;
    m_busyChannel = m_rxChannel;
}

void DMAProxyBusConnector::blockUntilTransferIsComplete()
{
    waitForDma();
}

tcb::span<uint8_t> DMAProxyBusConnector::requestWriteBuffer(const uint8_t index)
{
    if (index >= TX_BUFFER_COUNT)
    {
        SPDLOG_ERROR("Index {} out of bounds.", index);
        return {};
    }
    SPDLOG_DEBUG("Requested memory for index {} with size {}", index, BUFFER_SIZE);
    return { reinterpret_cast<uint8_t*>(&m_txChannel.buf_ptr[index].buffer[0]), BUFFER_SIZE };
}

tcb::span<uint8_t> DMAProxyBusConnector::requestReadBuffer(const uint8_t index)
{
    if (index >= RX_BUFFER_COUNT)
    {
        SPDLOG_ERROR("Index {} out of bounds.", index);
        return {};
    }
    SPDLOG_DEBUG("Requested memory for index {} with size {}", index, BUFFER_SIZE);
    return { reinterpret_cast<uint8_t*>(&m_rxChannel.buf_ptr[index].buffer[0]), BUFFER_SIZE };
}

uint8_t DMAProxyBusConnector::getWriteBufferCount() const
{
    return TX_BUFFER_COUNT;
}

uint8_t DMAProxyBusConnector::getReadBufferCount() const
{
    return RX_BUFFER_COUNT;
}

void DMAProxyBusConnector::waitForDma()
{
    if (m_busyBufferId == INVALID_BUFFER)
        return;
    ioctl(m_busyChannel.fd, FINISH_XFER, &m_busyBufferId);
    if (m_busyChannel.buf_ptr[m_busyBufferId].status != channel_buffer::proxy_status::PROXY_NO_ERROR)
    {
        SPDLOG_ERROR("wait for dma error 0x{:X}", m_busyChannel.buf_ptr[m_busyBufferId].status);
    }
    m_busyBufferId = INVALID_BUFFER;
}

} // namespace rr