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
#include "YOffsetData.hpp"
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

    void init(const TriangleStreamTypes::TriangleDesc& triangle)
    {
        m_yLineResolution = m_resolutionData.y;

        m_wXInc = triangle.param.wXInc;
        m_wYInc = triangle.param.wYInc;

        if (m_yOffset <= triangle.param.bbStartY)
        {
            m_w = triangle.param.wInit;

            m_yScreen = triangle.param.bbStartY;
            m_y = triangle.param.bbStartY - m_yOffset;
        }
        else
        {
            m_w = m_wYInc;
            m_w.mul<0>(m_yOffset - triangle.param.bbStartY);
            m_w += triangle.param.wInit;

            m_yScreen = m_yOffset;
            m_y = 0;
        }

        if ((m_yOffset + m_yLineResolution) <= triangle.param.bbEndY)
        {
            m_yScreenEnd = m_yOffset + m_yLineResolution;
        }
        else
        {
            m_yScreenEnd = triangle.param.bbEndY;
        }

        m_bbStartX = triangle.param.bbStartX;
        m_bbEndX = triangle.param.bbEndX;
        m_bbStartY = triangle.param.bbStartY;
        m_dir = EdgeWalkerDirection::RIGHT;
        m_state = EdgeWalkerState::INIT;
        m_x = m_bbStartX;
        m_tryOtherSide = false;
    }

    FragmentData hit() const
    {
        return {
            .hit = isInTriangleAndInBounds(),
            .index = (((m_yLineResolution - 1) - m_y) * m_resolutionData.x) + m_x,
            .bbx = m_x - m_bbStartX,
            .bby = m_yScreen - m_bbStartY,
            .spx = m_x,
            .spy = m_yScreen,
        };
    }

    void walk()
    {
        if (!isDone())
        {
            switch (m_state)
            {
            case EdgeWalkerState::INIT:
                if (isInTriangle())
                {
                    m_state = EdgeWalkerState::WALK_OUT;
                }
                else
                {
                    m_state = EdgeWalkerState::SEARCH_EDGE;
                }
                break;

            case EdgeWalkerState::SEARCH_EDGE:
                if (searchEdge())
                {
                    m_state = EdgeWalkerState::WALKING;
                }
                else
                {
                    xInc();
                }
                break;

            case EdgeWalkerState::WALK_OUT:
                if (!isInTriangle() || (m_x == m_bbStartX) || (m_x >= m_bbEndX))
                {
                    switchEdgeWalkDirection();
                    m_state = EdgeWalkerState::SEARCH_EDGE;
                }
                else
                {
                    xInc();
                }
                break;

            case EdgeWalkerState::CHECK_DIRECTION:
                if (isInTriangle())
                {
                    m_state = EdgeWalkerState::WALK_OUT;
                }
                else
                {
                    switchEdgeWalkDirection();
                    m_state = EdgeWalkerState::SEARCH_EDGE;
                }
                break;

            case EdgeWalkerState::WALKING:
                if (!isInTriangle())
                {
                    yInc();
                    m_state = EdgeWalkerState::CHECK_DIRECTION;
                }
                else
                {
                    xInc();
                }
                break;

            default:
                break;
            };
        }
    }

    bool isDone() const
    {
        return m_y >= m_yScreenEnd;
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

    bool searchEdge()
    {
        if (isInTriangleAndInBounds())
        {
            m_tryOtherSide = false;
            return true;
        }
        else if (m_x >= m_bbEndX)
        {
            if (m_dir == EdgeWalkerDirection::RIGHT && m_tryOtherSide)
            {
                m_tryOtherSide = false;
                return true;
            }
            else
            {
                m_tryOtherSide = true;
                m_dir = EdgeWalkerDirection::LEFT;
            }
        }
        else if (m_x <= m_bbStartX)
        {
            if ((m_dir == EdgeWalkerDirection::LEFT) && m_tryOtherSide)
            {
                m_tryOtherSide = false;
                return true;
            }
            else
            {
                m_tryOtherSide = true;
                m_dir = EdgeWalkerDirection::RIGHT;
            }
        }
        return false;
    }

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
