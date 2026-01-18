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
#include "VFramebufferReader.h"

TEST_CASE("Integration test of the framebuffer reader", "[FramebufferReader]")
{
    VFramebufferReader* t = new VFramebufferReader();

    t->s_fetch_tvalid = 0;
    t->m_frag_tready = 0;

    t->m_mem_axi_arready = 0;
    t->m_mem_axi_rvalid = 0;

    rr::ut::reset(t);
    CHECK(t->s_fetch_tready == 1);
    CHECK(t->m_frag_tvalid == 0);
    CHECK(t->m_mem_axi_arvalid == 0);
    CHECK(t->m_mem_axi_rready == 0);

    t->confAddr = 0x2000'0000;

    // Request one byte
    t->s_fetch_tvalid = 1;
    t->s_fetch_tlast = 1;
    t->s_fetch_taddr = 0x100;
    rr::ut::clk(t);
    CHECK(t->m_frag_tvalid == 0);
    t->s_fetch_tvalid = 0;
    rr::ut::clk(t);
    CHECK(t->m_frag_tvalid == 0);
    rr::ut::clk(t);
    CHECK(t->m_frag_tvalid == 0);

    // Check for addr request
    CHECK(t->s_fetch_tready == 1);
    CHECK(t->m_mem_axi_arvalid == 1);
    CHECK(t->m_mem_axi_araddr == 0x2000'0200);

    // Read Data
    t->m_mem_axi_rdata = 0x12345678;
    t->m_mem_axi_rvalid = 1;
    rr::ut::clk(t);
    CHECK(t->m_frag_tvalid == 1);
    CHECK(t->m_frag_tdata == 0x5678);
    CHECK(t->m_mem_axi_rready == 1);

    // Check that the data is don't read twice
    t->m_mem_axi_rvalid = 0;
    rr::ut::clk(t);
    CHECK(t->m_frag_tvalid == 1);

    delete t;
}

TEST_CASE("Check stalling. Only check that the fifo content is fine. Deeper tests are already done in the module specific tests.", "[FramebufferReader]")
{
    VFramebufferReader* t = new VFramebufferReader();

    t->s_fetch_tvalid = 0;
    t->m_frag_tready = 1;

    t->m_mem_axi_arready = 1; // Ignore the address channel
    t->m_mem_axi_rvalid = 0;

    rr::ut::reset(t);
    CHECK(t->s_fetch_tready == 1);
    CHECK(t->m_frag_tvalid == 0);
    CHECK(t->m_mem_axi_arvalid == 0);
    CHECK(t->m_mem_axi_rready == 0);

    t->confAddr = 0x2000'0000;

    // Create 128 + 2 (skid) read requests to fill the fifo
    for (uint32_t i = 0; i <= 129; i++)
    {
        t->s_fetch_tvalid = 1;
        t->s_fetch_tlast = 0;
        t->s_fetch_taddr = i;
        rr::ut::clk(t);
        CHECK(t->s_fetch_tready == 1);
    }

    // Check that the fifo is full
    t->s_fetch_tvalid = 1;
    t->s_fetch_tlast = 0;
    t->s_fetch_taddr = 300;
    rr::ut::clk(t);
    CHECK(t->s_fetch_tready == 0);

    t->s_fetch_tvalid = 0;

    // Enable read channel
    t->m_mem_axi_rvalid = 1;
    t->m_mem_axi_rdata = 0;

    // Readout the data from the fifo
    for (uint32_t i = 0; i <= 129; i++)
    {
        rr::ut::clk(t);
        CHECK(t->m_frag_tvalid == 1);
        CHECK(t->m_frag_taddr == i);
        if (i > 1) // Start checking this when the first two pixels are fetched. That is required till the ready is back propagated
            CHECK(t->s_fetch_tready == 1);
    }

    // Read the last data
    rr::ut::clk(t);
    CHECK(t->s_fetch_tready == 1);
    CHECK(t->m_frag_tvalid == 1);
    CHECK(t->m_frag_taddr == 300);

    delete t;
}