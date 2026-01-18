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
#include "VAttributeInterpolator.h"

union ScreenPos
{
    uint32_t u32;
    struct
    {
        uint16_t x;
        uint16_t y;
    } val;
};

struct Attributes
{
    float sb0 = 0.0f;
    float sx0 = 0.0f;
    float sy0 = 0.0f;
    float tb0 = 0.0f;
    float tx0 = 0.0f;
    float ty0 = 0.0f;
    float qb0 = 0.0f;
    float qx0 = 0.0f;
    float qy0 = 0.0f;
    float sb1 = 0.0f;
    float sx1 = 0.0f;
    float sy1 = 0.0f;
    float tb1 = 0.0f;
    float tx1 = 0.0f;
    float ty1 = 0.0f;
    float qb1 = 0.0f;
    float qx1 = 0.0f;
    float qy1 = 0.0f;
    float wb = 0.0f;
    float wx = 0.0f;
    float wy = 0.0f;
    float zb = 0.0f;
    float zx = 0.0f;
    float zy = 0.0f;
    float rb = 0.0f;
    float rx = 0.0f;
    float ry = 0.0f;
    float gb = 0.0f;
    float gx = 0.0f;
    float gy = 0.0f;
    float bb = 0.0f;
    float bx = 0.0f;
    float by = 0.0f;
    float ab = 0.0f;
    float ax = 0.0f;
    float ay = 0.0f;
};

struct AttributesResult
{
    float s0 = 0.0f;
    float t0 = 0.0f;
    float q0 = 0.0f;
    float s1 = 0.0f;
    float t1 = 0.0f;
    float q1 = 0.0f;
    float ms0 = 0.0f;
    float mt0 = 0.0f;
    float mq0 = 0.0f;
    float ms1 = 0.0f;
    float mt1 = 0.0f;
    float mq1 = 0.0f;
    float w = 0.0f;
    float z = 0.0f;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;
};

void calculateVertexAttributes(Attributes attr,
    uint16_t x, uint16_t y,
    AttributesResult& res)
{
    res.ms0 = attr.sx0 + attr.sy0;
    res.mt0 = attr.tx0 + attr.ty0;
    res.mq0 = attr.qx0 + attr.qy0;
    res.ms1 = attr.sx1 + attr.sy1;
    res.mt1 = attr.tx1 + attr.ty1;
    res.mq1 = attr.qx1 + attr.qy1;

    attr.sx0 *= x;
    attr.sy0 *= y;
    attr.tx0 *= x;
    attr.ty0 *= y;
    attr.qx0 *= x;
    attr.qy0 *= y;
    attr.sx1 *= x;
    attr.sy1 *= y;
    attr.tx1 *= x;
    attr.ty1 *= y;
    attr.qx1 *= x;
    attr.qy1 *= y;
    attr.wx *= x;
    attr.wy *= y;
    attr.zx *= x;
    attr.zy *= y;
    attr.rx *= x;
    attr.ry *= y;
    attr.gx *= x;
    attr.gy *= y;
    attr.bx *= x;
    attr.by *= y;
    attr.ax *= x;
    attr.ay *= y;

    res.w = attr.wb + (attr.wx + attr.wy);
    res.s0 = attr.sb0 + (attr.sx0 + attr.sy0);
    res.t0 = attr.tb0 + (attr.tx0 + attr.ty0);
    res.q0 = attr.qb0 + (attr.qx0 + attr.qy0);
    res.s1 = attr.sb1 + (attr.sx1 + attr.sy1);
    res.t1 = attr.tb1 + (attr.tx1 + attr.ty1);
    res.q1 = attr.qb1 + (attr.qx1 + attr.qy1);
    res.z = attr.zb + (attr.zx + attr.zy);
    res.r = attr.rb + (attr.rx + attr.ry);
    res.g = attr.gb + (attr.gx + attr.gy);
    res.b = attr.bb + (attr.bx + attr.by);
    res.a = attr.ab + (attr.ax + attr.ay);

    res.ms0 += res.s0;
    res.mt0 += res.t0;
    res.mq0 += res.q0;
    res.ms1 += res.s1;
    res.mt1 += res.t1;
    res.mq1 += res.q1;

    res.w = 1.0f / res.w;
    res.q0 = 1.0f / res.q0;
    res.q1 = 1.0f / res.q1;
    res.mq0 = 1.0f / res.mq0;
    res.mq1 = 1.0f / res.mq1;

    res.s0 *= res.q0;
    res.t0 *= res.q0;
    res.q0 *= res.q0;
    res.ms0 *= res.mq0;
    res.mt0 *= res.mq0;
    res.mq0 *= res.mq0;

    res.s1 *= res.q1;
    res.t1 *= res.q1;
    res.q1 *= res.q1;
    res.ms1 *= res.mq1;
    res.mt1 *= res.mq1;
    res.mq1 *= res.mq1;

    // Perspective correction for colors is currently disabled
    // res.r *= res.w;
    // res.g *= res.w;
    // res.b *= res.w;
    // res.a *= res.w;
}

TEST_CASE("Check the interpolation through the pipeline", "[AttributeInterpolator]")
{
    static const uint32_t CLOCK_DELAY = 31;
    VAttributeInterpolator* top = new VAttributeInterpolator();

    // Reset cycle
    rr::ut::reset(top);

    // Reset the pipeline
    for (int i = 0; i < CLOCK_DELAY; i++)
    {
        top->tex0_s = 0;
        top->tex0_s_inc_x = 0;
        top->tex0_s_inc_y = 0;
        top->tex0_t = 0;
        top->tex0_t_inc_x = 0;
        top->tex0_t_inc_y = 0;
        top->tex0_q = 0;
        top->tex0_q_inc_x = 0;
        top->tex0_q_inc_y = 0;
        top->depth_w = 0;
        top->depth_w_inc_x = 0;
        top->depth_w_inc_y = 0;
        top->depth_z = 0;
        top->depth_z_inc_x = 0;
        top->depth_z_inc_y = 0;
        top->color_r = 0;
        top->color_r_inc_x = 0;
        top->color_r_inc_y = 0;
        top->color_g = 0;
        top->color_g_inc_x = 0;
        top->color_g_inc_y = 0;
        top->color_b = 0;
        top->color_b_inc_x = 0;
        top->color_b_inc_y = 0;
        top->color_a = 0;
        top->color_a_inc_x = 0;
        top->color_a_inc_y = 0;

        top->s_attrb_tbbx = 0;
        top->s_attrb_tbby = 0;
        top->s_attrb_tindex = 0;

        top->s_attrb_tvalid = 0;
        top->s_attrb_tpixel = 0;
        top->s_attrb_tlast = 0;
    }
    REQUIRE(top->m_attrb_tindex == 0);
    REQUIRE(top->m_attrb_tdepth_w == 0);
    REQUIRE(top->m_attrb_tdepth_z == 0);
    REQUIRE(top->m_attrb_ttexture0_t == 0);
    REQUIRE(top->m_attrb_ttexture0_s == 0);
    REQUIRE(top->m_attrb_tcolor_a == 0);
    REQUIRE(top->m_attrb_tcolor_b == 0);
    REQUIRE(top->m_attrb_tcolor_g == 0);
    REQUIRE(top->m_attrb_tcolor_r == 0);
    REQUIRE(top->m_attrb_tvalid == 0);
    REQUIRE(top->m_attrb_tpixel == 0);
    REQUIRE(top->m_attrb_tlast == 0);

    ScreenPos sp;
    ScreenPos bb;
    sp.val.x = 400;
    sp.val.y = 200;
    bb.val.x = 0;
    bb.val.y = 100;

    Attributes attr;

    attr.sb0 = 3.0;
    attr.sx0 = 0.50;
    attr.sy0 = 0.25;
    attr.tb0 = 4.0;
    attr.tx0 = 0.20;
    attr.ty0 = 0.10;
    attr.qb0 = 0.06754551082849503;
    attr.qx0 = -1.5070709196152166e-05;
    attr.qy0 = 3.105480573140085e-05;
    attr.sb1 = 1.0;
    attr.sx1 = 2.50;
    attr.sy1 = 3.25;
    attr.tb1 = 3.0;
    attr.tx1 = 2.20;
    attr.ty1 = 1.10;
    attr.qb1 = 1.06754551082849503;
    attr.qx1 = -2.5070709196152166e-05;
    attr.qy1 = 0.105480573140085e-05;
    attr.wb = 5.06754551082849503;
    attr.wx = -3.5070709196152166e-02;
    attr.wy = 9.105480573140085e-03;
    attr.zb = 1.1;
    attr.zx = 0.1;
    attr.zy = 0.2;
    attr.rb = 1.2;
    attr.rx = 0.3;
    attr.ry = 0.4;
    attr.gb = 1.3;
    attr.gx = 0.5;
    attr.gy = 0.6;
    attr.bb = 1.4;
    attr.bx = 0.7;
    attr.by = 0.8;
    attr.ab = 1.5;
    attr.ax = 0.1;
    attr.ay = 0.2;

    // Set the vertex attributes
    top->tex0_s = *(uint32_t*)&attr.sb0;
    top->tex0_s_inc_x = *(uint32_t*)&attr.sx0;
    top->tex0_s_inc_y = *(uint32_t*)&attr.sy0;
    top->tex0_t = *(uint32_t*)&attr.tb0;
    top->tex0_t_inc_x = *(uint32_t*)&attr.tx0;
    top->tex0_t_inc_y = *(uint32_t*)&attr.ty0;
    top->tex0_q = *(uint32_t*)&attr.qb0;
    top->tex0_q_inc_x = *(uint32_t*)&attr.qx0;
    top->tex0_q_inc_y = *(uint32_t*)&attr.qy0;
    top->tex1_s = *(uint32_t*)&attr.sb1;
    top->tex1_s_inc_x = *(uint32_t*)&attr.sx1;
    top->tex1_s_inc_y = *(uint32_t*)&attr.sy1;
    top->tex1_t = *(uint32_t*)&attr.tb1;
    top->tex1_t_inc_x = *(uint32_t*)&attr.tx1;
    top->tex1_t_inc_y = *(uint32_t*)&attr.ty1;
    top->tex1_q = *(uint32_t*)&attr.qb1;
    top->tex1_q_inc_x = *(uint32_t*)&attr.qx1;
    top->tex1_q_inc_y = *(uint32_t*)&attr.qy1;
    top->depth_w = *(uint32_t*)&attr.wb;
    top->depth_w_inc_x = *(uint32_t*)&attr.wx;
    top->depth_w_inc_y = *(uint32_t*)&attr.wy;
    top->depth_z = *(uint32_t*)&attr.zb;
    top->depth_z_inc_x = *(uint32_t*)&attr.zx;
    top->depth_z_inc_y = *(uint32_t*)&attr.zy;
    top->color_r = *(uint32_t*)&attr.rb;
    top->color_r_inc_x = *(uint32_t*)&attr.rx;
    top->color_r_inc_y = *(uint32_t*)&attr.ry;
    top->color_g = *(uint32_t*)&attr.gb;
    top->color_g_inc_x = *(uint32_t*)&attr.gx;
    top->color_g_inc_y = *(uint32_t*)&attr.gy;
    top->color_b = *(uint32_t*)&attr.bb;
    top->color_b_inc_x = *(uint32_t*)&attr.bx;
    top->color_b_inc_y = *(uint32_t*)&attr.by;
    top->color_a = *(uint32_t*)&attr.ab;
    top->color_a_inc_x = *(uint32_t*)&attr.ax;
    top->color_a_inc_y = *(uint32_t*)&attr.ay;

    top->s_attrb_tbbx = bb.val.x;
    top->s_attrb_tbby = bb.val.y;
    top->s_attrb_tspx = sp.val.x;
    top->s_attrb_tspy = sp.val.y;
    top->s_attrb_tindex = 0;

    top->s_attrb_tvalid = 1;
    top->s_attrb_tpixel = 1;
    top->m_attrb_tready = 1;
    top->s_attrb_tlast = 0;

    // Check the pipeline with 100 pixel
    for (int i = 0; i < 100; i++)
    {
        rr::ut::clk(top);
        if (i >= CLOCK_DELAY)
        {
            REQUIRE(top->m_attrb_tindex == i - CLOCK_DELAY);
            REQUIRE(top->m_attrb_tvalid == 1);
            REQUIRE(top->m_attrb_tpixel == 1);
            REQUIRE(top->m_attrb_tlast == (i - CLOCK_DELAY) % 5);
            REQUIRE(top->m_attrb_tspx == (400 + static_cast<uint16_t>(i - CLOCK_DELAY)));
            REQUIRE(top->m_attrb_tspy == (200 + static_cast<uint16_t>(i - CLOCK_DELAY)));

            float w = *(float*)&(top->m_attrb_tdepth_w);
            float z = *(float*)&(top->m_attrb_tdepth_z);
            float s0 = *(float*)&(top->m_attrb_ttexture0_s);
            float t0 = *(float*)&(top->m_attrb_ttexture0_t);
            float ms0 = *(float*)&(top->m_attrb_tmipmap0_s);
            float mt0 = *(float*)&(top->m_attrb_tmipmap0_t);
            float s1 = *(float*)&(top->m_attrb_ttexture1_s);
            float t1 = *(float*)&(top->m_attrb_ttexture1_t);
            float ms1 = *(float*)&(top->m_attrb_tmipmap1_s);
            float mt1 = *(float*)&(top->m_attrb_tmipmap1_t);
            float r = *(float*)&(top->m_attrb_tcolor_r);
            float g = *(float*)&(top->m_attrb_tcolor_g);
            float b = *(float*)&(top->m_attrb_tcolor_b);
            float a = *(float*)&(top->m_attrb_tcolor_a);

            AttributesResult expectedResult;

            calculateVertexAttributes(attr,
                bb.val.x - CLOCK_DELAY, bb.val.y + CLOCK_DELAY,
                expectedResult);

            // printf("%d %f %f %f %f %f %f\n", i, s, sr, t, tr, w, wr);

            REQUIRE(Approx(w).epsilon(0.10) == expectedResult.w);
            REQUIRE(Approx(z).epsilon(0.01) == expectedResult.z);
            REQUIRE(Approx(s0).epsilon(0.005) == expectedResult.s0);
            REQUIRE(Approx(t0).epsilon(0.005) == expectedResult.t0);
            REQUIRE(Approx(ms0).epsilon(0.005) == expectedResult.ms0);
            REQUIRE(Approx(mt0).epsilon(0.005) == expectedResult.mt0);
            REQUIRE(Approx(s1).epsilon(0.005) == expectedResult.s1);
            REQUIRE(Approx(t1).epsilon(0.005) == expectedResult.t1);
            REQUIRE(Approx(ms1).epsilon(0.005) == expectedResult.ms1);
            REQUIRE(Approx(mt1).epsilon(0.005) == expectedResult.mt1);
            REQUIRE(Approx(r).epsilon(0.01) == expectedResult.r);
            REQUIRE(Approx(g).epsilon(0.01) == expectedResult.g);
            REQUIRE(Approx(b).epsilon(0.01) == expectedResult.b);
            REQUIRE(Approx(a).epsilon(0.01) == expectedResult.a);
        }

        top->s_attrb_tvalid = 1;
        top->s_attrb_tpixel = 1;
        top->s_attrb_tlast = i % 5;

        top->s_attrb_tbbx = bb.val.x;
        top->s_attrb_tbby = bb.val.y;
        top->s_attrb_tspx = sp.val.x;
        top->s_attrb_tspy = sp.val.y;
        top->s_attrb_tindex = i;

        sp.val.x++;
        sp.val.y++;
        bb.val.x++;
        bb.val.y--;
    }

    // Reset counter for the rest of the pixels in the pipeline
    sp.val.x = 400 + CLOCK_DELAY;
    sp.val.y = 200 + CLOCK_DELAY;
    bb.val.x = 100 - CLOCK_DELAY;
    bb.val.y = CLOCK_DELAY;

    // Check the pending pixels in the pipeline
    for (int i = 100 - CLOCK_DELAY; i < 100; i++)
    {
        rr::ut::clk(top);

        REQUIRE(top->m_attrb_tindex == i);
        REQUIRE(top->m_attrb_tvalid == 1);
        REQUIRE(top->m_attrb_tpixel == 1);
        REQUIRE(top->m_attrb_tlast == i % 5);
        REQUIRE(top->m_attrb_tspx == (400 + static_cast<uint16_t>(i)));
        REQUIRE(top->m_attrb_tspy == (200 + static_cast<uint16_t>(i)));

        float w = *(float*)&(top->m_attrb_tdepth_w);
        float z = *(float*)&(top->m_attrb_tdepth_z);
        float s0 = *(float*)&(top->m_attrb_ttexture0_s);
        float t0 = *(float*)&(top->m_attrb_ttexture0_t);
        float ms0 = *(float*)&(top->m_attrb_tmipmap0_s);
        float mt0 = *(float*)&(top->m_attrb_tmipmap0_t);
        float s1 = *(float*)&(top->m_attrb_ttexture1_s);
        float t1 = *(float*)&(top->m_attrb_ttexture1_t);
        float ms1 = *(float*)&(top->m_attrb_tmipmap1_s);
        float mt1 = *(float*)&(top->m_attrb_tmipmap1_t);
        float r = *(float*)&(top->m_attrb_tcolor_r);
        float g = *(float*)&(top->m_attrb_tcolor_g);
        float b = *(float*)&(top->m_attrb_tcolor_b);
        float a = *(float*)&(top->m_attrb_tcolor_a);

        AttributesResult expectedResult;

        calculateVertexAttributes(attr,
            bb.val.x, bb.val.y,
            expectedResult);

        REQUIRE(Approx(s0).epsilon(0.005) == expectedResult.s0);
        REQUIRE(Approx(t0).epsilon(0.005) == expectedResult.t0);
        REQUIRE(Approx(ms0).epsilon(0.005) == expectedResult.ms0);
        REQUIRE(Approx(mt0).epsilon(0.005) == expectedResult.mt0);
        REQUIRE(Approx(s1).epsilon(0.005) == expectedResult.s1);
        REQUIRE(Approx(t1).epsilon(0.005) == expectedResult.t1);
        REQUIRE(Approx(ms1).epsilon(0.005) == expectedResult.ms1);
        REQUIRE(Approx(mt1).epsilon(0.005) == expectedResult.mt1);
        REQUIRE(Approx(w).epsilon(0.10) == expectedResult.w);
        REQUIRE(Approx(z).epsilon(0.01) == expectedResult.z);
        REQUIRE(Approx(r).epsilon(0.01) == expectedResult.r);
        REQUIRE(Approx(g).epsilon(0.01) == expectedResult.g);
        REQUIRE(Approx(b).epsilon(0.01) == expectedResult.b);
        REQUIRE(Approx(a).epsilon(0.01) == expectedResult.a);

        // Set init values
        top->s_attrb_tvalid = 0;
        top->s_attrb_tpixel = 0;
        top->s_attrb_tlast = 0;
        top->s_attrb_tbbx = 0;
        top->s_attrb_tbby = 0;
        top->s_attrb_tspx = 0;
        top->s_attrb_tspy = 0;
        top->s_attrb_tindex = 0;

        bb.val.x++;
        bb.val.y--;
    }

    // Check again if the pipeline is now empty
    rr::ut::clk(top);

    REQUIRE(top->m_attrb_tindex == 0);
    REQUIRE(top->m_attrb_tvalid == 0);
    REQUIRE(top->m_attrb_tpixel == 0);
    REQUIRE(top->m_attrb_tlast == 0);

    // Destroy model
    delete top;
}
