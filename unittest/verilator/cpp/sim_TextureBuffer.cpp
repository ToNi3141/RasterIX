// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2026 ToNi3141

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
#include <array>
#include <cstdint>

#include "VTextureBuffer.h"

void requestTexels(VTextureBuffer* top,
    const std::array<uint32_t, 4>& addresses,
    const std::array<uint16_t, 4>& expected)
{
    top->texelAddr00 = addresses[0];
    top->texelAddr01 = addresses[1];
    top->texelAddr10 = addresses[2];
    top->texelAddr11 = addresses[3];
    top->texelAddrValid = 1;
    REQUIRE(top->texelAddrReady == 1);
    rr::ut::clk(top);
    top->texelAddrValid = 0;

    for (int cycle = 0; cycle < 3 && !top->texelOutputValid; ++cycle)
    {
        rr::ut::clk(top);
    }

    REQUIRE(top->texelOutputValid == 1);
    REQUIRE(top->texelOutput00 == expected[0]);
    REQUIRE(top->texelOutput01 == expected[1]);
    REQUIRE(top->texelOutput10 == expected[2]);
    REQUIRE(top->texelOutput11 == expected[3]);

    rr::ut::clk(top);
    REQUIRE(top->texelOutputValid == 0);
}

TEST_CASE("Read repeated texture addresses independently", "[TextureBuffer]")
{
    VTextureBuffer* top = rr::ut::makeTop<VTextureBuffer>();
    rr::ut::reset(top);
    top->texelOutputReady = 1;
    top->texelAddrValid = 0;

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 0;
    top->s_axis_tdata = 0x22221111;
    rr::ut::clk(top);
    top->s_axis_tlast = 1;
    top->s_axis_tdata = 0x44443333;
    rr::ut::clk(top);
    top->s_axis_tvalid = 0;
    top->s_axis_tlast = 0;

    requestTexels(top, { 0, 0, 2, 2 }, { 0x1111, 0x1111, 0x3333, 0x3333 });
    requestTexels(top, { 0, 1, 0, 1 }, { 0x1111, 0x2222, 0x1111, 0x2222 });
    requestTexels(top, { 3, 3, 3, 3 }, { 0x4444, 0x4444, 0x4444, 0x4444 });

    delete top;
}