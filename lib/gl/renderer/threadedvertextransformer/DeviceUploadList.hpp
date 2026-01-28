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

#ifndef DEVICE_UPLOAD_LIST_HPP
#define DEVICE_UPLOAD_LIST_HPP

#include "renderer/displaylist/DisplayList.hpp"
#include <array>
#include <cstdint>
#include <spdlog/spdlog.h>
#include <tcb/span.hpp>

namespace rr::threadedvertextransformer
{

template <std::size_t BUFFER_SIZE, std::size_t PAGE_SIZE>
class DeviceUploadList
{
public:
    struct Page
    {
        uint32_t addr;
        uint8_t data[PAGE_SIZE];
    };

    DeviceUploadList()
    {
        m_pageList.setBuffer(tcb::span<uint8_t>(m_buffer.data(), m_buffer.size()));
    }

    bool addPage(tcb::span<const uint8_t> pageData, uint32_t addr)
    {
        if (pageData.size() > PAGE_SIZE)
        {
            SPDLOG_ERROR("Page data size exceeds PAGE_SIZE limit");
            return false;
        }

        Page* page = m_pageList.create<Page>();
        if (page)
        {
            page->addr = addr;
            std::copy(pageData.begin(), pageData.end(), page->data);
            return true;
        }
        SPDLOG_ERROR("Failed to allocate page in display list");
        return false;
    }

    const Page* getNextPage()
    {
        if (m_pageList.atEnd() == false)
        {
            return m_pageList.getNext<Page>();
        }
        return nullptr;
    }

    bool atEnd() const
    {
        return m_pageList.atEnd();
    }

    bool empty() const
    {
        return m_pageList.getSize() == 0;
    }

    void clear()
    {
        m_pageList.resetGet();
        m_pageList.clear();
    }

private:
    displaylist::DisplayList m_pageList {};
    std::array<uint8_t, BUFFER_SIZE> m_buffer {};
};

} // namespace rr::threadedvertextransformer
#endif // DEVICE_UPLOAD_LIST_HPP