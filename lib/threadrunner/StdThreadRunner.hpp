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

#ifndef STDTHREADRUNNER_HPP
#define STDTHREADRUNNER_HPP

#include "IThreadRunner.hpp"
#include <condition_variable>
#include <mutex>
#include <thread>

namespace rr
{

class StdThreadRunner : public IThreadRunner
{
public:
    StdThreadRunner()
    {
        m_renderThread = std::thread([this]()
            { threadMain(); });
#ifdef WIN32
        SetThreadPriority(m_renderThread.native_handle(), 2);
#else
        sched_param sch_params;
        sch_params.sched_priority = 2;
        pthread_setschedparam(m_renderThread.native_handle(), SCHED_RR, &sch_params);
#endif
    }

    ~StdThreadRunner()
    {
        {
            std::lock_guard<std::mutex> lock { m_mutex };
            m_stop = true;
        }
        m_workAvailable.notify_one();
        if (m_renderThread.joinable())
        {
            m_renderThread.join();
        }
    }

    void wait() override
    {
        std::unique_lock<std::mutex> lock { m_mutex };
        m_workCompleted.wait(lock, [this]()
            { return !m_busy; });
    }

    bool isBusy() const override
    {
        std::lock_guard<std::mutex> lock { m_mutex };
        return m_busy;
    }

    void run(const std::function<void()>& operation) override
    {
        {
            std::unique_lock<std::mutex> lock { m_mutex };
            m_workCompleted.wait(lock, [this]()
                { return !m_busy; });
    
            m_operation = operation;
            m_busy = true;
        }
        m_workAvailable.notify_one();
    }

private:
    void threadMain()
    {
        for (;;)
        {
            std::function<void()> operation;
            {
                std::unique_lock<std::mutex> lock { m_mutex };
                m_workAvailable.wait(lock, [this]()
                    { return m_stop || m_busy; });
                if (m_stop)
                {
                    return;
                }
                operation = std::move(m_operation);
            }

            operation();

            {
                std::lock_guard<std::mutex> lock { m_mutex };
                m_busy = false;
            }
            m_workCompleted.notify_all();
        }
    }

    std::thread m_renderThread;
    mutable std::mutex m_mutex;
    std::condition_variable m_workAvailable;
    std::condition_variable m_workCompleted;
    std::function<void()> m_operation;
    bool m_busy { false };
    bool m_stop { false };
};

} // namespace rr

#endif // STDTHREADRUNNER_HPP
