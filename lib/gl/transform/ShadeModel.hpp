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

#ifndef SHADE_MODEL_HPP
#define SHADE_MODEL_HPP

#include "Enums.hpp"
#include "Types.hpp"
#include "math/Vec.hpp"

namespace rr::shademodel
{
struct ShadeModelData
{
    bool shadeModelFlat { false };
};

class ShadeModelCalc
{
public:
    ShadeModelCalc(const ShadeModelData& ShadeModelData)
        : m_data { ShadeModelData }
    {
    }

    void updateShadeModelTriangle(VertexParameter& vp0, VertexParameter& vp1, VertexParameter& vp2) const
    {
        if (m_data.shadeModelFlat)
        {
            vp1.color = vp0.color;
            vp2.color = vp0.color;
        }
    }

    void updateShadeModelLine(VertexParameter& vp0, VertexParameter& vp1)
    {
        if (m_data.shadeModelFlat)
        {
            vp1.color = vp0.color;
        }
    }

private:
    const ShadeModelData& m_data;
};

class ShadeModelSetter
{
public:
    ShadeModelSetter(ShadeModelData& data)
        : m_data { data }
    {
    }
    void setShadeModelFlat(const bool val) { m_data.shadeModelFlat = val; }

private:
    ShadeModelData& m_data;
};

} // namespace rr::shademodel
#endif // SHADE_MODEL_HPP
