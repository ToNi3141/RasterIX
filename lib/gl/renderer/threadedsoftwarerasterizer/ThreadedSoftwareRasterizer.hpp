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
#include "renderer/IDevice.hpp"
#include "renderer/softwarerasterizer/SoftwareRasterizer.hpp"
#include <array>
#include <atomic>
#include <spdlog/spdlog.h>
#include <tcb/span.hpp>

namespace rr::threadedsoftwarerasterizer
{

template <std::size_t NUMBER_OF_THREADS = 1>
class ThreadedSoftwareRasterizer : public IDevice
{
public:
    ThreadedSoftwareRasterizer(IBusConnector& busConnector, std::array<IThreadRunner*, NUMBER_OF_THREADS>& workerThreads)
        : m_busConnector { busConnector }
        , m_workerThreads { workerThreads }
    {
        SPDLOG_INFO("Threaded Software Rasterizer initialized with {} threads", NUMBER_OF_THREADS);
        for (auto& a : m_softwareRasterizers)
        {
            a.setSwapFramebufferEventHandler([this](const softwarerasterizer::SoftwareRasterizer* sender)
                { waitExceptForOneThread(); });
        }
    }

    void deinit()
    {
        waitForAllThreads();
    }

    void streamDisplayList(const uint8_t index, const uint32_t size) override
    {
        waitTillThreadIsFree();
        drawInThread(m_threadIndex, index, size);
    }

    bool writeToDeviceMemory(tcb::span<const uint8_t> data, const uint32_t addr) override
    {
        return m_softwareRasterizers[0].writeToDeviceMemory(data, addr);
    }

    bool readFromDeviceMemory(tcb::span<uint8_t> data, const uint32_t addr) override
    {
        return m_softwareRasterizers[0].readFromDeviceMemory(data, addr);
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

    void waitTillThreadIsFree()
    {
        if (m_threadsRunning >= NUMBER_OF_THREADS)
        {
            m_workerThreads[m_threadIndex]->wait();
        }
    }

    void drawInThread(const std::size_t threadIndex, const uint8_t displayListIndex, const uint32_t displayListSize)
    {
        std::function<void()> drawFunction = [this, threadIndex, displayListIndex, displayListSize]()
        {
            m_softwareRasterizers[threadIndex].streamExternalDisplayList(tcb::span<const uint8_t> { m_buffer[displayListIndex].data(), displayListSize });
            m_threadsRunning--;
        };
        m_workerThreads[m_threadIndex]->run(drawFunction);
        m_threadIndex = (m_threadIndex + 1) % NUMBER_OF_THREADS;
        m_threadsRunning++;
    }

    void waitExceptForOneThread()
    {
        while (m_threadsRunning != 1)
        {
        }
    }

    IBusConnector& m_busConnector;
    std::array<std::array<uint8_t, RenderConfig::THREADED_RASTERIZATION_DISPLAY_LIST_BUFFER_SIZE>, RenderConfig::getDisplayLines() * 2> m_buffer;
    std::array<IThreadRunner*, NUMBER_OF_THREADS>& m_workerThreads;
    std::array<softwarerasterizer::SoftwareRasterizer, NUMBER_OF_THREADS> m_softwareRasterizers {
        softwarerasterizer::SoftwareRasterizer { m_busConnector },
        softwarerasterizer::SoftwareRasterizer { m_busConnector },
    };
    std::atomic_size_t m_threadIndex { 0 };
    std::atomic_size_t m_threadsRunning { 0 };
};

} // namespace rr::threadedsoftwarerasterizer

#endif // _THREADED_SOFTWARE_RASTERIZER_HPP_
