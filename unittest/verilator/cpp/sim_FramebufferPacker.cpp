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
#include "VFramebufferPacker.h"

TEST_CASE("Write one pixel into framebuffer", "[FramebufferPacker]")
{
    VFramebufferPacker* t = new VFramebufferPacker();

    t->s_frag_tvalid = 0;
    t->s_frag_tlast = 0;

    t->m_mem_axi_awready = 1;
    t->m_mem_axi_wready = 1;

    rr::ut::reset(t);

    CHECK(t->m_mem_axi_awvalid == 0);
    CHECK(t->m_mem_axi_wvalid == 0);
    CHECK(t->m_mem_axi_awsize == 2);
    CHECK(t->m_mem_axi_awburst == 1);
    CHECK(t->m_mem_axi_awlock == 0);
    CHECK(t->m_mem_axi_awcache == 3);
    CHECK(t->m_mem_axi_awprot == 0);
    CHECK(t->m_mem_axi_bready == 1);

    CHECK(t->s_frag_tready == 1);

    // Write one pixel
    for (uint32_t i = 0; i < 2; i++)
    {
        t->s_frag_tvalid = 1;
        t->s_frag_tlast = 1;
        t->s_frag_tdata = 0x1234;
        t->s_frag_tstrb = 0x3;
        t->s_frag_taddr = 0x0;

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        CHECK(t->s_frag_tready == 0);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 1);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_awaddr == 0x0000'0000);
        CHECK(t->m_mem_axi_awlen == 0);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == 0x0000'1234);
        CHECK(t->m_mem_axi_wstrb == 0b00'11);

        rr::ut::clk(t);
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);
    }

    delete t;
}

TEST_CASE("Write stream of pixels into framebuffer", "[FramebufferPacker]")
{
    VFramebufferPacker* t = new VFramebufferPacker();

    t->s_frag_tvalid = 0;
    t->s_frag_tlast = 0;

    t->m_mem_axi_awready = 1;
    t->m_mem_axi_wready = 1;

    rr::ut::reset(t);

    CHECK(t->s_frag_tready == 1);

    CHECK(t->m_mem_axi_awvalid == 0);
    CHECK(t->m_mem_axi_wvalid == 0);
    CHECK(t->m_mem_axi_awsize == 2);
    CHECK(t->m_mem_axi_awburst == 1);
    CHECK(t->m_mem_axi_awlock == 0);
    CHECK(t->m_mem_axi_awcache == 3);
    CHECK(t->m_mem_axi_awprot == 0);
    CHECK(t->m_mem_axi_bready == 1);

    // Write stream of pixels
    for (uint32_t i = 0; i < 2; i++)
    {
        t->s_frag_tvalid = 1;
        t->s_frag_tlast = 0;
        t->s_frag_tdata = 0x1234;
        t->s_frag_tstrb = 0x3;
        t->s_frag_taddr = 0x0;

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0x5678;
        t->s_frag_taddr = 0x2;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xaaaa;
        t->s_frag_taddr = 0x4;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xbbbb;
        t->s_frag_taddr = 0x6;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xcccc;
        t->s_frag_taddr = 0x8;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 1);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_awaddr == 0x0000'0000);
        CHECK(t->m_mem_axi_awlen == 0);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == 0x5678'1234);
        CHECK(t->m_mem_axi_wstrb == 0b11'11);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xdddd;
        t->s_frag_taddr = 0xA;
        t->s_frag_tlast = 1;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        CHECK(t->s_frag_tready == 0);
        CHECK(t->m_mem_axi_awvalid == 1);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_awaddr == 0x0000'0004);
        CHECK(t->m_mem_axi_awlen == 0);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == 0xbbbb'aaaa);
        CHECK(t->m_mem_axi_wstrb == 0b11'11);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 1);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_awaddr == 0x0000'0008);
        CHECK(t->m_mem_axi_awlen == 0);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == 0xdddd'cccc);
        CHECK(t->m_mem_axi_wstrb == 0b11'11);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);
    }

    delete t;
}

TEST_CASE("Write stream of pixels with interruptions", "[FramebufferPacker]")
{
    VFramebufferPacker* t = new VFramebufferPacker();

    t->s_frag_tvalid = 0;
    t->s_frag_tlast = 0;

    t->m_mem_axi_awready = 1;
    t->m_mem_axi_wready = 1;

    rr::ut::reset(t);

    CHECK(t->s_frag_tready == 1);

    CHECK(t->m_mem_axi_awvalid == 0);
    CHECK(t->m_mem_axi_wvalid == 0);
    CHECK(t->m_mem_axi_awsize == 2);
    CHECK(t->m_mem_axi_awburst == 1);
    CHECK(t->m_mem_axi_awlock == 0);
    CHECK(t->m_mem_axi_awcache == 3);
    CHECK(t->m_mem_axi_awprot == 0);
    CHECK(t->m_mem_axi_bready == 1);

    // Write stream of pixels
    for (uint32_t i = 0; i < 2; i++)
    {
        t->s_frag_tvalid = 1;
        t->s_frag_tlast = 0;
        t->s_frag_tdata = 0x1234;
        t->s_frag_tstrb = 0x3;
        t->s_frag_taddr = 0x0;

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        t->s_frag_tdata = 0x5678;
        t->s_frag_taddr = 0x2;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0x5678;
        t->s_frag_taddr = 0x2;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        t->s_frag_tdata = 0xaaaa;
        t->s_frag_taddr = 0x4;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        t->s_frag_tdata = 0xaaaa;
        t->s_frag_taddr = 0x4;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xaaaa;
        t->s_frag_taddr = 0x4;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        t->s_frag_tdata = 0xbbbb;
        t->s_frag_taddr = 0x6;
        t->s_frag_tlast = 1;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xbbbb;
        t->s_frag_taddr = 0x6;
        t->s_frag_tlast = 1;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 1);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_awaddr == 0x0000'0000);
        CHECK(t->m_mem_axi_awlen == 0);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == 0x5678'1234);
        CHECK(t->m_mem_axi_wstrb == 0b11'11);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        CHECK(t->s_frag_tready == 0);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 1);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_awaddr == 0x0000'0004);
        CHECK(t->m_mem_axi_awlen == 0);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == 0xbbbb'aaaa);
        CHECK(t->m_mem_axi_wstrb == 0b11'11);

        rr::ut::clk(t);
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);
    }

    delete t;
}

TEST_CASE("Write stream of pixels with slow memory", "[FramebufferPacker]")
{
    VFramebufferPacker* t = new VFramebufferPacker();

    t->s_frag_tvalid = 0;
    t->s_frag_tlast = 0;

    t->m_mem_axi_awready = 0;
    t->m_mem_axi_wready = 0;

    rr::ut::reset(t);

    CHECK(t->s_frag_tready == 1);

    CHECK(t->m_mem_axi_awvalid == 0);
    CHECK(t->m_mem_axi_wvalid == 0);
    CHECK(t->m_mem_axi_awsize == 2);
    CHECK(t->m_mem_axi_awburst == 1);
    CHECK(t->m_mem_axi_awlock == 0);
    CHECK(t->m_mem_axi_awcache == 3);
    CHECK(t->m_mem_axi_awprot == 0);
    CHECK(t->m_mem_axi_bready == 1);

    // Write stream of pixels
    for (uint32_t i = 0; i < 2; i++)
    {
        t->m_mem_axi_awready = 0;
        t->m_mem_axi_wready = 0;

        t->s_frag_tvalid = 1;
        t->s_frag_tlast = 0;
        t->s_frag_tdata = 0x1234;
        t->s_frag_tstrb = 0x3;
        t->s_frag_taddr = 0x0;

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0x5678;
        t->s_frag_taddr = 0x2;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xaaaa;
        t->s_frag_taddr = 0x4;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xbbbb;
        t->s_frag_taddr = 0x6;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xcccc;
        t->s_frag_taddr = 0x8;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 1);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_awaddr == 0x0000'0000);
        CHECK(t->m_mem_axi_awlen == 0);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == 0x5678'1234);
        CHECK(t->m_mem_axi_wstrb == 0b11'11);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xdddd;
        t->s_frag_taddr = 0xA;
        t->s_frag_tlast = 0;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 1);
        CHECK(t->m_mem_axi_wvalid == 1);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xeeee;
        t->s_frag_taddr = 0xC;
        t->s_frag_tlast = 0;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 1);
        CHECK(t->m_mem_axi_wvalid == 1);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xffff;
        t->s_frag_taddr = 0xE;
        t->s_frag_tlast = 1;
        CHECK(t->s_frag_tready == 0);
        CHECK(t->m_mem_axi_awvalid == 1);
        CHECK(t->m_mem_axi_wvalid == 1);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xffff;
        t->s_frag_taddr = 0xE;
        t->s_frag_tlast = 1;
        t->m_mem_axi_awready = 1;
        t->m_mem_axi_wready = 1;
        CHECK(t->s_frag_tready == 0);
        CHECK(t->m_mem_axi_awvalid == 1);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_awaddr == 0x0000'0000);
        CHECK(t->m_mem_axi_awlen == 0);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == 0x5678'1234);
        CHECK(t->m_mem_axi_wstrb == 0b11'11);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xffff;
        t->s_frag_taddr = 0xE;
        t->s_frag_tlast = 1;
        t->m_mem_axi_awready = 1;
        t->m_mem_axi_wready = 1;
        CHECK(t->s_frag_tready == 0);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xffff;
        t->s_frag_taddr = 0xE;
        t->s_frag_tlast = 1;
        t->m_mem_axi_awready = 1;
        t->m_mem_axi_wready = 1;
        CHECK(t->s_frag_tready == 0);
        CHECK(t->m_mem_axi_awvalid == 1);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_awaddr == 0x0000'0004);
        CHECK(t->m_mem_axi_awlen == 0);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == 0xbbbb'aaaa);
        CHECK(t->m_mem_axi_wstrb == 0b11'11);

        rr::ut::clk(t);
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xffff;
        t->s_frag_taddr = 0xE;
        t->s_frag_tlast = 1;
        t->m_mem_axi_awready = 1;
        t->m_mem_axi_wready = 1;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        t->m_mem_axi_awready = 1;
        t->m_mem_axi_wready = 1;
        CHECK(t->s_frag_tready == 0);
        CHECK(t->m_mem_axi_awvalid == 1);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_awaddr == 0x0000'0008);
        CHECK(t->m_mem_axi_awlen == 0);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == 0xdddd'cccc);
        CHECK(t->m_mem_axi_wstrb == 0b11'11);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        t->m_mem_axi_awready = 1;
        t->m_mem_axi_wready = 1;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        t->m_mem_axi_awready = 1;
        t->m_mem_axi_wready = 1;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 1);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_awaddr == 0x0000'000C);
        CHECK(t->m_mem_axi_awlen == 0);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == 0xffff'eeee);
        CHECK(t->m_mem_axi_wstrb == 0b11'11);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        t->m_mem_axi_awready = 1;
        t->m_mem_axi_wready = 1;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);
    }

    delete t;
}

TEST_CASE("Write pixels with selective strobe", "[FramebufferPacker]")
{
    VFramebufferPacker* t = new VFramebufferPacker();

    t->s_frag_tvalid = 0;
    t->s_frag_tlast = 0;

    t->m_mem_axi_awready = 1;
    t->m_mem_axi_wready = 1;

    rr::ut::reset(t);

    CHECK(t->s_frag_tready == 1);

    // Write stream of pixels where some have zeroed strobe (simulating scissor/mask)
    for (uint32_t i = 0; i < 2; i++)
    {
        // pixel 0: strb=0x3 (write)
        t->s_frag_tvalid = 1;
        t->s_frag_tlast = 0;
        t->s_frag_tdata = 0x1234;
        t->s_frag_tstrb = 0x3;
        t->s_frag_taddr = 0x0;

        rr::ut::clk(t);
        // pixel 1: strb=0x0 (masked out)
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0x5678;
        t->s_frag_tstrb = 0x0;
        t->s_frag_taddr = 0x2;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        // pixel 2: strb=0x3 (write)
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xaaaa;
        t->s_frag_tstrb = 0x3;
        t->s_frag_taddr = 0x4;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        // pixel 3: strb=0x0 (masked out)
        t->s_frag_tvalid = 1;
        t->s_frag_tdata = 0xbbbb;
        t->s_frag_tstrb = 0x0;
        t->s_frag_taddr = 0x6;
        t->s_frag_tlast = 1;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        CHECK(t->s_frag_tready == 0);
        CHECK(t->m_mem_axi_awvalid == 1);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_awaddr == 0x0000'0000);
        CHECK(t->m_mem_axi_awlen == 0);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == 0x5678'1234);
        CHECK(t->m_mem_axi_wstrb == 0b00'11);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 1);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_awaddr == 0x0000'0004);
        CHECK(t->m_mem_axi_awlen == 0);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == 0xbbbb'aaaa);
        CHECK(t->m_mem_axi_wstrb == 0b00'11);

        rr::ut::clk(t);
        t->s_frag_tvalid = 0;
        CHECK(t->s_frag_tready == 1);
        CHECK(t->m_mem_axi_awvalid == 0);
        CHECK(t->m_mem_axi_wvalid == 0);
    }

    delete t;
}
