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

#ifndef _TESTFUNC_HPP_
#define _TESTFUNC_HPP_

#include "Enums.hpp"
#include <cstdint>

namespace rr::softwarerasterizer
{
template <typename T>
class TestFunc
{
public:
    bool check(const T value) const
    {
        if (!m_enable)
        {
            return true;
        }

        switch (m_func)
        {
        case rr::TestFunc::ALWAYS:
            return true;
        case rr::TestFunc::NEVER:
            return false;
        case rr::TestFunc::LESS:
            return value < m_refValue;
        case rr::TestFunc::EQUAL:
            return value == m_refValue;
        case rr::TestFunc::LEQUAL:
            return value <= m_refValue;
        case rr::TestFunc::GREATER:
            return value > m_refValue;
        case rr::TestFunc::NOTEQUAL:
            return value != m_refValue;
        case rr::TestFunc::GEQUAL:
            return value >= m_refValue;
        default:
            return false;
        }
    }

    void setFunction(rr::TestFunc func)
    {
        m_func = func;
    }

    void setReferenceValue(const T value)
    {
        m_refValue = value;
    }

    void setEnable(const bool enable)
    {
        m_enable = enable;
    }

private:
    rr::TestFunc m_func { rr::TestFunc::ALWAYS };
    bool m_enable { false };
    T m_refValue { 0 };
};
} // namespace rr::softwarerasterizer

#endif // _TESTFUNC_HPP_