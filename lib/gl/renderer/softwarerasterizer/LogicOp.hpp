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
    Vec4iColorRGBA op(const Vec4iColorRGBA& src, const Vec4iColorRGBA& dst) const
    {
        if (!m_enable)
            return src;

        return calcOp(src, dst);
    }

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
    static Vec4iColorRGBA invertColor(const Vec4iColorRGBA& color)
    {
        return Vec4iColorRGBA { Vec4iColorRGBA::FracMax, Vec4iColorRGBA::FracMax, Vec4iColorRGBA::FracMax, Vec4iColorRGBA::FracMax } - color;
    }

    Vec4iColorRGBA calcOp(const Vec4iColorRGBA& src, const Vec4iColorRGBA& dst) const
    {
        Vec4iColorRGBA result;
        switch (m_logicOp)
        {
        case rr::LogicOp::CLEAR:
            result = Vec4iColorRGBA { Vec4iColorRGBA::Zero, Vec4iColorRGBA::Zero, Vec4iColorRGBA::Zero, Vec4iColorRGBA::Zero };
            break;
        case rr::LogicOp::SET:
            result = Vec4iColorRGBA { Vec4iColorRGBA::FracMax, Vec4iColorRGBA::FracMax, Vec4iColorRGBA::FracMax, Vec4iColorRGBA::FracMax };
            break;
        case rr::LogicOp::COPY:
            result = src;
            break;
        case rr::LogicOp::COPY_INVERTED:
            result = invertColor(src);
            break;
        case rr::LogicOp::NOOP:
            result = dst;
            break;
        case rr::LogicOp::INVERT:
            result = invertColor(dst);
            break;
        case rr::LogicOp::AND:
            result = src & dst;
            break;
        case rr::LogicOp::NAND:
            result = invertColor(src & dst);
            break;
        case rr::LogicOp::OR:
            result = src | dst;
            break;
        case rr::LogicOp::NOR:
            result = invertColor(src | dst);
            break;
        case rr::LogicOp::XOR:
            result = src ^ dst;
            break;
        case rr::LogicOp::EQUIV:
            result = invertColor(src ^ dst);
            break;
        case rr::LogicOp::AND_REVERSE:
            result = src & invertColor(dst);
            break;
        case rr::LogicOp::AND_INVERTED:
            result = invertColor(src) & dst;
            break;
        case rr::LogicOp::OR_REVERSE:
            result = src | invertColor(dst);
            break;
        case rr::LogicOp::OR_INVERTED:
            result = invertColor(src) | dst;
            break;
        default:
            result = src; // Fallback
            break;
        }
        return result;
    }

    bool m_enable { false };
    rr::LogicOp m_logicOp { rr::LogicOp::COPY };
};
} // namespace rr::softwarerasterizer

#endif // _LOGICOP_HPP_
