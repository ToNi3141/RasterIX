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
#include "renderer/softwarerasterizer/LogicOp.hpp"

using rr::Vec4i16;
using rr::ut::vec4i16Approx;

// LogicOp converts to/from 8-bit integers, so needs larger tolerance for fixed-point
// S8.8 format: tolerance of 2 accounts for rounding in bit operations
constexpr int16_t LOGIC_OP_TOLERANCE = 2;

TEST_CASE("LogicOp disabled returns source unchanged", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(false);

    Vec4i16 src = Vec4i16 { 127, 153, 178, 204 };
    Vec4i16 dst = Vec4i16 { 25, 51, 76, 102 };

    Vec4i16 result = logicOp.op(src, dst);

    REQUIRE(vec4i16Approx(result, src, LOGIC_OP_TOLERANCE));
}

TEST_CASE("LogicOp CLEAR", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::CLEAR);

    Vec4i16 src = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF };
    Vec4i16 dst = Vec4i16 { 127, 127, 127, 127 };

    Vec4i16 result = logicOp.op(src, dst);
    Vec4i16 expected { Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero };

    REQUIRE(vec4i16Approx(result, expected, LOGIC_OP_TOLERANCE));
}

TEST_CASE("LogicOp SET", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::SET);

    Vec4i16 src = Vec4i16 { 0, 0, 0, 0 };
    Vec4i16 dst = Vec4i16 { 127, 127, 127, 127 };

    Vec4i16 result = logicOp.op(src, dst);
    Vec4i16 expected = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF };

    REQUIRE(vec4i16Approx(result, expected, LOGIC_OP_TOLERANCE));
}

TEST_CASE("LogicOp COPY", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::COPY);

    Vec4i16 src = Vec4i16 { 127, 153, 178, 204 };
    Vec4i16 dst = Vec4i16 { 25, 51, 76, 102 };

    Vec4i16 result = logicOp.op(src, dst);

    REQUIRE(vec4i16Approx(result, src, LOGIC_OP_TOLERANCE));
}

TEST_CASE("LogicOp COPY_INVERTED", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::COPY_INVERTED);

    Vec4i16 src = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF };
    Vec4i16 dst = Vec4i16 { 127, 127, 127, 127 };

    Vec4i16 result = logicOp.op(src, dst);
    Vec4i16 expected { Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero };

    REQUIRE(vec4i16Approx(result, expected, LOGIC_OP_TOLERANCE));
}

TEST_CASE("LogicOp NOOP", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::NOOP);

    Vec4i16 src = Vec4i16 { 127, 153, 178, 204 };
    Vec4i16 dst = Vec4i16 { 25, 51, 76, 102 };

    Vec4i16 result = logicOp.op(src, dst);

    REQUIRE(vec4i16Approx(result, dst, LOGIC_OP_TOLERANCE));
}

TEST_CASE("LogicOp INVERT", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::INVERT);

    Vec4i16 src = Vec4i16 { 127, 127, 127, 127 };
    Vec4i16 dst = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF };

    Vec4i16 result = logicOp.op(src, dst);
    Vec4i16 expected { Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero };

    REQUIRE(vec4i16Approx(result, expected, LOGIC_OP_TOLERANCE));
}

TEST_CASE("LogicOp AND", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::AND);

    // 0xFF & 0x3C = 0x3C
    Vec4i16 src { 0xFF, 0xFF, 0xFF, 0xFF }; // 0xFF
    Vec4i16 dst { 0x3C, 0x3C, 0x3C, 0x3C }; // 0x3C

    Vec4i16 result = logicOp.op(src, dst);
    // 0xFF & 0x3C = 0x3C
    REQUIRE(vec4i16Approx(result, dst, LOGIC_OP_TOLERANCE));
}

TEST_CASE("LogicOp OR", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::OR);

    Vec4i16 src = Vec4i16 { 0x55, 0x55, 0x55, 0x55 }; // 0x55
    Vec4i16 dst = Vec4i16 { 0xAA, 0xAA, 0xAA, 0xAA }; // 0xAA

    Vec4i16 result = logicOp.op(src, dst);
    // 0x55 | 0xAA = 0xFF
    Vec4i16 expected = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF };

    REQUIRE(vec4i16Approx(result, expected, LOGIC_OP_TOLERANCE));
}

TEST_CASE("LogicOp XOR", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::XOR);

    Vec4i16 src = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF }; // 0xFF
    Vec4i16 dst = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF }; // 0xFF

    Vec4i16 result = logicOp.op(src, dst);
    // 0xFF ^ 0xFF = 0x00
    Vec4i16 expected { Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero };

    REQUIRE(vec4i16Approx(result, expected, LOGIC_OP_TOLERANCE));
}

TEST_CASE("LogicOp NAND", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::NAND);

    Vec4i16 src = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF }; // 0xFF
    Vec4i16 dst = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF }; // 0xFF

    Vec4i16 result = logicOp.op(src, dst);
    // ~(0xFF & 0xFF) = ~0xFF = 0x00
    Vec4i16 expected { Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero };

    REQUIRE(vec4i16Approx(result, expected, LOGIC_OP_TOLERANCE));
}

TEST_CASE("LogicOp NOR", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::NOR);

    Vec4i16 src { Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero }; // 0x00
    Vec4i16 dst { Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero }; // 0x00

    Vec4i16 result = logicOp.op(src, dst);
    // ~(0x00 | 0x00) = ~0x00 = 0xFF
    Vec4i16 expected = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF };

    REQUIRE(vec4i16Approx(result, expected, LOGIC_OP_TOLERANCE));
}

TEST_CASE("LogicOp EQUIV", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::EQUIV);

    Vec4i16 src = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF }; // 0xFF
    Vec4i16 dst = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF }; // 0xFF

    Vec4i16 result = logicOp.op(src, dst);
    // ~(0xFF ^ 0xFF) = ~0x00 = 0xFF
    Vec4i16 expected = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF };

    REQUIRE(vec4i16Approx(result, expected, LOGIC_OP_TOLERANCE));
}

TEST_CASE("LogicOp AND_REVERSE", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::AND_REVERSE);

    Vec4i16 src = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF }; // 0xFF
    Vec4i16 dst { Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero }; // 0x00

    Vec4i16 result = logicOp.op(src, dst);
    // 0xFF & ~0x00 = 0xFF & 0xFF = 0xFF
    Vec4i16 expected = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF };

    REQUIRE(vec4i16Approx(result, expected, LOGIC_OP_TOLERANCE));
}

TEST_CASE("LogicOp AND_INVERTED", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::AND_INVERTED);

    Vec4i16 src { Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero }; // 0x00
    Vec4i16 dst = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF }; // 0xFF

    Vec4i16 result = logicOp.op(src, dst);
    // ~0x00 & 0xFF = 0xFF & 0xFF = 0xFF
    Vec4i16 expected = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF };

    REQUIRE(vec4i16Approx(result, expected, LOGIC_OP_TOLERANCE));
}

TEST_CASE("LogicOp OR_REVERSE", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::OR_REVERSE);

    Vec4i16 src { Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero }; // 0x00
    Vec4i16 dst = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF }; // 0xFF

    Vec4i16 result = logicOp.op(src, dst);
    // 0x00 | ~0xFF = 0x00 | 0x00 = 0x00
    Vec4i16 expected { Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero };

    REQUIRE(vec4i16Approx(result, expected, LOGIC_OP_TOLERANCE));
}

TEST_CASE("LogicOp OR_INVERTED", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::OR_INVERTED);

    Vec4i16 src = Vec4i16 { 0xFF, 0xFF, 0xFF, 0xFF }; // 0xFF
    Vec4i16 dst { Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero }; // 0x00

    Vec4i16 result = logicOp.op(src, dst);
    // ~0xFF | 0x00 = 0x00 | 0x00 = 0x00
    Vec4i16 expected { Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero, Vec4i16::Zero };

    REQUIRE(vec4i16Approx(result, expected, LOGIC_OP_TOLERANCE));
}
