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

#include "AttributeInterpolator.hpp"

#include "math/Vec.hpp"
#include <array>

namespace rr::softwarerasterizer
{

InterpolatedAttributesData AttributeInterpolator::interpolate(
    const TriangleStreamTypes::TriangleDesc& attributesData,
    const int32_t boundingBoxX,
    const int32_t boundingBoxY) const
{
    const float bbx = static_cast<float>(boundingBoxX);
    const float bby = static_cast<float>(boundingBoxY);
    const float bbxMipMap = static_cast<float>(bbx + 1);
    const float bbyMipMap = static_cast<float>(bby + 1);
    std::array<InterpolatedAttributesData::Texture, RenderConfig::TMU_COUNT> textures;
    std::array<InterpolatedAttributesData::Texture, RenderConfig::TMU_COUNT> textureMipmap;
    // Texture 0 (texStq: [S, T, Q])
    for (std::size_t i = 0; i < attributesData.texture.size(); i++)
    {
        if (!m_tmuEnable[i])
            continue;
        textures[i] = interpolateTexture(attributesData.texture[i], bbx, bby);
        textureMipmap[i] = interpolateTexture(attributesData.texture[i], bbxMipMap, bbyMipMap);
    }
    // Depth: depthZw = { Z, W }
    const float depthW = interpolateAttribute(
        attributesData.param.depthZw[1],
        attributesData.param.depthZwXInc[1],
        attributesData.param.depthZwYInc[1],
        bbx,
        bby);
    const float depthZ = interpolateAttribute(
        attributesData.param.depthZw[0],
        attributesData.param.depthZwXInc[0],
        attributesData.param.depthZwYInc[0],
        bbx,
        bby);
    // Color RGBA
    const float colorR = interpolateAttribute(
        attributesData.param.color[0],
        attributesData.param.colorXInc[0],
        attributesData.param.colorYInc[0],
        bbx,
        bby);
    const float colorG = interpolateAttribute(
        attributesData.param.color[1],
        attributesData.param.colorXInc[1],
        attributesData.param.colorYInc[1],
        bbx,
        bby);
    const float colorB = interpolateAttribute(
        attributesData.param.color[2],
        attributesData.param.colorXInc[2],
        attributesData.param.colorYInc[2],
        bbx,
        bby);
    const float colorA = interpolateAttribute(
        attributesData.param.color[3],
        attributesData.param.colorXInc[3],
        attributesData.param.colorYInc[3],
        bbx,
        bby);

    return {
        textures,
        textureMipmap,
        1.0f / depthW,
        depthZ,
        Vec4 { std::clamp(colorR, 0.0f, 1.0f), std::clamp(colorG, 0.0f, 1.0f), std::clamp(colorB, 0.0f, 1.0f), std::clamp(colorA, 0.0f, 1.0f) },
    };
}

} // namespace rr::softwarerasterizer