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
#include "VFramebufferClear.h"

TEST_CASE("Check forwarding", "[FramebufferClear]")
{
    VFramebufferClear* t = rr::ut::makeTop<VFramebufferClear>();
    t->apply = 0;
    rr::ut::reset(t);

    t->confClearColor = 0x12345678;
    t->confClearDepth = 0xabcd;
    t->confClearStencil = 0xa;
    t->confXResolution = 16;
    t->confYResolution = 8;
    t->confYOffset = 0;

    t->s_frag_tvalid = 1;
    t->s_frag_tlast = 0;
    t->s_frag_color_tdata = 0x87654321;
    t->s_frag_color_tstrb = 1;
    t->s_frag_depth_tdata = 0x4321;
    t->s_frag_depth_tstrb = 0;
    t->s_frag_stencil_tdata = 0x5;
    t->s_frag_stencil_tstrb = 1;
    t->s_frag_taddr = 0x1234;
    t->s_frag_txpos = 10;
    t->s_frag_typos = 8;
    t->m_frag_tready = 1;
    t->eval();

    CHECK(t->s_frag_tready == 1);
    CHECK(t->m_frag_tvalid == 1);
    CHECK(t->m_frag_tlast == 0);
    CHECK(t->m_frag_color_tdata == 0x87654321);
    CHECK(t->m_frag_color_tstrb == 1);
    CHECK(t->m_frag_depth_tdata == 0x4321);
    CHECK(t->m_frag_depth_tstrb == 0);
    CHECK(t->m_frag_stencil_tdata == 0x5);
    CHECK(t->m_frag_stencil_tstrb == 1);
    CHECK(t->m_frag_taddr == 0x1234);
    CHECK(t->m_frag_txpos == 10);
    CHECK(t->m_frag_typos == 8);

    delete t;
}

TEST_CASE("Check clear", "[FramebufferClear]")
{
    static constexpr uint32_t X_RES { 10 };
    static constexpr uint32_t Y_RES { 8 };
    VFramebufferClear* t = rr::ut::makeTop<VFramebufferClear>();
    t->apply = 0;
    rr::ut::reset(t);

    t->confClearColor = 0x12345678;
    t->confClearDepth = 0xabcd;
    t->confClearStencil = 0xa;
    t->confColorBufferSelect = 1;
    t->confDepthBufferSelect = 1;
    t->confStencilBufferSelect = 1;
    t->confXResolution = X_RES;
    t->confYResolution = Y_RES;
    t->confYOffset = 0;
    t->confEnableScissor = 0;
    t->m_frag_tready = 1;
    t->apply = 1;

    rr::ut::clk(t);
    REQUIRE(t->m_frag_tvalid == 0);
    t->apply = 0;

    static constexpr uint32_t Y_RES_MAX_INDEX = Y_RES - 1;
    static constexpr uint32_t X_RES_MAX_INDEX = X_RES - 1;
    uint32_t x = 0;
    uint32_t y = 0;
    while (y < Y_RES)
    {
        rr::ut::clk(t);
        REQUIRE(t->s_frag_tready == 0);
        REQUIRE(t->m_frag_tvalid == 1);
        REQUIRE(t->m_frag_tlast == ((y == Y_RES_MAX_INDEX) && (x == X_RES_MAX_INDEX)));
        REQUIRE(t->m_frag_color_tdata == 0x12345678);
        REQUIRE(t->m_frag_color_tstrb == 1);
        REQUIRE(t->m_frag_depth_tdata == 0xabcd);
        REQUIRE(t->m_frag_depth_tstrb == 1);
        REQUIRE(t->m_frag_stencil_tdata == 0xa);
        REQUIRE(t->m_frag_stencil_tstrb == 1);
        REQUIRE(t->m_frag_taddr == x + ((Y_RES_MAX_INDEX - y) * X_RES));
        REQUIRE(t->m_frag_txpos == x);
        REQUIRE(t->m_frag_typos == y);
        REQUIRE(t->applied == ((y == Y_RES_MAX_INDEX) && (x == X_RES_MAX_INDEX)));

        if (t->m_frag_tlast)
        {
            break;
        }

        x++;
        if (x >= X_RES)
        {
            y++;
            x = 0;
        }
    }

    rr::ut::clk(t);
    REQUIRE(t->m_frag_tvalid == 0);
    REQUIRE(t->applied == 1);
    delete t;
}

TEST_CASE("Check flow control", "[FramebufferClear]")
{
    static constexpr uint32_t X_RES { 10 };
    static constexpr uint32_t Y_RES { 8 };
    VFramebufferClear* t = rr::ut::makeTop<VFramebufferClear>();
    t->apply = 0;
    rr::ut::reset(t);

    t->confClearColor = 0x12345678;
    t->confClearDepth = 0xabcd;
    t->confClearStencil = 0xa;
    t->confColorBufferSelect = 1;
    t->confDepthBufferSelect = 1;
    t->confStencilBufferSelect = 1;
    t->confXResolution = X_RES;
    t->confYResolution = Y_RES;
    t->confYOffset = 0;
    t->confEnableScissor = 0;
    t->m_frag_tready = 0;
    t->apply = 1;

    rr::ut::clk(t);
    t->apply = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_frag_tready == 0);
    REQUIRE(t->m_frag_tvalid == 0);
    REQUIRE(t->applied == 0);

    t->m_frag_tready = 1;
    rr::ut::clk(t);
    t->m_frag_tready = 0;
    REQUIRE(t->s_frag_tready == 0);
    REQUIRE(t->m_frag_tvalid == 1);
    REQUIRE(t->m_frag_tlast == 0);
    REQUIRE(t->m_frag_color_tdata == 0x12345678);
    REQUIRE(t->m_frag_color_tstrb == 1);
    REQUIRE(t->m_frag_depth_tdata == 0xabcd);
    REQUIRE(t->m_frag_depth_tstrb == 1);
    REQUIRE(t->m_frag_stencil_tdata == 0xa);
    REQUIRE(t->m_frag_stencil_tstrb == 1);
    REQUIRE(t->m_frag_taddr == (Y_RES - 1) * X_RES);
    REQUIRE(t->m_frag_txpos == 0);
    REQUIRE(t->m_frag_typos == 0);
    REQUIRE(t->applied == 0);

    rr::ut::clk(t);
    REQUIRE(t->m_frag_tvalid == 1);
    REQUIRE(t->m_frag_taddr == (Y_RES - 1) * X_RES);
    REQUIRE(t->m_frag_txpos == 0);
    REQUIRE(t->m_frag_typos == 0);
    REQUIRE(t->applied == 0);

    t->m_frag_tready = 1;
    rr::ut::clk(t);
    t->m_frag_tready = 0;
    REQUIRE(t->m_frag_tvalid == 1);
    REQUIRE(t->m_frag_taddr == ((Y_RES - 1) * X_RES) + 1);
    REQUIRE(t->m_frag_txpos == 1);
    REQUIRE(t->m_frag_typos == 0);
    REQUIRE(t->applied == 0);
    delete t;
}

TEST_CASE("Check scissored clear", "[FramebufferClear]")
{
    static constexpr uint32_t X_RES { 16 };
    static constexpr uint32_t Y_RES { 12 };
    static constexpr uint32_t START_X { 3 };
    static constexpr uint32_t START_Y { 4 };
    static constexpr uint32_t END_X { 7 };
    static constexpr uint32_t END_Y { 9 };
    VFramebufferClear* t = rr::ut::makeTop<VFramebufferClear>();
    rr::ut::reset(t);

    t->confClearColor = 0x12345678;
    t->confClearDepth = 0xabcd;
    t->confClearStencil = 0xa;
    t->confColorBufferSelect = 1;
    t->confDepthBufferSelect = 1;
    t->confStencilBufferSelect = 1;
    t->confXResolution = X_RES;
    t->confYResolution = Y_RES;
    t->confYOffset = 0;
    t->confEnableScissor = 1;
    t->confScissorStartX = START_X;
    t->confScissorStartY = START_Y;
    t->confScissorEndX = END_X;
    t->confScissorEndY = END_Y;
    t->m_frag_tready = 1;
    t->apply = 1;

    rr::ut::clk(t);
    REQUIRE(t->m_frag_tvalid == 0);
    t->apply = 0;

    for (uint32_t y = START_Y; y < END_Y; ++y)
    {
        for (uint32_t x = START_X; x < END_X; ++x)
        {
            rr::ut::clk(t);
            REQUIRE(t->m_frag_tvalid == 1);
            REQUIRE(t->m_frag_color_tdata == 0x12345678);
            REQUIRE(t->m_frag_color_tstrb == 1);
            REQUIRE(t->m_frag_depth_tdata == 0xabcd);
            REQUIRE(t->m_frag_depth_tstrb == 1);
            REQUIRE(t->m_frag_stencil_tdata == 0xa);
            REQUIRE(t->m_frag_stencil_tstrb == 1);
            REQUIRE(t->m_frag_txpos == x);
            REQUIRE(t->m_frag_typos == y);
            REQUIRE(t->m_frag_taddr == x + ((Y_RES - 1 - y) * X_RES));
            REQUIRE(t->m_frag_tlast == (x == END_X - 1 && y == END_Y - 1));
            REQUIRE(t->applied == (x == END_X - 1 && y == END_Y - 1));
        }
    }

    rr::ut::clk(t);
    CHECK(t->applied == 1);
    REQUIRE(t->m_frag_tvalid == 0);
    delete t;
}

TEST_CASE("Check malformed scissor terminates", "[FramebufferClear]")
{
    static constexpr uint32_t START_X { 3 };
    static constexpr uint32_t END_X { 7 };
    static constexpr uint32_t START_Y { 8 };
    static constexpr uint32_t END_Y { 4 };
    VFramebufferClear* t = rr::ut::makeTop<VFramebufferClear>();
    rr::ut::reset(t);

    t->confXResolution = 16;
    t->confYResolution = 12;
    t->confYOffset = 0;
    t->confEnableScissor = 1;
    t->confColorBufferSelect = 1;
    t->confScissorStartX = START_X;
    t->confScissorEndX = END_X;
    t->confScissorStartY = START_Y;
    t->confScissorEndY = END_Y;
    t->m_frag_tready = 1;
    t->apply = 1;

    rr::ut::clk(t);
    t->apply = 0;

    bool completed = false;
    for (uint32_t cycle = 0; cycle < (END_X - START_X) + 2; ++cycle)
    {
        rr::ut::clk(t);
        if (t->applied)
        {
            completed = true;
            break;
        }
    }
    CHECK(completed);
    delete t;
}

TEST_CASE("Clear strobes follow selected buffers", "[FramebufferClear]")
{
    for (uint32_t selects = 0; selects < 8; ++selects)
    {
        VFramebufferClear* t = rr::ut::makeTop<VFramebufferClear>();
        rr::ut::reset(t);

        t->confClearColor = 0x12345678;
        t->confClearDepth = 0xabcd;
        t->confClearStencil = 0xa;
        t->confColorBufferSelect = (selects & 1) != 0;
        t->confDepthBufferSelect = (selects & 2) != 0;
        t->confStencilBufferSelect = (selects & 4) != 0;
        t->confXResolution = 1;
        t->confYResolution = 1;
        t->confYOffset = 0;
        t->confEnableScissor = 0;
        t->m_frag_tready = 1;
        t->apply = 1;
        rr::ut::clk(t);
        t->apply = 0;

        REQUIRE(t->m_frag_tvalid == 0);
        rr::ut::clk(t);
        rr::ut::clk(t);
        REQUIRE(t->m_frag_tvalid == 1);
        REQUIRE(t->m_frag_tlast == 1);
        REQUIRE(t->m_frag_color_tdata == 0x12345678);
        REQUIRE(t->m_frag_depth_tdata == 0xabcd);
        REQUIRE(t->m_frag_stencil_tdata == 0xa);
        REQUIRE(t->m_frag_color_tstrb == ((selects & 1) != 0));
        REQUIRE(t->m_frag_depth_tstrb == ((selects & 2) != 0));
        REQUIRE(t->m_frag_stencil_tstrb == ((selects & 4) != 0));
        CHECK(t->applied == 1);
        delete t;
    }
}

TEST_CASE("Scissor clear uses offset screen Y and local framebuffer address", "[FramebufferClear]")
{
    static constexpr uint32_t X_RES { 8 };
    static constexpr uint32_t Y_RES { 6 };
    static constexpr uint32_t Y_OFFSET { 10 };
    static constexpr uint32_t START_X { 2 };
    static constexpr uint32_t END_X { 4 };
    static constexpr uint32_t START_SCREEN_Y { 8 };
    static constexpr uint32_t END_SCREEN_Y { 20 };

    VFramebufferClear* t = rr::ut::makeTop<VFramebufferClear>();
    rr::ut::reset(t);

    t->confClearColor = 0x12345678;
    t->confClearDepth = 0xabcd;
    t->confClearStencil = 0xa;
    t->confColorBufferSelect = 1;
    t->confDepthBufferSelect = 0;
    t->confStencilBufferSelect = 0;
    t->confXResolution = X_RES;
    t->confYResolution = Y_RES;
    t->confYOffset = Y_OFFSET;
    t->confEnableScissor = 1;
    t->confScissorStartX = START_X;
    t->confScissorEndX = END_X;
    t->confScissorStartY = START_SCREEN_Y;
    t->confScissorEndY = END_SCREEN_Y;
    t->m_frag_tready = 1;
    t->apply = 1;
    rr::ut::clk(t);
    REQUIRE(t->m_frag_tvalid == 0);
    t->apply = 0;

    for (uint32_t localY = 0; localY < Y_RES; ++localY)
    {
        for (uint32_t x = START_X; x < END_X; ++x)
        {
            rr::ut::clk(t);
            REQUIRE(t->m_frag_tvalid == 1);
            REQUIRE(t->m_frag_txpos == x);
            REQUIRE(t->m_frag_typos == localY + Y_OFFSET);
            REQUIRE(t->m_frag_taddr == x + ((Y_RES - 1 - localY) * X_RES));
            REQUIRE(t->m_frag_color_tstrb == 1);
            REQUIRE(t->m_frag_depth_tstrb == 0);
            REQUIRE(t->m_frag_stencil_tstrb == 0);
            REQUIRE(t->m_frag_tlast == (localY == Y_RES - 1 && x == END_X - 1));
            REQUIRE(t->applied == (localY == Y_RES - 1 && x == END_X - 1));
        }
    }
    CHECK(t->applied == 1);
    rr::ut::clk(t);
    CHECK(t->m_frag_tvalid == 0);
    delete t;
}

TEST_CASE("Empty offset scissor completes without generated fragments", "[FramebufferClear]")
{
    VFramebufferClear* t = rr::ut::makeTop<VFramebufferClear>();
    rr::ut::reset(t);

    t->confXResolution = 8;
    t->confYResolution = 6;
    t->confYOffset = 10;
    t->confEnableScissor = 1;
    t->confScissorStartX = 2;
    t->confScissorEndX = 4;
    t->confScissorStartY = 4;
    t->confScissorEndY = 8;
    t->m_frag_tready = 1;
    t->apply = 1;

    rr::ut::clk(t);
    t->apply = 0;
    CHECK(t->applied == 0);

    for (uint32_t cycle = 0; cycle < 4 && !t->applied; ++cycle)
    {
        CHECK(t->m_frag_tvalid == 0);
        rr::ut::clk(t);
    }

    CHECK(t->applied == 1);
    CHECK(t->m_frag_tvalid == 0);
    delete t;
}

TEST_CASE("Applied preserves the original clear-pipeline boundary", "[FramebufferClear]")
{
    VFramebufferClear* t = rr::ut::makeTop<VFramebufferClear>();
    rr::ut::reset(t);

    t->confClearColor = 0x12345678;
    t->confClearDepth = 0xabcd;
    t->confClearStencil = 0xa;
    t->confColorBufferSelect = 1;
    t->confDepthBufferSelect = 0;
    t->confStencilBufferSelect = 0;
    t->confXResolution = 1;
    t->confYResolution = 1;
    t->confYOffset = 0;
    t->confEnableScissor = 0;
    t->m_frag_tready = 1;
    t->apply = 1;
    rr::ut::clk(t);
    t->apply = 0;

    rr::ut::clk(t);
    REQUIRE(t->m_frag_tvalid == 1);
    REQUIRE(t->m_frag_tlast == 0);
    CHECK(t->applied == 0);

    rr::ut::clk(t);
    REQUIRE(t->m_frag_tvalid == 1);
    REQUIRE(t->m_frag_tlast == 1);
    CHECK(t->applied == 1);

    t->m_frag_tready = 0;
    const auto addr = t->m_frag_taddr;
    rr::ut::clk(t);
    CHECK(t->m_frag_tvalid == 1);
    CHECK(t->m_frag_tlast == 1);
    CHECK(t->m_frag_taddr == addr);
    CHECK(t->applied == 1);
    delete t;
}
