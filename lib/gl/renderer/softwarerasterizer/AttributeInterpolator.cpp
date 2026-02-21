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
    const TriangleStreamTypes::TriangleDescX& attributesData,
    const int32_t boundingBoxX,
    const int32_t boundingBoxY) const
{
    const int32_t bbx = boundingBoxX;
    const int32_t bby = boundingBoxY;
    const int32_t bbxMipMap = bbx + 1;
    const int32_t bbyMipMap = bby + 1;
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
    const int32_t ooDepthW = interpolateAttribute(
        attributesData.param.depthZw[1],
        attributesData.param.depthZwXInc[1],
        attributesData.param.depthZwYInc[1],
        bbx,
        bby);
    const int32_t depthZ = interpolateAttribute(
        attributesData.param.depthZw[0],
        attributesData.param.depthZwXInc[0],
        attributesData.param.depthZwYInc[0],
        bbx,
        bby);
    // Color RGBA
    const int32_t colorR = interpolateAttribute(
        attributesData.param.color[0],
        attributesData.param.colorXInc[0],
        attributesData.param.colorYInc[0],
        bbx,
        bby);
    const int32_t colorG = interpolateAttribute(
        attributesData.param.color[1],
        attributesData.param.colorXInc[1],
        attributesData.param.colorYInc[1],
        bbx,
        bby);
    const int32_t colorB = interpolateAttribute(
        attributesData.param.color[2],
        attributesData.param.colorXInc[2],
        attributesData.param.colorYInc[2],
        bbx,
        bby);
    const int32_t colorA = interpolateAttribute(
        attributesData.param.color[3],
        attributesData.param.colorXInc[3],
        attributesData.param.colorYInc[3],
        bbx,
        bby);

    const float colorRfloat = static_cast<float>(colorR) / static_cast<float>(1 << 24);
    const float colorGfloat = static_cast<float>(colorG) / static_cast<float>(1 << 24);
    const float colorBfloat = static_cast<float>(colorB) / static_cast<float>(1 << 24);
    const float colorAfloat = static_cast<float>(colorA) / static_cast<float>(1 << 24);

    const float depthZfloat = static_cast<float>(depthZ) / static_cast<float>(1 << 30);

    const int32_t depthW = static_cast<int32_t>(1 << 30) / (ooDepthW >> 15); // S1.30 / Sx.15 -> Sx.15
    const float depthWfloat = static_cast<float>(depthW) / static_cast<float>(1 << 15);

    return {
        textures,
        textureMipmap,
        depthWfloat,
        std::clamp(depthZfloat, 0.0f, 1.0f),
        Vec4 { std::clamp(colorRfloat, 0.0f, 1.0f), std::clamp(colorGfloat, 0.0f, 1.0f), std::clamp(colorBfloat, 0.0f, 1.0f), std::clamp(colorAfloat, 0.0f, 1.0f) },
    };
}

} // namespace rr::softwarerasterizer