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
#include "math/ColorTypes.hpp"
#include <cstdint>

namespace rr::softwarerasterizer
{

class TexEnv
{
public:
    Vec4iColorRGBA apply(
        const Vec4iColorRGBA& previousColor,
        const Vec4iColorRGBA& texSrcColor,
        const Vec4iColorRGBA& primaryColor) const;

    void setEnvColor(const Vec4iColorRGBA& color)
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
    Vec4iColorRGBA::Type selectSrcAlpha(
        const SrcReg& srcReg,
        const Vec4iColorRGBA& texture,
        const Vec4iColorRGBA& constant,
        const Vec4iColorRGBA& primaryColor,
        const Vec4iColorRGBA& previous) const
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
            return Vec4iColorRGBA::Zero;
        }
    }

    const Vec4iColorRGBA& selectSrc(
        const SrcReg& srcReg,
        const Vec4iColorRGBA& texture,
        const Vec4iColorRGBA& constant,
        const Vec4iColorRGBA& primaryColor,
        const Vec4iColorRGBA& previous) const
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

    Vec3iColorRGB selectRgbOperand(const Operand& operand, const Vec4iColorRGBA& color) const
    {
        switch (operand)
        {
        case Operand::SRC_ALPHA:
            return Vec3iColorRGB { color[3], color[3], color[3] };
        case Operand::ONE_MINUS_SRC_ALPHA:
            return Vec3iColorRGB {
                static_cast<Vec3iColorRGB::Type>(Vec4iColorRGBA::FracMax - color[3]),
                static_cast<Vec3iColorRGB::Type>(Vec4iColorRGBA::FracMax - color[3]),
                static_cast<Vec3iColorRGB::Type>(Vec4iColorRGBA::FracMax - color[3])
            };
        case Operand::SRC_COLOR:
            return Vec3iColorRGB { color[0], color[1], color[2] };
        case Operand::ONE_MINUS_SRC_COLOR:
            return Vec3iColorRGB {
                static_cast<Vec3iColorRGB::Type>(Vec4iColorRGBA::FracMax - color[0]),
                static_cast<Vec3iColorRGB::Type>(Vec4iColorRGBA::FracMax - color[1]),
                static_cast<Vec3iColorRGB::Type>(Vec4iColorRGBA::FracMax - color[2])
            };
        default:
            return Vec3iColorRGB { Vec3iColorRGB::Zero, Vec3iColorRGB::Zero, Vec3iColorRGB::Zero };
        }
    }

    Vec3iColorRGB::Type selectAlphaOperand(const Operand& operand, const Vec3iColorRGB::Type& color) const
    {
        switch (operand)
        {
        case Operand::SRC_ALPHA:
            return color;
        case Operand::ONE_MINUS_SRC_ALPHA:
            return Vec3iColorRGB::FracMax - color;
        default:
            return Vec3iColorRGB::Zero;
        }
    }

    Vec3iColorRGB combineRgb(
        const Combine& combine,
        const Vec3iColorRGB& op0,
        const Vec3iColorRGB& op1,
        const Vec3iColorRGB& op2) const
    {
        Vec3iColorRGB result {};
        switch (combine)
        {
        case Combine::REPLACE:
            result = op0;
            break;
        case Combine::MODULATE:
            result = op0 * op1;
            break;
        case Combine::ADD:
            result = op0 + op1;
            break;
        case Combine::ADD_SIGNED:
            result = op0 + op1 - Vec3iColorRGB { Vec3iColorRGB::Half, Vec3iColorRGB::Half, Vec3iColorRGB::Half };
            break;
        case Combine::INTERPOLATE:
            result = op0 * op2 + op1 * (Vec3iColorRGB { Vec3iColorRGB::FracMax, Vec3iColorRGB::FracMax, Vec3iColorRGB::FracMax } - op2);
            break;
        case Combine::SUBTRACT:
            result = op0 - op1;
            break;
        case Combine::DOT3_RGB:
        case Combine::DOT3_RGBA:
        {
            const Vec3iColorRGB::Type dot = (op0 - Vec3iColorRGB { Vec3iColorRGB::Half, Vec3iColorRGB::Half, Vec3iColorRGB::Half })
                                                .dot(op1 - Vec3iColorRGB { Vec3iColorRGB::Half, Vec3iColorRGB::Half, Vec3iColorRGB::Half })
                << 2;
            result = Vec3iColorRGB { dot, dot, dot };
        }
        break;
        default:
            break;
        }
        return result;
    }

    Vec1iColorR combineAlpha(
        const Combine& combine,
        const Vec1iColorR& op0,
        const Vec1iColorR& op1,
        const Vec1iColorR& op2) const
    {
        Vec1iColorR result;
        switch (combine)
        {
        case Combine::REPLACE:
            result = op0;
            break;
        case Combine::MODULATE:
            result = op0 * op1;
            break;
        case Combine::ADD:
            result = op0 + op1;
            break;
        case Combine::ADD_SIGNED:
            result = op0 + op1 - Vec1iColorR { Vec1iColorR::Half };
            break;
        case Combine::INTERPOLATE:
            result = (op0 * op2) + (op1 * (Vec1iColorR { Vec1iColorR::FracMax } - op2));
            break;
        case Combine::SUBTRACT:
            result = op0 - op1;
            break;
        default:
            result = Vec1iColorR { Vec1iColorR::Zero };
            break;
        }
        return result;
    }

    Vec4iColorRGBA m_envColor {};

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