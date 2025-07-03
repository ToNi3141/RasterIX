#ifndef DEVICE_UPLOAD_LIST_HPP
#define DEVICE_UPLOAD_LIST_HPP

#include "renderer/displaylist/DisplayList.hpp"
#include <array>
#include <cstdint>
#include <spdlog/spdlog.h>
#include <tcb/span.hpp>

namespace rr
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

} // namespace rr
#endif // DEVICE_UPLOAD_LIST_HPP