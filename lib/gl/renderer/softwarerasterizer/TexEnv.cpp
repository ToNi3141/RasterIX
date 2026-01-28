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

} // namespace rr::softwarerasterizer
