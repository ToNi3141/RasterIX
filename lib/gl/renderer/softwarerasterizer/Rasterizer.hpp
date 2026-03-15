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

    void init(const TriangleStreamTypes::TriangleDescX& triangle);

    const FragmentData& fragmentData() const
    {
        return m_fragmentData;
    }

    bool hit() const
    {
        return m_hit;
    }

    void walk()
    {
        if ((m_x >= m_bbEndX) || (m_x < m_bbStartX))
        {
            yInc();
        }
        m_hit = isInTriangle() && isInBounds();
        calcFragmentData();
        xInc();
    }

    bool isDone() const
    {
        return m_yScreen >= m_yScreenEnd;
    }

    void setYOffset(const uint32_t yOffset)
    {
        m_yOffset = yOffset;
    }

private:
    void calcFragmentData()
    {
        int32_t bby = 0;
        if constexpr (RenderConfig::USE_FLOAT_INTERPOLATION)
        {
            // In this case, the attributes are not preprocessed. The rasterizer
            // needs to calculate the correct y position within the bounding box,
            // based on the current screen y position.
            // The current triangle might start outside of the current tile.
            bby = m_yScreen - m_bbStartY;
        }
        else
        {
            // In this case, the attributes are preprocessed. They starting always
            // in the current tile. The vertex transformer adjusts it.
            // That means, our reference position is the current position on the line,
            // and not the screen position like it is above.
            bby = m_y - m_yi;
        }
        m_fragmentData.index = (((m_yLineResolution - 1) - m_y) * m_resolutionData.x) + m_x;
        m_fragmentData.bbx = m_x - m_bbStartX;
        m_fragmentData.bby = bby;
        m_fragmentData.spx = m_x;
        m_fragmentData.spy = m_yScreen;
    }

    bool isInTriangle() const
    {
        return (m_w[0] >= 0) && (m_w[1] >= 0) && (m_w[2] >= 0);
    }

    bool isInBounds() const
    {
        return (m_x < m_bbEndX) && (m_x >= m_bbStartX);
    }

    void yInc()
    {
        m_y++;
        m_yScreen++;
        m_w += m_wYInc;
        m_x = m_bbStartX;
    }

    void xInc()
    {
        m_x++;
        m_w += m_wXInc;
    }

    const ResolutionData& m_resolutionData;

    uint32_t m_yOffset { 0 };

    Vec3i m_w {};
    Vec3i m_wXInc {};
    Vec3i m_wYInc {};

    int32_t m_x {};
    int32_t m_y {};
    int32_t m_yi {};

    int32_t m_yScreen {};
    int32_t m_yScreenEnd {};
    int32_t m_bbStartX {};
    int32_t m_bbEndX {};
    int32_t m_bbStartY {};
    int32_t m_yLineResolution {};

    bool m_hit { false };
    FragmentData m_fragmentData {};
};

} // namespace rr::softwarerasterizer

#endif // _RASTERIZER_HPP_
