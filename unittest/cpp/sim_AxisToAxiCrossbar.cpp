// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2025 ToNi3141

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
#include <algorithm>
#include <array>
#include <math.h>

// Include model header, generated from Verilating "top.v"
#include "VAxisToAxiCrossbar.h"

static constexpr std::size_t BYTES_PER_BEAT = 4; // 32 bit
static constexpr std::size_t TEST_SIZE = 0x80; // 128 bytes
static constexpr std::size_t DATA_WIDTH = 32; // 32 bit data width

TEST_CASE("check write channel 0", "[VAxisToAxiCrossbar]")
{
    VAxisToAxiCrossbar* t = new VAxisToAxiCrossbar();

    rr::ut::reset(t);
    CHECK(t->s_aready == 0b0'0);

    t->m_mem_axi_awready = 1;

    t->s_avalid = 0b0'1;
    t->s_aaddr = 0x00000000'10000000;
    t->s_arnw = 0b0'1; // Write
    t->s_abytes = 0x00000000'00000000 + TEST_SIZE;
    rr::ut::clk(t); // port 1 because the rr::ut::reset() already triggers with the extra clock cycle a scheduling
    rr::ut::clk(t); // port 0
    CHECK(t->s_aready == 0b0'1);

    rr::ut::clk(t); // Few clock cycles are required till the AxisToAxiAdapter is ready to send the address
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_awvalid == 1);
    CHECK(t->m_mem_axi_awaddr == t->s_aaddr);
    CHECK(t->m_mem_axi_awlen == 15);
    CHECK(t->m_mem_axi_awsize == 0b010); // 32 bit
    CHECK(t->m_mem_axi_awburst == 0b01); // INCR
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_awvalid == 1);
    CHECK(t->m_mem_axi_awaddr == 0x1000'0040);
    CHECK(t->m_mem_axi_awlen == 15);
    CHECK(t->m_mem_axi_awsize == 0b010); // 32 bit
    CHECK(t->m_mem_axi_awburst == 0b01); // INCR
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_awvalid == 0);
    CHECK(t->s_wready == 0b0'1);

    for (std::size_t i = 0; i < t->s_abytes; i += BYTES_PER_BEAT)
    {
        t->s_wvalid = 0b0'1;
        t->s_wdata = i;
        t->s_wstrb = 0b0000'1111;
        t->s_wlast = (i + BYTES_PER_BEAT) >= t->s_abytes;
        t->m_mem_axi_wready = 1;
        rr::ut::clk(t);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == i);
        CHECK(t->m_mem_axi_wstrb == 0b1111);
        CHECK(t->m_mem_axi_wlast == (((i + BYTES_PER_BEAT) % 64) == 0));
        CHECK(t->s_wready == 0b0'1);
    }
    rr::ut::clk(t);
    CHECK(t->m_mem_axi_wvalid == 0);
    CHECK(t->s_wready == 0b0'0);

    // Destroy model
    delete t;
}

TEST_CASE("check write channel 1", "[VAxisToAxiCrossbar]")
{
    VAxisToAxiCrossbar* t = new VAxisToAxiCrossbar();

    rr::ut::reset(t);
    CHECK(t->s_aready == 0b0'0);

    t->m_mem_axi_awready = 1;

    t->s_avalid = 0b1'0;
    t->s_aaddr = 0x10000000'00000000;
    t->s_arnw = 0b1'0; // Write
    t->s_abytes = 0x00000000'00000000 + (TEST_SIZE << 32);
    rr::ut::clk(t); // port 0
    CHECK(t->s_aready == 0b1'0);

    rr::ut::clk(t); // Few clock cycles are required till the AxisToAxiAdapter is ready to send the address
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_awvalid == 1);
    CHECK(t->m_mem_axi_awaddr == 0x1000'0000);
    CHECK(t->m_mem_axi_awlen == 15);
    CHECK(t->m_mem_axi_awsize == 0b010); // 32 bit
    CHECK(t->m_mem_axi_awburst == 0b01); // INCR
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_awvalid == 1);
    CHECK(t->m_mem_axi_awaddr == 0x1000'0040);
    CHECK(t->m_mem_axi_awlen == 15);
    CHECK(t->m_mem_axi_awsize == 0b010); // 32 bit
    CHECK(t->m_mem_axi_awburst == 0b01); // INCR
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_awvalid == 0);
    CHECK(t->s_wready == 0b1'0);

    for (std::size_t i = 0; i < TEST_SIZE; i += BYTES_PER_BEAT)
    {
        t->s_wvalid = 0b1'0;
        t->s_wdata = (i << DATA_WIDTH);
        t->s_wstrb = 0b1111'0000;
        t->s_wlast = ((i + BYTES_PER_BEAT) >= TEST_SIZE) << 1;
        t->m_mem_axi_wready = 1;
        rr::ut::clk(t);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == i);
        CHECK(t->m_mem_axi_wstrb == 0b1111);
        CHECK(t->m_mem_axi_wlast == (((i + BYTES_PER_BEAT) % 64) == 0));
        CHECK(t->s_wready == 0b1'0);
    }
    rr::ut::clk(t);
    CHECK(t->m_mem_axi_wvalid == 0);
    CHECK(t->s_wready == 0b0'0);

    // Destroy model
    delete t;
}

TEST_CASE("check both write channels", "[VAxisToAxiCrossbar]")
{
    VAxisToAxiCrossbar* t = new VAxisToAxiCrossbar();

    rr::ut::reset(t);
    CHECK(t->s_aready == 0b0'0);

    t->m_mem_axi_awready = 1;

    t->s_avalid = 0b1'1;
    t->s_aaddr = 0x10000000'20000000;
    t->s_arnw = 0b1'1; // Write
    t->s_abytes = 0x00000080'00000040;
    rr::ut::clk(t); // port 0
    CHECK(t->s_aready == 0b1'0);

    // Handle Channel 1
    t->s_avalid = 0b0'1;
    rr::ut::clk(t); // Few clock cycles are required till the AxisToAxiAdapter is ready to send the address
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_awvalid == 1);
    CHECK(t->m_mem_axi_awaddr == 0x1000'0000);
    CHECK(t->m_mem_axi_awlen == 15);
    CHECK(t->m_mem_axi_awsize == 0b010); // 32 bit
    CHECK(t->m_mem_axi_awburst == 0b01); // INCR
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_awvalid == 1);
    CHECK(t->m_mem_axi_awaddr == 0x1000'0040);
    CHECK(t->m_mem_axi_awlen == 15);
    CHECK(t->m_mem_axi_awsize == 0b010); // 32 bit
    CHECK(t->m_mem_axi_awburst == 0b01); // INCR
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_awvalid == 0);
    CHECK(t->s_wready == 0b1'0);

    for (std::size_t i = 0; i < TEST_SIZE; i += BYTES_PER_BEAT)
    {
        t->s_wvalid = 0b1'1;
        t->s_wdata = (i << DATA_WIDTH);
        t->s_wstrb = 0b1111'0000;
        t->s_wlast = ((i + BYTES_PER_BEAT) >= TEST_SIZE) << 1;
        t->m_mem_axi_wready = 1;
        rr::ut::clk(t);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == i);
        CHECK(t->m_mem_axi_wstrb == 0b1111);
        CHECK(t->m_mem_axi_wlast == (((i + BYTES_PER_BEAT) % 64) == 0));
        CHECK(t->s_wready == 0b1'0);
    }

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_wvalid == 0);
    CHECK(t->s_wready == 0b0'0);

    t->s_wvalid = 0b0'0; // This is not really AXI confirm to reset a not acknowledged valid signal, but simplifies the test
    t->m_mem_axi_wready = 0;
    rr::ut::clk(t);
    CHECK(t->m_mem_axi_wvalid == 0);
    CHECK(t->s_wready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_wvalid == 0);
    CHECK(t->s_wready == 0b0'0);
    CHECK(t->s_aready == 0b0'1);

    // Handle channel 0
    rr::ut::clk(t);
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_awvalid == 1);
    CHECK(t->m_mem_axi_awaddr == 0x2000'0000);
    CHECK(t->m_mem_axi_awlen == 15);
    CHECK(t->m_mem_axi_awsize == 0b010); // 32 bit
    CHECK(t->m_mem_axi_awburst == 0b01); // INCR
    CHECK(t->s_aready == 0b0'0);

    t->s_avalid = 0b0'0;
    rr::ut::clk(t);
    CHECK(t->m_mem_axi_awvalid == 0);
    CHECK(t->s_wready == 0b0'1);

    for (std::size_t i = 0; i < TEST_SIZE / 2; i += BYTES_PER_BEAT)
    {
        t->s_wvalid = 0b1'1;
        t->s_wdata = i;
        t->s_wstrb = 0b0000'1111;
        t->s_wlast = ((i + BYTES_PER_BEAT) >= (TEST_SIZE / 2));
        t->m_mem_axi_wready = 1;
        rr::ut::clk(t);
        CHECK(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == i);
        CHECK(t->m_mem_axi_wstrb == 0b1111);
        CHECK(t->m_mem_axi_wlast == (((i + BYTES_PER_BEAT) % 64) == 0));
        CHECK(t->s_wready == 0b0'1);
    }
    rr::ut::clk(t);
    CHECK(t->m_mem_axi_wvalid == 0);
    CHECK(t->s_wready == 0b0'0);

    // Destroy model
    delete t;
}

TEST_CASE("check read channel 0", "[VAxisToAxiCrossbar]")
{
    VAxisToAxiCrossbar* t = new VAxisToAxiCrossbar();

    rr::ut::reset(t);
    CHECK(t->s_aready == 0b0'0);

    t->m_mem_axi_arready = 1;

    t->s_avalid = 0b0'1;
    t->s_aaddr = 0x00000000'10000000;
    t->s_arnw = 0b0'0; // Read
    t->s_abytes = 0x00000000'00000080;
    rr::ut::clk(t); // port 1 because the rr::ut::reset() already triggers with the extra clock cycle a scheduling
    rr::ut::clk(t); // port 0
    CHECK(t->s_aready == 0b0'1);

    rr::ut::clk(t); // Few clock cycles are required till the AxisToAxiAdapter is ready to send the address
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_arvalid == 1);
    CHECK(t->m_mem_axi_araddr == 0x1000'0000);
    CHECK(t->m_mem_axi_arlen == 15);
    CHECK(t->m_mem_axi_arsize == 0b010); // 32 bit
    CHECK(t->m_mem_axi_arburst == 0b01); // INCR
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_arvalid == 1);
    CHECK(t->m_mem_axi_araddr == 0x1000'0040);
    CHECK(t->m_mem_axi_arlen == 15);
    CHECK(t->m_mem_axi_arsize == 0b010); // 32 bit
    CHECK(t->m_mem_axi_arburst == 0b01); // INCR
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_arvalid == 0);
    CHECK(t->m_mem_axi_rready == 0b1);

    for (std::size_t i = 0; i < TEST_SIZE; i += BYTES_PER_BEAT)
    {
        t->m_mem_axi_rvalid = 0b1;
        t->m_mem_axi_rdata = i;
        t->m_mem_axi_rlast = (((i + BYTES_PER_BEAT) % 64) == 0);
        t->s_rready = 0b0'1;
        rr::ut::clk(t);
        CHECK((t->s_rvalid & 0b0'1) == 1);
        CHECK((t->s_rdata & 0xffffffff) == i);
        CHECK((t->s_rlast & 0b0'1) == (i + BYTES_PER_BEAT) >= TEST_SIZE);
        CHECK(t->m_mem_axi_rready == 0b1);
    }
    rr::ut::clk(t);
    CHECK(t->s_rvalid == 0);
    CHECK(t->m_mem_axi_rready == 0b0);

    // Destroy model
    delete t;
}

TEST_CASE("check read channel 1", "[VAxisToAxiCrossbar]")
{
    VAxisToAxiCrossbar* t = new VAxisToAxiCrossbar();

    rr::ut::reset(t);
    CHECK(t->s_aready == 0b0'0);

    t->m_mem_axi_arready = 1;

    t->s_avalid = 0b1'0;
    t->s_aaddr = 0x10000000'00000000;
    t->s_arnw = 0b0'0; // Read
    t->s_abytes = 0x00000080'00000000;
    rr::ut::clk(t); // port 1 because the rr::ut::reset() already triggers with the extra clock cycle a scheduling
    CHECK(t->s_aready == 0b1'0);

    rr::ut::clk(t); // Few clock cycles are required till the AxisToAxiAdapter is ready to send the address
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_arvalid == 1);
    CHECK(t->m_mem_axi_araddr == 0x1000'0000);
    CHECK(t->m_mem_axi_arlen == 15);
    CHECK(t->m_mem_axi_arsize == 0b010); // 32 bit
    CHECK(t->m_mem_axi_arburst == 0b01); // INCR
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_arvalid == 1);
    CHECK(t->m_mem_axi_araddr == 0x1000'0040);
    CHECK(t->m_mem_axi_arlen == 15);
    CHECK(t->m_mem_axi_arsize == 0b010); // 32 bit
    CHECK(t->m_mem_axi_arburst == 0b01); // INCR
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_arvalid == 0);
    CHECK(t->m_mem_axi_rready == 0b1);

    for (std::size_t i = 0; i < TEST_SIZE; i += BYTES_PER_BEAT)
    {
        t->m_mem_axi_rvalid = 0b1;
        t->m_mem_axi_rdata = i;
        t->m_mem_axi_rlast = (((i + BYTES_PER_BEAT) % 64) == 0);
        t->s_rready = 0b1'0;
        rr::ut::clk(t);
        CHECK(((t->s_rvalid & 0b1'0) >> 1) == 1);
        CHECK(((t->s_rdata >> DATA_WIDTH) & 0xffffffff) == i);
        CHECK(((t->s_rlast & 0b1'0) >> 1) == (i + BYTES_PER_BEAT) >= TEST_SIZE);
        CHECK(t->m_mem_axi_rready == 0b1);
    }
    rr::ut::clk(t);
    CHECK(t->s_rvalid == 0);
    CHECK(t->m_mem_axi_rready == 0b0);

    // Destroy model
    delete t;
}

TEST_CASE("check both read channels", "[VAxisToAxiCrossbar]")
{
    VAxisToAxiCrossbar* t = new VAxisToAxiCrossbar();

    rr::ut::reset(t);
    CHECK(t->s_aready == 0b0'0);

    t->m_mem_axi_arready = 1;

    t->s_avalid = 0b1'1;
    t->s_aaddr = 0x10000000'20000000;
    t->s_arnw = 0b0'0; // Read
    t->s_abytes = 0x00000080'00000040;
    rr::ut::clk(t); // port 1 because the rr::ut::reset() already triggers with the extra clock cycle a scheduling
    CHECK(t->s_aready == 0b1'0);

    rr::ut::clk(t); // Few clock cycles are required till the AxisToAxiAdapter is ready to send the address
    rr::ut::clk(t);
    CHECK(t->m_mem_axi_arvalid == 1);
    CHECK(t->m_mem_axi_araddr == 0x1000'0000);
    CHECK(t->m_mem_axi_arlen == 15);
    CHECK(t->m_mem_axi_arsize == 0b010); // 32 bit
    CHECK(t->m_mem_axi_arburst == 0b01); // INCR
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_arvalid == 1);
    CHECK(t->m_mem_axi_araddr == 0x1000'0040);
    CHECK(t->m_mem_axi_arlen == 15);
    CHECK(t->m_mem_axi_arsize == 0b010); // 32 bit
    CHECK(t->m_mem_axi_arburst == 0b01); // INCR
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_arvalid == 0);
    CHECK(t->m_mem_axi_rready == 0b1);

    for (std::size_t i = 0; i < TEST_SIZE; i += BYTES_PER_BEAT)
    {
        t->m_mem_axi_rvalid = 0b1;
        t->m_mem_axi_rdata = i;
        t->m_mem_axi_rlast = (((i + BYTES_PER_BEAT) % 64) == 0);
        t->s_rready = 0b1'0;
        rr::ut::clk(t);
        CHECK(((t->s_rvalid & 0b1'0) >> 1) == 1);
        CHECK(((t->s_rdata >> DATA_WIDTH) & 0xffffffff) == i);
        CHECK(((t->s_rlast & 0b1'0) >> 1) == (i + BYTES_PER_BEAT) >= TEST_SIZE);
        CHECK(t->m_mem_axi_rready == 0b1);
    }
    t->m_mem_axi_rvalid = 0;
    rr::ut::clk(t);
    CHECK(t->s_rvalid == 0);
    CHECK(t->m_mem_axi_rready == 0b0);
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->s_rvalid == 0);
    CHECK(t->m_mem_axi_rready == 0b0);
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->s_rvalid == 0);
    CHECK(t->m_mem_axi_rready == 0b0);
    CHECK(t->s_aready == 0b0'1);

    rr::ut::clk(t); // Few clock cycles are required till the AxisToAxiAdapter is ready to send the address
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_arvalid == 1);
    CHECK(t->m_mem_axi_araddr == 0x2000'0000);
    CHECK(t->m_mem_axi_arlen == 15);
    CHECK(t->m_mem_axi_arsize == 0b010); // 32 bit
    CHECK(t->m_mem_axi_arburst == 0b01); // INCR
    CHECK(t->s_aready == 0b0'0);

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_arvalid == 0);
    CHECK(t->m_mem_axi_rready == 0b1);

    for (std::size_t i = 0; i < TEST_SIZE / 2; i += BYTES_PER_BEAT)
    {
        t->m_mem_axi_rvalid = 0b1;
        t->m_mem_axi_rdata = i;
        t->m_mem_axi_rlast = (((i + BYTES_PER_BEAT) % 64) == 0);
        t->s_rready = 0b0'1;
        rr::ut::clk(t);
        CHECK((t->s_rvalid & 0b0'1) == 1);
        CHECK((t->s_rdata & 0xffffffff) == i);
        CHECK((t->s_rlast & 0b0'1) == (i + BYTES_PER_BEAT) >= (TEST_SIZE / 2));
        CHECK(t->m_mem_axi_rready == 0b1);
    }
    rr::ut::clk(t);
    CHECK(t->s_rvalid == 0);
    CHECK(t->m_mem_axi_rready == 0b0);

    // Destroy model
    delete t;
}