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
#include "VFramebufferWriterClear.h"

TEST_CASE("Check forwarding", "[FramebufferWriterClear]")
{
    VFramebufferWriterClear* t = rr::ut::makeTop<VFramebufferWriterClear>();

    t->apply = 0;

    rr::ut::reset(t);

    CHECK(t->applied == 1);

    t->confClearColor = 0xabcd;
    t->confXResolution = 16;
    t->confYResolution = 8;

    t->s_frag_tvalid = 1;
    t->s_frag_tlast = 0;
    t->s_frag_tdata = 0xdcba;
    t->s_frag_tstrb = 1;
    t->s_frag_taddr = 0x1234;
    t->s_frag_txpos = 10;
    t->s_frag_typos = 8;
    t->m_frag_tready = 1;

    t->eval();

    CHECK(t->s_frag_tready == 1);
    CHECK(t->m_frag_tvalid == 1);
    CHECK(t->m_frag_tlast == 0);
    CHECK(t->m_frag_tdata == 0xdcba);
    CHECK(t->m_frag_tstrb == 1);
    CHECK(t->m_frag_taddr == 0x1234);
    CHECK(t->m_frag_txpos == 10);
    CHECK(t->m_frag_typos == 8);

    delete t;
}

TEST_CASE("Check clear", "[FramebufferWriterClear]")
{
    static constexpr uint32_t X_RES { 10 };
    static constexpr uint32_t Y_RES { 8 };
    VFramebufferWriterClear* t = rr::ut::makeTop<VFramebufferWriterClear>();

    t->apply = 0;

    rr::ut::reset(t);

    t->confClearColor = 0xabcd;
    t->confXResolution = X_RES;
    t->confYResolution = Y_RES;
    t->confEnableScissor = 0;
    t->m_frag_tready = 1;
    t->apply = 1;

    // The clear command is latched first, then the generated fragment passes
    // through the additional pipeline stage.
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
        REQUIRE(t->m_frag_tdata == 0xabcd);
        REQUIRE(t->m_frag_tstrb == 1);
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

TEST_CASE("Check flow control", "[FramebufferWriterClear]")
{
    static constexpr uint32_t X_RES { 10 };
    static constexpr uint32_t Y_RES { 8 };
    VFramebufferWriterClear* t = rr::ut::makeTop<VFramebufferWriterClear>();

    t->apply = 0;

    rr::ut::reset(t);

    t->confClearColor = 0xabcd;
    t->confXResolution = X_RES;
    t->confYResolution = Y_RES;
    t->confEnableScissor = 0;
    t->m_frag_tready = 0;
    t->apply = 1;

    rr::ut::clk(t);
    t->apply = 0;

    // Step 1 cannot capture a generated pixel while the output is stalled.
    rr::ut::clk(t);
    REQUIRE(t->s_frag_tready == 0);
    REQUIRE(t->m_frag_tvalid == 0);
    REQUIRE(t->applied == 0);

    // Let Step 1 capture the first generated pixel, then deassert ready to
    // verify that the pixel is held.
    t->m_frag_tready = 1;
    rr::ut::clk(t);
    t->m_frag_tready = 0;
    REQUIRE(t->s_frag_tready == 0);
    REQUIRE(t->m_frag_tvalid == 1);
    REQUIRE(t->m_frag_tlast == 0);
    REQUIRE(t->m_frag_tdata == 0xabcd);
    REQUIRE(t->m_frag_tstrb == 1);
    REQUIRE(t->m_frag_taddr == (Y_RES - 1) * X_RES);
    REQUIRE(t->m_frag_txpos == 0);
    REQUIRE(t->m_frag_typos == 0);
    REQUIRE(t->applied == 0);

    // The generated pixel remains stable while the output is not ready.
    rr::ut::clk(t);
    REQUIRE(t->m_frag_tvalid == 1);
    REQUIRE(t->m_frag_taddr == (Y_RES - 1) * X_RES);
    REQUIRE(t->m_frag_txpos == 0);
    REQUIRE(t->m_frag_typos == 0);
    REQUIRE(t->applied == 0);

    // Release the held pixel and confirm that the next pixel appears.
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

TEST_CASE("Check scissored clear", "[FramebufferWriterClear]")
{
    static constexpr uint32_t X_RES { 16 };
    static constexpr uint32_t Y_RES { 12 };
    static constexpr uint32_t START_X { 3 };
    static constexpr uint32_t START_Y { 4 };
    static constexpr uint32_t END_X { 7 };
    static constexpr uint32_t END_Y { 9 };
    VFramebufferWriterClear* t = rr::ut::makeTop<VFramebufferWriterClear>();

    rr::ut::reset(t);

    t->confClearColor = 0xabcd;
    t->confXResolution = X_RES;
    t->confYResolution = Y_RES;
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

    uint32_t expected = 0;
    for (uint32_t y = START_Y; y < END_Y; ++y)
    {
        for (uint32_t x = START_X; x < END_X; ++x)
        {
            rr::ut::clk(t);
            REQUIRE(t->m_frag_tvalid == 1);
            REQUIRE(t->m_frag_tdata == 0xabcd);
            REQUIRE(t->m_frag_tstrb == 1);
            REQUIRE(t->m_frag_txpos == x);
            REQUIRE(t->m_frag_typos == y);
            REQUIRE(t->m_frag_taddr == x + ((Y_RES - 1 - y) * X_RES));
            REQUIRE(t->m_frag_tlast == (x == END_X - 1 && y == END_Y - 1));
            REQUIRE(t->applied == (x == END_X - 1 && y == END_Y - 1));
            ++expected;
        }
    }

    CHECK(expected == (END_X - START_X) * (END_Y - START_Y));
    rr::ut::clk(t);
    CHECK(t->applied == 1);
    REQUIRE(t->m_frag_tvalid == 0);

    delete t;
}

TEST_CASE("Check malformed scissor terminates", "[FramebufferWriterClear]")
{
    static constexpr uint32_t START_X { 3 };
    static constexpr uint32_t END_X { 7 };
    static constexpr uint32_t START_Y { 8 };
    static constexpr uint32_t END_Y { 4 };
    VFramebufferWriterClear* t = rr::ut::makeTop<VFramebufferWriterClear>();

    rr::ut::reset(t);

    t->confXResolution = 16;
    t->confYResolution = 12;
    t->confEnableScissor = 1;
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