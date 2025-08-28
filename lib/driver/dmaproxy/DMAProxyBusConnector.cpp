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
    // const char *rx_channel_names[] = { "dma_proxy_rx" };

    char channel_name[64] = "/dev/";
    strcat(channel_name, tx_channel_names[0]);
    m_txChannel.fd = open(channel_name, O_RDWR);
    if (m_txChannel.fd < 1)
    {
        SPDLOG_ERROR("Unable to open DMA proxy device file: {}", channel_name);
        exit(EXIT_FAILURE);
    }
    m_txChannel.buf_ptr = reinterpret_cast<struct channel_buffer*>(mmap(NULL, sizeof(channel_buffer) * TX_BUFFER_COUNT,
        PROT_READ | PROT_WRITE, MAP_SHARED, m_txChannel.fd, 0));
    if (m_txChannel.buf_ptr == MAP_FAILED)
    {
        printf("Failed to mmap tx channel\n");
        SPDLOG_ERROR("Failed to mmap tx channel");
        exit(EXIT_FAILURE);
    }
}

DMAProxyBusConnector::~DMAProxyBusConnector()
{
    blockUntilWriteComplete();
}

void DMAProxyBusConnector::writeData(const uint8_t index, const uint32_t size)
{
    if (index >= BUFFER_COUNT)
    {
        SPDLOG_ERROR("Index {} out of bounds.", index);
        return;
    }
    blockUntilWriteComplete();
    int buffer_id = index;
    m_txChannel.buf_ptr[buffer_id].length = size;
    ioctl(m_txChannel.fd, START_XFER, &buffer_id);
    if (m_txChannel.buf_ptr[buffer_id].status != channel_buffer::proxy_status::PROXY_NO_ERROR)
    {
        SPDLOG_ERROR("Proxy tx transfer error");
    }
    m_busyBufferId = index;
}

void DMAProxyBusConnector::readData(const uint8_t index, const uint32_t size)
{
    // TODO: Implement
}

void DMAProxyBusConnector::blockUntilWriteComplete()
{
    waitForDma();
}

tcb::span<uint8_t> DMAProxyBusConnector::requestBuffer(const uint8_t index)
{
    if (index >= BUFFER_COUNT)
    {
        SPDLOG_ERROR("Index {} out of bounds.", index);
        return {};
    }
    SPDLOG_DEBUG("Requested memory for index {} with size {}", index, BUFFER_SIZE);
    return { reinterpret_cast<uint8_t*>(&m_txChannel.buf_ptr[index].buffer[0]), BUFFER_SIZE };
}

uint8_t DMAProxyBusConnector::getBufferCount() const
{
    return BUFFER_COUNT;
}

void DMAProxyBusConnector::waitForDma()
{
    if (m_busyBufferId == INVALID_BUFFER)
        return;
    ioctl(m_txChannel.fd, FINISH_XFER, &m_busyBufferId);
    m_busyBufferId = INVALID_BUFFER;
}

} // namespace rr