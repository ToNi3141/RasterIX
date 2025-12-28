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
        m_gram = {
            reinterpret_cast<FBType*>(gram.subspan(m_address).data()),
            gram.size() / sizeof(FBType)
        };
    }

    void setAddress(const uint32_t addr)
    {
        m_address = addr / sizeof(FBType);
    }

    void setClearColor(const FBType color)
    {
        m_clearColor = color;
    }

    void clear()
    {
        const std::size_t startX = (m_scissorData.enabled) ? m_scissorData.startX : 0;
        const std::size_t endX = (m_scissorData.enabled) ? m_scissorData.endX : m_resolutionData.x;
        const std::size_t startY = (m_scissorData.enabled) ? m_scissorData.startY : 0;
        const std::size_t endY = (m_scissorData.enabled) ? m_scissorData.endY : m_resolutionData.y;
        for (std::size_t y = startY; y < endY; y++)
        {
            for (std::size_t x = startX; x < endX; x++)
            {
                const std::size_t fbPos = x + ((m_resolutionData.y - y - 1) * m_resolutionData.x);
                m_gram[fbPos + m_address] = m_clearColor;
            }
        }
    }

    void writeFragment(const FBType fragment, std::size_t index)
    {
        if (!m_enable)
        {
            return;
        }
        m_gram[index + m_address] = fragment;
    }

    FBType readFragment(std::size_t index) const
    {
        if (!m_enable)
        {
            return m_clearColor;
        }
        return m_gram[index + m_address];
    }

    void setEnable(const bool enable)
    {
        m_enable = enable;
    }

private:
    tcb::span<FBType> m_gram {};
    uint32_t m_address {};
    FBType m_clearColor {};
    bool m_enable { true };

    const ResolutionData& m_resolutionData {};
    const ScissorData& m_scissorData {};
};

} // namespace rr::softwarerasterizer

#endif // _FRAMEBUFFER_HPP_
