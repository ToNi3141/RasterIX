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

#ifndef _RASTERIZER_HPP_
#define _RASTERIZER_HPP_

#include "FragmentData.hpp"
#include "ResolutionData.hpp"
#include "renderer/commands/TriangleStreamTypes.hpp"
#include <cstdint>
#include <tcb/span.hpp>

namespace rr::softwarerasterizer
{

class Rasterizer
{
public:
    Rasterizer(const ResolutionData& resolutionData)
        : m_resolutionData { resolutionData }
    {
    }
    virtual ~Rasterizer() = default;

    void init(const TriangleStreamTypes::TriangleDesc& triangle);

    FragmentData hit() const
    {
        return {
            isInTriangleAndInBounds() && (m_state == EdgeWalkerState::WALKING),
            (((m_yLineResolution - 1) - m_y) * m_resolutionData.x) + m_x,
            m_x - m_bbStartX,
            m_yScreen - m_bbStartY,
            m_x,
            m_yScreen,
        };
    }

    void walk();

    bool isDone() const
    {
        return m_y >= m_yScreenEnd;
    }

    void setYOffset(const uint32_t yOffset)
    {
        m_yOffset = yOffset;
    }

private:
    enum class EdgeWalkerDirection
    {
        LEFT,
        RIGHT,
    };

    enum class EdgeWalkerState
    {
        INIT,
        SEARCH_EDGE,
        WALKING,
        WALK_OUT,
        CHECK_DIRECTION,
        DONE,
    };

    bool isInTriangle() const
    {
        return (m_w[0] >= 0) && (m_w[1] >= 0) && (m_w[2] >= 0);
    }

    bool isInTriangleAndInBounds() const
    {
        return isInTriangle() && (m_x < m_bbEndX) && (m_x >= m_bbStartX);
    }

    void switchEdgeWalkDirection()
    {
        if (m_dir == EdgeWalkerDirection::RIGHT)
        {
            m_dir = EdgeWalkerDirection::LEFT;
        }
        else
        {
            m_dir = EdgeWalkerDirection::RIGHT;
        }
    }

    bool searchEdge();

    void yInc()
    {
        m_y++;
        m_yScreen++;
        m_w += m_wYInc;
    }

    void xInc()
    {
        if (m_dir == EdgeWalkerDirection::LEFT)
        {
            m_x--;
            m_w -= m_wXInc;
        }
        else
        {
            m_x++;
            m_w += m_wXInc;
        }
    }

    const ResolutionData& m_resolutionData;

    uint32_t m_yOffset { 0 };

    Vec3i m_w {};
    Vec3i m_wXInc {};
    Vec3i m_wYInc {};

    int32_t m_x {};
    int32_t m_y {};

    int32_t m_yScreen {};
    int32_t m_yScreenEnd {};
    int32_t m_bbStartX {};
    int32_t m_bbEndX {};
    int32_t m_bbStartY {};
    int32_t m_yLineResolution {};

    EdgeWalkerDirection m_dir {};
    EdgeWalkerState m_state {};

    bool m_tryOtherSide { false };
};

} // namespace rr::softwarerasterizer

#endif // _RASTERIZER_HPP_
