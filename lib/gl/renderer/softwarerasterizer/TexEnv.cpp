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

Vec4 TexEnv::apply(
    const Vec4& previousColorf,
    const Vec4& texSrcColorf,
    const Vec4& primaryColorf) const
{
    const Vec4i16 previousColor = Vec4i16::createFromVecToInt<Vec4, 8>(previousColorf);
    const Vec4i16 texSrcColor = Vec4i16::createFromVecToInt<Vec4, 8>(texSrcColorf);
    const Vec4i16 primaryColor = Vec4i16::createFromVecToInt<Vec4, 8>(primaryColorf);
    if (!m_enable)
    {
        return Vec4 { static_cast<float>(previousColor[0]) / static_cast<float>(Vec4i16::One),
            static_cast<float>(previousColor[1]) / static_cast<float>(Vec4i16::One),
            static_cast<float>(previousColor[2]) / static_cast<float>(Vec4i16::One),
            static_cast<float>(previousColor[3]) / static_cast<float>(Vec4i16::One) };
    }

    const Vec4i16 constantColor = m_envColor;

    // Select source colors
    const Vec4i16& srcColorRgb0 = selectSrc(m_srcRegRgb0, texSrcColor, constantColor, primaryColor, previousColor);
    const Vec4i16& srcColorRgb1 = selectSrc(m_srcRegRgb1, texSrcColor, constantColor, primaryColor, previousColor);
    const Vec4i16& srcColorRgb2 = selectSrc(m_srcRegRgb2, texSrcColor, constantColor, primaryColor, previousColor);

    const Vec3i16::Type srcColorAlpha0 = selectSrcAlpha(m_srcRegAlpha0, texSrcColor, constantColor, primaryColor, previousColor);
    const Vec3i16::Type srcColorAlpha1 = selectSrcAlpha(m_srcRegAlpha1, texSrcColor, constantColor, primaryColor, previousColor);
    const Vec3i16::Type srcColorAlpha2 = selectSrcAlpha(m_srcRegAlpha2, texSrcColor, constantColor, primaryColor, previousColor);

    // Select operands
    const Vec3i16 operandRgb0 = selectRgbOperand(m_operandRgb0, srcColorRgb0);
    const Vec3i16 operandRgb1 = selectRgbOperand(m_operandRgb1, srcColorRgb1);
    const Vec3i16 operandRgb2 = selectRgbOperand(m_operandRgb2, srcColorRgb2);

    const Vec3i16::Type operandAlpha0 = selectAlphaOperand(m_operandAlpha0, srcColorAlpha0);
    const Vec3i16::Type operandAlpha1 = selectAlphaOperand(m_operandAlpha1, srcColorAlpha1);
    const Vec3i16::Type operandAlpha2 = selectAlphaOperand(m_operandAlpha2, srcColorAlpha2);

    Vec3i16 resultRgb = combineRgb(
        m_combineRgb,
        operandRgb0,
        operandRgb1,
        operandRgb2);
    Vec3i16::Type resultAlpha = combineAlpha(
        m_combineAlpha,
        operandAlpha0,
        operandAlpha1,
        operandAlpha2);

    // Apply scale
    resultRgb <<= m_scaleRgb;
    resultAlpha <<= m_scaleAlpha;

    resultRgb.clamp(Vec3i16::Zero, Vec3i16::One);
    resultAlpha = std::clamp(resultAlpha, Vec3i16::Zero, Vec3i16::One);

    if (m_combineRgb == Combine::DOT3_RGB || m_combineRgb == Combine::DOT3_RGBA)
    {
        resultAlpha = resultRgb[0];
    }

    return Vec4 { static_cast<float>(resultRgb[0]) / static_cast<float>(Vec4i16::One),
        static_cast<float>(resultRgb[1]) / static_cast<float>(Vec4i16::One),
        static_cast<float>(resultRgb[2]) / static_cast<float>(Vec4i16::One),
        static_cast<float>(resultAlpha) / static_cast<float>(Vec4i16::One) };
}

} // namespace rr::softwarerasterizer
