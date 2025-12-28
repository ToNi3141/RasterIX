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
    AttributeInterpolator(const TriangleStreamTypes::TriangleDesc& attributesData)
        : m_attributesData { attributesData }
    {
    }

    InterpolatedAttributesData interpolate(const int32_t boundingBoxX, const int32_t boundingBoxY)
    {
        const float bbx = static_cast<float>(boundingBoxX);
        const float bby = static_cast<float>(boundingBoxY);
        const float bbxMipMap = static_cast<float>(bbx + 1);
        const float bbyMipMap = static_cast<float>(bby + 1);
        std::array<InterpolatedAttributesData::Texture, RenderConfig::TMU_COUNT> textures;
        std::array<InterpolatedAttributesData::Texture, RenderConfig::TMU_COUNT> textureMipmap;
        // Texture 0 (texStq: [S, T, Q])
        for (std::size_t i = 0; i < m_attributesData.texture.size(); i++)
        {
            textures[i] = interpolateTexture(m_attributesData.texture[i], bbx, bby);
            textureMipmap[i] = interpolateTexture(m_attributesData.texture[i], bbxMipMap, bbyMipMap);
        }
        // Depth: depthZw = { Z, W }
        const float depthW = interpolateAttribute(
            m_attributesData.param.depthZw[1],
            m_attributesData.param.depthZwXInc[1],
            m_attributesData.param.depthZwYInc[1],
            bbx,
            bby);
        const float depthZ = interpolateAttribute(
            m_attributesData.param.depthZw[0],
            m_attributesData.param.depthZwXInc[0],
            m_attributesData.param.depthZwYInc[0],
            bbx,
            bby);
        // Color RGBA
        const float colorR = interpolateAttribute(
            m_attributesData.param.color[0],
            m_attributesData.param.colorXInc[0],
            m_attributesData.param.colorYInc[0],
            bbx,
            bby);
        const float colorG = interpolateAttribute(
            m_attributesData.param.color[1],
            m_attributesData.param.colorXInc[1],
            m_attributesData.param.colorYInc[1],
            bbx,
            bby);
        const float colorB = interpolateAttribute(
            m_attributesData.param.color[2],
            m_attributesData.param.colorXInc[2],
            m_attributesData.param.colorYInc[2],
            bbx,
            bby);
        const float colorA = interpolateAttribute(
            m_attributesData.param.color[3],
            m_attributesData.param.colorXInc[3],
            m_attributesData.param.colorYInc[3],
            bbx,
            bby);

        return {
            .tex = textures,
            .texMipmap = textureMipmap,
            .depthW = 1.0f / depthW,
            .depthZ = depthZ,
            .colorR = std::clamp(colorR, 0.0f, 1.0f),
            .colorG = std::clamp(colorG, 0.0f, 1.0f),
            .colorB = std::clamp(colorB, 0.0f, 1.0f),
            .colorA = std::clamp(colorA, 0.0f, 1.0f),
        };
    }

private:
    float interpolateAttribute(
        const float attrStart,
        const float attrIncX,
        const float attrIncY,
        const float bbx,
        const float bby) const
    {
        return attrStart + (attrIncX * bbx) + (attrIncY * bby);
    }

    InterpolatedAttributesData::Texture interpolateTexture(
        const TriangleStreamTypes::Texture& texture,
        const float bbx,
        const float bby) const
    {
        InterpolatedAttributesData::Texture tex {
            .s = interpolateAttribute(
                texture.texStq[0],
                texture.texStqXInc[0],
                texture.texStqYInc[0],
                bbx,
                bby),
            .t = interpolateAttribute(
                texture.texStq[1],
                texture.texStqXInc[1],
                texture.texStqYInc[1],
                bbx,
                bby),
            .q = interpolateAttribute(
                texture.texStq[2],
                texture.texStqXInc[2],
                texture.texStqYInc[2],
                bbx,
                bby)
        };

        tex.q = 1.0f / tex.q;
        tex.s *= tex.q;
        tex.t *= tex.q;
        return tex;
    }

    const TriangleStreamTypes::TriangleDesc& m_attributesData;
};

} // namespace rr::softwarerasterizer

#endif // _ATTRIBUTEINTERPOLATOR_HPP_
