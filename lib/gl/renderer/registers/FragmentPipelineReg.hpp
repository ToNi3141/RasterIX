// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2023 ToNi3141

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

#ifndef _FRAGMENT_PIPELINE_REG_
#define _FRAGMENT_PIPELINE_REG_

#include "Enums.hpp"
#include <cstdint>

namespace rr
{
class FragmentPipelineReg
{
public:
    static_assert(static_cast<uint32_t>(BlendFunc::ZERO) == 0);
    static_assert(static_cast<uint32_t>(BlendFunc::ONE) == 1);
    static_assert(static_cast<uint32_t>(BlendFunc::DST_COLOR) == 2);
    static_assert(static_cast<uint32_t>(BlendFunc::SRC_COLOR) == 3);
    static_assert(static_cast<uint32_t>(BlendFunc::ONE_MINUS_DST_COLOR) == 4);
    static_assert(static_cast<uint32_t>(BlendFunc::ONE_MINUS_SRC_COLOR) == 5);
    static_assert(static_cast<uint32_t>(BlendFunc::SRC_ALPHA) == 6);
    static_assert(static_cast<uint32_t>(BlendFunc::ONE_MINUS_SRC_ALPHA) == 7);
    static_assert(static_cast<uint32_t>(BlendFunc::DST_ALPHA) == 8);
    static_assert(static_cast<uint32_t>(BlendFunc::ONE_MINUS_DST_ALPHA) == 9);
    static_assert(static_cast<uint32_t>(BlendFunc::SRC_ALPHA_SATURATE) == 10);

    static_assert(static_cast<uint32_t>(LogicOp::CLEAR) == 0);
    static_assert(static_cast<uint32_t>(LogicOp::SET) == 1);
    static_assert(static_cast<uint32_t>(LogicOp::COPY) == 2);
    static_assert(static_cast<uint32_t>(LogicOp::COPY_INVERTED) == 3);
    static_assert(static_cast<uint32_t>(LogicOp::NOOP) == 4);
    static_assert(static_cast<uint32_t>(LogicOp::INVERT) == 5);
    static_assert(static_cast<uint32_t>(LogicOp::AND) == 6);
    static_assert(static_cast<uint32_t>(LogicOp::NAND) == 7);
    static_assert(static_cast<uint32_t>(LogicOp::OR) == 8);
    static_assert(static_cast<uint32_t>(LogicOp::NOR) == 9);
    static_assert(static_cast<uint32_t>(LogicOp::XOR) == 10);
    static_assert(static_cast<uint32_t>(LogicOp::EQUIV) == 11);
    static_assert(static_cast<uint32_t>(LogicOp::AND_REVERSE) == 12);
    static_assert(static_cast<uint32_t>(LogicOp::AND_INVERTED) == 13);
    static_assert(static_cast<uint32_t>(LogicOp::OR_REVERSE) == 14);
    static_assert(static_cast<uint32_t>(LogicOp::OR_INVERTED) == 15);

    static_assert(static_cast<uint32_t>(TestFunc::ALWAYS) == 0);
    static_assert(static_cast<uint32_t>(TestFunc::NEVER) == 1);
    static_assert(static_cast<uint32_t>(TestFunc::LESS) == 2);
    static_assert(static_cast<uint32_t>(TestFunc::EQUAL) == 3);
    static_assert(static_cast<uint32_t>(TestFunc::LEQUAL) == 4);
    static_assert(static_cast<uint32_t>(TestFunc::GREATER) == 5);
    static_assert(static_cast<uint32_t>(TestFunc::NOTEQUAL) == 6);
    static_assert(static_cast<uint32_t>(TestFunc::GEQUAL) == 7);

    FragmentPipelineReg() = default;

    void setDepthFunc(const TestFunc val) { m_regVal.fields.depthFunc = static_cast<uint32_t>(val); }
    void setAlphaFunc(const TestFunc val) { m_regVal.fields.alphaFunc = static_cast<uint32_t>(val); }
    void setRefAlphaValue(const uint8_t val) { m_regVal.fields.referenceAlphaValue = val; }
    void setDepthMask(const bool val) { m_regVal.fields.depthMask = val; }
    void setColorMaskA(const bool val) { m_regVal.fields.colorMaskA = val; }
    void setColorMaskR(const bool val) { m_regVal.fields.colorMaskR = val; }
    void setColorMaskG(const bool val) { m_regVal.fields.colorMaskG = val; }
    void setColorMaskB(const bool val) { m_regVal.fields.colorMaskB = val; }
    void setBlendFuncSFactor(const BlendFunc val) { m_regVal.fields.blendFuncSFactor = static_cast<uint32_t>(val); }
    void setBlendFuncDFactor(const BlendFunc val) { m_regVal.fields.blendFuncDFactor = static_cast<uint32_t>(val); }
    void setLogicOp(const LogicOp val) { m_regVal.fields.logicOp = static_cast<uint32_t>(val); }

    TestFunc getDepthFunc() const { return static_cast<TestFunc>(m_regVal.fields.depthFunc); }
    TestFunc getAlphaFunc() const { return static_cast<TestFunc>(m_regVal.fields.alphaFunc); }
    uint8_t getRefAlphaValue() const { return m_regVal.fields.referenceAlphaValue; }
    bool getDepthMask() const { return m_regVal.fields.depthMask; }
    bool getColorMaskA() const { return m_regVal.fields.colorMaskA; }
    bool getColorMaskR() const { return m_regVal.fields.colorMaskR; }
    bool getColorMaskG() const { return m_regVal.fields.colorMaskG; }
    bool getColorMaskB() const { return m_regVal.fields.colorMaskB; }
    BlendFunc getBlendFuncSFactor() const { return static_cast<BlendFunc>(m_regVal.fields.blendFuncSFactor); }
    BlendFunc getBlendFuncDFactor() const { return static_cast<BlendFunc>(m_regVal.fields.blendFuncDFactor); }
    LogicOp getLogicOp() const { return static_cast<LogicOp>(m_regVal.fields.logicOp); }

    uint32_t serialize() const { return m_regVal.data; }
    void deserialize(const uint32_t data) { m_regVal.data = data; }
    static constexpr uint32_t getAddr() { return 0x3; }

private:
    union RegVal
    {
#pragma pack(push, 1)
        struct RegContent
        {
            RegContent()
                : depthFunc { static_cast<uint32_t>(TestFunc::LESS) }
                , alphaFunc { static_cast<uint32_t>(TestFunc::ALWAYS) }
                , referenceAlphaValue { 0xff }
                , depthMask { false }
                , colorMaskA { true }
                , colorMaskB { true }
                , colorMaskG { true }
                , colorMaskR { true }
                , blendFuncSFactor { static_cast<uint32_t>(BlendFunc::ONE) }
                , blendFuncDFactor { static_cast<uint32_t>(BlendFunc::ZERO) }
                , logicOp { static_cast<uint32_t>(LogicOp::COPY) }
            {
            }

            uint32_t depthFunc : 3;
            uint32_t alphaFunc : 3;
            uint32_t referenceAlphaValue : 8;
            uint32_t depthMask : 1;
            uint32_t colorMaskA : 1;
            uint32_t colorMaskB : 1;
            uint32_t colorMaskG : 1;
            uint32_t colorMaskR : 1;
            uint32_t blendFuncSFactor : 4;
            uint32_t blendFuncDFactor : 4;
            uint32_t logicOp : 4;
        } fields {};
        uint32_t data;
#pragma pack(pop)
    } m_regVal;
};
} // namespace rr

#endif // _FRAGMENT_PIPELINE_REG_
