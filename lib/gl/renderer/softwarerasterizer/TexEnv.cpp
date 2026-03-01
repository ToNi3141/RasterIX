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
#include "math/Veci.hpp"
#include <algorithm>
#include <cmath>

namespace rr::softwarerasterizer
{

Vec4iColorRGBA TexEnv::apply(
    const Vec4iColorRGBA& previousColor,
    const Vec4iColorRGBA& texSrcColor,
    const Vec4iColorRGBA& primaryColor) const
{
    if (!m_enable)
    {
        return previousColor;
    }

    const Vec4iColorRGBA& constantColor = m_envColor;

    // Select source colors
    const Vec4iColorRGBA& srcColorRgb0 = selectSrc(m_srcRegRgb0, texSrcColor, constantColor, primaryColor, previousColor);
    const Vec4iColorRGBA& srcColorRgb1 = selectSrc(m_srcRegRgb1, texSrcColor, constantColor, primaryColor, previousColor);
    const Vec4iColorRGBA& srcColorRgb2 = selectSrc(m_srcRegRgb2, texSrcColor, constantColor, primaryColor, previousColor);

    const Vec3iColorRGB::Type srcColorAlpha0 = selectSrcAlpha(m_srcRegAlpha0, texSrcColor, constantColor, primaryColor, previousColor);
    const Vec3iColorRGB::Type srcColorAlpha1 = selectSrcAlpha(m_srcRegAlpha1, texSrcColor, constantColor, primaryColor, previousColor);
    const Vec3iColorRGB::Type srcColorAlpha2 = selectSrcAlpha(m_srcRegAlpha2, texSrcColor, constantColor, primaryColor, previousColor);

    // Select operands
    const Vec3iColorRGB operandRgb0 = selectRgbOperand(m_operandRgb0, srcColorRgb0);
    const Vec3iColorRGB operandRgb1 = selectRgbOperand(m_operandRgb1, srcColorRgb1);
    const Vec3iColorRGB operandRgb2 = selectRgbOperand(m_operandRgb2, srcColorRgb2);

    const Vec3iColorRGB::Type operandAlpha0 = selectAlphaOperand(m_operandAlpha0, srcColorAlpha0);
    const Vec3iColorRGB::Type operandAlpha1 = selectAlphaOperand(m_operandAlpha1, srcColorAlpha1);
    const Vec3iColorRGB::Type operandAlpha2 = selectAlphaOperand(m_operandAlpha2, srcColorAlpha2);

    Vec3iColorRGB resultRgb = combineRgb(
        m_combineRgb,
        operandRgb0,
        operandRgb1,
        operandRgb2);
    Vec1iColorR resultAlpha = combineAlpha(
        m_combineAlpha,
        { operandAlpha0 },
        { operandAlpha1 },
        { operandAlpha2 });

    // Apply scale
    resultRgb <<= m_scaleRgb;
    resultAlpha <<= m_scaleAlpha;

    resultRgb.clamp(Vec3iColorRGB::Zero, Vec3iColorRGB::FracMax);
    resultAlpha.clamp(Vec1iColorR::Zero, Vec1iColorR::FracMax);

    if (m_combineRgb == Combine::DOT3_RGB || m_combineRgb == Combine::DOT3_RGBA)
    {
        if (m_combineRgb == Combine::DOT3_RGB)
        {
            return Vec4iColorRGBA { resultRgb[0], resultRgb[0], resultRgb[0], resultAlpha[0] };
        }
        return Vec4iColorRGBA { resultRgb[0], resultRgb[0], resultRgb[0], resultRgb[0] };
    }

    return Vec4iColorRGBA { resultRgb[0], resultRgb[1], resultRgb[2], resultAlpha[0] };
}

} // namespace rr::softwarerasterizer
