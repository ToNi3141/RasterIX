// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

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

#include "Texture.hpp"

namespace rr
{

Texture::Texture(Renderer& renderer)
    : m_renderer(renderer)
{
    for (std::size_t i = 0; i < m_tmuConf.size(); i++)
    {
        m_tmuConf[i].texEnvConf.setTmu(i);
        m_renderer.setTexEnv(m_tmuConf[i].texEnvConf);
    }
}

bool Texture::updateTexture()
{
    bool ret { true };

    for (std::size_t i = 0; i < m_tmuConf.size(); i++)
    {
        if ((m_tmuConf[i].texEnvMode == TexEnvMode::COMBINE)
            && (m_tmuConf[i].texEnvConfUploaded.serialize() != m_tmuConf[i].texEnvConf.serialize()))
        {
            m_tmuConf[i].texEnvConf.setTmu(i);
            ret = ret && m_renderer.setTexEnv(m_tmuConf[i].texEnvConf);
            m_tmuConf[i].texEnvConfUploaded = m_tmuConf[i].texEnvConf;
        }
    }

    if (m_textureObject)
    {
        ret = m_renderer.updateTexture(m_tmuConf[m_tmu].boundTexture, *m_textureObject);
        m_textureObject = std::nullopt;

        // Rebind texture to update the rasterizer with the new texture meta information
        // TODO: Check if this is still required
        ret = ret && m_renderer.useTexture(m_tmu, m_tmuConf[m_tmu].boundTexture);
    }
    return ret;
}

TextureObject& Texture::getTexture()
{
    if (!m_textureObject)
    {
        m_textureObject = std::make_optional<TextureObject>(m_renderer.getTexture(m_tmuConf[m_tmu].boundTexture));
    }
    return *m_textureObject;
}

bool Texture::useTexture()
{
    updateTexture();
    return m_renderer.useTexture(m_tmu, m_tmuConf[m_tmu].boundTexture);
}

bool Texture::setTexEnvMode(const TexEnvMode mode)
{
    m_tmuConf[m_tmu].texEnvMode = mode;
    TexEnvReg texEnvConf {};
    switch (mode)
    {
    case TexEnvMode::DISABLE:
        texEnvConf.setCombineRgb(Combine::REPLACE);
        texEnvConf.setCombineRgb(Combine::REPLACE);
        texEnvConf.setCombineAlpha(Combine::REPLACE);
        texEnvConf.setSrcRegRgb0(SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegAlpha0(SrcReg::PRIMARY_COLOR);
        if (m_tmu != 0) // Is this the right mode?
        {
            texEnvConf.setSrcRegRgb0(SrcReg::PREVIOUS);
            texEnvConf.setSrcRegAlpha0(SrcReg::PREVIOUS);
        }
        break;
    case TexEnvMode::REPLACE:
        texEnvConf.setCombineRgb(Combine::REPLACE);
        texEnvConf.setCombineAlpha(Combine::REPLACE);
        texEnvConf.setSrcRegRgb0(SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha0(SrcReg::TEXTURE);
        break;
    case TexEnvMode::MODULATE:
        texEnvConf.setCombineRgb(Combine::MODULATE);
        texEnvConf.setCombineAlpha(Combine::MODULATE);
        texEnvConf.setSrcRegRgb0(SrcReg::TEXTURE);
        texEnvConf.setSrcRegRgb1(SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegAlpha0(SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha1(SrcReg::PRIMARY_COLOR);
        if (m_tmu != 0) // Is this the right mode?
        {
            texEnvConf.setSrcRegRgb1(SrcReg::PREVIOUS);
            texEnvConf.setSrcRegAlpha1(SrcReg::PREVIOUS);
        }
        break;
    case TexEnvMode::DECAL:
        texEnvConf.setCombineRgb(Combine::INTERPOLATE);
        texEnvConf.setCombineAlpha(Combine::REPLACE);
        texEnvConf.setSrcRegRgb0(SrcReg::TEXTURE);
        texEnvConf.setSrcRegRgb1(SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegRgb2(SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha0(SrcReg::PRIMARY_COLOR);
        texEnvConf.setOperandRgb2(Operand::SRC_ALPHA);
        if (m_tmu != 0) // Is this the right mode?
        {
            texEnvConf.setSrcRegRgb1(SrcReg::PREVIOUS);
            texEnvConf.setSrcRegAlpha0(SrcReg::PREVIOUS);
        }
        break;
    case TexEnvMode::BLEND:
        texEnvConf.setCombineRgb(Combine::INTERPOLATE);
        texEnvConf.setCombineAlpha(Combine::MODULATE);
        texEnvConf.setSrcRegRgb0(SrcReg::CONSTANT);
        texEnvConf.setSrcRegRgb1(SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegRgb2(SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha0(SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegAlpha1(SrcReg::TEXTURE);
        if (m_tmu != 0) // Is this the right mode?
        {
            texEnvConf.setSrcRegRgb1(SrcReg::PREVIOUS);
            texEnvConf.setSrcRegAlpha0(SrcReg::PREVIOUS);
        }
        break;
    case TexEnvMode::ADD:
        texEnvConf.setCombineRgb(Combine::ADD);
        texEnvConf.setCombineAlpha(Combine::ADD);
        texEnvConf.setSrcRegRgb0(SrcReg::TEXTURE);
        texEnvConf.setSrcRegRgb1(SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegAlpha0(SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha1(SrcReg::PRIMARY_COLOR);
        if (m_tmu != 0) // Is this the right mode?
        {
            texEnvConf.setSrcRegRgb1(SrcReg::PREVIOUS);
            texEnvConf.setSrcRegAlpha1(SrcReg::PREVIOUS);
        }
        break;
    case TexEnvMode::COMBINE:
        // Nothing to do here.
        break;
    default:
        break;
    }
    if (mode != TexEnvMode::COMBINE)
    {
        texEnvConf.setTmu(m_tmu);
        return m_renderer.setTexEnv(texEnvConf);
    }
    return true;
}

bool Texture::setTexEnvColor(const Vec4& color)
{
    return m_renderer.setTexEnvColor(
        {
            m_tmu,
            Vec4i {
                static_cast<uint8_t>(color[0] * 255.0f),
                static_cast<uint8_t>(color[1] * 255.0f),
                static_cast<uint8_t>(color[2] * 255.0f),
                static_cast<uint8_t>(color[3] * 255.0f),
            },
        });
}

Vec4 Texture::getTexEnvColor() const
{
    const Vec4i colorInt = m_renderer.getTexEnvColor(m_tmu);
    return Vec4 {
        static_cast<float>(colorInt[0]) / 255.0f,
        static_cast<float>(colorInt[1]) / 255.0f,
        static_cast<float>(colorInt[2]) / 255.0f,
        static_cast<float>(colorInt[3]) / 255.0f,
    };
}

void Texture::setBoundTexture(const uint16_t val)
{
    updateTexture();
    m_tmuConf[m_tmu].boundTexture = val;
}

void Texture::activateTmu(const std::size_t tmu)
{
    updateTexture();
    m_tmu = tmu;
}
} // namespace rr