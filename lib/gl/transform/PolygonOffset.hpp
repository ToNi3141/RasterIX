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

#ifndef POLYGON_OFFSET_HPP
#define POLYGON_OFFSET_HPP

#include "math/Vec.hpp"
#include <algorithm>
#include <array>
#include <limits>
#include <tcb/span.hpp>

namespace rr::polygonoffset
{

struct PolygonOffsetData
{
    float factor;
    float units;
    bool enable;
};

class PolygonOffsetCalc
{
public:
    PolygonOffsetCalc(const PolygonOffsetData& data)
        : m_data { data }
    {
    }

    void addOffset(Vec4& v0, Vec4& v1, Vec4& v2) const
    {
        const float o = calculateO(v0, v1, v2);
        v0[2] = std::clamp(v0[2] + o, 0.0f, 0.99999f);
        v1[2] = std::clamp(v1[2] + o, 0.0f, 0.99999f);
        v2[2] = std::clamp(v2[2] + o, 0.0f, 0.99999f);
    }

private:
    // Calculating it with the cramer's rule
    inline static float dZw(const Vec4& v0, const Vec4& v1, const Vec4& v2);
    inline static float dXw(const Vec4& v0, const Vec4& v1, const Vec4& v2);
    inline static float dYw(const Vec4& v0, const Vec4& v1, const Vec4& v2);
    inline static float approximatedM(const Vec4& v0, const Vec4& v1, const Vec4& v2);
    float calculateO(const Vec4& v0, const Vec4& v1, const Vec4& v2) const;

    // Multiply a small factor to it to guarantee that the system can always distinguish the z values.
    static constexpr float m_r { (1.0f / static_cast<float>(std::numeric_limits<uint16_t>::max())) * 2 };

    const PolygonOffsetData& m_data;
};

class PolygonOffsetSetter
{
public:
    PolygonOffsetSetter(PolygonOffsetData& data)
        : m_data { data }
    {
    }

    void setFactor(const float factor)
    {
        m_data.factor = factor;
    }

    void setUnits(const float units)
    {
        m_data.units = units;
    }

    void setEnableFill(const bool enable)
    {
        m_data.enable = enable;
    }

private:
    PolygonOffsetData& m_data;
};

} // namespace rr::polygonoffset

#endif // POLYGON_OFFSET_HPP
