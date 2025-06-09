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

#ifndef _BASE_SINGLE_REG_
#define _BASE_SINGLE_REG_

#include "math/Veci.hpp"
#include <cstdint>

namespace rr
{
template <uint32_t MASK>
class BaseSingleReg
{
public:
    BaseSingleReg() = default;
    BaseSingleReg(const uint32_t val) { setValue(val); }

    void setValue(const uint32_t val) { m_regVal = val & MASK; }

    uint32_t getValue() const { return m_regVal; }

    uint32_t serialize() const { return m_regVal; }
    void deserialize(const uint32_t data) { m_regVal = data; }

private:
    uint32_t m_regVal { 0 };
};
} // namespace rr

#endif // _BASE_SINGLE_REG_
