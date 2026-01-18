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

#define CATCH_CONFIG_MAIN
#include "../3rdParty/catch.hpp"
#include "Helper.hpp"
#include "renderer/softwarerasterizer/TexEnv.hpp"

using rr::Combine;
using rr::Operand;
using rr::SrcReg;
using rr::Vec3;
using rr::Vec4;
using rr::ut::vec4Approx;

TEST_CASE("TexEnv disabled returns previous color unchanged", "[TexEnv]")
{
    rr::softwarerasterizer::TexEnv texEnv;
    texEnv.setEnable(false);

    Vec4 previousColor { 0.1f, 0.2f, 0.3f, 0.4f };
    Vec4 texSrcColor { 0.5f, 0.6f, 0.7f, 0.8f };
    Vec4 primaryColor { 0.9f, 0.8f, 0.7f, 0.6f };

    Vec4 result = texEnv.apply(previousColor, texSrcColor, primaryColor);

    REQUIRE(vec4Approx(result, previousColor));
}

TEST_CASE("TexEnv Combine REPLACE", "[TexEnv]")
{
    rr::softwarerasterizer::TexEnv texEnv;
    texEnv.setEnable(true);
    texEnv.setCombineRgb(Combine::REPLACE);
    texEnv.setCombineAlpha(Combine::REPLACE);
    texEnv.setSrcRegRgb0(SrcReg::TEXTURE);
    texEnv.setSrcRegAlpha0(SrcReg::TEXTURE);
    texEnv.setOperandRgb0(Operand::SRC_COLOR);
    texEnv.setOperandAlpha0(Operand::SRC_ALPHA);
    texEnv.setShiftRgb(0); // scale = 2^0 = 1
    texEnv.setShiftAlpha(0); // scale = 2^0 = 1

    Vec4 previousColor { 0.1f, 0.2f, 0.3f, 0.4f };
    Vec4 texSrcColor { 0.5f, 0.6f, 0.7f, 0.8f };
    Vec4 primaryColor { 0.9f, 0.8f, 0.7f, 0.6f };

    Vec4 result = texEnv.apply(previousColor, texSrcColor, primaryColor);

    REQUIRE(vec4Approx(result, texSrcColor));
}

TEST_CASE("TexEnv Combine MODULATE", "[TexEnv]")
{
    rr::softwarerasterizer::TexEnv texEnv;
    texEnv.setEnable(true);
    texEnv.setCombineRgb(Combine::MODULATE);
    texEnv.setCombineAlpha(Combine::MODULATE);
    texEnv.setSrcRegRgb0(SrcReg::TEXTURE);
    texEnv.setSrcRegRgb1(SrcReg::PRIMARY_COLOR);
    texEnv.setSrcRegAlpha0(SrcReg::TEXTURE);
    texEnv.setSrcRegAlpha1(SrcReg::PRIMARY_COLOR);
    texEnv.setOperandRgb0(Operand::SRC_COLOR);
    texEnv.setOperandRgb1(Operand::SRC_COLOR);
    texEnv.setOperandAlpha0(Operand::SRC_ALPHA);
    texEnv.setOperandAlpha1(Operand::SRC_ALPHA);
    texEnv.setShiftRgb(0); // scale = 2^0 = 1
    texEnv.setShiftAlpha(0); // scale = 2^0 = 1

    Vec4 previousColor { 0.0f, 0.0f, 0.0f, 0.0f };
    Vec4 texSrcColor { 0.5f, 0.5f, 0.5f, 0.5f };
    Vec4 primaryColor { 0.8f, 0.6f, 0.4f, 0.2f };

    Vec4 result = texEnv.apply(previousColor, texSrcColor, primaryColor);
    // texture * primary = (0.5*0.8, 0.5*0.6, 0.5*0.4, 0.5*0.2) = (0.4, 0.3, 0.2, 0.1)
    Vec4 expected { 0.4f, 0.3f, 0.2f, 0.1f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("TexEnv Combine ADD", "[TexEnv]")
{
    rr::softwarerasterizer::TexEnv texEnv;
    texEnv.setEnable(true);
    texEnv.setCombineRgb(Combine::ADD);
    texEnv.setCombineAlpha(Combine::ADD);
    texEnv.setSrcRegRgb0(SrcReg::TEXTURE);
    texEnv.setSrcRegRgb1(SrcReg::PRIMARY_COLOR);
    texEnv.setSrcRegAlpha0(SrcReg::TEXTURE);
    texEnv.setSrcRegAlpha1(SrcReg::PRIMARY_COLOR);
    texEnv.setOperandRgb0(Operand::SRC_COLOR);
    texEnv.setOperandRgb1(Operand::SRC_COLOR);
    texEnv.setOperandAlpha0(Operand::SRC_ALPHA);
    texEnv.setOperandAlpha1(Operand::SRC_ALPHA);
    texEnv.setShiftRgb(0); // scale = 2^0 = 1
    texEnv.setShiftAlpha(0); // scale = 2^0 = 1

    Vec4 previousColor { 0.0f, 0.0f, 0.0f, 0.0f };
    Vec4 texSrcColor { 0.3f, 0.3f, 0.3f, 0.3f };
    Vec4 primaryColor { 0.2f, 0.4f, 0.2f, 0.2f };

    Vec4 result = texEnv.apply(previousColor, texSrcColor, primaryColor);
    // texture + primary = (0.3+0.2, 0.3+0.4, 0.3+0.2, 0.3+0.2) = (0.5, 0.7, 0.5, 0.5)
    Vec4 expected { 0.5f, 0.7f, 0.5f, 0.5f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("TexEnv Combine SUBTRACT", "[TexEnv]")
{
    rr::softwarerasterizer::TexEnv texEnv;
    texEnv.setEnable(true);
    texEnv.setCombineRgb(Combine::SUBTRACT);
    texEnv.setCombineAlpha(Combine::SUBTRACT);
    texEnv.setSrcRegRgb0(SrcReg::TEXTURE);
    texEnv.setSrcRegRgb1(SrcReg::PRIMARY_COLOR);
    texEnv.setSrcRegAlpha0(SrcReg::TEXTURE);
    texEnv.setSrcRegAlpha1(SrcReg::PRIMARY_COLOR);
    texEnv.setOperandRgb0(Operand::SRC_COLOR);
    texEnv.setOperandRgb1(Operand::SRC_COLOR);
    texEnv.setOperandAlpha0(Operand::SRC_ALPHA);
    texEnv.setOperandAlpha1(Operand::SRC_ALPHA);
    texEnv.setShiftRgb(0); // scale = 2^0 = 1
    texEnv.setShiftAlpha(0); // scale = 2^0 = 1

    Vec4 previousColor { 0.0f, 0.0f, 0.0f, 0.0f };
    Vec4 texSrcColor { 0.8f, 0.8f, 0.8f, 0.8f };
    Vec4 primaryColor { 0.3f, 0.2f, 0.1f, 0.4f };

    Vec4 result = texEnv.apply(previousColor, texSrcColor, primaryColor);
    // texture - primary = (0.8-0.3, 0.8-0.2, 0.8-0.1, 0.8-0.4) = (0.5, 0.6, 0.7, 0.4)
    Vec4 expected { 0.5f, 0.6f, 0.7f, 0.4f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("TexEnv SrcReg selectors", "[TexEnv]")
{
    rr::softwarerasterizer::TexEnv texEnv;
    texEnv.setEnable(true);
    texEnv.setCombineRgb(Combine::REPLACE);
    texEnv.setCombineAlpha(Combine::REPLACE);
    texEnv.setOperandRgb0(Operand::SRC_COLOR);
    texEnv.setOperandAlpha0(Operand::SRC_ALPHA);
    texEnv.setShiftRgb(0); // scale = 2^0 = 1
    texEnv.setShiftAlpha(0); // scale = 2^0 = 1

    Vec4 previousColor { 0.1f, 0.1f, 0.1f, 0.1f };
    Vec4 texSrcColor { 0.2f, 0.2f, 0.2f, 0.2f };
    Vec4 primaryColor { 0.3f, 0.3f, 0.3f, 0.3f };
    Vec4 constantColor { 0.4f, 0.4f, 0.4f, 0.4f };

    SECTION("TEXTURE source")
    {
        texEnv.setSrcRegRgb0(SrcReg::TEXTURE);
        texEnv.setSrcRegAlpha0(SrcReg::TEXTURE);
        Vec4 result = texEnv.apply(previousColor, texSrcColor, primaryColor);
        REQUIRE(vec4Approx(result, texSrcColor));
    }

    SECTION("PRIMARY_COLOR source")
    {
        texEnv.setSrcRegRgb0(SrcReg::PRIMARY_COLOR);
        texEnv.setSrcRegAlpha0(SrcReg::PRIMARY_COLOR);
        Vec4 result = texEnv.apply(previousColor, texSrcColor, primaryColor);
        REQUIRE(vec4Approx(result, primaryColor));
    }

    SECTION("PREVIOUS source")
    {
        texEnv.setSrcRegRgb0(SrcReg::PREVIOUS);
        texEnv.setSrcRegAlpha0(SrcReg::PREVIOUS);
        Vec4 result = texEnv.apply(previousColor, texSrcColor, primaryColor);
        REQUIRE(vec4Approx(result, previousColor));
    }

    SECTION("CONSTANT source")
    {
        texEnv.setSrcRegRgb0(SrcReg::CONSTANT);
        texEnv.setSrcRegAlpha0(SrcReg::CONSTANT);
        texEnv.setEnvColor(constantColor);
        Vec4 result = texEnv.apply(previousColor, texSrcColor, primaryColor);
        REQUIRE(vec4Approx(result, constantColor));
    }
}

TEST_CASE("TexEnv Operand ONE_MINUS_SRC_COLOR", "[TexEnv]")
{
    rr::softwarerasterizer::TexEnv texEnv;
    texEnv.setEnable(true);
    texEnv.setCombineRgb(Combine::REPLACE);
    texEnv.setCombineAlpha(Combine::REPLACE);
    texEnv.setSrcRegRgb0(SrcReg::TEXTURE);
    texEnv.setSrcRegAlpha0(SrcReg::TEXTURE);
    texEnv.setOperandRgb0(Operand::ONE_MINUS_SRC_COLOR);
    texEnv.setOperandAlpha0(Operand::ONE_MINUS_SRC_ALPHA);
    texEnv.setShiftRgb(0); // scale = 2^0 = 1
    texEnv.setShiftAlpha(0); // scale = 2^0 = 1

    Vec4 previousColor { 0.0f, 0.0f, 0.0f, 0.0f };
    Vec4 texSrcColor { 0.3f, 0.4f, 0.5f, 0.6f };
    Vec4 primaryColor { 0.0f, 0.0f, 0.0f, 0.0f };

    Vec4 result = texEnv.apply(previousColor, texSrcColor, primaryColor);
    // 1 - texture = (0.7, 0.6, 0.5, 0.4)
    Vec4 expected { 0.7f, 0.6f, 0.5f, 0.4f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("TexEnv result clamping", "[TexEnv]")
{
    rr::softwarerasterizer::TexEnv texEnv;
    texEnv.setEnable(true);
    texEnv.setCombineRgb(Combine::ADD);
    texEnv.setCombineAlpha(Combine::ADD);
    texEnv.setSrcRegRgb0(SrcReg::TEXTURE);
    texEnv.setSrcRegRgb1(SrcReg::PRIMARY_COLOR);
    texEnv.setSrcRegAlpha0(SrcReg::TEXTURE);
    texEnv.setSrcRegAlpha1(SrcReg::PRIMARY_COLOR);
    texEnv.setOperandRgb0(Operand::SRC_COLOR);
    texEnv.setOperandRgb1(Operand::SRC_COLOR);
    texEnv.setOperandAlpha0(Operand::SRC_ALPHA);
    texEnv.setOperandAlpha1(Operand::SRC_ALPHA);
    texEnv.setShiftRgb(0); // scale = 2^0 = 1
    texEnv.setShiftAlpha(0); // scale = 2^0 = 1

    Vec4 previousColor { 0.0f, 0.0f, 0.0f, 0.0f };
    Vec4 texSrcColor { 0.8f, 0.8f, 0.8f, 0.8f };
    Vec4 primaryColor { 0.8f, 0.8f, 0.8f, 0.8f };

    Vec4 result = texEnv.apply(previousColor, texSrcColor, primaryColor);
    // 0.8 + 0.8 = 1.6 -> clamped to 1.0
    Vec4 expected { 1.0f, 1.0f, 1.0f, 1.0f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("TexEnv scale/shift", "[TexEnv]")
{
    rr::softwarerasterizer::TexEnv texEnv;
    texEnv.setEnable(true);
    texEnv.setCombineRgb(Combine::REPLACE);
    texEnv.setCombineAlpha(Combine::REPLACE);
    texEnv.setSrcRegRgb0(SrcReg::TEXTURE);
    texEnv.setSrcRegAlpha0(SrcReg::TEXTURE);
    texEnv.setOperandRgb0(Operand::SRC_COLOR);
    texEnv.setOperandAlpha0(Operand::SRC_ALPHA);
    texEnv.setShiftRgb(1); // 2^1 = 2x scale
    texEnv.setShiftAlpha(1);

    Vec4 previousColor { 0.0f, 0.0f, 0.0f, 0.0f };
    Vec4 texSrcColor { 0.25f, 0.25f, 0.25f, 0.25f };
    Vec4 primaryColor { 0.0f, 0.0f, 0.0f, 0.0f };

    Vec4 result = texEnv.apply(previousColor, texSrcColor, primaryColor);
    // 0.25 * 2 = 0.5
    Vec4 expected { 0.5f, 0.5f, 0.5f, 0.5f };

    REQUIRE(vec4Approx(result, expected));
}
