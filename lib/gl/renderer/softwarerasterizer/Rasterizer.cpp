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

void Rasterizer::init(const TriangleStreamTypes::TriangleDescX& triangle)
{
    m_yLineResolution = m_resolutionData.y;

    if constexpr (RenderConfig::USE_FLOAT_INTERPOLATION)
    {
        if (m_yOffset <= triangle.param.bbStartY)
        {
            m_w = triangle.param.wInit;
        }
        else
        {
            const int32_t lineBBStartY = m_yOffset - static_cast<int32_t>(triangle.param.bbStartY);
            m_w = triangle.param.wYInc;
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

    m_wXInc = triangle.param.wXInc;
    m_wYInc = triangle.param.wYInc - (triangle.param.wXInc * (triangle.param.bbEndX - triangle.param.bbStartX));
    m_bbStartX = triangle.param.bbStartX;
    m_bbEndX = triangle.param.bbEndX;
    m_bbStartY = triangle.param.bbStartY;
    m_x = triangle.param.bbStartX;
    m_yi = m_y;
    m_hit = false;
}

} // namespace rr::softwarerasterizer
