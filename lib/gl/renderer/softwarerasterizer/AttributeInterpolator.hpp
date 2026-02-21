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

#ifndef _ATTRIBUTEINTERPOLATOR_HPP_
#define _ATTRIBUTEINTERPOLATOR_HPP_

#include "FragmentData.hpp"
#include "InterpolatedAttributesData.hpp"
#include "renderer/commands/TriangleStreamTypes.hpp"
#include <algorithm>
#include <cstdint>

namespace rr::softwarerasterizer
{
class AttributeInterpolator
{
public:
    InterpolatedAttributesData interpolate(
        const TriangleStreamTypes::TriangleDescX& attributesData,
        const int32_t boundingBoxX,
        const int32_t boundingBoxY) const;

    void setEnableTMU(const std::size_t tmuIndex, const bool enable)
    {
        m_tmuEnable[tmuIndex] = enable;
    }

private:
    static float interpolateAttribute(
        const int32_t attrStart,
        const int32_t attrIncX,
        const int32_t attrIncY,
        const int32_t bbx,
        const int32_t bby)
    {
        return attrStart + (attrIncX * bbx) + (attrIncY * bby);
    }

    static InterpolatedAttributesData::Texture interpolateTexture(
        const TriangleStreamTypes::TextureX& texture,
        const int32_t bbx,
        const int32_t bby)
    {
        int32_t s = interpolateAttribute(texture.texStq[0], texture.texStqXInc[0], texture.texStqYInc[0], bbx, bby); // S3.28
        int32_t t = interpolateAttribute(texture.texStq[1], texture.texStqXInc[1], texture.texStqYInc[1], bbx, bby); // S3.28
        int32_t q = interpolateAttribute(texture.texStq[2], texture.texStqXInc[2], texture.texStqYInc[2], bbx, bby); // S3.28

        s = s >> 17; // S3.28 -> Sx.11
        t = t >> 17; // S3.28 -> Sx.11
        q = q >> 13; // S3.28 -> Sx.15

        q = (static_cast<int32_t>(1) << 30) / q; // S1.30 / S3.15 -> Sx.15
        s *= q; // Sx.11 * Sx.15 -> Sx.26
        t *= q; // Sx.11 * Sx.15 -> Sx.26
        s = s >> 11; // Sx.26 -> Sx.15
        t = t >> 11; // Sx.26 -> Sx.15

        const auto tmp = InterpolatedAttributesData::Texture {
            static_cast<float>(s) / static_cast<float>(1 << 15),
            static_cast<float>(t) / static_cast<float>(1 << 15),
            static_cast<float>(q) / static_cast<float>(1 << 15)
        };
        return tmp;
    }

    std::array<bool, RenderConfig::TMU_COUNT> m_tmuEnable {};
};

} // namespace rr::softwarerasterizer

#endif // _ATTRIBUTEINTERPOLATOR_HPP_
