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

#ifndef _LOGICOP_HPP_
#define _LOGICOP_HPP_

#include "Enums.hpp"
#include "math/Vec.hpp"
#include "math/Veci.hpp"
#include <cstdint>

namespace rr::softwarerasterizer
{
class LogicOp
{
public:
    Vec4 op(const Vec4& src, const Vec4& dst) const;

    void setLogicOp(const rr::LogicOp logicOp)
    {
        m_logicOp = logicOp;
    }

    void setEnable(const bool enable)
    {
        m_enable = enable;
    }

    bool getEnable() const
    {
        return m_enable;
    }

private:
    static Vec4ui8 invertColor(const Vec4ui8& color)
    {
        return Vec4ui8 { 255, 255, 255, 255 } - color;
    }

    Vec4ui8 calcOp(const Vec4ui8& src, const Vec4ui8& dst) const;

    bool m_enable { false };
    rr::LogicOp m_logicOp { rr::LogicOp::COPY };
};
} // namespace rr::softwarerasterizer

#endif // _LOGICOP_HPP_
