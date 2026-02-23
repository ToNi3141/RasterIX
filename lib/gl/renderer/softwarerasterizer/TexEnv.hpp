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
#include "math/Veci.hpp"
#include <cstdint>

namespace rr::softwarerasterizer
{

class TexEnv
{
public:
    Vec4i16 apply(
        const Vec4i16& previousColor,
        const Vec4i16& texSrcColor,
        const Vec4i16& primaryColor) const;

    void setEnvColor(const Vec4i16& color)
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
        m_scaleRgb = val;
    }

    void setShiftAlpha(const uint8_t val)
    {
        m_scaleAlpha = val;
    }

    void setEnable(bool enable)
    {
        m_enable = enable;
    }

private:
    float selectSrcAlpha(
        const SrcReg& srcReg,
        const Vec4i16& texture,
        const Vec4i16& constant,
        const Vec4i16& primaryColor,
        const Vec4i16& previous) const
    {
        switch (srcReg)
        {
        case SrcReg::TEXTURE:
            return texture[3];
        case SrcReg::CONSTANT:
            return constant[3];
        case SrcReg::PRIMARY_COLOR:
            return primaryColor[3];
        case SrcReg::PREVIOUS:
            return previous[3];
        default:
            return Vec4i16::Zero;
        }
    }

    const Vec4i16& selectSrc(
        const SrcReg& srcReg,
        const Vec4i16& texture,
        const Vec4i16& constant,
        const Vec4i16& primaryColor,
        const Vec4i16& previous) const
    {
        switch (srcReg)
        {
        case SrcReg::TEXTURE:
            return texture;
        case SrcReg::CONSTANT:
            return constant;
        case SrcReg::PRIMARY_COLOR:
            return primaryColor;
        case SrcReg::PREVIOUS:
        default:
            return previous;
        }
    }

    Vec3i16 selectRgbOperand(const Operand& operand, const Vec4i16& color) const
    {
        switch (operand)
        {
        case Operand::SRC_ALPHA:
            return Vec3i16 { color[3], color[3], color[3] };
        case Operand::ONE_MINUS_SRC_ALPHA:
            return Vec3i16 {
                static_cast<Vec3i16::Type>(Vec4i16::One - color[3]),
                static_cast<Vec3i16::Type>(Vec4i16::One - color[3]),
                static_cast<Vec3i16::Type>(Vec4i16::One - color[3])
            };
        case Operand::SRC_COLOR:
            return Vec3i16 { color[0], color[1], color[2] };
        case Operand::ONE_MINUS_SRC_COLOR:
            return Vec3i16 {
                static_cast<Vec3i16::Type>(Vec4i16::One - color[0]),
                static_cast<Vec3i16::Type>(Vec4i16::One - color[1]),
                static_cast<Vec3i16::Type>(Vec4i16::One - color[2])
            };
        default:
            return Vec3i16 { Vec3i16::Zero, Vec3i16::Zero, Vec3i16::Zero };
        }
    }

    Vec3i16::Type selectAlphaOperand(const Operand& operand, const Vec3i16::Type& color) const
    {
        switch (operand)
        {
        case Operand::SRC_ALPHA:
            return color;
        case Operand::ONE_MINUS_SRC_ALPHA:
            return Vec3i16::One - color;
        default:
            return Vec3i16::Zero;
        }
    }

    Vec3i16 combineRgb(
        const Combine& combine,
        const Vec3i16& op0,
        const Vec3i16& op1,
        const Vec3i16& op2) const
    {
        Vec3i16 result {};
        switch (combine)
        {
        case Combine::REPLACE:
            result = op0;
            break;
        case Combine::MODULATE:
            result = op0;
            result *= op1;
            break;
        case Combine::ADD:
            result = op0;
            result += op1;
            break;
        case Combine::ADD_SIGNED:
            result = op0;
            result += op1;
            result -= Vec3i16 { Vec3i16::Half, Vec3i16::Half, Vec3i16::Half };
            break;
        case Combine::INTERPOLATE:
            result = op0 + (op1 - op2) * op2;
            break;
        case Combine::SUBTRACT:
            result = op0;
            result -= op1;
            break;
        case Combine::DOT3_RGB:
        case Combine::DOT3_RGBA:
        {
            const Vec3i16::Type dot = (op0 - Vec3i16 { Vec3i16::Half, Vec3i16::Half, Vec3i16::Half }).dot(op1 - Vec3i16 { Vec3i16::Half, Vec3i16::Half, Vec3i16::Half }) << 2;
            result = Vec3i16 { dot, dot, dot };
        }
        break;
        default:
            break;
        }
        return result;
    }

    float combineAlpha(
        const Combine& combine,
        const Vec3i16::Type& op0,
        const Vec3i16::Type& op1,
        const Vec3i16::Type& op2) const
    {
        float result {};
        switch (combine)
        {
        case Combine::REPLACE:
            result = op0;
            break;
        case Combine::MODULATE:
            result = (static_cast<Vec3i16::Type>(op0) * op1) >> Vec3i16::Shift;
            break;
        case Combine::ADD:
            result = op0 + op1;
            break;
        case Combine::ADD_SIGNED:
            result = op0 + op1 - Vec3i16::Half;
            break;
        case Combine::INTERPOLATE:
            result = op0 + ((static_cast<Vec3i16::Type>(op1 - op2) * op2) >> Vec3i16::Shift);
            break;
        case Combine::SUBTRACT:
            result = op0 - op1;
            break;
        default:
            break;
        }
        return result;
    }

    Vec4i16 m_envColor {};

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

    Vec3i16::Type m_scaleRgb { Vec3i16::One };
    Vec3i16::Type m_scaleAlpha { Vec3i16::One };

    bool m_enable { false };
};
} // namespace rr::softwarerasterizer

#endif // _TEXENV_HPP_