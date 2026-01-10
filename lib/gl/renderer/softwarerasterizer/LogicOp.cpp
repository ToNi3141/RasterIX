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

#include "LogicOp.hpp"

namespace rr::softwarerasterizer
{

Vec4 LogicOp::op(const Vec4& src, const Vec4& dst) const
{
    if (!m_enable)
        return src;

    const Vec4ui8 srcInt = Vec4ui8::createFromVec<Vec4, 8>(src);
    const Vec4ui8 dstInt = Vec4ui8::createFromVec<Vec4, 8>(dst);
    const Vec4ui8 resultInt = calcOp(srcInt, dstInt);
    const Vec4 result { static_cast<float>(resultInt[0]) / 255.0f,
        static_cast<float>(resultInt[1]) / 255.0f,
        static_cast<float>(resultInt[2]) / 255.0f,
        static_cast<float>(resultInt[3]) / 255.0f };
    return result;
}

Vec4ui8 LogicOp::calcOp(const Vec4ui8& src, const Vec4ui8& dst) const
{
    Vec4ui8 result;
    switch (m_logicOp)
    {
    case rr::LogicOp::CLEAR:
        result = Vec4ui8 { 0, 0, 0, 0 };
        break;
    case rr::LogicOp::SET:
        result = Vec4ui8 { 255, 255, 255, 255 };
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

} // namespace rr::softwarerasterizer
