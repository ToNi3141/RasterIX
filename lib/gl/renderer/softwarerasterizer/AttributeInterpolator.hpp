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
        const TriangleStreamTypes::TriangleDesc& attributesData,
        const int32_t boundingBoxX,
        const int32_t boundingBoxY) const;

    void setEnableTMU(const std::size_t tmuIndex, const bool enable)
    {
        m_tmuEnable[tmuIndex] = enable;
    }

private:
    static float interpolateAttribute(
        const float attrStart,
        const float attrIncX,
        const float attrIncY,
        const float bbx,
        const float bby)
    {
        return attrStart + (attrIncX * bbx) + (attrIncY * bby);
    }

    static InterpolatedAttributesData::Texture interpolateTexture(
        const TriangleStreamTypes::Texture& texture,
        const float bbx,
        const float bby);

    std::array<bool, RenderConfig::TMU_COUNT> m_tmuEnable {};
};

} // namespace rr::softwarerasterizer

#endif // _ATTRIBUTEINTERPOLATOR_HPP_
