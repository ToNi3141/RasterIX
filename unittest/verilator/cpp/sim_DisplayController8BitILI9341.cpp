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

// #define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
// #include "../Unittests/3rdParty/catch.hpp"

#include "general.hpp"

// Include model header, generated from Verilating "top.v"
#include "VDisplayController8BitILI9341.h"

void reset(VDisplayController8BitILI9341& t)
{
    t.resetn = 0;

    t.s_axis_tvalid = 0;
    t.s_axis_tlast = 0;
    t.s_axis_tdata = 0;

    rr::ut::clk(&t);

    t.resetn = 1;

    rr::ut::clk(&t);

    REQUIRE(t.rst == 1);
}

void initTest(VDisplayController8BitILI9341& t, const bool dc, const uint8_t data, bool last = false)
{
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 0);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == dc);
    REQUIRE(t.data == data);
    rr::ut::clk(&t);
    REQUIRE(t.s_axis_tready == last);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 1);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == dc);
    REQUIRE(t.data == data);
    rr::ut::clk(&t);
}

TEST_CASE("Stream data", "[Display]")
{
    VDisplayController8BitILI9341* top = new VDisplayController8BitILI9341();
    VDisplayController8BitILI9341& t = *top;

    reset(t);

    t.s_axis_tvalid = 1;
    t.s_axis_tdata = 0xffaa;

    // Initialization sequence for a ILI9341 with 240x320px
    initTest(t, false, 0x01);
    initTest(t, true, 0);
    initTest(t, false, 0x28);
    initTest(t, true, 0);
    initTest(t, false, 0xC0);
    initTest(t, true, 0x23);
    initTest(t, false, 0xC1);
    initTest(t, true, 0x10);
    initTest(t, false, 0xC5);
    initTest(t, true, 0x2B);
    initTest(t, true, 0x2B);
    initTest(t, false, 0xC7);
    initTest(t, true, 0xC0);
    initTest(t, false, 0x36);
    initTest(t, true, 0x00 | 0x00 | 0x40 | 0x08);
    initTest(t, false, 0x3A);
    initTest(t, true, 0x55);
    initTest(t, false, 0xB1);
    initTest(t, true, 0x00);
    initTest(t, true, 0x1B);
    initTest(t, false, 0xB7);
    initTest(t, true, 0x07);
    initTest(t, false, 0x11);
    initTest(t, true, 0);
    initTest(t, false, 0x29);
    initTest(t, true, 0);
    initTest(t, false, 0x2A);
    initTest(t, true, 0);
    initTest(t, true, 0);
    initTest(t, true, 0x00);
    initTest(t, true, 0xEF);
    initTest(t, false, 0x2B);
    initTest(t, true, 0);
    initTest(t, true, 0);
    initTest(t, true, 0x01);
    initTest(t, true, 0x3F);
    initTest(t, false, 0xF2);
    initTest(t, true, 0x00);
    initTest(t, false, 0x26);
    initTest(t, true, 0x01);
    initTest(t, false, 0xE0);
    initTest(t, true, 0x0F);
    initTest(t, true, 0x31);
    initTest(t, true, 0x2B);
    initTest(t, true, 0x0C);
    initTest(t, true, 0x0E);
    initTest(t, true, 0x08);
    initTest(t, true, 0x4E);
    initTest(t, true, 0xF1);
    initTest(t, true, 0x37);
    initTest(t, true, 0x07);
    initTest(t, true, 0x10);
    initTest(t, true, 0x03);
    initTest(t, true, 0x0E);
    initTest(t, true, 0x09);
    initTest(t, true, 0x00);
    initTest(t, false, 0xE1);
    initTest(t, true, 0x00);
    initTest(t, true, 0x0E);
    initTest(t, true, 0x14);
    initTest(t, true, 0x03);
    initTest(t, true, 0x11);
    initTest(t, true, 0x07);
    initTest(t, true, 0x31);
    initTest(t, true, 0xC1);
    initTest(t, true, 0x48);
    initTest(t, true, 0x08);
    initTest(t, true, 0x0F);
    initTest(t, true, 0x0C);
    initTest(t, true, 0x31);
    initTest(t, true, 0x36);
    initTest(t, true, 0x0F);
    initTest(t, false, 0x2C, true);

    // stream clk 0 fetch
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 1);
    REQUIRE(t.cs == 0);
    // REQUIRE(t.dc == 1);
    // REQUIRE(t.data == 0x08);
    t.s_axis_tvalid = 1;
    t.s_axis_tdata = 0x550f;
    rr::ut::clk(top);
    // stream clk 0 stream
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 0);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == 1);
    REQUIRE(t.data == 0xff);
    rr::ut::clk(top);
    // stream clk 0 stream
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 1);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == 1);
    REQUIRE(t.data == 0xff);
    rr::ut::clk(top);
    // stream clk 0 stream
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 0);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == 1);
    REQUIRE(t.data == 0xaa);
    rr::ut::clk(top);
    // stream clk 0 stream
    REQUIRE(t.s_axis_tready == 1);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 1);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == 1);
    REQUIRE(t.data == 0xaa);
    rr::ut::clk(top);

    // stream clk 1 fetch
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 1);
    REQUIRE(t.cs == 0);
    // REQUIRE(t.dc == 1);
    // REQUIRE(t.data == 0x08);
    rr::ut::clk(top);
    // stream clk 1 stream
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 0);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == 1);
    REQUIRE(t.data == 0x55);
    rr::ut::clk(top);
    // stream clk 1 stream
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 1);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == 1);
    REQUIRE(t.data == 0x55);
    rr::ut::clk(top);
    // stream clk 1 stream
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 0);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == 1);
    REQUIRE(t.data == 0x0f);
    rr::ut::clk(top);
    // stream clk 1 stream
    REQUIRE(t.s_axis_tready == 1);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 1);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == 1);
    REQUIRE(t.data == 0x0f);
    rr::ut::clk(top);

    // Final model cleanup
    top->final();

    // Destroy model
    delete top;
}
