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

#include "ViewPort.hpp"

namespace rr::viewport
{

void ViewPortSetter::setViewport(const float x, const float y, const float width, const float height)
{
    m_data.viewportHeight = height;
    m_data.viewportWidth = width;
    m_data.viewportX = x;
    m_data.viewportY = y;

    m_data.viewportHeightHalf = m_data.viewportHeight / 2.0f;
    m_data.viewportWidthHalf = m_data.viewportWidth / 2.0f;
}

void ViewPortSetter::setDepthRange(const float zNear, const float zFar)
{
    m_data.depthRangeScale = ((zFar - zNear) / 2.0f);
    m_data.depthRangeOffset = ((zNear + zFar) / 2.0f);
    m_zNear = zNear;
    m_zFar = zFar;
}
} // namespace rr