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

#include "BlendFunc.hpp"

namespace rr::softwarerasterizer
{

Vec4 BlendFunc::blend(const Vec4& src, const Vec4& dst) const
{
    if (!m_enable)
        return src;
    const Vec4 srcFactor = calcBlendFactor(src, dst, m_sFactor);
    const Vec4 dstFactor = calcBlendFactor(src, dst, m_dFactor);
    Vec4 result = src * srcFactor + dst * dstFactor;
    result.clamp(0.0f, 1.0f);
    return result;
}

Vec4 BlendFunc::calcBlendFactor(const Vec4& src, const Vec4& dst, const rr::BlendFunc& factor) const
{
    Vec4 color;
    switch (factor)
    {
    case rr::BlendFunc::ZERO:
        color = Vec4 { 0.0f, 0.0f, 0.0f, 0.0f };
        break;
    case rr::BlendFunc::ONE:
        color = Vec4 { 1.0f, 1.0f, 1.0f, 1.0f };
        break;
    case rr::BlendFunc::DST_COLOR:
        color = dst;
        break;
    case rr::BlendFunc::ONE_MINUS_SRC_COLOR:
        color = Vec4 { 1.0f - src[0], 1.0f - src[1], 1.0f - src[2], 1.0f - src[3] };
        break;
    case rr::BlendFunc::ONE_MINUS_DST_COLOR:
        color = Vec4 { 1.0f - dst[0], 1.0f - dst[1], 1.0f - dst[2], 1.0f - dst[3] };
        break;
    case rr::BlendFunc::SRC_ALPHA:
        color = Vec4 { src[3], src[3], src[3], src[3] };
        break;
    case rr::BlendFunc::DST_ALPHA:
        color = Vec4 { dst[3], dst[3], dst[3], dst[3] };
        break;
    case rr::BlendFunc::ONE_MINUS_SRC_ALPHA:
        color = Vec4 { 1.0f - src[3], 1.0f - src[3], 1.0f - src[3], 1.0f - src[3] };
        break;
    case rr::BlendFunc::ONE_MINUS_DST_ALPHA:
        color = Vec4 { 1.0f - dst[3], 1.0f - dst[3], 1.0f - dst[3], 1.0f - dst[3] };
        break;
    case rr::BlendFunc::SRC_ALPHA_SATURATE:
    {
        const float f = (src[3] < 1.0f - dst[3]) ? src[3] : (1.0f - dst[3]);
        color = Vec4 { f, f, f, 1.0f };
        break;
    }
    default:
        color = Vec4 { 1.0f, 1.0f, 1.0f, 1.0f };
        break;
    }
    return color;
}

} // namespace rr::softwarerasterizer
