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
#include "VStreamConcatFifo.h"

TEST_CASE("Check stream concatenation", "[StreamConcatFifo]")
{
    VStreamConcatFifo* t = new VStreamConcatFifo();

    t->s_stream0_tenable = 1;
    t->s_stream1_tenable = 1;
    t->s_stream2_tenable = 1;
    t->s_stream3_tenable = 1;

    t->s_stream0_tvalid = 0;
    t->s_stream1_tvalid = 0;
    t->s_stream2_tvalid = 0;
    t->s_stream3_tvalid = 0;

    t->m_stream_tready = 0;

    rr::ut::reset(t);

    CHECK(t->m_stream_tvalid == 0);
    CHECK(t->s_stream0_tready == 1);
    CHECK(t->s_stream1_tready == 1);
    CHECK(t->s_stream2_tready == 1);
    CHECK(t->s_stream3_tready == 1);

    t->m_stream_tready = 1;

    for (uint32_t i = 0; i < 2; i++)
    {
        // Push one byte into stream0
        t->s_stream0_tvalid = 1;
        t->s_stream0_tdata = 0;
        rr::ut::clk(t);
        CHECK(t->m_stream_tvalid == 0);
        CHECK(t->s_stream0_tready == 1);

        // Push one byte into stream1
        t->s_stream0_tvalid = 0;
        t->s_stream1_tvalid = 1;
        t->s_stream1_tdata = 1;
        rr::ut::clk(t);
        CHECK(t->m_stream_tvalid == 0);
        CHECK(t->s_stream1_tready == 1);

        // Push one byte into stream2
        t->s_stream1_tvalid = 0;
        t->s_stream2_tvalid = 1;
        t->s_stream2_tdata = 2;
        rr::ut::clk(t);
        CHECK(t->m_stream_tvalid == 0);
        CHECK(t->s_stream2_tready == 1);

        // Push one byte into stream3
        t->s_stream2_tvalid = 0;
        t->s_stream3_tvalid = 1;
        t->s_stream3_tdata = 3;
        rr::ut::clk(t);
        CHECK(t->m_stream_tvalid == 1);
        CHECK(t->m_stream_tdata == 0x03020100);
        CHECK(t->s_stream3_tready == 1);

        t->s_stream3_tvalid = 0;
        rr::ut::clk(t);
        CHECK(t->m_stream_tvalid == 0);
    }

    delete t;
}

TEST_CASE("Check stream interruption", "[StreamConcatFifo]")
{
    VStreamConcatFifo* t = new VStreamConcatFifo();

    t->s_stream0_tenable = 1;
    t->s_stream1_tenable = 1;
    t->s_stream2_tenable = 1;
    t->s_stream3_tenable = 1;

    t->s_stream0_tvalid = 0;
    t->s_stream1_tvalid = 0;
    t->s_stream2_tvalid = 0;
    t->s_stream3_tvalid = 0;

    t->m_stream_tready = 0;

    rr::ut::reset(t);

    // Trigger one transfer
    t->s_stream0_tvalid = 1;
    t->s_stream0_tdata = 0;
    t->s_stream1_tvalid = 1;
    t->s_stream1_tdata = 1;
    t->s_stream2_tvalid = 1;
    t->s_stream2_tdata = 2;
    t->s_stream3_tvalid = 1;
    t->s_stream3_tdata = 3;
    rr::ut::clk(t);
    CHECK(t->m_stream_tvalid == 1);
    CHECK(t->m_stream_tdata == 0x03020100);
    CHECK(t->s_stream0_tready == 1);
    CHECK(t->s_stream1_tready == 1);
    CHECK(t->s_stream2_tready == 1);
    CHECK(t->s_stream3_tready == 1);

    // Cycle without read
    t->s_stream3_tvalid = 0;
    rr::ut::clk(t);
    CHECK(t->m_stream_tvalid == 1);
    CHECK(t->m_stream_tdata == 0x03020100);
    CHECK(t->s_stream3_tready == 1);

    // Set output (combinatorial). Must have a direct effect on the output.
    t->m_stream_tready = 1;
    rr::ut::clk(t);
    CHECK(t->m_stream_tvalid == 0);

    delete t;
}

TEST_CASE("Check full", "[StreamConcatFifo]")
{
    VStreamConcatFifo* t = new VStreamConcatFifo();

    t->s_stream0_tenable = 1;
    t->s_stream1_tenable = 1;
    t->s_stream2_tenable = 1;
    t->s_stream3_tenable = 1;

    t->s_stream0_tvalid = 0;
    t->s_stream1_tvalid = 0;
    t->s_stream2_tvalid = 0;
    t->s_stream3_tvalid = 0;

    t->m_stream_tready = 0;

    rr::ut::reset(t);

    CHECK(t->m_stream_tvalid == 0);
    CHECK(t->s_stream0_tready == 1);
    CHECK(t->s_stream1_tready == 1);
    CHECK(t->s_stream2_tready == 1);
    CHECK(t->s_stream3_tready == 1);

    t->m_stream_tready = 0;

    // Make the fifo full
    for (uint32_t i = 0; i < 31; i++)
    {
        t->s_stream0_tvalid = 1;
        t->s_stream0_tdata = 0;
        t->s_stream1_tvalid = 1;
        t->s_stream1_tdata = 1;
        t->s_stream2_tvalid = 1;
        t->s_stream2_tdata = 2;
        t->s_stream3_tvalid = 1;
        t->s_stream3_tdata = 3;
        rr::ut::clk(t);
        CHECK(t->m_stream_tvalid == 1);
        CHECK(t->s_stream0_tready == 1);
        CHECK(t->s_stream1_tready == 1);
        CHECK(t->s_stream2_tready == 1);
        CHECK(t->s_stream3_tready == 1);
    }

    // Check that the fifos are full
    rr::ut::clk(t);
    CHECK(t->m_stream_tvalid == 1);
    CHECK(t->s_stream0_tready == 0);
    CHECK(t->s_stream1_tready == 0);
    CHECK(t->s_stream2_tready == 0);
    CHECK(t->s_stream3_tready == 0);

    delete t;
}

TEST_CASE("Check read while channels are disabled", "[StreamConcatFifo]")
{
    VStreamConcatFifo* t = new VStreamConcatFifo();

    t->s_stream0_tenable = 1;
    t->s_stream1_tenable = 1;
    t->s_stream2_tenable = 0;
    t->s_stream3_tenable = 1;

    t->s_stream0_tvalid = 0;
    t->s_stream1_tvalid = 0;
    t->s_stream2_tvalid = 0;
    t->s_stream3_tvalid = 0;

    t->m_stream_tready = 0;

    rr::ut::reset(t);

    CHECK(t->m_stream_tvalid == 0);
    CHECK(t->s_stream0_tready == 1);
    CHECK(t->s_stream1_tready == 1);
    CHECK(t->s_stream2_tready == 1);
    CHECK(t->s_stream3_tready == 1);

    t->m_stream_tready = 1;

    for (uint32_t i = 0; i < 2; i++)
    {
        // Push one byte into stream0
        t->s_stream0_tvalid = 1;
        t->s_stream0_tdata = 0;
        rr::ut::clk(t);
        CHECK(t->m_stream_tvalid == 0);
        CHECK(t->s_stream0_tready == 1);

        // Push one byte into stream1
        t->s_stream0_tvalid = 0;
        t->s_stream1_tvalid = 1;
        t->s_stream1_tdata = 1;
        rr::ut::clk(t);
        CHECK(t->m_stream_tvalid == 0);
        CHECK(t->s_stream1_tready == 1);

        // Push no byte into stream2
        t->s_stream1_tvalid = 0;
        t->s_stream2_tvalid = 0;
        t->s_stream2_tdata = 2;
        rr::ut::clk(t);
        CHECK(t->m_stream_tvalid == 0);
        CHECK(t->s_stream2_tready == 1);

        // Push one byte into stream3
        t->s_stream2_tvalid = 0;
        t->s_stream3_tvalid = 1;
        t->s_stream3_tdata = 3;
        rr::ut::clk(t);
        CHECK(t->m_stream_tvalid == 1);
        CHECK(t->m_stream_tdata == 0x03000100);
        CHECK(t->s_stream3_tready == 1);

        t->s_stream3_tvalid = 0;
        rr::ut::clk(t);
        CHECK(t->m_stream_tvalid == 0);
    }

    delete t;
}