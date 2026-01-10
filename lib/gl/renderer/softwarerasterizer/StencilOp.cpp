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

#include "StencilOp.hpp"

namespace rr::softwarerasterizer
{

uint8_t StencilOp::op(const uint8_t val, const bool zTestPassed, const bool stencilTestPassed) const
{
    if (!m_enable)
        return val;

    if (!stencilTestPassed)
    {
        return applyOp(val, m_stencilFailOp);
    }
    else if (!zTestPassed)
    {
        return applyOp(val, m_zFailOp);
    }
    else
    {
        return applyOp(val, m_zPassOp);
    }
}

uint8_t StencilOp::applyOp(const uint8_t val, const rr::StencilOp op) const
{
    switch (op)
    {
    case rr::StencilOp::KEEP:
        return val & MAX_STENCIL_VALUE;
    case rr::StencilOp::ZERO:
        return 0;
    case rr::StencilOp::REPLACE:
        return m_refValue;
    case rr::StencilOp::INCR:
        return (val == MAX_STENCIL_VALUE) ? MAX_STENCIL_VALUE : (val + 1);
    case rr::StencilOp::INCR_WRAP:
        return (val == MAX_STENCIL_VALUE) ? 0 : (val + 1);
    case rr::StencilOp::DECR:
        return (val == 0) ? 0 : (val - 1);
    case rr::StencilOp::DECR_WRAP:
        return (val == 0) ? MAX_STENCIL_VALUE : (val - 1);
    case rr::StencilOp::INVERT:
        return ~val & MAX_STENCIL_VALUE;
    default:
        return val;
    }
}

} // namespace rr::softwarerasterizer
