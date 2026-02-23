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
#include "math/Veci.hpp"
#include <cstdint>

namespace rr::softwarerasterizer
{
class BlendFunc
{
public:
    // Function pointer type for blend factor calculation
    using BlendFactorFn = Vec4i16 (*)(const Vec4i16&, const Vec4i16&);

    Vec4i16 blend(const Vec4i16& src, const Vec4i16& dst) const
    {
        if (!m_enable)
            return src;

        const Vec4i16 srcFactor = m_sFactorFn(src, dst);
        const Vec4i16 dstFactor = m_dFactorFn(src, dst);
        Vec4i16 result = src * srcFactor + dst * dstFactor;
        result.clamp(Vec4i16::Zero, Vec4i16::One);

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
    static Vec4i16 blendZero(const Vec4i16&, const Vec4i16&)
    {
        return Vec4i16 { Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero };
    }

    static Vec4i16 blendOne(const Vec4i16&, const Vec4i16&)
    {
        return Vec4i16 { Vec4i16::One, Vec4i16::One, Vec4i16::One, Vec4i16::One };
    }

    static Vec4i16 blendSrcColor(const Vec4i16& src, const Vec4i16&)
    {
        return src;
    }

    static Vec4i16 blendDstColor(const Vec4i16&, const Vec4i16& dst)
    {
        return dst;
    }

    static Vec4i16 blendOneMinusSrcColor(const Vec4i16& src, const Vec4i16&)
    {
        return Vec4i16 { static_cast<Vec4i16::Type>(Vec4i16::One - src[0]),
            static_cast<Vec4i16::Type>(Vec4i16::One - src[1]),
            static_cast<Vec4i16::Type>(Vec4i16::One - src[2]),
            static_cast<Vec4i16::Type>(Vec4i16::One - src[3]) };
    }

    static Vec4i16 blendOneMinusDstColor(const Vec4i16&, const Vec4i16& dst)
    {
        return Vec4i16 { static_cast<Vec4i16::Type>(Vec4i16::One - dst[0]),
            static_cast<Vec4i16::Type>(Vec4i16::One - dst[1]),
            static_cast<Vec4i16::Type>(Vec4i16::One - dst[2]),
            static_cast<Vec4i16::Type>(Vec4i16::One - dst[3]) };
    }

    static Vec4i16 blendSrcAlpha(const Vec4i16& src, const Vec4i16&)
    {
        return Vec4i16 { src[3], src[3], src[3], src[3] };
    }

    static Vec4i16 blendDstAlpha(const Vec4i16&, const Vec4i16& dst)
    {
        return Vec4i16 { dst[3], dst[3], dst[3], dst[3] };
    }

    static Vec4i16 blendOneMinusSrcAlpha(const Vec4i16& src, const Vec4i16&)
    {
        Vec4i16::Type a = Vec4i16::One - src[3];
        return Vec4i16 { a, a, a, a };
    }

    static Vec4i16 blendOneMinusDstAlpha(const Vec4i16&, const Vec4i16& dst)
    {
        Vec4i16::Type a = Vec4i16::One - dst[3];
        return Vec4i16 { a, a, a, a };
    }

    static Vec4i16 blendSrcAlphaSaturate(const Vec4i16& src, const Vec4i16& dst)
    {
        Vec4i16::Type f = std::min(src[3], static_cast<Vec4i16::Type>(Vec4i16::One - dst[3]));
        return Vec4i16 { f, f, f, Vec4i16::One };
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
