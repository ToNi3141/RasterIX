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

#ifndef _BASE_COLOR_REG_
#define _BASE_COLOR_REG_

#include "math/Veci.hpp"
#include <cstdint>

namespace rr
{
class BaseColorReg
{
public:
    BaseColorReg() = default;

    BaseColorReg(const Vec4i& val)
    {
        setColor(val);
    }

    void setColor(const Vec4i& val)
    {
        setRed(val[0]);
        setGreen(val[1]);
        setBlue(val[2]);
        setAlpha(val[3]);
    }

    Vec4i getColor() const
    {
        return Vec4i { getRed(), getGreen(), getBlue(), getAlpha() };
    }

    void setRed(const uint8_t val) { m_regVal.fields.red = val; }
    void setGreen(const uint8_t val) { m_regVal.fields.green = val; }
    void setBlue(const uint8_t val) { m_regVal.fields.blue = val; }
    void setAlpha(const uint8_t val) { m_regVal.fields.alpha = val; }

    uint8_t getRed() const { return m_regVal.fields.red; }
    uint8_t getGreen() const { return m_regVal.fields.green; }
    uint8_t getBlue() const { return m_regVal.fields.blue; }
    uint8_t getAlpha() const { return m_regVal.fields.alpha; }

    uint32_t serialize() const { return m_regVal.data; }
    void deserialize(const uint32_t data) { m_regVal.data = data; }

private:
    union RegVal
    {
#pragma pack(push, 1)
        struct RegContent
        {
            RegContent()
                : alpha { 0 }
                , blue { 0 }
                , green { 0 }
                , red { 0 }
            {
            }

            uint32_t alpha : 8;
            uint32_t blue : 8;
            uint32_t green : 8;
            uint32_t red : 8;
        } fields {};
        uint32_t data;
#pragma pack(pop)
    } m_regVal;
};
} // namespace rr

#endif // _BASE_COLOR_REG_
