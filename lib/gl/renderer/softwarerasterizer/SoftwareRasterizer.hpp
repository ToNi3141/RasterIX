// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2026 ToNi3141

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

#ifndef _THREADED_SOFTWARE_RASTERIZER_HPP_
#define _THREADED_SOFTWARE_RASTERIZER_HPP_

#include "IBusConnector.hpp"
#include "IThreadRunner.hpp"
#include "RenderConfigs.hpp"
#include "SoftwareRasterizationInstance.hpp"
#include "renderer/IDevice.hpp"
#include <array>
#include <atomic>
#include <spdlog/spdlog.h>
#include <tcb/span.hpp>

namespace rr::softwarerasterizer
{

template <std::size_t NUMBER_OF_THREADS = 1>
class SoftwareRasterizer : public IDevice
{
public:
    SoftwareRasterizer(IBusConnector& busConnector, std::array<IThreadRunner*, NUMBER_OF_THREADS>& workerThreads)
        : m_busConnector { busConnector }
        , m_workerThreads { workerThreads }
    {
        m_gram = busConnector.requestWriteBuffer(0);
        SPDLOG_INFO("Software Rasterizer initialized with {} threads", NUMBER_OF_THREADS);
        for (auto& a : m_softwareRasterizers)
        {
            a.setSwapFramebufferEventHandler([this](const softwarerasterizer::SoftwareRasterizationInstance*)
                { waitExceptForOneThread(); });
        }
    }

    void deinit()
    {
        waitForAllThreads();
    }

    void streamDisplayList(const uint8_t index, const uint32_t size) override
    {
        const std::size_t threadIndex = findFreeThread();
        drawInThread(threadIndex, index, size);
    }

    bool writeToDeviceMemory(tcb::span<const uint8_t> data, const uint32_t addr) override
    {
        std::copy(data.begin(), data.end(), m_gram.data() + addr);
        return true;
    }

    bool readFromDeviceMemory(tcb::span<uint8_t> data, const uint32_t addr) override
    {
        std::copy(m_gram.data() + addr, m_gram.data() + addr + data.size(), data.begin());
        return true;
    }

    void blockUntilDeviceIsIdle() override
    {
        waitForAllThreads();
    }

    tcb::span<uint8_t> requestDisplayListBuffer(const uint8_t index) override
    {
        return { m_buffer[index] };
    }

    uint8_t getDisplayListBufferCount() const override
    {
        return m_buffer.size();
    }

private:
    void waitForAllThreads()
    {
        for (std::size_t i = 0; i < NUMBER_OF_THREADS; i++)
        {
            m_workerThreads[i]->wait();
        }
    }

    std::size_t findFreeThread()
    {
        // First, try to find a thread that is already free
        for (std::size_t i = 0; i < NUMBER_OF_THREADS; i++)
        {
            if (!m_workerThreads[i]->isBusy())
            {
                return i;
            }
        }
        // All threads are busy, wait for the next one in round-robin order
        m_workerThreads[m_nextThreadToWait]->wait();
        const std::size_t freeThread = m_nextThreadToWait;
        m_nextThreadToWait = (m_nextThreadToWait + 1) % NUMBER_OF_THREADS;
        return freeThread;
    }

    void drawInThread(const std::size_t threadIndex, const uint8_t displayListIndex, const uint32_t displayListSize)
    {
        std::function<void()> drawFunction = [this, threadIndex, displayListIndex, displayListSize]()
        {
            m_softwareRasterizers[threadIndex].streamDisplayList(tcb::span<const uint8_t> { m_buffer[displayListIndex].data(), displayListSize });
            m_threadsRunning--;
        };
        m_threadsRunning++;
        m_workerThreads[threadIndex]->run(drawFunction);
    }

    void waitExceptForOneThread()
    {
        while (m_threadsRunning != 1)
        {
        }
    }

    template <std::size_t... Is>
    std::array<softwarerasterizer::SoftwareRasterizationInstance, NUMBER_OF_THREADS>
    makeSoftwareRasterizers(std::index_sequence<Is...>)
    {
        return { { ((void)Is, softwarerasterizer::SoftwareRasterizationInstance { m_busConnector })... } };
    }

    IBusConnector& m_busConnector;
    tcb::span<uint8_t> m_gram {};
    std::array<std::array<uint8_t, RenderConfig::THREADED_RASTERIZATION_DISPLAY_LIST_BUFFER_SIZE>, RenderConfig::getDisplayLines() * 2> m_buffer;
    std::array<IThreadRunner*, NUMBER_OF_THREADS>& m_workerThreads;
    std::array<softwarerasterizer::SoftwareRasterizationInstance, NUMBER_OF_THREADS> m_softwareRasterizers {
        makeSoftwareRasterizers(std::make_index_sequence<NUMBER_OF_THREADS> {})
    };
    std::size_t m_nextThreadToWait { 0 };
    std::atomic_size_t m_threadsRunning { 0 };
};

} // namespace rr::softwarerasterizer

#endif // _THREADED_SOFTWARE_RASTERIZER_HPP_
