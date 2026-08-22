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

// Include model header, generated from Verilating "top.v"
#include "VAttributePerspectiveCorrectionX.h"

TEST_CASE("Perspective correction", "[AttributePerspectiveCorrectionX]")
{
    VAttributePerspectiveCorrectionX* top = rr::ut::makeTop<VAttributePerspectiveCorrectionX>();
    rr::ut::reset(top);
    top->ce = 1;

    top->s_attrb_tvalid = 1;
    top->s_attrb_tlast = 0;
    top->s_attrb_tkeep = 1;
    top->s_attrb_tspx = 123;
    top->s_attrb_tspy = 321;
    top->s_attrb_tindex = 100;
    top->s_attrb_tpixel = 1;

    top->tex0_s = 0x200000;
    top->tex0_t = 0x400000;
    top->tex0_q = 0x100000;
    top->tex0_mipmap_s = 0x300000;
    top->tex0_mipmap_t = 0x500000;
    top->tex0_mipmap_q = 0x100000;
    top->tex1_s = 0x6000000;
    top->tex1_t = 0x7000000;
    top->tex1_q = 0x2000000;
    top->tex1_mipmap_s = 0x9000000;
    top->tex1_mipmap_t = 0x10000000;
    top->tex1_mipmap_q = 0x4000000;
    top->depth_w = 0x2000'0000;
    top->depth_z = 0x12340000;
    top->color_r = 0x00080000;
    top->color_g = 0x00090000;
    top->color_b = 0x00010000;
    top->color_a = 0x00110000;

    rr::ut::clk(top);

    top->s_attrb_tvalid = 1;
    top->s_attrb_tlast = 1;
    top->s_attrb_tkeep = 1;
    top->s_attrb_tspx = 223;
    top->s_attrb_tspy = 421;
    top->s_attrb_tindex = 101;
    top->s_attrb_tpixel = 0;

    top->tex0_s = 0x250000;
    top->tex0_t = 0x450000;
    top->tex0_q = 0x110000;
    top->tex0_mipmap_s = 0x350000;
    top->tex0_mipmap_t = 0x550000;
    top->tex0_mipmap_q = 0x120000;
    top->tex1_s = 0x6500000;
    top->tex1_t = 0x7500000;
    top->tex1_q = 0x2100000;
    top->tex1_mipmap_s = 0x9500000;
    top->tex1_mipmap_t = 0x15000000;
    top->tex1_mipmap_q = 0x4100000;
    top->depth_w = 0x1000'0000;
    top->depth_z = 0x22340000;
    top->color_r = 0x00180000;
    top->color_g = 0x00190000;
    top->color_b = 0x00310000;
    top->color_a = 0x00410000;

    rr::ut::clk(top);

    top->s_attrb_tvalid = 0;
    top->s_attrb_tpixel = 0;

    for (uint32_t i = 0; i < 13; i++)
    {
        rr::ut::clk(top);
    }
    // Check stalling
    top->ce = 0;
    for (uint32_t i = 0; i < 14; i++)
    {
        rr::ut::clk(top);
    }
    // Enable pipeline
    top->ce = 1;
    rr::ut::clk(top);

    CHECK(top->m_attrb_tvalid == 1);
    CHECK(top->m_attrb_tpixel == 1);
    CHECK(top->m_attrb_tlast == 0);
    CHECK(top->m_attrb_tkeep == 1);
    CHECK(top->m_attrb_tspx == 123);
    CHECK(top->m_attrb_tspy == 321);
    CHECK(top->m_attrb_tindex == 100);

    CHECK(top->m_attrb_tdepth_w == 0x3f000000);
    CHECK(top->m_attrb_tdepth_z == 0x48d0);
    CHECK(top->m_attrb_ttexture0_s == 0xffff); // 2.0
    CHECK(top->m_attrb_ttexture0_t == 0x1ffff); // 4.0
    CHECK(top->m_attrb_tmipmap0_s == 0x17fff); // 3.0
    CHECK(top->m_attrb_tmipmap0_t == 0x27fff); // 5.0
    CHECK(top->m_attrb_ttexture1_s == 0x17fff); // 3.0
    CHECK(top->m_attrb_ttexture1_t == 0x1bfff); // 3.5
    CHECK(top->m_attrb_tmipmap1_s == 0x11fff); // 2.25
    CHECK(top->m_attrb_tmipmap1_t == 0x1ffff); // 4.0
    CHECK(top->m_attrb_tcolor_r == 0x8);
    CHECK(top->m_attrb_tcolor_g == 0x9);
    CHECK(top->m_attrb_tcolor_b == 0x1);
    CHECK(top->m_attrb_tcolor_a == 0x11);

    rr::ut::clk(top);

    CHECK(top->m_attrb_tvalid == 1);
    CHECK(top->m_attrb_tpixel == 0);
    CHECK(top->m_attrb_tlast == 1);
    CHECK(top->m_attrb_tkeep == 1);
    CHECK(top->m_attrb_tspx == 223);
    CHECK(top->m_attrb_tspy == 421);
    CHECK(top->m_attrb_tindex == 101);

    CHECK(top->m_attrb_tdepth_w == 0x3e800000);
    CHECK(top->m_attrb_tdepth_z == 0x88d0);
    CHECK(top->m_attrb_ttexture0_s == 0x11696); // 2.176470
    CHECK(top->m_attrb_ttexture0_t == 0x20787); // 4.058823
    CHECK(top->m_attrb_tmipmap0_s == 0x178e3); // 2.944444
    CHECK(top->m_attrb_tmipmap0_t == 0x25c71); // 4.722222
    CHECK(top->m_attrb_ttexture1_s == 0x187c1); // 3.060606
    CHECK(top->m_attrb_ttexture1_t == 0x1c5d1); // 3.545454
    CHECK(top->m_attrb_tmipmap1_s == 0x1256a); // 2.292307
    CHECK(top->m_attrb_tmipmap1_t == 0x295a9); // 5.169230
    CHECK(top->m_attrb_tcolor_r == 0x18);
    CHECK(top->m_attrb_tcolor_g == 0x19);
    CHECK(top->m_attrb_tcolor_b == 0x31);
    CHECK(top->m_attrb_tcolor_a == 0x41);

    rr::ut::clk(top);

    CHECK(top->m_attrb_tvalid == 0);
    CHECK(top->m_attrb_tpixel == 0);

    rr::ut::clk(top);

    CHECK(top->m_attrb_tvalid == 0);
    CHECK(top->m_attrb_tpixel == 0);

    // Destroy model
    delete top;
}
