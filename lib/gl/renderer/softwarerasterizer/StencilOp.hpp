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

#ifndef _STENCILOP_HPP_
#define _STENCILOP_HPP_

#include "Enums.hpp"
#include "math/Vec.hpp"
#include "math/Veci.hpp"
#include <cstdint>

namespace rr::softwarerasterizer
{
class StencilOp
{
public:
    uint8_t op(const uint8_t val, const bool zTestPassed, const bool stencilTestPassed) const
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

    void setZFailOp(const rr::StencilOp op)
    {
        m_zFailOp = op;
    }

    void setZPassOp(const rr::StencilOp op)
    {
        m_zPassOp = op;
    }

    void setFailOp(const rr::StencilOp op)
    {
        m_stencilFailOp = op;
    }

    void setRefValue(const uint8_t ref)
    {
        m_refValue = ref & MAX_STNECIL_VALUE;
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
    static constexpr uint8_t MAX_STNECIL_VALUE { 15 };

    uint8_t applyOp(const uint8_t val, const rr::StencilOp op) const
    {
        switch (op)
        {
        case rr::StencilOp::KEEP:
            return val & MAX_STNECIL_VALUE;
        case rr::StencilOp::ZERO:
            return 0;
        case rr::StencilOp::REPLACE:
            return m_refValue;
        case rr::StencilOp::INCR:
            return (val == MAX_STNECIL_VALUE) ? MAX_STNECIL_VALUE : (val + 1);
        case rr::StencilOp::INCR_WRAP:
            return (val == MAX_STNECIL_VALUE) ? 0 : (val + 1);
        case rr::StencilOp::DECR:
            return (val == 0) ? 0 : (val - 1);
        case rr::StencilOp::DECR_WRAP:
            return (val == 0) ? MAX_STNECIL_VALUE : (val - 1);
        case rr::StencilOp::INVERT:
            return ~val & MAX_STNECIL_VALUE;
        default:
            return val;
        }
    }

    rr::StencilOp m_zFailOp { rr::StencilOp::KEEP };
    rr::StencilOp m_zPassOp { rr::StencilOp::KEEP };
    rr::StencilOp m_stencilFailOp { rr::StencilOp::KEEP };
    uint8_t m_refValue { 0 };
    bool m_enable { false };
};
} // namespace rr::softwarerasterizer

#endif // _STENCILOP_HPP_
