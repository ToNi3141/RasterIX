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
#include "VDisplayController8BitILI9486.h"

void reset(VDisplayController8BitILI9486& t)
{
    t.resetn = 0;

    t.s_axis_tvalid = 0;
    t.s_axis_tlast = 0;
    t.s_axis_tdata = 0;

    rr::ut::clk(&t);

    REQUIRE(t.rst == 0);
    t.resetn = 1;

    rr::ut::clk(&t);

    REQUIRE(t.rst == 1);
}

void initTest(VDisplayController8BitILI9486& t, const bool dc, const uint8_t data, bool last = false)
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
    VDisplayController8BitILI9486* top = new VDisplayController8BitILI9486();
    VDisplayController8BitILI9486& t = *top;

    reset(t);

    t.s_axis_tvalid = 1;
    t.s_axis_tdata = 0xffaa;

    // Initialization sequence for a ILI9486 with 320x480px
    initTest(t, false, 0xF9);
    initTest(t, true, 0x00);
    initTest(t, true, 0x08);
    initTest(t, false, 0xC0);
    initTest(t, true, 0x19);
    initTest(t, true, 0x1A);
    initTest(t, false, 0xC1);
    initTest(t, true, 0x45);
    initTest(t, true, 0X00);
    initTest(t, false, 0xC2);
    initTest(t, true, 0x33);
    initTest(t, false, 0xC5);
    initTest(t, true, 0x00);
    initTest(t, true, 0x28);
    initTest(t, false, 0xB1);
    initTest(t, true, 0x90);
    initTest(t, true, 0x11);
    initTest(t, false, 0xB4);
    initTest(t, true, 0x01);
    initTest(t, false, 0xB6);
    initTest(t, true, 0x00);
    initTest(t, true, 0x42);
    initTest(t, true, 0x3B);
    initTest(t, false, 0xB7);
    initTest(t, true, 0x07);
    initTest(t, false, 0xE0);
    initTest(t, true, 0x1F);
    initTest(t, true, 0x25);
    initTest(t, true, 0x22);
    initTest(t, true, 0x0B);
    initTest(t, true, 0x06);
    initTest(t, true, 0x0A);
    initTest(t, true, 0x4E);
    initTest(t, true, 0xC6);
    initTest(t, true, 0x39);
    initTest(t, true, 0x00);
    initTest(t, true, 0x00);
    initTest(t, true, 0x00);
    initTest(t, true, 0x00);
    initTest(t, true, 0x00);
    initTest(t, true, 0x00);
    initTest(t, false, 0xE1);
    initTest(t, true, 0x1F);
    initTest(t, true, 0x3F);
    initTest(t, true, 0x3F);
    initTest(t, true, 0x0F);
    initTest(t, true, 0x1F);
    initTest(t, true, 0x0F);
    initTest(t, true, 0x46);
    initTest(t, true, 0x49);
    initTest(t, true, 0x31);
    initTest(t, true, 0x05);
    initTest(t, true, 0x09);
    initTest(t, true, 0x03);
    initTest(t, true, 0x1C);
    initTest(t, true, 0x1A);
    initTest(t, true, 0x00);
    initTest(t, false, 0xF1);
    initTest(t, true, 0x36);
    initTest(t, true, 0x04);
    initTest(t, true, 0x00);
    initTest(t, true, 0x3C);
    initTest(t, true, 0x0F);
    initTest(t, true, 0x0F);
    initTest(t, true, 0xA4);
    initTest(t, true, 0x02);
    initTest(t, false, 0xF2);
    initTest(t, true, 0x18);
    initTest(t, true, 0xA3);
    initTest(t, true, 0x12);
    initTest(t, true, 0x02);
    initTest(t, true, 0x32);
    initTest(t, true, 0x12);
    initTest(t, true, 0xFF);
    initTest(t, true, 0x32);
    initTest(t, true, 0x00);
    initTest(t, false, 0xF4);
    initTest(t, true, 0x40);
    initTest(t, true, 0x00);
    initTest(t, true, 0x08);
    initTest(t, true, 0x91);
    initTest(t, true, 0x04);
    initTest(t, false, 0xF8);
    initTest(t, true, 0x21);
    initTest(t, true, 0x04);
    initTest(t, false, 0x36);
    initTest(t, true, 0xC8);
    initTest(t, false, 0x3A);
    initTest(t, true, 0x55);
    initTest(t, false, 0x11); // Exit Sleep
    initTest(t, false, 0x29); // Display on
    initTest(t, false, 0x2a);
    initTest(t, true, 0x00); // 0
    initTest(t, true, 0x00);
    initTest(t, true, 0x01); // 319
    initTest(t, true, 0x3F);
    initTest(t, false, 0x2b);
    initTest(t, true, 0x00); // 0
    initTest(t, true, 0x00);
    initTest(t, true, 0x01); // 479
    initTest(t, true, 0xDF);
    initTest(t, false, 0x2c, true);

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
