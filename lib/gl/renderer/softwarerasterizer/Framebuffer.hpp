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

#ifndef _FRAMEBUFFER_HPP_
#define _FRAMEBUFFER_HPP_

#include "ResolutionData.hpp"
#include "ScissorData.hpp"
#include <cstdint>
#include <tcb/span.hpp>

namespace rr::softwarerasterizer
{

template <typename FBType>
class Framebuffer
{
public:
    Framebuffer(const ScissorData& scissorData, const ResolutionData& resolutionData)
        : m_scissorData { scissorData }
        , m_resolutionData { resolutionData }
    {
    }
    virtual ~Framebuffer() = default;

    void setGRAM(tcb::span<uint8_t> gram)
    {
        m_gram = gram;
    }

    void setYOffset(const uint32_t yOffset)
    {
        m_yOffset = yOffset;
    }

    void setAddress(const uint32_t addr)
    {
        m_address = addr / sizeof(FBType);
        m_fb = {
            reinterpret_cast<FBType*>(m_gram.subspan(addr).data()),
            (m_gram.size() / sizeof(FBType)) - m_address
        };
    }

    uint32_t getAddress() const
    {
        return m_address * sizeof(FBType);
    }

    void setClearColor(const FBType color)
    {
        m_clearColor = color;
    }

    void clear()
    {
        const std::size_t startX = (m_scissorData.enabled) ? std::min(m_scissorData.startX, static_cast<int32_t>(m_resolutionData.x)) : 0;
        const std::size_t endX = (m_scissorData.enabled) ? std::min(m_scissorData.endX, static_cast<int32_t>(m_resolutionData.x)) : m_resolutionData.x;
        const std::size_t startY = (m_scissorData.enabled) ? std::min(m_scissorData.startY, static_cast<int32_t>(m_resolutionData.y + m_yOffset)) : 0;
        const std::size_t endY = (m_scissorData.enabled) ? std::min(m_scissorData.endY, static_cast<int32_t>(m_resolutionData.y + m_yOffset)) : (m_resolutionData.y + m_yOffset);
        for (std::size_t y = startY; y < endY; y++)
        {
            for (std::size_t x = startX; x < endX; x++)
            {
                const std::size_t yLine = y - m_yOffset;
                const std::size_t index = x + ((m_resolutionData.y - yLine - 1) * m_resolutionData.x);
                writeFragment(m_clearColor, index, x, y);
            }
        }
    }

    void writeFragment(const FBType fragment, const std::size_t index, const std::size_t x, const std::size_t y)
    {
        if (!checkScissor(x, y))
        {
            return;
        }
        m_fb[index] = (fragment & m_mask) | (m_fb[index] & ~m_mask);
    }

    FBType readFragment(std::size_t index) const
    {
        return m_fb[index];
    }

    void setMask(const FBType mask)
    {
        m_mask = mask;
    }

private:
    bool checkScissor(const std::size_t x, const std::size_t y) const
    {
        if (m_scissorData.enabled)
        {
            if (x < static_cast<std::size_t>(m_scissorData.startX) || x >= static_cast<std::size_t>(m_scissorData.endX)
                || y < static_cast<std::size_t>(m_scissorData.startY) || y >= static_cast<std::size_t>(m_scissorData.endY))
            {
                return false;
            }
        }
        return true;
    }

    tcb::span<uint8_t> m_gram {};
    tcb::span<FBType> m_fb {};
    uint32_t m_address {};
    FBType m_clearColor {};
    FBType m_mask {};

    const ScissorData& m_scissorData {};
    const ResolutionData& m_resolutionData {};

    uint32_t m_yOffset {};
};

} // namespace rr::softwarerasterizer

#endif // _FRAMEBUFFER_HPP_
