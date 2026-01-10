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
    Vec4 blend(const Vec4& src, const Vec4& dst) const;

    void setEnable(const bool enable)
    {
        m_enable = enable;
    }

    void setSFactor(const rr::BlendFunc sFactor)
    {
        m_sFactor = sFactor;
    }

    void setDFactor(const rr::BlendFunc dFactor)
    {
        m_dFactor = dFactor;
    }

private:
    Vec4 calcBlendFactor(const Vec4& src, const Vec4& dst, const rr::BlendFunc& factor) const;

    rr::BlendFunc m_sFactor { rr::BlendFunc::ONE };
    rr::BlendFunc m_dFactor { rr::BlendFunc::ZERO };

    bool m_enable { false };
};
} // namespace rr::softwarerasterizer

#endif // _BLENDFUNC_HPP_
