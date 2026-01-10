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

#include "TexEnv.hpp"
#include <algorithm>
#include <cmath>

namespace rr::softwarerasterizer
{

Vec4 TexEnv::apply(
    const Vec4 previousColor,
    const Vec4 texSrcColor,
    const Vec4 primaryColor) const
{
    if (!m_enable)
    {
        return previousColor;
    }

    const Vec4 constantColor = m_envColor;

    // Select source colors
    const Vec3 srcColorRgb0 = selectSrcRgb(m_srcRegRgb0, texSrcColor, constantColor, primaryColor, previousColor);
    const Vec3 srcColorRgb1 = selectSrcRgb(m_srcRegRgb1, texSrcColor, constantColor, primaryColor, previousColor);
    const Vec3 srcColorRgb2 = selectSrcRgb(m_srcRegRgb2, texSrcColor, constantColor, primaryColor, previousColor);

    const float srcColorAlpha0 = selectSrcAlpha(m_srcRegAlpha0, texSrcColor, constantColor, primaryColor, previousColor);
    const float srcColorAlpha1 = selectSrcAlpha(m_srcRegAlpha1, texSrcColor, constantColor, primaryColor, previousColor);
    const float srcColorAlpha2 = selectSrcAlpha(m_srcRegAlpha2, texSrcColor, constantColor, primaryColor, previousColor);

    // Select operands
    const Vec3 operandRgb0 = selectRgbOperand(m_operandRgb0, srcColorRgb0);
    const Vec3 operandRgb1 = selectRgbOperand(m_operandRgb1, srcColorRgb1);
    const Vec3 operandRgb2 = selectRgbOperand(m_operandRgb2, srcColorRgb2);

    const float operandAlpha0 = selectAlphaOperand(m_operandAlpha0, srcColorAlpha0);
    const float operandAlpha1 = selectAlphaOperand(m_operandAlpha1, srcColorAlpha1);
    const float operandAlpha2 = selectAlphaOperand(m_operandAlpha2, srcColorAlpha2);

    Vec3 resultRgb = combineRgb(
        m_combineRgb,
        operandRgb0,
        operandRgb1,
        operandRgb2);
    float resultAlpha = combineAlpha(
        m_combineAlpha,
        operandAlpha0,
        operandAlpha1,
        operandAlpha2);

    // Apply scale
    resultRgb *= m_scaleRgb;
    resultAlpha *= m_scaleAlpha;

    resultRgb.clamp(0.0f, 1.0f);
    resultAlpha = std::clamp(resultAlpha, 0.0f, 1.0f);

    if (m_combineRgb == Combine::DOT3_RGB || m_combineRgb == Combine::DOT3_RGBA)
    {
        resultAlpha = resultRgb[0];
    }

    return Vec4 { resultRgb[0], resultRgb[1], resultRgb[2], resultAlpha };
}

float TexEnv::selectSrcAlpha(
    const SrcReg& srcReg,
    const Vec4& texture,
    const Vec4& constant,
    const Vec4& primaryColor,
    const Vec4& previous) const
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
        return 0.0f;
    }
}

Vec3 TexEnv::selectSrcRgb(
    const SrcReg& srcReg,
    const Vec4& texture,
    const Vec4& constant,
    const Vec4& primaryColor,
    const Vec4& previous) const
{
    switch (srcReg)
    {
    case SrcReg::TEXTURE:
        return Vec3 { texture[0], texture[1], texture[2] };
    case SrcReg::CONSTANT:
        return Vec3 { constant[0], constant[1], constant[2] };
    case SrcReg::PRIMARY_COLOR:
        return Vec3 { primaryColor[0], primaryColor[1], primaryColor[2] };
    case SrcReg::PREVIOUS:
        return Vec3 { previous[0], previous[1], previous[2] };
    default:
        return Vec3 { 0.0f, 0.0f, 0.0f };
    }
}

Vec3 TexEnv::selectRgbOperand(const Operand& operand, const Vec3& color) const
{
    switch (operand)
    {
    case Operand::SRC_ALPHA:
        return Vec3 { color[3], color[3], color[3] };
    case Operand::ONE_MINUS_SRC_ALPHA:
        return Vec3 { 1.0f - color[3], 1.0f - color[3], 1.0f - color[3] };
    case Operand::SRC_COLOR:
        return color;
    case Operand::ONE_MINUS_SRC_COLOR:
        return Vec3 { 1.0f - color[0], 1.0f - color[1], 1.0f - color[2] };
    default:
        return Vec3 { 0.0f, 0.0f, 0.0f };
    }
}

float TexEnv::selectAlphaOperand(const Operand& operand, const float& color) const
{
    switch (operand)
    {
    case Operand::SRC_ALPHA:
        return color;
    case Operand::ONE_MINUS_SRC_ALPHA:
        return 1.0f - color;
    default:
        return 0.0f;
    }
}

Vec3 TexEnv::combineRgb(const Combine& combine, const Vec3& op0, const Vec3& op1, const Vec3& op2) const
{
    Vec3 result {};
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
        result -= Vec3 { 0.5f, 0.5f, 0.5f };
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
        const float dot = (op0 - Vec3 { 0.5f, 0.5f, 0.5f }).dot(op1 - Vec3 { 0.5f, 0.5f, 0.5f }) * 4.0f;
        result = Vec3 { dot, dot, dot };
    }
    break;
    default:
        break;
    }
    return result;
}

float TexEnv::combineAlpha(const Combine& combine, const float& op0, const float& op1, const float& op2) const
{
    float result {};
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
        result = op0 + op1 - 0.5f;
        break;
    case Combine::INTERPOLATE:
        result = op0 + (op1 - op2) * op2;
        break;
    case Combine::SUBTRACT:
        result = op0 - op1;
        break;
    default:
        break;
    }
    return result;
}

} // namespace rr::softwarerasterizer
