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

#ifndef TEXTURE_HPP_
#define TEXTURE_HPP_

#include "Enums.hpp"
#include "math/Vec.hpp"
#include "renderer/Renderer.hpp"
#include <optional>

namespace rr
{
class Texture
{
public:
    Texture(Renderer& renderer);

    bool updateTexture();
    TextureObject& getTexture();
    bool useTexture();
    bool isTextureValid(const uint16_t texId) const { return m_renderer.isTextureValid(texId); };
    std::pair<bool, uint16_t> createTexture() { return m_renderer.createTexture(); }
    bool createTextureWithName(const uint16_t texId) { return m_renderer.createTextureWithName(texId); };
    bool deleteTexture(const uint16_t texture) { return m_renderer.deleteTexture(texture); }
    void setBoundTexture(const uint16_t val);
    void setTexWrapModeS(const TextureWrapMode mode) { m_renderer.setTextureWrapModeS(m_tmu, m_tmuConf[m_tmu].boundTexture, mode); }
    void setTexWrapModeT(const TextureWrapMode mode) { m_renderer.setTextureWrapModeT(m_tmu, m_tmuConf[m_tmu].boundTexture, mode); }
    void setEnableMagFilter(const bool val) { m_renderer.enableTextureMagFiltering(m_tmu, m_tmuConf[m_tmu].boundTexture, val); }
    void setEnableMinFilter(const bool val) { m_renderer.enableTextureMinFiltering(m_tmu, m_tmuConf[m_tmu].boundTexture, val); }

    bool setTexEnvMode(const TexEnvMode mode);
    void setCombineRgb(const Combine val) { texEnv().setCombineRgb(val); }
    void setCombineAlpha(const Combine val) { texEnv().setCombineAlpha(val); }
    void setSrcRegRgb0(const SrcReg val) { texEnv().setSrcRegRgb0(val); }
    void setSrcRegRgb1(const SrcReg val) { texEnv().setSrcRegRgb1(val); }
    void setSrcRegRgb2(const SrcReg val) { texEnv().setSrcRegRgb2(val); }
    void setSrcRegAlpha0(const SrcReg val) { texEnv().setSrcRegAlpha0(val); }
    void setSrcRegAlpha1(const SrcReg val) { texEnv().setSrcRegAlpha1(val); }
    void setSrcRegAlpha2(const SrcReg val) { texEnv().setSrcRegAlpha2(val); }
    void setOperandRgb0(const Operand val) { texEnv().setOperandRgb0(val); }
    void setOperandRgb1(const Operand val) { texEnv().setOperandRgb1(val); }
    void setOperandRgb2(const Operand val) { texEnv().setOperandRgb2(val); }
    void setOperandAlpha0(const Operand val) { texEnv().setOperandAlpha0(val); }
    void setOperandAlpha1(const Operand val) { texEnv().setOperandAlpha1(val); }
    void setOperandAlpha2(const Operand val) { texEnv().setOperandAlpha2(val); }
    void setShiftRgb(const uint8_t val) { texEnv().setShiftRgb(val); }
    void setShiftAlpha(const uint8_t val) { texEnv().setShiftAlpha(val); }
    bool setTexEnvColor(const Vec4& color);

    TexEnvMode getTexEnvMode() const { return m_tmuConf[m_tmu].texEnvMode; }
    Vec4 getTexEnvColor() const;
    Combine getCombineRgb() const { return texEnv().getCombineRgb(); }
    Combine getCombineAlpha() const { return texEnv().getCombineAlpha(); }
    SrcReg getSrcRegRgb0() const { return texEnv().getSrcRegRgb0(); }
    SrcReg getSrcRegRgb1() const { return texEnv().getSrcRegRgb1(); }
    SrcReg getSrcRegRgb2() const { return texEnv().getSrcRegRgb2(); }
    SrcReg getSrcRegAlpha0() const { return texEnv().getSrcRegAlpha0(); }
    SrcReg getSrcRegAlpha1() const { return texEnv().getSrcRegAlpha1(); }
    SrcReg getSrcRegAlpha2() const { return texEnv().getSrcRegAlpha2(); }
    Operand getOperandRgb0() const { return texEnv().getOperandRgb0(); }
    Operand getOperandRgb1() const { return texEnv().getOperandRgb1(); }
    Operand getOperandRgb2() const { return texEnv().getOperandRgb2(); }
    Operand getOperandAlpha0() const { return texEnv().getOperandAlpha0(); }
    Operand getOperandAlpha1() const { return texEnv().getOperandAlpha1(); }
    Operand getOperandAlpha2() const { return texEnv().getOperandAlpha2(); }
    uint8_t getShiftRgb() const { return texEnv().getShiftRgb(); }
    uint8_t getShiftAlpha() const { return texEnv().getShiftAlpha(); }

    void activateTmu(const std::size_t tmu);
    std::size_t getActiveTmu() const { return m_tmu; }

private:
    struct TmuConfig
    {
        // Textures
        uint16_t boundTexture { 0 };

        // TMU
        TexEnvMode texEnvMode { TexEnvMode::REPLACE };
        TexEnvReg texEnvConf {};
        TexEnvReg texEnvConfUploaded {};
    };

    TexEnvReg& texEnv() { return m_tmuConf[m_tmu].texEnvConf; }
    const TexEnvReg& texEnv() const { return m_tmuConf[m_tmu].texEnvConf; }

    Renderer& m_renderer;

    // TMU
    std::array<TmuConfig, RenderConfig::TMU_COUNT> m_tmuConf {};
    std::size_t m_tmu { 0 };
    std::optional<TextureObject> m_textureObject {};
};

} // namespace rr
#endif // TEXTURE_HPP_
