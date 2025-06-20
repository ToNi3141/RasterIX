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

#ifndef _TEX_ENV_REG_
#define _TEX_ENV_REG_

#include "Enums.hpp"
#include <cstdint>

namespace rr
{

class TexEnvReg
{
public:
    static_assert(static_cast<uint32_t>(Combine::REPLACE) == 0);
    static_assert(static_cast<uint32_t>(Combine::MODULATE) == 1);
    static_assert(static_cast<uint32_t>(Combine::ADD) == 2);
    static_assert(static_cast<uint32_t>(Combine::ADD_SIGNED) == 3);
    static_assert(static_cast<uint32_t>(Combine::INTERPOLATE) == 4);
    static_assert(static_cast<uint32_t>(Combine::SUBTRACT) == 5);
    static_assert(static_cast<uint32_t>(Combine::DOT3_RGB) == 6);
    static_assert(static_cast<uint32_t>(Combine::DOT3_RGBA) == 7);

    static_assert(static_cast<uint32_t>(Operand::SRC_ALPHA) == 0);
    static_assert(static_cast<uint32_t>(Operand::ONE_MINUS_SRC_ALPHA) == 1);
    static_assert(static_cast<uint32_t>(Operand::SRC_COLOR) == 2);
    static_assert(static_cast<uint32_t>(Operand::ONE_MINUS_SRC_COLOR) == 3);

    static_assert(static_cast<uint32_t>(SrcReg::TEXTURE) == 0);
    static_assert(static_cast<uint32_t>(SrcReg::CONSTANT) == 1);
    static_assert(static_cast<uint32_t>(SrcReg::PRIMARY_COLOR) == 2);
    static_assert(static_cast<uint32_t>(SrcReg::PREVIOUS) == 3);

    TexEnvReg() = default;

    void setCombineRgb(const Combine val) { m_regVal.fields.combineRgb = static_cast<uint32_t>(val); }
    void setCombineAlpha(const Combine val) { m_regVal.fields.combineAlpha = static_cast<uint32_t>(val); }
    void setSrcRegRgb0(const SrcReg val) { m_regVal.fields.srcRegRgb0 = static_cast<uint32_t>(val); }
    void setSrcRegRgb1(const SrcReg val) { m_regVal.fields.srcRegRgb1 = static_cast<uint32_t>(val); }
    void setSrcRegRgb2(const SrcReg val) { m_regVal.fields.srcRegRgb2 = static_cast<uint32_t>(val); }
    void setSrcRegAlpha0(const SrcReg val) { m_regVal.fields.srcRegAlpha0 = static_cast<uint32_t>(val); }
    void setSrcRegAlpha1(const SrcReg val) { m_regVal.fields.srcRegAlpha1 = static_cast<uint32_t>(val); }
    void setSrcRegAlpha2(const SrcReg val) { m_regVal.fields.srcRegAlpha2 = static_cast<uint32_t>(val); }
    void setOperandRgb0(const Operand val) { m_regVal.fields.operandRgb0 = static_cast<uint32_t>(val); }
    void setOperandRgb1(const Operand val) { m_regVal.fields.operandRgb1 = static_cast<uint32_t>(val); }
    void setOperandRgb2(const Operand val) { m_regVal.fields.operandRgb2 = static_cast<uint32_t>(val); }
    void setOperandAlpha0(const Operand val) { m_regVal.fields.operandAlpha0 = static_cast<uint32_t>(val); }
    void setOperandAlpha1(const Operand val) { m_regVal.fields.operandAlpha1 = static_cast<uint32_t>(val); }
    void setOperandAlpha2(const Operand val) { m_regVal.fields.operandAlpha2 = static_cast<uint32_t>(val); }
    void setShiftRgb(const uint8_t val) { m_regVal.fields.shiftRgb = val; }
    void setShiftAlpha(const uint8_t val) { m_regVal.fields.shiftAlpha = val; }

    Combine getCombineRgb() const { return static_cast<Combine>(m_regVal.fields.combineRgb); }
    Combine getCombineAlpha() const { return static_cast<Combine>(m_regVal.fields.combineAlpha); }
    SrcReg getSrcRegRgb0() const { return static_cast<SrcReg>(m_regVal.fields.srcRegRgb0); }
    SrcReg getSrcRegRgb1() const { return static_cast<SrcReg>(m_regVal.fields.srcRegRgb1); }
    SrcReg getSrcRegRgb2() const { return static_cast<SrcReg>(m_regVal.fields.srcRegRgb2); }
    SrcReg getSrcRegAlpha0() const { return static_cast<SrcReg>(m_regVal.fields.srcRegAlpha0); }
    SrcReg getSrcRegAlpha1() const { return static_cast<SrcReg>(m_regVal.fields.srcRegAlpha1); }
    SrcReg getSrcRegAlpha2() const { return static_cast<SrcReg>(m_regVal.fields.srcRegAlpha2); }
    Operand getOperandRgb0() const { return static_cast<Operand>(m_regVal.fields.operandRgb0); }
    Operand getOperandRgb1() const { return static_cast<Operand>(m_regVal.fields.operandRgb1); }
    Operand getOperandRgb2() const { return static_cast<Operand>(m_regVal.fields.operandRgb2); }
    Operand getOperandAlpha0() const { return static_cast<Operand>(m_regVal.fields.operandAlpha0); }
    Operand getOperandAlpha1() const { return static_cast<Operand>(m_regVal.fields.operandAlpha1); }
    Operand getOperandAlpha2() const { return static_cast<Operand>(m_regVal.fields.operandAlpha2); }
    uint8_t getShiftRgb() const { return m_regVal.fields.shiftRgb; }
    uint8_t getShiftAlpha() const { return m_regVal.fields.shiftAlpha; }

    void setTmu(const std::size_t tmu) { m_tmu = tmu; }
    uint32_t serialize() const { return m_regVal.data; }
    void deserialize(const uint32_t data) { m_regVal.data = data; }
    uint32_t getAddr() const { return getAddr(m_tmu); }
    static constexpr uint32_t getAddr(std::size_t tmu) { return 0xA + (tmu * TMU_OFFSET); }

private:
    static constexpr std::size_t TMU_OFFSET { 3 };
    union RegVal
    {
#pragma pack(push, 1)
        struct RegContent
        {
            RegContent()
                : combineRgb { static_cast<uint32_t>(Combine::MODULATE) }
                , combineAlpha { static_cast<uint32_t>(Combine::MODULATE) }
                , srcRegRgb0 { static_cast<uint32_t>(SrcReg::TEXTURE) }
                , srcRegRgb1 { static_cast<uint32_t>(SrcReg::PREVIOUS) }
                , srcRegRgb2 { static_cast<uint32_t>(SrcReg::CONSTANT) }
                , srcRegAlpha0 { static_cast<uint32_t>(SrcReg::TEXTURE) }
                , srcRegAlpha1 { static_cast<uint32_t>(SrcReg::PREVIOUS) }
                , srcRegAlpha2 { static_cast<uint32_t>(SrcReg::CONSTANT) }
                , operandRgb0 { static_cast<uint32_t>(Operand::SRC_COLOR) }
                , operandRgb1 { static_cast<uint32_t>(Operand::SRC_COLOR) }
                , operandRgb2 { static_cast<uint32_t>(Operand::SRC_COLOR) }
                , operandAlpha0 { static_cast<uint32_t>(Operand::SRC_ALPHA) }
                , operandAlpha1 { static_cast<uint32_t>(Operand::SRC_ALPHA) }
                , operandAlpha2 { static_cast<uint32_t>(Operand::SRC_ALPHA) }
                , shiftRgb { 0 }
                , shiftAlpha { 0 }
            {
            }

            uint32_t combineRgb : 3;
            uint32_t combineAlpha : 3;
            uint32_t srcRegRgb0 : 2;
            uint32_t srcRegRgb1 : 2;
            uint32_t srcRegRgb2 : 2;
            uint32_t srcRegAlpha0 : 2;
            uint32_t srcRegAlpha1 : 2;
            uint32_t srcRegAlpha2 : 2;
            uint32_t operandRgb0 : 2;
            uint32_t operandRgb1 : 2;
            uint32_t operandRgb2 : 2;
            uint32_t operandAlpha0 : 1;
            uint32_t operandAlpha1 : 1;
            uint32_t operandAlpha2 : 1;
            uint32_t shiftRgb : 2;
            uint32_t shiftAlpha : 2;
        } fields {};
        uint32_t data;
#pragma pack(pop)
    } m_regVal;
    uint8_t m_tmu { 0 };
};

} // namespace rr

#endif // _TEX_ENV_REG_