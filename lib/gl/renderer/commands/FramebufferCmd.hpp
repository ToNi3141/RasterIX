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

#ifndef _FRAMEBUFFER_CMD_HPP_
#define _FRAMEBUFFER_CMD_HPP_

#include <array>
#include <cstdint>
#include <tcb/span.hpp>

namespace rr
{

class FramebufferCmd
{
    static constexpr uint32_t OP_FRAMEBUFFER { 0x2000'0000 };
    static constexpr uint32_t OP_FRAMEBUFFER_COMMIT { OP_FRAMEBUFFER | 0x0000'0001 };
    static constexpr uint32_t OP_FRAMEBUFFER_MEMSET { OP_FRAMEBUFFER | 0x0000'0002 };
    static constexpr uint32_t OP_FRAMEBUFFER_SWAP { OP_FRAMEBUFFER | 0x0000'0004 };
    static constexpr uint32_t OP_FRAMEBUFFER_COLOR_BUFFER_SELECT { OP_FRAMEBUFFER | 0x0000'0010 };
    static constexpr uint32_t OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT { OP_FRAMEBUFFER | 0x0000'0020 };
    static constexpr uint32_t OP_FRAMEBUFFER_STENCIL_BUFFER_SELECT { OP_FRAMEBUFFER | 0x0000'0040 };
    static constexpr uint32_t OP_FRAMEBUFFER_SWAP_ENABLE_VSYNC { OP_FRAMEBUFFER | 0x0000'0080 };
    static constexpr uint32_t OP_FRAMEBUFFER_SIZE_POS { 8 };
    static constexpr uint32_t OP_FRAMEBUFFER_SIZE_MASK { 0xFFFFF };
    static constexpr uint32_t OP_MASK { 0xF000'0000 };

public:
    using PayloadType = tcb::span<const uint8_t>;
    using CommandType = uint32_t;

    FramebufferCmd() = default;
    FramebufferCmd(const bool selColorBuffer,
        const bool selDepthBuffer,
        const bool selStencilBuffer,
        const std::size_t sizeInPixel)
    {
        if (selColorBuffer)
        {
            selectColorBuffer();
        }
        if (selDepthBuffer)
        {
            selectDepthBuffer();
        }
        if (selStencilBuffer)
        {
            selectStencilBuffer();
        }
        setFramebufferSizeInPixel(sizeInPixel);
    }

    FramebufferCmd(const CommandType& cmd)
        : m_op { cmd }
    {
    }

    void swapFramebuffer()
    {
        m_op |= OP_FRAMEBUFFER_SWAP;
    }
    void commitFramebuffer()
    {
        m_op |= OP_FRAMEBUFFER_COMMIT;
    }
    void enableMemset()
    {
        m_op |= OP_FRAMEBUFFER_MEMSET;
    }
    void selectColorBuffer()
    {
        m_op |= OP_FRAMEBUFFER_COLOR_BUFFER_SELECT;
    }
    void selectDepthBuffer()
    {
        m_op |= OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT;
    }
    void selectStencilBuffer()
    {
        m_op |= OP_FRAMEBUFFER_STENCIL_BUFFER_SELECT;
    }
    void setFramebufferSizeInPixel(const std::size_t size)
    {
        m_op |= (static_cast<uint32_t>(size) & OP_FRAMEBUFFER_SIZE_MASK) << OP_FRAMEBUFFER_SIZE_POS;
    }
    void enableVSync()
    {
        m_op |= OP_FRAMEBUFFER_SWAP_ENABLE_VSYNC;
    }

    bool getSwapFramebuffer() const
    {
        return (m_op & ~OP_MASK) & OP_FRAMEBUFFER_SWAP;
    }
    bool getCommitFramebuffer() const
    {
        return (m_op & ~OP_MASK) & OP_FRAMEBUFFER_COMMIT;
    }
    bool getEnableMemset() const
    {
        return (m_op & ~OP_MASK) & OP_FRAMEBUFFER_MEMSET;
    }
    bool getSelectColorBuffer() const
    {
        return (m_op & ~OP_MASK) & OP_FRAMEBUFFER_COLOR_BUFFER_SELECT;
    }
    bool setSelectDepthBuffer() const
    {
        return (m_op & ~OP_MASK) & OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT;
    }
    bool getSelectStencilBuffer() const
    {
        return (m_op & ~OP_MASK) & OP_FRAMEBUFFER_STENCIL_BUFFER_SELECT;
    }
    std::size_t getFramebufferSizeInPixel(const std::size_t size) const
    {
        return static_cast<std::size_t>(((m_op & ~OP_MASK) >> OP_FRAMEBUFFER_SIZE_POS) & OP_FRAMEBUFFER_SIZE_MASK);
    }
    bool getEnableVSync() const
    {
        return (m_op & ~OP_MASK) & OP_FRAMEBUFFER_SWAP_ENABLE_VSYNC;
    }

    const PayloadType payload() const { return {}; }
    CommandType command() const { return m_op; }

    static std::size_t getNumberOfElementsInPayloadByCommand(const uint32_t) { return 0; }
    static bool isThis(const CommandType cmd) { return (cmd & OP_MASK) == OP_FRAMEBUFFER; }

private:
    CommandType m_op {};
};

} // namespace rr

#endif // _FRAMEBUFFER_CMD_HPP_
