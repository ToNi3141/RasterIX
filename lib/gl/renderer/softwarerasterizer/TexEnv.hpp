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

#ifndef _TEXENV_HPP_
#define _TEXENV_HPP_

#include "Enums.hpp"
#include "math/Vec.hpp"
#include <cstdint>

namespace rr::softwarerasterizer
{

class TexEnv
{
public:
    Vec4 apply(
        const Vec4 previousColor,
        const Vec4 texSrcColor,
        const Vec4 primaryColor) const;

    void setEnvColor(const Vec4& color)
    {
        m_envColor = color;
    }

    void setCombineRgb(const Combine val)
    {
        m_combineRgb = val;
    }

    void setCombineAlpha(const Combine val)
    {
        m_combineAlpha = val;
    }

    void setSrcRegRgb0(const SrcReg val)
    {
        m_srcRegRgb0 = val;
    }

    void setSrcRegRgb1(const SrcReg val)
    {
        m_srcRegRgb1 = val;
    }

    void setSrcRegRgb2(const SrcReg val)
    {
        m_srcRegRgb2 = val;
    }

    void setSrcRegAlpha0(const SrcReg val)
    {
        m_srcRegAlpha0 = val;
    }

    void setSrcRegAlpha1(const SrcReg val)
    {
        m_srcRegAlpha1 = val;
    }

    void setSrcRegAlpha2(const SrcReg val)
    {
        m_srcRegAlpha2 = val;
    }

    void setOperandRgb0(const Operand val)
    {
        m_operandRgb0 = val;
    }

    void setOperandRgb1(const Operand val)
    {
        m_operandRgb1 = val;
    }

    void setOperandRgb2(const Operand val)
    {
        m_operandRgb2 = val;
    }

    void setOperandAlpha0(const Operand val)
    {
        m_operandAlpha0 = val;
    }

    void setOperandAlpha1(const Operand val)
    {
        m_operandAlpha1 = val;
    }

    void setOperandAlpha2(const Operand val)
    {
        m_operandAlpha2 = val;
    }

    void setShiftRgb(const uint8_t val)
    {
        m_scaleRgb = std::pow(2.0f, val);
    }

    void setShiftAlpha(const uint8_t val)
    {
        m_scaleAlpha = std::pow(2.0f, val);
    }

    void setEnable(bool enable)
    {
        m_enable = enable;
    }

private:
    float selectSrcAlpha(
        const SrcReg& srcReg,
        const Vec4& texture,
        const Vec4& constant,
        const Vec4& primaryColor,
        const Vec4& previous) const;

    Vec3 selectSrcRgb(
        const SrcReg& srcReg,
        const Vec4& texture,
        const Vec4& constant,
        const Vec4& primaryColor,
        const Vec4& previous) const;

    Vec3 selectRgbOperand(const Operand& operand, const Vec3& color) const;

    float selectAlphaOperand(const Operand& operand, const float& color) const;

    Vec3 combineRgb(const Combine& combine, const Vec3& op0, const Vec3& op1, const Vec3& op2) const;

    float combineAlpha(const Combine& combine, const float& op0, const float& op1, const float& op2) const;

    Vec4 m_envColor {};

    Combine m_combineRgb {};
    Combine m_combineAlpha {};

    SrcReg m_srcRegRgb0 {};
    SrcReg m_srcRegRgb1 {};
    SrcReg m_srcRegRgb2 {};
    SrcReg m_srcRegAlpha0 {};
    SrcReg m_srcRegAlpha1 {};
    SrcReg m_srcRegAlpha2 {};

    Operand m_operandRgb0 {};
    Operand m_operandRgb1 {};
    Operand m_operandRgb2 {};
    Operand m_operandAlpha0 {};
    Operand m_operandAlpha1 {};
    Operand m_operandAlpha2 {};

    uint8_t m_scaleRgb { 0 };
    uint8_t m_scaleAlpha { 0 };

    bool m_enable { false };
};
} // namespace rr::softwarerasterizer

#endif // _TEXENV_HPP_