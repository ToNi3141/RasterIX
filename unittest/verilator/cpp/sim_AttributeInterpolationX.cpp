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
#include "VAttributeInterpolationX.h"

static constexpr uint8_t RR_CMD_INIT { 0 };
static constexpr uint8_t RR_CMD_X_INC { 1 };
static constexpr uint8_t RR_CMD_X_DEC { 2 };
static constexpr uint8_t RR_CMD_Y_INC { 3 };

void init(VAttributeInterpolationX* top)
{
    top->ce = 1;
    top->tex0_s = 0;
    top->tex0_t = 100;
    top->tex0_q = 200;
    top->tex0_s_inc_x = 1;
    top->tex0_t_inc_x = 2;
    top->tex0_q_inc_x = 3;
    top->tex0_s_inc_y = 4;
    top->tex0_t_inc_y = 5;
    top->tex0_q_inc_y = 6;
    top->tex1_s = 300;
    top->tex1_t = 400;
    top->tex1_q = 500;
    top->tex1_s_inc_x = 7;
    top->tex1_t_inc_x = 8;
    top->tex1_q_inc_x = 9;
    top->tex1_s_inc_y = 10;
    top->tex1_t_inc_y = 11;
    top->tex1_q_inc_y = 12;
    top->depth_w = 600;
    top->depth_w_inc_x = 13;
    top->depth_w_inc_y = 14;
    top->depth_z = 700;
    top->depth_z_inc_x = 15;
    top->depth_z_inc_y = 16;
    top->color_r = 800;
    top->color_g = 900;
    top->color_b = 1000;
    top->color_a = 1100;
    top->color_r_inc_x = 17;
    top->color_g_inc_x = 18;
    top->color_b_inc_x = 19;
    top->color_a_inc_x = 20;
    top->color_r_inc_y = 21;
    top->color_g_inc_y = 22;
    top->color_b_inc_y = 23;
    top->color_a_inc_y = 24;

    top->valid = 1;
    top->cmd = RR_CMD_INIT;

    rr::ut::clk(top);
}

TEST_CASE("Check init", "[AttributeInterpolationX]")
{
    VAttributeInterpolationX* top = new VAttributeInterpolationX();
    rr::ut::reset(top);

    init(top);

    CHECK(top->curr_tex0_s == 0);
    CHECK(top->curr_tex0_t == 100);
    CHECK(top->curr_tex0_q == 200);

    CHECK(top->curr_tex0_mipmap_s == 5);
    CHECK(top->curr_tex0_mipmap_t == 107);
    CHECK(top->curr_tex0_mipmap_q == 209);

    CHECK(top->curr_tex1_s == 300);
    CHECK(top->curr_tex1_t == 400);
    CHECK(top->curr_tex1_q == 500);

    CHECK(top->curr_tex1_mipmap_s == 317);
    CHECK(top->curr_tex1_mipmap_t == 419);
    CHECK(top->curr_tex1_mipmap_q == 521);

    CHECK(top->curr_depth_w == 600);

    CHECK(top->curr_depth_z == 700);

    CHECK(top->curr_color_r == 800);
    CHECK(top->curr_color_g == 900);
    CHECK(top->curr_color_b == 1000);
    CHECK(top->curr_color_a == 1100);

    // Destroy model
    delete top;
}

TEST_CASE("Check x inc", "[AttributeInterpolationX]")
{
    VAttributeInterpolationX* top = new VAttributeInterpolationX();
    rr::ut::reset(top);

    init(top);

    top->cmd = RR_CMD_X_INC;
    rr::ut::clk(top);

    CHECK(top->curr_tex0_s == 0 + 1);
    CHECK(top->curr_tex0_t == 100 + 2);
    CHECK(top->curr_tex0_q == 200 + 3);

    CHECK(top->curr_tex0_mipmap_s == 5 + 1);
    CHECK(top->curr_tex0_mipmap_t == 107 + 2);
    CHECK(top->curr_tex0_mipmap_q == 209 + 3);

    CHECK(top->curr_tex1_s == 300 + 7);
    CHECK(top->curr_tex1_t == 400 + 8);
    CHECK(top->curr_tex1_q == 500 + 9);

    CHECK(top->curr_tex1_mipmap_s == 317 + 7);
    CHECK(top->curr_tex1_mipmap_t == 419 + 8);
    CHECK(top->curr_tex1_mipmap_q == 521 + 9);

    CHECK(top->curr_depth_w == 600 + 13);

    CHECK(top->curr_depth_z == 700 + 15);

    CHECK(top->curr_color_r == 800 + 17);
    CHECK(top->curr_color_g == 900 + 18);
    CHECK(top->curr_color_b == 1000 + 19);
    CHECK(top->curr_color_a == 1100 + 20);

    // Destroy model
    delete top;
}

TEST_CASE("Check x dec", "[AttributeInterpolationX]")
{
    VAttributeInterpolationX* top = new VAttributeInterpolationX();
    rr::ut::reset(top);

    init(top);

    top->cmd = RR_CMD_X_DEC;
    rr::ut::clk(top);

    CHECK(top->curr_tex0_s == 0 - 1);
    CHECK(top->curr_tex0_t == 100 - 2);
    CHECK(top->curr_tex0_q == 200 - 3);

    CHECK(top->curr_tex0_mipmap_s == 5 - 1);
    CHECK(top->curr_tex0_mipmap_t == 107 - 2);
    CHECK(top->curr_tex0_mipmap_q == 209 - 3);

    CHECK(top->curr_tex1_s == 300 - 7);
    CHECK(top->curr_tex1_t == 400 - 8);
    CHECK(top->curr_tex1_q == 500 - 9);

    CHECK(top->curr_tex1_mipmap_s == 317 - 7);
    CHECK(top->curr_tex1_mipmap_t == 419 - 8);
    CHECK(top->curr_tex1_mipmap_q == 521 - 9);

    CHECK(top->curr_depth_w == 600 - 13);

    CHECK(top->curr_depth_z == 700 - 15);

    CHECK(top->curr_color_r == 800 - 17);
    CHECK(top->curr_color_g == 900 - 18);
    CHECK(top->curr_color_b == 1000 - 19);
    CHECK(top->curr_color_a == 1100 - 20);

    // Destroy model
    delete top;
}

TEST_CASE("Check y inc", "[AttributeInterpolationX]")
{
    VAttributeInterpolationX* top = new VAttributeInterpolationX();
    rr::ut::reset(top);

    init(top);

    top->cmd = RR_CMD_Y_INC;
    rr::ut::clk(top);

    CHECK(top->curr_tex0_s == 0 + 4);
    CHECK(top->curr_tex0_t == 100 + 5);
    CHECK(top->curr_tex0_q == 200 + 6);

    CHECK(top->curr_tex0_mipmap_s == 5 + 4);
    CHECK(top->curr_tex0_mipmap_t == 107 + 5);
    CHECK(top->curr_tex0_mipmap_q == 209 + 6);

    CHECK(top->curr_tex1_s == 300 + 10);
    CHECK(top->curr_tex1_t == 400 + 11);
    CHECK(top->curr_tex1_q == 500 + 12);

    CHECK(top->curr_tex1_mipmap_s == 317 + 10);
    CHECK(top->curr_tex1_mipmap_t == 419 + 11);
    CHECK(top->curr_tex1_mipmap_q == 521 + 12);

    CHECK(top->curr_depth_w == 600 + 14);

    CHECK(top->curr_depth_z == 700 + 16);

    CHECK(top->curr_color_r == 800 + 21);
    CHECK(top->curr_color_g == 900 + 22);
    CHECK(top->curr_color_b == 1000 + 23);
    CHECK(top->curr_color_a == 1100 + 24);

    // Destroy model
    delete top;
}

TEST_CASE("Check stall", "[AttributeInterpolationX]")
{
    VAttributeInterpolationX* top = new VAttributeInterpolationX();
    rr::ut::reset(top);

    init(top);

    top->cmd = RR_CMD_Y_INC;
    top->ce = 0;
    rr::ut::clk(top);

    CHECK(top->curr_tex0_s == 0);
    CHECK(top->curr_tex0_t == 100);
    CHECK(top->curr_tex0_q == 200);

    CHECK(top->curr_tex0_mipmap_s == 5);
    CHECK(top->curr_tex0_mipmap_t == 107);
    CHECK(top->curr_tex0_mipmap_q == 209);

    CHECK(top->curr_tex1_s == 300);
    CHECK(top->curr_tex1_t == 400);
    CHECK(top->curr_tex1_q == 500);

    CHECK(top->curr_tex1_mipmap_s == 317);
    CHECK(top->curr_tex1_mipmap_t == 419);
    CHECK(top->curr_tex1_mipmap_q == 521);

    CHECK(top->curr_depth_w == 600);

    CHECK(top->curr_depth_z == 700);

    CHECK(top->curr_color_r == 800);
    CHECK(top->curr_color_g == 900);
    CHECK(top->curr_color_b == 1000);
    CHECK(top->curr_color_a == 1100);

    top->ce = 1;
    rr::ut::clk(top);

    CHECK(top->curr_tex0_s == 0 + 4);
    CHECK(top->curr_tex0_t == 100 + 5);
    CHECK(top->curr_tex0_q == 200 + 6);

    CHECK(top->curr_tex0_mipmap_s == 5 + 4);
    CHECK(top->curr_tex0_mipmap_t == 107 + 5);
    CHECK(top->curr_tex0_mipmap_q == 209 + 6);

    CHECK(top->curr_tex1_s == 300 + 10);
    CHECK(top->curr_tex1_t == 400 + 11);
    CHECK(top->curr_tex1_q == 500 + 12);

    CHECK(top->curr_tex1_mipmap_s == 317 + 10);
    CHECK(top->curr_tex1_mipmap_t == 419 + 11);
    CHECK(top->curr_tex1_mipmap_q == 521 + 12);

    CHECK(top->curr_depth_w == 600 + 14);

    CHECK(top->curr_depth_z == 700 + 16);

    CHECK(top->curr_color_r == 800 + 21);
    CHECK(top->curr_color_g == 900 + 22);
    CHECK(top->curr_color_b == 1000 + 23);
    CHECK(top->curr_color_a == 1100 + 24);

    // Destroy model
    delete top;
}