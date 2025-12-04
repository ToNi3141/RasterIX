// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2023 ToNi3141

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

#ifndef STACK_HPP
#define STACK_HPP

#include <array>

namespace rr
{
template <typename T, std::size_t DEPTH>
class Stack
{
public:
    bool push(const T& v)
    {
        if (m_stackPointer < m_stack.size())
        {
            m_stack[m_stackPointer] = v;
            m_stackPointer++;
            return true;
        }
        return false;
    }

    bool pop(T& v)
    {
        if (m_stackPointer > 0)
        {
            m_stackPointer--;
            v = m_stack[m_stackPointer];
            return true;
        }
        return false;
    }

    std::size_t getCurrentDepth() const { return m_stackPointer; }

private:
    std::size_t m_stackPointer { 0 };
    std::array<T, DEPTH> m_stack {};
};
} // namespace rr

#endif // STACK_HPP