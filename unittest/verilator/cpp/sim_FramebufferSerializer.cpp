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
#include "VFramebufferSerializer.h"

TEST_CASE("Check normal burst transfer with interrupted memory stream", "[FramebufferSerializer]")
{
    VFramebufferSerializer* t = new VFramebufferSerializer();

    t->m_frag_tready = 0;

    t->s_fetch_tvalid = 0;
    t->s_fetch_tlast = 0;

    t->m_mem_axi_rid = 0;
    t->m_mem_axi_rresp = 0;
    t->m_mem_axi_rlast = 0;
    t->m_mem_axi_rvalid = 0;

    rr::ut::reset(t);

    // Source and destination are valid
    t->s_fetch_tvalid = 1;
    t->s_fetch_taddr = 0x100;
    t->s_fetch_tlast = 0;
    t->m_frag_tready = 1;
    // Memory is initially not valid
    t->m_mem_axi_rvalid = 0;
    t->m_mem_axi_rdata = 0xffff'5555;

    // Receive new fragment without data in the internal buffer and available from memory
    rr::ut::clk(t);
    REQUIRE(t->m_frag_tvalid == 0);
    REQUIRE(t->m_mem_axi_rready == 0);

    // Memory has now valid data, fetch and acknowledge data and send the current fragment
    t->m_mem_axi_rvalid = 1; // Make memory valid now
    rr::ut::clk(t);
    REQUIRE(t->m_mem_axi_rready == 1); // Acknowledges the memory data
    REQUIRE(t->m_frag_tvalid == 1);
    REQUIRE(t->m_frag_tdata == 0x5555);
    REQUIRE(t->m_frag_taddr == 0x100);
    REQUIRE(t->m_frag_tlast == 0);

    // Fetch cached data. Dont request data from memory
    t->s_fetch_taddr = 0x101;
    rr::ut::clk(t);
    REQUIRE(t->m_mem_axi_rready == 0);
    REQUIRE(t->m_frag_tvalid == 1);
    REQUIRE(t->m_frag_tdata == 0xffff);
    REQUIRE(t->m_frag_taddr == 0x101);
    REQUIRE(t->m_frag_tlast == 0);

    // Request data from memory and directly output it
    t->s_fetch_taddr = 0x102;
    t->m_mem_axi_rdata = 0xaaaa'0000;
    rr::ut::clk(t);
    REQUIRE(t->m_mem_axi_rready == 1); // Acknowledges the memory data
    REQUIRE(t->m_frag_tvalid == 1);
    REQUIRE(t->m_frag_tdata == 0x0000);
    REQUIRE(t->m_frag_taddr == 0x102);
    REQUIRE(t->m_frag_tlast == 0);

    // Fetch cached data
    t->s_fetch_taddr = 0x103;
    t->m_mem_axi_rvalid = 0; // Disable memory
    rr::ut::clk(t);
    REQUIRE(t->m_mem_axi_rready == 0);
    REQUIRE(t->m_frag_tvalid == 1);
    REQUIRE(t->m_frag_tdata == 0xaaaa);
    REQUIRE(t->m_frag_taddr == 0x103);
    REQUIRE(t->m_frag_tlast == 0);

    // New data required but no new data on the memory available
    t->s_fetch_tlast = 1;
    t->s_fetch_taddr = 0x104;
    rr::ut::clk(t);
    REQUIRE(t->m_mem_axi_rready == 0);
    REQUIRE(t->m_frag_tvalid == 0);

    // New data required but no new data on the memory available
    t->s_fetch_taddr = 0x104;
    rr::ut::clk(t);
    REQUIRE(t->m_mem_axi_rready == 0);
    REQUIRE(t->m_frag_tvalid == 0);

    // New data on memory available, last fetch
    t->s_fetch_taddr = 0x104;
    t->m_mem_axi_rdata = 0x1111'2222;
    t->m_mem_axi_rvalid = 1;
    rr::ut::clk(t);
    REQUIRE(t->m_mem_axi_rready == 1);
    REQUIRE(t->m_frag_tvalid == 1);
    REQUIRE(t->m_frag_tdata == 0x2222);
    REQUIRE(t->m_frag_taddr == 0x104);
    REQUIRE(t->m_frag_tlast == 1);

    // Fetch last data (check that the after the tlast a new memory request is forced)
    t->s_fetch_taddr = 0x105;
    t->m_mem_axi_rvalid = 0;
    rr::ut::clk(t);
    REQUIRE(t->m_mem_axi_rready == 0);
    REQUIRE(t->m_frag_tvalid == 0);

    rr::ut::clk(t);
    REQUIRE(t->m_mem_axi_rready == 0);
    REQUIRE(t->m_frag_tvalid == 0);

    rr::ut::clk(t);
    REQUIRE(t->m_mem_axi_rready == 0);
    REQUIRE(t->m_frag_tvalid == 0);

    delete t;
}

TEST_CASE("Check slow memory port", "[FramebufferSerializer]")
{
    VFramebufferSerializer* t = new VFramebufferSerializer();

    t->m_frag_tready = 0;

    t->s_fetch_tvalid = 0;
    t->s_fetch_tlast = 0;

    t->m_mem_axi_rid = 0;
    t->m_mem_axi_rresp = 0;
    t->m_mem_axi_rlast = 0;
    t->m_mem_axi_rvalid = 0;

    rr::ut::reset(t);

    for (uint32_t i = 0; i < 2; i++)
    {
        // Source and destination are valid
        t->s_fetch_tvalid = 1;
        t->s_fetch_taddr = 0x100;
        t->s_fetch_tlast = 0;
        t->m_frag_tready = 1;
        t->m_mem_axi_rvalid = 1;
        t->m_mem_axi_rdata = 0xffff'5555;

        // Memory has now valid data, fetch and acknowledge data and send the current fragment
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 1);
        REQUIRE(t->m_frag_tvalid == 1);
        REQUIRE(t->m_frag_tdata == 0x5555);
        REQUIRE(t->m_frag_taddr == 0x100);
        REQUIRE(t->m_frag_tlast == 0);

        // Fetch cached data. Dont request data from memory
        t->s_fetch_taddr = 0x102;
        t->m_mem_axi_rdata = 0xaaaa'0000;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 0);
        REQUIRE(t->m_frag_tvalid == 0);

        // Request data from memory and directly output it
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 1);
        REQUIRE(t->m_frag_tvalid == 1);
        REQUIRE(t->m_frag_tdata == 0x0000);
        REQUIRE(t->m_frag_taddr == 0x102);
        REQUIRE(t->m_frag_tlast == 0);

        // Fetch cached data
        t->s_fetch_tlast = 1;
        t->s_fetch_taddr = 0x105;
        t->m_mem_axi_rdata = 0x1111'2222;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 0);
        REQUIRE(t->m_frag_tvalid == 0);

        // New data on memory available, last fetch
        t->s_fetch_tvalid = 0;
        t->s_fetch_tlast = 0;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 1);
        REQUIRE(t->m_frag_tvalid == 1);
        REQUIRE(t->m_frag_tdata == 0x1111);
        REQUIRE(t->m_frag_taddr == 0x105);
        REQUIRE(t->m_frag_tlast == 1);

        rr::ut::clk(t); // Extra clock because of the memory bubble cycle
    }

    delete t;
}

TEST_CASE("Check slow fragment port", "[FramebufferSerializer]")
{
    VFramebufferSerializer* t = new VFramebufferSerializer();

    t->m_frag_tready = 0;

    t->s_fetch_tvalid = 0;
    t->s_fetch_tlast = 0;

    t->m_mem_axi_rid = 0;
    t->m_mem_axi_rresp = 0;
    t->m_mem_axi_rlast = 0;
    t->m_mem_axi_rvalid = 0;

    rr::ut::reset(t);

    for (uint32_t i = 0; i < 2; i++)
    {
        // Source and destination are valid
        t->s_fetch_tvalid = 1;
        t->s_fetch_taddr = 0x100;
        t->s_fetch_tlast = 0;
        t->m_mem_axi_rvalid = 1;
        t->m_mem_axi_rdata = 0xffff'5555;

        // Fetch data from memory interface and output it
        t->m_frag_tready = 0;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 1);
        REQUIRE(t->m_frag_tvalid == 1);
        REQUIRE(t->m_frag_tdata == 0x5555);
        REQUIRE(t->m_frag_taddr == 0x100);
        REQUIRE(t->m_frag_tlast == 0);

        // Skid cycle because fragment port is not ready
        t->s_fetch_taddr = 0x101;
        t->m_frag_tready = 0;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 0);
        REQUIRE(t->m_frag_tvalid == 1);
        REQUIRE(t->m_frag_tdata == 0x5555);
        REQUIRE(t->m_frag_taddr == 0x100);
        REQUIRE(t->m_frag_tlast == 0);

        // Fragment port is ready -> output new data
        t->s_fetch_taddr = 0x101;
        t->m_frag_tready = 1;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 0);
        REQUIRE(t->m_frag_tvalid == 1);
        REQUIRE(t->m_frag_tdata == 0xffff);
        REQUIRE(t->m_frag_taddr == 0x101);
        REQUIRE(t->m_frag_tlast == 0);

        // Skid cycle because fragment port is not ready
        t->s_fetch_taddr = 0x102;
        t->m_mem_axi_rdata = 0x1234'5678;
        t->m_frag_tready = 0;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 1);
        REQUIRE(t->m_frag_tvalid == 1);
        REQUIRE(t->m_frag_tdata == 0xffff);
        REQUIRE(t->m_frag_taddr == 0x101);
        REQUIRE(t->m_frag_tlast == 0);

        // Fragment port is ready -> output new data, fetch new data from memory
        t->s_fetch_taddr = 0x102;
        t->m_frag_tready = 1;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 0);
        REQUIRE(t->m_frag_tvalid == 1);
        REQUIRE(t->m_frag_tdata == 0x5678);
        REQUIRE(t->m_frag_taddr == 0x102);
        REQUIRE(t->m_frag_tlast == 0);

        // Skid cycle because fragment port is not ready
        t->s_fetch_taddr = 0x103;
        t->s_fetch_tlast = 1;
        t->m_frag_tready = 0;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 0);
        REQUIRE(t->m_frag_tvalid == 1);
        REQUIRE(t->m_frag_tdata == 0x5678);
        REQUIRE(t->m_frag_taddr == 0x102);
        REQUIRE(t->m_frag_tlast == 0);

        // Fragment port is ready -> output new data
        t->s_fetch_taddr = 0x103;
        t->m_frag_tready = 1;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 0);
        REQUIRE(t->m_frag_tvalid == 1);
        REQUIRE(t->m_frag_tdata == 0x1234);
        REQUIRE(t->m_frag_taddr == 0x103);
        REQUIRE(t->m_frag_tlast == 1);

        // Skid cycle because fragment port is not ready
        t->m_frag_tready = 0;
        t->s_fetch_tvalid = 0;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 0);
        REQUIRE(t->m_frag_tvalid == 1);
        REQUIRE(t->m_frag_tdata == 0x1234);
        REQUIRE(t->m_frag_taddr == 0x103);
        REQUIRE(t->m_frag_tlast == 1);

        // Acknowledge the last data
        t->m_frag_tready = 1;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 0);
        REQUIRE(t->m_frag_tvalid == 0);
        t->m_frag_tready = 0;
    }

    delete t;
}

TEST_CASE("Check slow fetch port", "[FramebufferSerializer]")
{
    VFramebufferSerializer* t = new VFramebufferSerializer();

    t->m_frag_tready = 0;

    t->s_fetch_tvalid = 0;
    t->s_fetch_tlast = 0;

    t->m_mem_axi_rid = 0;
    t->m_mem_axi_rresp = 0;
    t->m_mem_axi_rlast = 0;
    t->m_mem_axi_rvalid = 0;

    rr::ut::reset(t);

    for (uint32_t i = 0; i < 2; i++)
    {
        // Source and destination are valid
        t->s_fetch_tvalid = 0; // No valid data on the fetch interface
        t->s_fetch_taddr = 0x100;
        t->s_fetch_tlast = 0;
        t->m_frag_tready = 1;
        t->m_mem_axi_rvalid = 1;
        t->m_mem_axi_rdata = 0xffff'5555;

        // Wait for valid data on the fetch interface
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 0);
        REQUIRE(t->m_frag_tvalid == 0);

        // Fetch data from fetch interface and memory interface, output it
        t->s_fetch_tvalid = 1;
        t->s_fetch_taddr = 0x100;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 1);
        REQUIRE(t->m_frag_tvalid == 1);
        REQUIRE(t->m_frag_tdata == 0x5555);
        REQUIRE(t->m_frag_taddr == 0x100);
        REQUIRE(t->m_frag_tlast == 0);

        // Wait for valid data on the fetch interface
        t->s_fetch_tvalid = 0;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 0);
        REQUIRE(t->m_frag_tvalid == 0);

        // Fetch cached data from fetch interface, output it
        t->s_fetch_tvalid = 1;
        t->s_fetch_taddr = 0x101;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 0);
        REQUIRE(t->m_frag_tvalid == 1);
        REQUIRE(t->m_frag_tdata == 0xffff);
        REQUIRE(t->m_frag_taddr == 0x101);
        REQUIRE(t->m_frag_tlast == 0);

        // Wait for valid data on the fetch interface
        t->s_fetch_tvalid = 0;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 0);
        REQUIRE(t->m_frag_tvalid == 0);

        // Fetch last data from fetch interface and memory interface, output it
        t->s_fetch_tvalid = 1;
        t->s_fetch_taddr = 0x102;
        t->s_fetch_tlast = 1;
        t->m_mem_axi_rdata = 0xaaaa'bbbb;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_rready == 1);
        REQUIRE(t->m_frag_tvalid == 1);
        REQUIRE(t->m_frag_tdata == 0xbbbb);
        REQUIRE(t->m_frag_taddr == 0x102);
        REQUIRE(t->m_frag_tlast == 1);

        t->s_fetch_tvalid = 0;
        rr::ut::clk(t); // memory bubble cycle
        REQUIRE(t->m_mem_axi_rready == 0);
        REQUIRE(t->m_frag_tvalid == 0);
    }

    delete t;
}