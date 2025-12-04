// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

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

#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP

#include "math/Vec.hpp"

namespace rr::viewport
{
struct ViewPortData
{
    float depthRangeOffset { 0.0f };
    float depthRangeScale { 1.0f };
    float viewportX { 0.0f };
    float viewportY { 0.0f };
    float viewportHeightHalf { 0.0f };
    float viewportWidthHalf { 0.0f };
    float viewportHeight { 0.0f };
    float viewportWidth { 0.0f };
};

class ViewPortCalc
{
public:
    ViewPortCalc(const ViewPortData& viewPortData)
        : m_data { viewPortData }
    {
    }

    void transform(Vec4& v) const
    {
        // v has the range from -1 to 1. When we multiply it, it has a range from -viewPortWidth/2 to viewPortWidth/2
        // With the addition we shift it from -viewPortWidth/2 to 0 + viewPortX
        v[0] = ((v[0] + 1.0f) * m_data.viewportWidthHalf) + m_data.viewportX;
        v[1] = ((v[1] + 1.0f) * m_data.viewportHeightHalf) + m_data.viewportY;
        v[2] = (m_data.depthRangeScale * v[2]) + m_data.depthRangeOffset;
        v[2] *= 65534.0f / 65536.0f; // Scales down the z value a bit, because the hardware expects a range of [0 .. 65535], which is basically [0.0 .. 0.999..]
    }

private:
    const ViewPortData& m_data;
};

class ViewPortSetter
{
public:
    ViewPortSetter(ViewPortData& data)
        : m_data { data } {};

    void setViewport(const float x, const float y, const float width, const float height);
    void setDepthRange(const float zNear, const float zFar);

    float getViewportX() const { return m_data.viewportX; }
    float getViewportY() const { return m_data.viewportY; }
    float getViewportWidth() const { return m_data.viewportWidth; }
    float getViewportHeight() const { return m_data.viewportHeight; }

    float getDepthRangeNear() const { return m_zNear; }
    float getDepthRangeFar() const { return m_zFar; }

    static constexpr float getMaxViewportDimsWidth() { return 2048.0f; } // Arbitrary limit to avoid too large viewports. Hardware may support larger sizes.
    static constexpr float getMaxViewportDimsHeight() { return 2048.0f; } // Arbitrary limit to avoid too large viewports. Hardware may support larger sizes.
private:
    ViewPortData& m_data;
    float m_zNear { 0.0f };
    float m_zFar { 1.0f };
};

} // namespace rr::viewport
#endif // VIEWPORT_HPP
