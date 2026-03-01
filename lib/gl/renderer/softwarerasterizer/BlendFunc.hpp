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
    using BlendFactorFn = Vec4iColorRGBA (*)(const Vec4iColorRGBA&, const Vec4iColorRGBA&);

    Vec4iColorRGBA blend(const Vec4iColorRGBA& src, const Vec4iColorRGBA& dst) const
    {
        if (!m_enable)
            return src;

        const Vec4iColorRGBA srcFactor = m_sFactorFn(src, dst);
        const Vec4iColorRGBA dstFactor = m_dFactorFn(src, dst);
        Vec4iColorRGBA result = src * srcFactor + dst * dstFactor;
        result.clamp(Vec4iColorRGBA::Zero, Vec4iColorRGBA::FracMax);

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
    static Vec4iColorRGBA blendZero(const Vec4iColorRGBA&, const Vec4iColorRGBA&)
    {
        return Vec4iColorRGBA { Vec4iColorRGBA::Zero, Vec4iColorRGBA::Zero, Vec4iColorRGBA::Zero, Vec4iColorRGBA::Zero };
    }

    static Vec4iColorRGBA blendOne(const Vec4iColorRGBA&, const Vec4iColorRGBA&)
    {
        return Vec4iColorRGBA { Vec4iColorRGBA::FracMax, Vec4iColorRGBA::FracMax, Vec4iColorRGBA::FracMax, Vec4iColorRGBA::FracMax };
    }

    static Vec4iColorRGBA blendSrcColor(const Vec4iColorRGBA& src, const Vec4iColorRGBA&)
    {
        return src;
    }

    static Vec4iColorRGBA blendDstColor(const Vec4iColorRGBA&, const Vec4iColorRGBA& dst)
    {
        return dst;
    }

    static Vec4iColorRGBA blendOneMinusSrcColor(const Vec4iColorRGBA& src, const Vec4iColorRGBA&)
    {
        return Vec4iColorRGBA { static_cast<Vec4iColorRGBA::Type>(Vec4iColorRGBA::FracMax - src[0]),
            static_cast<Vec4iColorRGBA::Type>(Vec4iColorRGBA::FracMax - src[1]),
            static_cast<Vec4iColorRGBA::Type>(Vec4iColorRGBA::FracMax - src[2]),
            static_cast<Vec4iColorRGBA::Type>(Vec4iColorRGBA::FracMax - src[3]) };
    }

    static Vec4iColorRGBA blendOneMinusDstColor(const Vec4iColorRGBA&, const Vec4iColorRGBA& dst)
    {
        return Vec4iColorRGBA { static_cast<Vec4iColorRGBA::Type>(Vec4iColorRGBA::FracMax - dst[0]),
            static_cast<Vec4iColorRGBA::Type>(Vec4iColorRGBA::FracMax - dst[1]),
            static_cast<Vec4iColorRGBA::Type>(Vec4iColorRGBA::FracMax - dst[2]),
            static_cast<Vec4iColorRGBA::Type>(Vec4iColorRGBA::FracMax - dst[3]) };
    }

    static Vec4iColorRGBA blendSrcAlpha(const Vec4iColorRGBA& src, const Vec4iColorRGBA&)
    {
        return Vec4iColorRGBA { src[3], src[3], src[3], src[3] };
    }

    static Vec4iColorRGBA blendDstAlpha(const Vec4iColorRGBA&, const Vec4iColorRGBA& dst)
    {
        return Vec4iColorRGBA { dst[3], dst[3], dst[3], dst[3] };
    }

    static Vec4iColorRGBA blendOneMinusSrcAlpha(const Vec4iColorRGBA& src, const Vec4iColorRGBA&)
    {
        Vec4iColorRGBA::Type a = Vec4iColorRGBA::FracMax - src[3];
        return Vec4iColorRGBA { a, a, a, a };
    }

    static Vec4iColorRGBA blendOneMinusDstAlpha(const Vec4iColorRGBA&, const Vec4iColorRGBA& dst)
    {
        Vec4iColorRGBA::Type a = Vec4iColorRGBA::FracMax - dst[3];
        return Vec4iColorRGBA { a, a, a, a };
    }

    static Vec4iColorRGBA blendSrcAlphaSaturate(const Vec4iColorRGBA& src, const Vec4iColorRGBA& dst)
    {
        Vec4iColorRGBA::Type f = std::min(src[3], static_cast<Vec4iColorRGBA::Type>(Vec4iColorRGBA::FracMax - dst[3]));
        return Vec4iColorRGBA { f, f, f, Vec4iColorRGBA::FracMax };
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
