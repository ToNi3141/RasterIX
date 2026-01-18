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
#include "renderer/softwarerasterizer/StencilOp.hpp"

TEST_CASE("StencilOp disabled returns value unchanged", "[StencilOp]")
{
    rr::softwarerasterizer::StencilOp stencilOp;
    stencilOp.setEnable(false);

    uint8_t val = 5;

    REQUIRE(stencilOp.op(val, true, true) == val);
    REQUIRE(stencilOp.op(val, false, true) == val);
    REQUIRE(stencilOp.op(val, true, false) == val);
}

TEST_CASE("StencilOp KEEP", "[StencilOp]")
{
    rr::softwarerasterizer::StencilOp stencilOp;
    stencilOp.setEnable(true);
    stencilOp.setZPassOp(rr::StencilOp::KEEP);
    stencilOp.setZFailOp(rr::StencilOp::KEEP);
    stencilOp.setFailOp(rr::StencilOp::KEEP);

    uint8_t val = 7;

    REQUIRE(stencilOp.op(val, true, true) == (val & 15));
    REQUIRE(stencilOp.op(val, false, true) == (val & 15));
    REQUIRE(stencilOp.op(val, true, false) == (val & 15));
}

TEST_CASE("StencilOp ZERO", "[StencilOp]")
{
    rr::softwarerasterizer::StencilOp stencilOp;
    stencilOp.setEnable(true);
    stencilOp.setZPassOp(rr::StencilOp::KEEP);
    stencilOp.setZFailOp(rr::StencilOp::KEEP);
    stencilOp.setFailOp(rr::StencilOp::KEEP);

    uint8_t val = 10;

    SECTION("zPassOp = ZERO")
    {
        stencilOp.setZPassOp(rr::StencilOp::ZERO);
        REQUIRE(stencilOp.op(val, true, true) == 0);
    }

    SECTION("zFailOp = ZERO (depth test fails)")
    {
        stencilOp.setZFailOp(rr::StencilOp::ZERO);
        REQUIRE(stencilOp.op(val, false, true) == 0);
    }

    SECTION("failOp = ZERO (stencil test fails)")
    {
        stencilOp.setFailOp(rr::StencilOp::ZERO);
        REQUIRE(stencilOp.op(val, true, false) == 0);
        REQUIRE(stencilOp.op(val, false, false) == 0);
    }
}

TEST_CASE("StencilOp REPLACE", "[StencilOp]")
{
    rr::softwarerasterizer::StencilOp stencilOp;
    stencilOp.setEnable(true);
    stencilOp.setZPassOp(rr::StencilOp::KEEP);
    stencilOp.setZFailOp(rr::StencilOp::KEEP);
    stencilOp.setFailOp(rr::StencilOp::KEEP);
    stencilOp.setRefValue(12);

    uint8_t val = 5;

    SECTION("zPassOp = REPLACE")
    {
        stencilOp.setZPassOp(rr::StencilOp::REPLACE);
        REQUIRE(stencilOp.op(val, true, true) == 12);
    }

    SECTION("zFailOp = REPLACE (depth test fails)")
    {
        stencilOp.setZFailOp(rr::StencilOp::REPLACE);
        REQUIRE(stencilOp.op(val, false, true) == 12);
    }

    SECTION("failOp = REPLACE (stencil test fails)")
    {
        stencilOp.setFailOp(rr::StencilOp::REPLACE);
        REQUIRE(stencilOp.op(val, true, false) == 12);
        REQUIRE(stencilOp.op(val, false, false) == 12);
    }
}

TEST_CASE("StencilOp INCR", "[StencilOp]")
{
    rr::softwarerasterizer::StencilOp stencilOp;
    stencilOp.setEnable(true);
    stencilOp.setZPassOp(rr::StencilOp::KEEP);
    stencilOp.setZFailOp(rr::StencilOp::KEEP);
    stencilOp.setFailOp(rr::StencilOp::KEEP);

    SECTION("zPassOp = INCR")
    {
        stencilOp.setZPassOp(rr::StencilOp::INCR);

        REQUIRE(stencilOp.op(5, true, true) == 6);
        REQUIRE(stencilOp.op(15, true, true) == 15); // Clamped at max
    }

    SECTION("zFailOp = INCR (depth test fails)")
    {
        stencilOp.setZFailOp(rr::StencilOp::INCR);

        REQUIRE(stencilOp.op(5, false, true) == 6);
        REQUIRE(stencilOp.op(15, false, true) == 15); // Clamped at max
    }

    SECTION("failOp = INCR (stencil test fails)")
    {
        stencilOp.setFailOp(rr::StencilOp::INCR);

        REQUIRE(stencilOp.op(5, true, false) == 6);
        REQUIRE(stencilOp.op(5, false, false) == 6);
        REQUIRE(stencilOp.op(15, true, false) == 15); // Clamped at max
    }
}

TEST_CASE("StencilOp INCR_WRAP", "[StencilOp]")
{
    rr::softwarerasterizer::StencilOp stencilOp;
    stencilOp.setEnable(true);
    stencilOp.setZPassOp(rr::StencilOp::KEEP);
    stencilOp.setZFailOp(rr::StencilOp::KEEP);
    stencilOp.setFailOp(rr::StencilOp::KEEP);

    SECTION("zPassOp = INCR_WRAP")
    {
        stencilOp.setZPassOp(rr::StencilOp::INCR_WRAP);

        REQUIRE(stencilOp.op(5, true, true) == 6);
        REQUIRE(stencilOp.op(15, true, true) == 0); // Wraps
    }

    SECTION("zFailOp = INCR_WRAP (depth test fails)")
    {
        stencilOp.setZFailOp(rr::StencilOp::INCR_WRAP);

        REQUIRE(stencilOp.op(5, false, true) == 6);
        REQUIRE(stencilOp.op(15, false, true) == 0); // Wraps
    }

    SECTION("failOp = INCR_WRAP (stencil test fails)")
    {
        stencilOp.setFailOp(rr::StencilOp::INCR_WRAP);

        REQUIRE(stencilOp.op(5, true, false) == 6);
        REQUIRE(stencilOp.op(15, false, false) == 0); // Wraps
    }
}

TEST_CASE("StencilOp DECR", "[StencilOp]")
{
    rr::softwarerasterizer::StencilOp stencilOp;
    stencilOp.setEnable(true);
    stencilOp.setZPassOp(rr::StencilOp::KEEP);
    stencilOp.setZFailOp(rr::StencilOp::KEEP);
    stencilOp.setFailOp(rr::StencilOp::KEEP);

    SECTION("zPassOp = DECR")
    {
        stencilOp.setZPassOp(rr::StencilOp::DECR);

        REQUIRE(stencilOp.op(5, true, true) == 4);
        REQUIRE(stencilOp.op(0, true, true) == 0); // Clamped at 0
    }

    SECTION("zFailOp = DECR (depth test fails)")
    {
        stencilOp.setZFailOp(rr::StencilOp::DECR);

        REQUIRE(stencilOp.op(5, false, true) == 4);
        REQUIRE(stencilOp.op(0, false, true) == 0); // Clamped at 0
    }

    SECTION("failOp = DECR (stencil test fails)")
    {
        stencilOp.setFailOp(rr::StencilOp::DECR);

        REQUIRE(stencilOp.op(5, true, false) == 4);
        REQUIRE(stencilOp.op(0, false, false) == 0); // Clamped at 0
    }
}

TEST_CASE("StencilOp DECR_WRAP", "[StencilOp]")
{
    rr::softwarerasterizer::StencilOp stencilOp;
    stencilOp.setEnable(true);
    stencilOp.setZPassOp(rr::StencilOp::KEEP);
    stencilOp.setZFailOp(rr::StencilOp::KEEP);
    stencilOp.setFailOp(rr::StencilOp::KEEP);

    SECTION("zPassOp = DECR_WRAP")
    {
        stencilOp.setZPassOp(rr::StencilOp::DECR_WRAP);

        REQUIRE(stencilOp.op(5, true, true) == 4);
        REQUIRE(stencilOp.op(0, true, true) == 15); // Wraps
    }

    SECTION("zFailOp = DECR_WRAP (depth test fails)")
    {
        stencilOp.setZFailOp(rr::StencilOp::DECR_WRAP);

        REQUIRE(stencilOp.op(5, false, true) == 4);
        REQUIRE(stencilOp.op(0, false, true) == 15); // Wraps
    }

    SECTION("failOp = DECR_WRAP (stencil test fails)")
    {
        stencilOp.setFailOp(rr::StencilOp::DECR_WRAP);

        REQUIRE(stencilOp.op(5, true, false) == 4);
        REQUIRE(stencilOp.op(0, false, false) == 15); // Wraps
    }
}

TEST_CASE("StencilOp INVERT", "[StencilOp]")
{
    rr::softwarerasterizer::StencilOp stencilOp;
    stencilOp.setEnable(true);
    stencilOp.setZPassOp(rr::StencilOp::KEEP);
    stencilOp.setZFailOp(rr::StencilOp::KEEP);
    stencilOp.setFailOp(rr::StencilOp::KEEP);

    uint8_t val = 0b0101; // 5
    // ~5 & 15 = 0b1010 = 10

    SECTION("zPassOp = INVERT")
    {
        stencilOp.setZPassOp(rr::StencilOp::INVERT);
        REQUIRE(stencilOp.op(val, true, true) == 10);
    }

    SECTION("zFailOp = INVERT (depth test fails)")
    {
        stencilOp.setZFailOp(rr::StencilOp::INVERT);
        REQUIRE(stencilOp.op(val, false, true) == 10);
    }

    SECTION("failOp = INVERT (stencil test fails)")
    {
        stencilOp.setFailOp(rr::StencilOp::INVERT);
        REQUIRE(stencilOp.op(val, true, false) == 10);
        REQUIRE(stencilOp.op(val, false, false) == 10);
    }
}

TEST_CASE("StencilOp selects correct operation based on test results", "[StencilOp]")
{
    rr::softwarerasterizer::StencilOp stencilOp;
    stencilOp.setEnable(true);
    stencilOp.setZPassOp(rr::StencilOp::REPLACE);
    stencilOp.setZFailOp(rr::StencilOp::INCR);
    stencilOp.setFailOp(rr::StencilOp::DECR);
    stencilOp.setRefValue(10);

    uint8_t val = 5;

    SECTION("Both tests pass - uses zPassOp (REPLACE)")
    {
        REQUIRE(stencilOp.op(val, true, true) == 10);
    }

    SECTION("Stencil passes, Z fails - uses zFailOp (INCR)")
    {
        REQUIRE(stencilOp.op(val, false, true) == 6);
    }

    SECTION("Stencil fails - uses failOp (DECR)")
    {
        REQUIRE(stencilOp.op(val, true, false) == 4);
        REQUIRE(stencilOp.op(val, false, false) == 4);
    }
}

TEST_CASE("StencilOp ref value is masked to 4 bits", "[StencilOp]")
{
    rr::softwarerasterizer::StencilOp stencilOp;
    stencilOp.setEnable(true);
    stencilOp.setZPassOp(rr::StencilOp::REPLACE);
    stencilOp.setRefValue(0xFF); // Should be masked to 15

    uint8_t val = 5;
    REQUIRE(stencilOp.op(val, true, true) == 15);
}
