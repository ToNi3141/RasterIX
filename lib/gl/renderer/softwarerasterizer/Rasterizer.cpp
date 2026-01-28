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

#include "Rasterizer.hpp"

namespace rr::softwarerasterizer
{

void Rasterizer::init(const TriangleStreamTypes::TriangleDesc& triangle)
{
    m_yLineResolution = m_resolutionData.y;

    m_wXInc = triangle.param.wXInc;
    m_wYInc = triangle.param.wYInc;
    if constexpr (RenderConfig::USE_FLOAT_INTERPOLATION)
    {
        if (m_yOffset <= triangle.param.bbStartY)
        {
            m_w = triangle.param.wInit;
        }
        else
        {
            const int32_t lineBBStartY = m_yOffset - static_cast<int32_t>(triangle.param.bbStartY);
            m_w = m_wYInc;
            m_w *= lineBBStartY;
            m_w += triangle.param.wInit;
        }
    }
    else
    {
        m_w = triangle.param.wInit;
    }

    if (m_yOffset <= triangle.param.bbStartY)
    {
        m_yScreen = triangle.param.bbStartY;
        m_y = triangle.param.bbStartY - m_yOffset;
    }
    else
    {
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
    m_yi = m_y;
}

void Rasterizer::walk()
{
    if (!isDone())
    {
        switch (m_state)
        {
        case EdgeWalkerState::INIT:
            if (isInTriangle())
            {
                m_state = EdgeWalkerState::WALKING;
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
            if (!isInTriangle() || (m_x <= m_bbStartX) || (m_x >= m_bbEndX))
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
            if (!isInTriangleAndInBounds())
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

bool Rasterizer::searchEdge()
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

} // namespace rr::softwarerasterizer
