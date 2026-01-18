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

using rr::Vec4;
using rr::ut::vec4Approx;

// LogicOp converts to/from 8-bit integers, so needs larger tolerance
constexpr float LOGIC_OP_EPSILON = 0.01f;

TEST_CASE("LogicOp disabled returns source unchanged", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(false);

    Vec4 src { 0.5f, 0.6f, 0.7f, 0.8f };
    Vec4 dst { 0.1f, 0.2f, 0.3f, 0.4f };

    Vec4 result = logicOp.op(src, dst);

    REQUIRE(vec4Approx(result, src, LOGIC_OP_EPSILON));
}

TEST_CASE("LogicOp CLEAR", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::CLEAR);

    Vec4 src { 1.0f, 1.0f, 1.0f, 1.0f };
    Vec4 dst { 0.5f, 0.5f, 0.5f, 0.5f };

    Vec4 result = logicOp.op(src, dst);
    Vec4 expected { 0.0f, 0.0f, 0.0f, 0.0f };

    REQUIRE(vec4Approx(result, expected, LOGIC_OP_EPSILON));
}

TEST_CASE("LogicOp SET", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::SET);

    Vec4 src { 0.0f, 0.0f, 0.0f, 0.0f };
    Vec4 dst { 0.5f, 0.5f, 0.5f, 0.5f };

    Vec4 result = logicOp.op(src, dst);
    Vec4 expected { 1.0f, 1.0f, 1.0f, 1.0f };

    REQUIRE(vec4Approx(result, expected, LOGIC_OP_EPSILON));
}

TEST_CASE("LogicOp COPY", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::COPY);

    Vec4 src { 0.5f, 0.6f, 0.7f, 0.8f };
    Vec4 dst { 0.1f, 0.2f, 0.3f, 0.4f };

    Vec4 result = logicOp.op(src, dst);

    REQUIRE(vec4Approx(result, src, LOGIC_OP_EPSILON));
}

TEST_CASE("LogicOp COPY_INVERTED", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::COPY_INVERTED);

    Vec4 src { 1.0f, 1.0f, 1.0f, 1.0f };
    Vec4 dst { 0.5f, 0.5f, 0.5f, 0.5f };

    Vec4 result = logicOp.op(src, dst);
    Vec4 expected { 0.0f, 0.0f, 0.0f, 0.0f };

    REQUIRE(vec4Approx(result, expected, LOGIC_OP_EPSILON));
}

TEST_CASE("LogicOp NOOP", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::NOOP);

    Vec4 src { 0.5f, 0.6f, 0.7f, 0.8f };
    Vec4 dst { 0.1f, 0.2f, 0.3f, 0.4f };

    Vec4 result = logicOp.op(src, dst);

    REQUIRE(vec4Approx(result, dst, LOGIC_OP_EPSILON));
}

TEST_CASE("LogicOp INVERT", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::INVERT);

    Vec4 src { 0.5f, 0.5f, 0.5f, 0.5f };
    Vec4 dst { 1.0f, 1.0f, 1.0f, 1.0f };

    Vec4 result = logicOp.op(src, dst);
    Vec4 expected { 0.0f, 0.0f, 0.0f, 0.0f };

    REQUIRE(vec4Approx(result, expected, LOGIC_OP_EPSILON));
}

TEST_CASE("LogicOp AND", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::AND);

    // 0xFF & 0x0F = 0x0F
    Vec4 src { 1.0f, 1.0f, 1.0f, 1.0f }; // 0xFF
    Vec4 dst { 60.0f / 255.0f, 60.0f / 255.0f, 60.0f / 255.0f, 60.0f / 255.0f }; // 0x3C

    Vec4 result = logicOp.op(src, dst);
    // 0xFF & 0x3C = 0x3C
    REQUIRE(vec4Approx(result, dst, LOGIC_OP_EPSILON));
}

TEST_CASE("LogicOp OR", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::OR);

    Vec4 src { 0x55 / 255.0f, 0x55 / 255.0f, 0x55 / 255.0f, 0x55 / 255.0f }; // 0x55
    Vec4 dst { 0xAA / 255.0f, 0xAA / 255.0f, 0xAA / 255.0f, 0xAA / 255.0f }; // 0xAA

    Vec4 result = logicOp.op(src, dst);
    // 0x55 | 0xAA = 0xFF
    Vec4 expected { 1.0f, 1.0f, 1.0f, 1.0f };

    REQUIRE(vec4Approx(result, expected, LOGIC_OP_EPSILON));
}

TEST_CASE("LogicOp XOR", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::XOR);

    Vec4 src { 1.0f, 1.0f, 1.0f, 1.0f }; // 0xFF
    Vec4 dst { 1.0f, 1.0f, 1.0f, 1.0f }; // 0xFF

    Vec4 result = logicOp.op(src, dst);
    // 0xFF ^ 0xFF = 0x00
    Vec4 expected { 0.0f, 0.0f, 0.0f, 0.0f };

    REQUIRE(vec4Approx(result, expected, LOGIC_OP_EPSILON));
}

TEST_CASE("LogicOp NAND", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::NAND);

    Vec4 src { 1.0f, 1.0f, 1.0f, 1.0f }; // 0xFF
    Vec4 dst { 1.0f, 1.0f, 1.0f, 1.0f }; // 0xFF

    Vec4 result = logicOp.op(src, dst);
    // ~(0xFF & 0xFF) = ~0xFF = 0x00
    Vec4 expected { 0.0f, 0.0f, 0.0f, 0.0f };

    REQUIRE(vec4Approx(result, expected, LOGIC_OP_EPSILON));
}

TEST_CASE("LogicOp NOR", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::NOR);

    Vec4 src { 0.0f, 0.0f, 0.0f, 0.0f }; // 0x00
    Vec4 dst { 0.0f, 0.0f, 0.0f, 0.0f }; // 0x00

    Vec4 result = logicOp.op(src, dst);
    // ~(0x00 | 0x00) = ~0x00 = 0xFF
    Vec4 expected { 1.0f, 1.0f, 1.0f, 1.0f };

    REQUIRE(vec4Approx(result, expected, LOGIC_OP_EPSILON));
}

TEST_CASE("LogicOp EQUIV", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::EQUIV);

    Vec4 src { 1.0f, 1.0f, 1.0f, 1.0f }; // 0xFF
    Vec4 dst { 1.0f, 1.0f, 1.0f, 1.0f }; // 0xFF

    Vec4 result = logicOp.op(src, dst);
    // ~(0xFF ^ 0xFF) = ~0x00 = 0xFF
    Vec4 expected { 1.0f, 1.0f, 1.0f, 1.0f };

    REQUIRE(vec4Approx(result, expected, LOGIC_OP_EPSILON));
}

TEST_CASE("LogicOp AND_REVERSE", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::AND_REVERSE);

    Vec4 src { 1.0f, 1.0f, 1.0f, 1.0f }; // 0xFF
    Vec4 dst { 0.0f, 0.0f, 0.0f, 0.0f }; // 0x00

    Vec4 result = logicOp.op(src, dst);
    // 0xFF & ~0x00 = 0xFF & 0xFF = 0xFF
    Vec4 expected { 1.0f, 1.0f, 1.0f, 1.0f };

    REQUIRE(vec4Approx(result, expected, LOGIC_OP_EPSILON));
}

TEST_CASE("LogicOp AND_INVERTED", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::AND_INVERTED);

    Vec4 src { 0.0f, 0.0f, 0.0f, 0.0f }; // 0x00
    Vec4 dst { 1.0f, 1.0f, 1.0f, 1.0f }; // 0xFF

    Vec4 result = logicOp.op(src, dst);
    // ~0x00 & 0xFF = 0xFF & 0xFF = 0xFF
    Vec4 expected { 1.0f, 1.0f, 1.0f, 1.0f };

    REQUIRE(vec4Approx(result, expected, LOGIC_OP_EPSILON));
}

TEST_CASE("LogicOp OR_REVERSE", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::OR_REVERSE);

    Vec4 src { 0.0f, 0.0f, 0.0f, 0.0f }; // 0x00
    Vec4 dst { 1.0f, 1.0f, 1.0f, 1.0f }; // 0xFF

    Vec4 result = logicOp.op(src, dst);
    // 0x00 | ~0xFF = 0x00 | 0x00 = 0x00
    Vec4 expected { 0.0f, 0.0f, 0.0f, 0.0f };

    REQUIRE(vec4Approx(result, expected, LOGIC_OP_EPSILON));
}

TEST_CASE("LogicOp OR_INVERTED", "[LogicOp]")
{
    rr::softwarerasterizer::LogicOp logicOp;
    logicOp.setEnable(true);
    logicOp.setLogicOp(rr::LogicOp::OR_INVERTED);

    Vec4 src { 1.0f, 1.0f, 1.0f, 1.0f }; // 0xFF
    Vec4 dst { 0.0f, 0.0f, 0.0f, 0.0f }; // 0x00

    Vec4 result = logicOp.op(src, dst);
    // ~0xFF | 0x00 = 0x00 | 0x00 = 0x00
    Vec4 expected { 0.0f, 0.0f, 0.0f, 0.0f };

    REQUIRE(vec4Approx(result, expected, LOGIC_OP_EPSILON));
}
