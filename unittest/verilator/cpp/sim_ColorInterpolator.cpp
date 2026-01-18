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
#include "VColorInterpolator.h"

TEST_CASE("Check interpolation", "[ColorInterpolator]")
{
    VColorInterpolator* top = new VColorInterpolator();
    rr::ut::reset(top);
    top->ce = 1;

    const uint32_t colorA = 0xffffffff;
    const uint32_t colorB = 0x00000000;

    for (uint32_t i = 0; i < 256; i++)
    {
        top->intensity = (255 - i) << 8;
        top->colorA = colorA;
        top->colorB = colorB;
        rr::ut::clk(top);
        rr::ut::clk(top);

        const uint32_t mixedColor = colorA - (0x01010101 * i);

        REQUIRE(top->mixedColor == mixedColor);
    }

    // Destroy model
    delete top;
}

TEST_CASE("Check pipelining", "[ColorInterpolator]")
{
    VColorInterpolator* top = new VColorInterpolator();
    rr::ut::reset(top);
    top->ce = 1;

    top->colorA = 0xffffffff;
    top->colorB = 0x00000000;

    top->intensity = 0x0;
    rr::ut::clk(top);

    top->intensity = 0x7fff;
    rr::ut::clk(top);
    REQUIRE(top->mixedColor == 0x0);

    top->intensity = 0xffff;
    rr::ut::clk(top);
    REQUIRE(top->mixedColor == 0x7f7f7f7f);

    rr::ut::clk(top);
    REQUIRE(top->mixedColor == 0xffffffff);

    // Destroy model
    delete top;
}

TEST_CASE("Check channels", "[ColorInterpolator]")
{
    VColorInterpolator* top = new VColorInterpolator();
    rr::ut::reset(top);
    top->ce = 1;

    top->intensity = 0x7fff;

    top->colorA = 0xff000000;
    top->colorB = 0x7f000000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->mixedColor == 0xbf000000);

    top->colorA = 0x00ff0000;
    top->colorB = 0x007f0000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->mixedColor == 0x00bf0000);

    top->colorA = 0x0000ff00;
    top->colorB = 0x00007f00;
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->mixedColor == 0x0000bf00);

    top->colorA = 0x000000ff;
    top->colorB = 0x0000007f;
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->mixedColor == 0x000000bf);

    // Destroy model
    delete top;
}

TEST_CASE("Check stalling", "[ColorInterpolator]")
{
    VColorInterpolator* top = new VColorInterpolator();
    rr::ut::reset(top);
    top->ce = 1;

    top->intensity = 0x7fff;

    top->colorA = 0xff000000;
    top->colorB = 0x7f000000;
    rr::ut::clk(top);

    top->colorA = 0;
    top->colorB = 0;
    top->ce = 0;
    rr::ut::clk(top);
    REQUIRE(top->mixedColor != 0xbf000000);

    top->ce = 1;
    rr::ut::clk(top);
    REQUIRE(top->mixedColor == 0xbf000000);

    // Destroy model
    delete top;
}
