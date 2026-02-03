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

#ifndef _BLENDFUNC_HPP_
#define _BLENDFUNC_HPP_

#include "Enums.hpp"
#include "math/Vec.hpp"
#include <cstdint>

namespace rr::softwarerasterizer
{
class BlendFunc
{
public:
    // Function pointer type for blend factor calculation
    using BlendFactorFn = Vec4 (*)(const Vec4&, const Vec4&);

    Vec4 blend(const Vec4& src, const Vec4& dst) const
    {
        if (!m_enable)
            return src;

        const Vec4 srcFactor = m_sFactorFn(src, dst);
        const Vec4 dstFactor = m_dFactorFn(src, dst);
        Vec4 result = src * srcFactor + dst * dstFactor;
        result.clamp(0.0f, 1.0f);
        return result;
    }

    void setEnable(const bool enable)
    {
        m_enable = enable;
    }

    void setSFactor(const rr::BlendFunc sFactor)
    {
        m_sFactorFn = getBlendFactorFn(sFactor);
    }

    void setDFactor(const rr::BlendFunc dFactor)
    {
        m_dFactorFn = getBlendFactorFn(dFactor);
    }

private:
    // Static blend factor functions; no switch in hot path
    static Vec4 blendZero(const Vec4&, const Vec4&)
    {
        return Vec4 { 0.0f, 0.0f, 0.0f, 0.0f };
    }

    static Vec4 blendOne(const Vec4&, const Vec4&)
    {
        return Vec4 { 1.0f, 1.0f, 1.0f, 1.0f };
    }

    static Vec4 blendSrcColor(const Vec4& src, const Vec4&)
    {
        return src;
    }

    static Vec4 blendDstColor(const Vec4&, const Vec4& dst)
    {
        return dst;
    }

    static Vec4 blendOneMinusSrcColor(const Vec4& src, const Vec4&)
    {
        return Vec4 { 1.0f - src[0], 1.0f - src[1], 1.0f - src[2], 1.0f - src[3] };
    }

    static Vec4 blendOneMinusDstColor(const Vec4&, const Vec4& dst)
    {
        return Vec4 { 1.0f - dst[0], 1.0f - dst[1], 1.0f - dst[2], 1.0f - dst[3] };
    }

    static Vec4 blendSrcAlpha(const Vec4& src, const Vec4&)
    {
        return Vec4 { src[3], src[3], src[3], src[3] };
    }

    static Vec4 blendDstAlpha(const Vec4&, const Vec4& dst)
    {
        return Vec4 { dst[3], dst[3], dst[3], dst[3] };
    }

    static Vec4 blendOneMinusSrcAlpha(const Vec4& src, const Vec4&)
    {
        const float a = 1.0f - src[3];
        return Vec4 { a, a, a, a };
    }

    static Vec4 blendOneMinusDstAlpha(const Vec4&, const Vec4& dst)
    {
        const float a = 1.0f - dst[3];
        return Vec4 { a, a, a, a };
    }

    static Vec4 blendSrcAlphaSaturate(const Vec4& src, const Vec4& dst)
    {
        const float f = std::min(src[3], 1.0f - dst[3]);
        return Vec4 { f, f, f, 1.0f };
    }

    static BlendFactorFn getBlendFactorFn(const rr::BlendFunc factor)
    {
        switch (factor)
        {
        case rr::BlendFunc::ZERO:
            return &blendZero;
        case rr::BlendFunc::ONE:
            return &blendOne;
        case rr::BlendFunc::SRC_COLOR:
            return &blendSrcColor;
        case rr::BlendFunc::DST_COLOR:
            return &blendDstColor;
        case rr::BlendFunc::ONE_MINUS_SRC_COLOR:
            return &blendOneMinusSrcColor;
        case rr::BlendFunc::ONE_MINUS_DST_COLOR:
            return &blendOneMinusDstColor;
        case rr::BlendFunc::SRC_ALPHA:
            return &blendSrcAlpha;
        case rr::BlendFunc::DST_ALPHA:
            return &blendDstAlpha;
        case rr::BlendFunc::ONE_MINUS_SRC_ALPHA:
            return &blendOneMinusSrcAlpha;
        case rr::BlendFunc::ONE_MINUS_DST_ALPHA:
            return &blendOneMinusDstAlpha;
        case rr::BlendFunc::SRC_ALPHA_SATURATE:
            return &blendSrcAlphaSaturate;
        default:
            return &blendOne;
        }
    }

    BlendFactorFn m_sFactorFn { &blendOne };
    BlendFactorFn m_dFactorFn { &blendZero };
    bool m_enable { false };
};
} // namespace rr::softwarerasterizer

#endif // _BLENDFUNC_HPP_
