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

#ifndef VERTEXBUFFER_HPP_
#define VERTEXBUFFER_HPP_

#include "Enums.hpp"
#include "RenderConfigs.hpp"
#include <array>
#include <memory>
#include <optional>
#include <tcb/span.hpp>

namespace rr
{
class VertexBuffer
{
public:
    std::size_t generateBuffer()
    {
        // Note: 0 has a special meaning
        for (std::size_t i = 1; i < m_buffers.size(); i++)
        {
            if (!m_buffers[i].has_value())
            {
                m_buffers[i] = BufferEntry {};
                return i;
            }
        }
        return InvalidBuffer;
    }

    void deleteBuffer(const std::size_t buffer)
    {
        if (buffer < m_buffers.size())
        {
            delete m_buffers[buffer]->data();
            m_buffers[buffer].reset();
            if (m_boundBuffer == buffer)
            {
                m_boundBuffer = InvalidBuffer;
            }
        }
    }

    void bindBuffer(const std::size_t buffer)
    {
        if (buffer < m_buffers.size() && m_buffers[buffer].has_value())
        {
            m_boundBuffer = buffer;
        }
        else
        {
            m_boundBuffer = InvalidBuffer;
        }
    }

    bool isBufferActive() const { return m_boundBuffer != InvalidBuffer; }

    std::size_t getBoundBuffer() const { return m_boundBuffer; }

    bool isBuffer(const std::size_t buffer) const { return (buffer < m_buffers.size() && m_buffers[buffer].has_value()); }

    bool bufferData(tcb::span<const std::uint8_t> data)
    {
        if (m_boundBuffer < m_buffers.size() && m_buffers[m_boundBuffer].has_value())
        {
            auto& buf = m_buffers[m_boundBuffer];
            buf = BufferEntry { new std::uint8_t[data.size()], data.size() };
            if (buf->data() == nullptr)
            {
                buf = BufferEntry {};
                return false;
            }
            std::copy(data.begin(), data.begin() + data.size(), buf->begin());
        }
        return true;
    }

    bool bufferSubData(const std::size_t offset, tcb::span<const std::uint8_t> data)
    {
        if (m_boundBuffer < m_buffers.size() && m_buffers[m_boundBuffer].has_value())
        {
            auto& buf = m_buffers[m_boundBuffer];
            if (offset + data.size() <= buf->size())
            {
                std::copy(data.begin(), data.begin() + data.size(), buf->begin() + offset);
            }
            return true;
        }
        return false;
    }

    tcb::span<const std::uint8_t> getBufferData() const
    {
        if (m_boundBuffer < m_buffers.size() && m_buffers[m_boundBuffer].has_value())
        {
            return *(m_buffers[m_boundBuffer]);
        }
        return tcb::span<const std::uint8_t> {};
    }

private:
    static constexpr std::size_t InvalidBuffer { static_cast<std::size_t>(0) };
    using BufferEntry = tcb::span<std::uint8_t>;

    std::array<std::optional<BufferEntry>, RenderConfig::MAX_VBO_COUNT> m_buffers {};
    std::size_t m_boundBuffer { InvalidBuffer };
};

} // namespace rr
#endif // VERTEXBUFFER_HPP_
