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

// #define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
// #include "../Unittests/3rdParty/catch.hpp"

#include "general.hpp"

// Include model header, generated from Verilating "top.v"
#include "VLogicOp.h"

static constexpr uint8_t CLEAR { 0 };
static constexpr uint8_t SET { 1 };
static constexpr uint8_t COPY { 2 };
static constexpr uint8_t COPY_INVERTED { 3 };
static constexpr uint8_t NOOP { 4 };
static constexpr uint8_t INVERT { 5 };
static constexpr uint8_t AND { 6 };
static constexpr uint8_t NAND { 7 };
static constexpr uint8_t OR { 8 };
static constexpr uint8_t NOR { 9 };
static constexpr uint8_t XOR { 10 };
static constexpr uint8_t EQUIV { 11 };
static constexpr uint8_t AND_REVERSE { 12 };
static constexpr uint8_t AND_INVERTED { 13 };
static constexpr uint8_t OR_REVERSE { 14 };
static constexpr uint8_t OR_INVERTED { 15 };

TEST_CASE("Chip Enable", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = AND;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == 0x00'00'FF'00);

    t->ce = 0;
    t->source = 0xFF'FF'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == 0x00'00'FF'00);

    t->ce = 1;
    rr::ut::clk(t);
    CHECK(t->out == 0xFF'00'FF'00);

    // Destroy model
    delete t;
}

TEST_CASE("Not Enabled", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = OR_INVERTED;
    t->ce = 1;
    t->enable = 0;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == t->source);

    // Destroy model
    delete t;
}

TEST_CASE("OR_INVERTED", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = OR_INVERTED;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == (~t->source | t->dest));

    // Destroy model
    delete t;
}

TEST_CASE("OR_REVERSE", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = OR_REVERSE;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == (t->source | ~t->dest));

    // Destroy model
    delete t;
}

TEST_CASE("AND_INVERTED", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = AND_INVERTED;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == (~t->source & t->dest));

    // Destroy model
    delete t;
}

TEST_CASE("AND_REVERSE", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = AND_REVERSE;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == (t->source & ~t->dest));

    // Destroy model
    delete t;
}

TEST_CASE("EQUIV", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = EQUIV;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == ~(t->source ^ t->dest));

    // Destroy model
    delete t;
}

TEST_CASE("XOR", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = XOR;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == (t->source ^ t->dest));

    // Destroy model
    delete t;
}

TEST_CASE("NOR", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = NOR;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == ~(t->source | t->dest));

    // Destroy model
    delete t;
}

TEST_CASE("OR", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = OR;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == (t->source | t->dest));

    // Destroy model
    delete t;
}

TEST_CASE("NAND", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = NAND;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == ~(t->source & t->dest));

    // Destroy model
    delete t;
}

TEST_CASE("AND", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = AND;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == (t->source & t->dest));

    // Destroy model
    delete t;
}

TEST_CASE("INVERT", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = INVERT;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == ~(t->dest));

    // Destroy model
    delete t;
}

TEST_CASE("NOOP", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = NOOP;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == t->dest);

    // Destroy model
    delete t;
}

TEST_CASE("COPY_INVERTED", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = COPY_INVERTED;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == ~(t->source));

    // Destroy model
    delete t;
}

TEST_CASE("COPY", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = COPY;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == t->source);

    // Destroy model
    delete t;
}

TEST_CASE("SET", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = SET;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == 0xFF'FF'FF'FF);

    // Destroy model
    delete t;
}

TEST_CASE("CLEAR", "[LogicOp]")
{
    VLogicOp* t = new VLogicOp();

    rr::ut::reset(t);

    t->op = CLEAR;
    t->ce = 1;
    t->enable = 1;
    t->source = 0x00'FF'FF'00;
    t->dest = 0xFF'00'FF'00;
    rr::ut::clk(t);
    CHECK(t->out == 0x00'00'00'00);

    // Destroy model
    delete t;
}
