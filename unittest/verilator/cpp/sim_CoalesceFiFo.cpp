// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2026 ToNi3141

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
#include "VCoalesceFiFo.h"

TEST_CASE("Push transaction", "[VCoalesceFiFo]")
{
    VCoalesceFiFo* t = new VCoalesceFiFo();

    // Run it two times to make sure, that the last handling works
    rr::ut::reset(t);

    REQUIRE(t->s_mem_axi_awready == 1);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Check that it stays idle when no transaction is pushed
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 1);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Push first transaction (data)
    t->s_mem_axi_wvalid = 1;
    t->s_mem_axi_wdata = 0xDEADBEEF;
    t->s_mem_axi_wstrb = 0xF;
    t->s_mem_axi_wlast = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 1);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Extra cycle for to forward from fifo to master
    // But the transaction is still pending. We have to wait for the address channel
    // to see how the last channel has to be handled. We don't know yet if this is the last
    // beat if a single transaction or the first beat of a burst transaction.
    t->s_mem_axi_wvalid = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 1);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Push first transaction (address)
    t->s_mem_axi_awburst = 1; // INCR
    t->s_mem_axi_awvalid = 1;
    t->s_mem_axi_awaddr = 0x1000;
    t->s_mem_axi_awlen = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->m_mem_axi_awvalid == 1);
    REQUIRE(t->m_mem_axi_awaddr == 0x1000);
    REQUIRE(t->m_mem_axi_awlen == 0);
    REQUIRE(t->m_mem_axi_awid == 1);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0); // The data is one cycle delayed. Simplifies the implementation

    // Acknowledge address
    t->m_mem_axi_awready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 1);
    REQUIRE(t->m_mem_axi_wdata == 0xDEADBEEF);
    REQUIRE(t->m_mem_axi_wstrb == 0xF);
    REQUIRE(t->m_mem_axi_wlast == 1);

    // Acknowledge data
    t->m_mem_axi_wready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 1);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Destroy model
    delete t;
}

TEST_CASE("Test Skid Buffer", "[VCoalesceFiFo]")
{
    VCoalesceFiFo* t = new VCoalesceFiFo();

    // Run it two times to make sure, that the last handling works
    rr::ut::reset(t);

    REQUIRE(t->s_mem_axi_awready == 1);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Check that it stays idle when no transaction is pushed
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 1);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Push first transaction (address)
    t->s_mem_axi_awburst = 1; // INCR
    t->s_mem_axi_awvalid = 1;
    t->s_mem_axi_awaddr = 0x1000;
    t->s_mem_axi_awlen = 2;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->m_mem_axi_awvalid == 1);
    REQUIRE(t->m_mem_axi_awaddr == 0x1000);
    REQUIRE(t->m_mem_axi_awlen == 2);
    REQUIRE(t->m_mem_axi_awid == 1);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Acknowledge address
    t->m_mem_axi_awready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Push first transaction (data)
    t->s_mem_axi_wvalid = 1;
    t->s_mem_axi_wdata = 0xDEADBEEF;
    t->s_mem_axi_wstrb = 0xF;
    t->s_mem_axi_wlast = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Forward from fifo to master and push second transaction
    t->s_mem_axi_wvalid = 1;
    t->s_mem_axi_wdata = 0xCAFEBABE;
    t->s_mem_axi_wstrb = 0xF;
    t->s_mem_axi_wlast = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 1);
    REQUIRE(t->m_mem_axi_wdata == 0xDEADBEEF);
    REQUIRE(t->m_mem_axi_wstrb == 0xF);
    REQUIRE(t->m_mem_axi_wlast == 0);

    // Third transaction
    t->s_mem_axi_wvalid = 1;
    t->s_mem_axi_wdata = 0xDEADC0DE;
    t->s_mem_axi_wstrb = 0xF;
    t->s_mem_axi_wlast = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 1);
    REQUIRE(t->m_mem_axi_wdata == 0xDEADBEEF);
    REQUIRE(t->m_mem_axi_wstrb == 0xF);
    REQUIRE(t->m_mem_axi_wlast == 0);

    // Idle cycle (data should also be in the skid buffer right now)
    t->s_mem_axi_wvalid = 0;
    t->m_mem_axi_wready = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 1);
    REQUIRE(t->m_mem_axi_wdata == 0xDEADBEEF);
    REQUIRE(t->m_mem_axi_wstrb == 0xF);
    REQUIRE(t->m_mem_axi_wlast == 0);

    // Fetch first transaction
    t->s_mem_axi_wvalid = 0;
    t->m_mem_axi_wready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 1);
    REQUIRE(t->m_mem_axi_wdata == 0xCAFEBABE);
    REQUIRE(t->m_mem_axi_wstrb == 0xF);
    REQUIRE(t->m_mem_axi_wlast == 0);

    // Idle
    t->s_mem_axi_wvalid = 0;
    t->m_mem_axi_wready = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 1);
    REQUIRE(t->m_mem_axi_wdata == 0xCAFEBABE);
    REQUIRE(t->m_mem_axi_wstrb == 0xF);
    REQUIRE(t->m_mem_axi_wlast == 0);

    // Fetch second transaction
    t->s_mem_axi_wvalid = 0;
    t->m_mem_axi_wready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 1);
    REQUIRE(t->m_mem_axi_wdata == 0xDEADC0DE);
    REQUIRE(t->m_mem_axi_wstrb == 0xF);
    REQUIRE(t->m_mem_axi_wlast == 1);

    // Idle
    t->s_mem_axi_wvalid = 0;
    t->m_mem_axi_wready = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 1);
    REQUIRE(t->m_mem_axi_wdata == 0xDEADC0DE);
    REQUIRE(t->m_mem_axi_wstrb == 0xF);
    REQUIRE(t->m_mem_axi_wlast == 1);

    // Fetch last transaction
    t->s_mem_axi_wvalid = 0;
    t->m_mem_axi_wready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 1);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Destroy model
    delete t;
}

TEST_CASE("Test Burst", "[VCoalesceFiFo]")
{
    VCoalesceFiFo* t = new VCoalesceFiFo();

    // Run it two times to make sure, that the last handling works
    rr::ut::reset(t);

    REQUIRE(t->s_mem_axi_awready == 1);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Check that it stays idle when no transaction is pushed
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 1);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Push first transaction (address)
    t->s_mem_axi_awburst = 1; // INCR
    t->s_mem_axi_awvalid = 1;
    t->s_mem_axi_awaddr = 0x1000;
    t->s_mem_axi_awlen = 2;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->m_mem_axi_awvalid == 1);
    REQUIRE(t->m_mem_axi_awaddr == 0x1000);
    REQUIRE(t->m_mem_axi_awlen == 2);
    REQUIRE(t->m_mem_axi_awid == 1);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Acknowledge address
    t->m_mem_axi_awready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Push first transaction (data)
    t->m_mem_axi_wready = 1;
    t->s_mem_axi_wvalid = 1;
    t->s_mem_axi_wdata = 0xDEADBEEF;
    t->s_mem_axi_wstrb = 0xF;
    t->s_mem_axi_wlast = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Push second transaction, fetch first
    t->m_mem_axi_wready = 1;
    t->s_mem_axi_wvalid = 1;
    t->s_mem_axi_wdata = 0xCAFEBABE;
    t->s_mem_axi_wstrb = 0xF;
    t->s_mem_axi_wlast = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 1);
    REQUIRE(t->m_mem_axi_wdata == 0xDEADBEEF);
    REQUIRE(t->m_mem_axi_wstrb == 0xF);
    REQUIRE(t->m_mem_axi_wlast == 0);

    // Push third transaction, fetch second
    t->m_mem_axi_wready = 1;
    t->s_mem_axi_wvalid = 1;
    t->s_mem_axi_wdata = 0xDEADC0DE;
    t->s_mem_axi_wstrb = 0xF;
    t->s_mem_axi_wlast = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 1);
    REQUIRE(t->m_mem_axi_wdata == 0xCAFEBABE);
    REQUIRE(t->m_mem_axi_wstrb == 0xF);
    REQUIRE(t->m_mem_axi_wlast == 0);

    // No push. fetch third
    t->m_mem_axi_wready = 1;
    t->s_mem_axi_wvalid = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 1);
    REQUIRE(t->m_mem_axi_wdata == 0xDEADC0DE);
    REQUIRE(t->m_mem_axi_wstrb == 0xF);
    REQUIRE(t->m_mem_axi_wlast == 1);

    // Check that it can handle new transactions
    t->m_mem_axi_wready = 1;
    t->s_mem_axi_wvalid = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 1);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Destroy model
    delete t;
}

TEST_CASE("Test Full/Interleaved FiFo", "[VCoalesceFiFo]")
{
    static constexpr std::size_t MAX_BEATS_TO_COALESCE = 8;

    VCoalesceFiFo* t = new VCoalesceFiFo();

    // Run it two times to make sure, that the last handling works
    rr::ut::reset(t);

    REQUIRE(t->s_mem_axi_awready == 1);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Check that it stays idle when no transaction is pushed
    t->m_mem_axi_wready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 1);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Push first transactions (data) to completely fill the fifo
    for (std::size_t i = 0; i < MAX_BEATS_TO_COALESCE; i++)
    {
        t->s_mem_axi_wvalid = 1;
        t->s_mem_axi_wdata = i;
        t->s_mem_axi_wstrb = 0xF;
        t->s_mem_axi_wlast = 1;
        rr::ut::clk(t);
        REQUIRE(t->s_mem_axi_awready == 1);
        REQUIRE(t->m_mem_axi_awvalid == 0);
        REQUIRE(t->s_mem_axi_wready == (i < MAX_BEATS_TO_COALESCE - 1));
        REQUIRE(t->m_mem_axi_wvalid == 0);
    }

    // Push first half of the beats (address)
    t->s_mem_axi_wvalid = 0;
    t->s_mem_axi_awburst = 1; // INCR
    t->s_mem_axi_awvalid = 1;
    t->s_mem_axi_awaddr = 0x1000;
    t->s_mem_axi_awlen = (MAX_BEATS_TO_COALESCE / 2) - 1;
    t->m_mem_axi_awready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->m_mem_axi_awvalid == 1);
    REQUIRE(t->m_mem_axi_awaddr == 0x1000);
    REQUIRE(t->m_mem_axi_awlen == 3);
    REQUIRE(t->m_mem_axi_awid == 1);
    REQUIRE(t->s_mem_axi_wready == 0);
    REQUIRE(t->m_mem_axi_wvalid == 0); // The data is one cycle delayed. Simplifies the implementation

    // Get data from frist burst
    for (std::size_t i = 0; i < MAX_BEATS_TO_COALESCE / 2; i++)
    {
        t->m_mem_axi_wready = 1;
        t->s_mem_axi_awvalid = 0;
        rr::ut::clk(t);
        REQUIRE(t->s_mem_axi_awready == 0);
        REQUIRE(t->m_mem_axi_awvalid == 0);
        REQUIRE(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == i);
        REQUIRE(t->m_mem_axi_wstrb == 0xF);
        REQUIRE(t->m_mem_axi_wlast == (i == ((MAX_BEATS_TO_COALESCE / 2) - 1)));
    }

    // Wait one cycle till the address channel is ready
    t->s_mem_axi_wvalid = 0;
    t->s_mem_axi_awburst = 1; // INCR
    t->s_mem_axi_awvalid = 1;
    t->s_mem_axi_awaddr = 0x1000;
    t->s_mem_axi_awlen = (MAX_BEATS_TO_COALESCE / 2) - 1;
    t->m_mem_axi_awready = 1;
    rr::ut::clk(t);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->m_mem_axi_wvalid == 0); // The data is one cycle delayed. Simplifies the implementation

    // Push second half of the beats (address)
    t->s_mem_axi_wvalid = 0;
    t->s_mem_axi_awburst = 1; // INCR
    t->s_mem_axi_awvalid = 1;
    t->s_mem_axi_awaddr = 0x2000;
    t->s_mem_axi_awlen = (MAX_BEATS_TO_COALESCE / 2) - 1;
    t->m_mem_axi_awready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 0);
    REQUIRE(t->m_mem_axi_awvalid == 1);
    REQUIRE(t->m_mem_axi_awaddr == 0x2000);
    REQUIRE(t->m_mem_axi_awlen == 3);
    REQUIRE(t->m_mem_axi_awid == 2);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0); // The data is one cycle delayed. Simplifies the implementation

    // Get data from second burst
    for (std::size_t i = MAX_BEATS_TO_COALESCE / 2; i < MAX_BEATS_TO_COALESCE; i++)
    {
        t->m_mem_axi_wready = 1;
        t->s_mem_axi_awvalid = 0;
        rr::ut::clk(t);
        REQUIRE(t->s_mem_axi_awready == 0);
        REQUIRE(t->m_mem_axi_awvalid == 0);
        REQUIRE(t->m_mem_axi_wvalid == 1);
        CHECK(t->m_mem_axi_wdata == i);
        REQUIRE(t->m_mem_axi_wstrb == 0xF);
        REQUIRE(t->m_mem_axi_wlast == (i == (MAX_BEATS_TO_COALESCE - 1)));
    }

    // End
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_awready == 1);
    REQUIRE(t->m_mem_axi_awvalid == 0);
    REQUIRE(t->s_mem_axi_wready == 1);
    REQUIRE(t->m_mem_axi_wvalid == 0);

    // Destroy model
    delete t;
}