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
    // Function pointer type for test function
    using TestFn = bool (*)(const T, const T);

    bool check(const T value) const
    {
        if (!m_enable)
        {
            return true;
        }
        return m_testFn(value, m_refValue);
    }

    void setFunction(rr::TestFunc func)
    {
        m_testFn = getTestFn(func);
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
    // Static test functions - no switch in hot path
    static bool testAlways(const T, const T)
    {
        return true;
    }

    static bool testNever(const T, const T)
    {
        return false;
    }

    static bool testLess(const T value, const T ref)
    {
        return value < ref;
    }

    static bool testEqual(const T value, const T ref)
    {
        return value == ref;
    }

    static bool testLEqual(const T value, const T ref)
    {
        return value <= ref;
    }

    static bool testGreater(const T value, const T ref)
    {
        return value > ref;
    }

    static bool testNotEqual(const T value, const T ref)
    {
        return value != ref;
    }

    static bool testGEqual(const T value, const T ref)
    {
        return value >= ref;
    }

    static TestFn getTestFn(const rr::TestFunc func)
    {
        switch (func)
        {
        case rr::TestFunc::ALWAYS:
            return &testAlways;
        case rr::TestFunc::NEVER:
            return &testNever;
        case rr::TestFunc::LESS:
            return &testLess;
        case rr::TestFunc::EQUAL:
            return &testEqual;
        case rr::TestFunc::LEQUAL:
            return &testLEqual;
        case rr::TestFunc::GREATER:
            return &testGreater;
        case rr::TestFunc::NOTEQUAL:
            return &testNotEqual;
        case rr::TestFunc::GEQUAL:
            return &testGEqual;
        default:
            return &testAlways;
        }
    }

    TestFn m_testFn { &testAlways };
    bool m_enable { false };
    T m_refValue { 0 };
};
} // namespace rr::softwarerasterizer

#endif // _TESTFUNC_HPP_