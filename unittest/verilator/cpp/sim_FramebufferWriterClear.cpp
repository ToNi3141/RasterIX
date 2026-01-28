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
    VFramebufferWriterClear* t = new VFramebufferWriterClear();

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
    VFramebufferWriterClear* t = new VFramebufferWriterClear();

    t->apply = 0;

    rr::ut::reset(t);

    t->confClearColor = 0xabcd;
    t->confXResolution = X_RES;
    t->confYResolution = Y_RES;

    t->m_frag_tready = 1;

    t->apply = 1;

    static constexpr uint32_t Y_RES_MAX_INDEX = Y_RES - 1;
    for (uint32_t x = 0, y = Y_RES_MAX_INDEX; x < X_RES && y == 0; x++)
    {
        rr::ut::clk(t);
        t->apply = 0;
        CHECK(t->s_frag_tready == 0);
        CHECK(t->m_frag_tvalid == 1);
        CHECK(t->m_frag_tlast == ((y == 0) && ((x + 1) == X_RES)));
        CHECK(t->m_frag_tdata == 0xabcd);
        CHECK(t->m_frag_tstrb == 1);
        CHECK(t->m_frag_taddr == x + ((Y_RES_MAX_INDEX - y) * Y_RES_MAX_INDEX));
        CHECK(t->m_frag_txpos == x);
        CHECK(t->m_frag_typos == y);
        if (x + 1 == X_RES)
        {
            y--;
        }
    }

    delete t;
}

TEST_CASE("Check flow control", "[FramebufferWriterClear]")
{
    static constexpr uint32_t X_RES { 10 };
    static constexpr uint32_t Y_RES { 8 };
    VFramebufferWriterClear* t = new VFramebufferWriterClear();

    t->apply = 0;

    rr::ut::reset(t);

    t->confClearColor = 0xabcd;
    t->confXResolution = X_RES;
    t->confYResolution = Y_RES;

    t->m_frag_tready = 0;

    t->apply = 1;

    static constexpr uint32_t Y_RES_MAX_INDEX = Y_RES - 1;
    for (uint32_t x = 0, y = Y_RES_MAX_INDEX; x < X_RES && y > 0; x++)
    {
        t->m_frag_tready = 0;
        rr::ut::clk(t);
        t->apply = 0;
        rr::ut::clk(t);
        rr::ut::clk(t);
        t->m_frag_tready = 1;
        CHECK(t->s_frag_tready == 0);
        CHECK(t->m_frag_tvalid == 1);
        CHECK(t->m_frag_tlast == ((y == 0) && ((x + 1) == X_RES)));
        CHECK(t->m_frag_tdata == 0xabcd);
        CHECK(t->m_frag_tstrb == 1);
        CHECK(t->m_frag_taddr == x + ((Y_RES_MAX_INDEX - y) * Y_RES_MAX_INDEX));
        CHECK(t->m_frag_txpos == x);
        CHECK(t->m_frag_typos == y);
        rr::ut::clk(t);
        if (x + 1 == X_RES)
        {
            y--;
        }
    }

    delete t;
}