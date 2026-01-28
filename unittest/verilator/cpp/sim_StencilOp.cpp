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

#include "general.hpp"
#include <algorithm>
#include <array>
#include <math.h>

// Include model header, generated from Verilating "top.v"
#include "VStencilOp.h"

static constexpr uint8_t KEEP = 0;
static constexpr uint8_t ZERO = 1;
static constexpr uint8_t REPLACE = 2;
static constexpr uint8_t INCR = 3;
static constexpr uint8_t INCR_WRAP = 4;
static constexpr uint8_t DECR = 5;
static constexpr uint8_t DECR_WRAP = 6;
static constexpr uint8_t INVERT = 7;

TEST_CASE("Test stencil fail op", "[VStencilOp]")
{
    VStencilOp* top = new VStencilOp();
    rr::ut::reset(top);
    top->ce = 1;

    top->opZFail = KEEP;
    top->opZPass = KEEP;
    top->opFail = KEEP;
    top->refVal = 0x5;
    top->enable = 1;

    top->zTest = 1;
    top->sTest = 0;

    top->opFail = KEEP;
    top->sValIn = 1;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 1);
    REQUIRE(top->write == 1);

    top->opFail = ZERO;
    top->sValIn = 2;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 0);
    REQUIRE(top->write == 1);

    top->opFail = REPLACE;
    top->sValIn = 3;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 5);
    REQUIRE(top->write == 1);

    top->opFail = INCR;
    top->sValIn = 4;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 5);
    REQUIRE(top->write == 1);

    top->opFail = INCR_WRAP;
    top->sValIn = 5;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 6);
    REQUIRE(top->write == 1);

    top->opFail = DECR;
    top->sValIn = 6;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 5);
    REQUIRE(top->write == 1);

    top->opFail = DECR_WRAP;
    top->sValIn = 7;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 6);
    REQUIRE(top->write == 1);

    top->opFail = INVERT;
    top->sValIn = 8;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 7);
    REQUIRE(top->write == 1);

    // Destroy model
    delete top;
}

TEST_CASE("Test z fail op", "[VStencilOp]")
{
    VStencilOp* top = new VStencilOp();
    rr::ut::reset(top);
    top->ce = 1;

    top->opZFail = KEEP;
    top->opZPass = KEEP;
    top->opFail = KEEP;
    top->refVal = 0x5;
    top->enable = 1;

    top->zTest = 0;
    top->sTest = 1;

    top->opZFail = KEEP;
    top->sValIn = 1;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 1);
    REQUIRE(top->write == 1);

    top->opZFail = ZERO;
    top->sValIn = 2;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 0);
    REQUIRE(top->write == 1);

    top->opZFail = REPLACE;
    top->sValIn = 3;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 5);
    REQUIRE(top->write == 1);

    top->opZFail = INCR;
    top->sValIn = 4;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 5);
    REQUIRE(top->write == 1);

    top->opZFail = INCR_WRAP;
    top->sValIn = 5;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 6);
    REQUIRE(top->write == 1);

    top->opZFail = DECR;
    top->sValIn = 6;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 5);
    REQUIRE(top->write == 1);

    top->opZFail = DECR_WRAP;
    top->sValIn = 7;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 6);
    REQUIRE(top->write == 1);

    top->opZFail = INVERT;
    top->sValIn = 8;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 7);
    REQUIRE(top->write == 1);

    // Destroy model
    delete top;
}

TEST_CASE("Test z pass op", "[VStencilOp]")
{
    VStencilOp* top = new VStencilOp();
    rr::ut::reset(top);
    top->ce = 1;

    top->opZFail = KEEP;
    top->opZPass = KEEP;
    top->opFail = KEEP;
    top->refVal = 0x5;
    top->enable = 1;

    top->zTest = 1;
    top->sTest = 1;

    top->opZPass = KEEP;
    top->sValIn = 1;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 1);
    REQUIRE(top->write == 1);

    top->opZPass = ZERO;
    top->sValIn = 2;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 0);
    REQUIRE(top->write == 1);

    top->opZPass = REPLACE;
    top->sValIn = 3;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 5);
    REQUIRE(top->write == 1);

    top->opZPass = INCR;
    top->sValIn = 4;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 5);
    REQUIRE(top->write == 1);

    top->opZPass = INCR_WRAP;
    top->sValIn = 5;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 6);
    REQUIRE(top->write == 1);

    top->opZPass = DECR;
    top->sValIn = 6;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 5);
    REQUIRE(top->write == 1);

    top->opZPass = DECR_WRAP;
    top->sValIn = 7;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 6);
    REQUIRE(top->write == 1);

    top->opZPass = INVERT;
    top->sValIn = 8;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 7);
    REQUIRE(top->write == 1);

    // Destroy model
    delete top;
}

TEST_CASE("Test wrapping", "[VStencilOp]")
{
    VStencilOp* top = new VStencilOp();
    rr::ut::reset(top);
    top->ce = 1;

    top->opZFail = KEEP;
    top->opZPass = KEEP;
    top->opFail = KEEP;
    top->refVal = 0x5;
    top->enable = 1;

    top->zTest = 1;
    top->sTest = 0;

    top->opFail = INCR;
    top->sValIn = 0xf;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 0xf);
    REQUIRE(top->write == 1);

    top->opFail = INCR_WRAP;
    top->sValIn = 0xf;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 0);
    REQUIRE(top->write == 1);

    top->opFail = DECR;
    top->sValIn = 0;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 0);
    REQUIRE(top->write == 1);

    top->opFail = DECR_WRAP;
    top->sValIn = 0;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 0xf);
    REQUIRE(top->write == 1);

    // Destroy model
    delete top;
}

TEST_CASE("Check stall", "[VStencilOp]")
{
    VStencilOp* top = new VStencilOp();
    rr::ut::reset(top);
    top->ce = 1;

    top->opZFail = KEEP;
    top->opZPass = KEEP;
    top->opFail = KEEP;
    top->refVal = 0x5;
    top->enable = 1;

    top->zTest = 1;
    top->sTest = 1;

    top->opZPass = KEEP;
    top->sValIn = 1;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 1);
    REQUIRE(top->write == 1);

    top->opZPass = ZERO;
    top->sValIn = 2;
    top->ce = 0;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 1);
    REQUIRE(top->write == 1);

    top->ce = 1;
    rr::ut::clk(top);
    REQUIRE(top->sValOut == 0);
    REQUIRE(top->write == 1);

    // Destroy model
    delete top;
}