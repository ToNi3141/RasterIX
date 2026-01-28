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
#include "VMemoryReadRequestGenerator.h"

TEST_CASE("Check complete memory request sequence (uninterrupted)", "[MemoryReadRequestGenerator]")
{
    VMemoryReadRequestGenerator* t = new VMemoryReadRequestGenerator();
    t->s_fetch_tlast = 0;
    t->s_fetch_tvalid = 0;
    t->m_mem_axi_arready = 1;
    rr::ut::reset(t);

    t->confAddr = 0x1000'0000;

    // Do this twice, to see, if also a second cycle is possible
    uint32_t id = 1;
    for (uint32_t i = 0; i < 2; i++)
    {
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 1. fetch (start memory request)
        t->s_fetch_tvalid = true;
        t->s_fetch_taddr = 0;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 2. fetch (do memory request)
        t->s_fetch_tvalid = true;
        t->s_fetch_taddr = 1;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arid == id++);
        REQUIRE(t->m_mem_axi_arlen == 0);
        REQUIRE(t->m_mem_axi_arsize == 2);
        REQUIRE(t->m_mem_axi_arburst == 1);
        REQUIRE(t->m_mem_axi_arlock == 0);
        REQUIRE(t->m_mem_axi_arcache == 0);
        REQUIRE(t->m_mem_axi_arprot == 0);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);

        // Send 3. fetch (start memory request)
        t->s_fetch_tvalid = true;
        t->s_fetch_taddr = 2;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 4. fetch (do memory request)
        t->s_fetch_tvalid = true;
        t->s_fetch_taddr = 3;
        rr::ut::clk(t);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0004);
        REQUIRE(t->m_mem_axi_arid == id++);

        // Send 5. fetch
        t->s_fetch_tvalid = true;
        // Only important for the next cycle, to request again 0. Otherwise it wouldn't
        // request zero again, because it seems that the same address was already requested
        t->s_fetch_tlast = true;
        t->s_fetch_taddr = 0;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Request
        t->s_fetch_tvalid = false;
        t->s_fetch_tlast = false;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);
        REQUIRE(t->m_mem_axi_arid == id++);

        // Execute more clock cycles, no additional requests should be executed
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);
    }

    // Destroy model
    delete t;
}

TEST_CASE("Interrupted fetch stream", "[MemoryReadRequestGenerator]")
{
    VMemoryReadRequestGenerator* t = new VMemoryReadRequestGenerator();
    t->s_fetch_tlast = 0;
    t->s_fetch_tvalid = 0;
    t->m_mem_axi_arready = 1;
    rr::ut::reset(t);

    t->confAddr = 0x1000'0000;

    // Do this twice, to see, if also a second cycle is possible
    for (uint32_t i = 0; i < 2; i++)
    {
        // Send 1. fetch
        t->s_fetch_tlast = false;
        t->s_fetch_tvalid = true;
        t->s_fetch_taddr = 0;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 2.1 fetch
        t->s_fetch_tvalid = false;
        t->s_fetch_taddr = 1;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);

        // Send 2.2 fetch
        t->s_fetch_tvalid = false;
        t->s_fetch_taddr = 1;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 2.3 fetch
        t->s_fetch_tvalid = true;
        t->s_fetch_taddr = 1;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 3. fetch.
        t->s_fetch_tvalid = true;
        t->s_fetch_taddr = 2;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 4.1 fetch.
        t->s_fetch_tvalid = false;
        t->s_fetch_tlast = true;
        t->s_fetch_taddr = 3;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0004);
    }

    // Destroy model
    delete t;
}

TEST_CASE("Interrupted memory stream with tlast in skid buffer", "[MemoryReadRequestGenerator]")
{
    VMemoryReadRequestGenerator* t = new VMemoryReadRequestGenerator();
    t->s_fetch_tlast = 0;
    t->s_fetch_tvalid = 0;
    t->m_mem_axi_arready = 0;
    rr::ut::reset(t);

    t->confAddr = 0x1000'0000;

    // Do this twice, to see, if also a second cycle is possible
    for (uint32_t i = 0; i < 2; i++)
    {
        t->m_mem_axi_arready = 0;

        // Send 1. fetch
        t->s_fetch_tvalid = true;
        t->s_fetch_taddr = 0;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 2. fetch
        t->s_fetch_tvalid = true;
        t->s_fetch_taddr = 1;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);

        // Send 3. fetch
        t->s_fetch_tvalid = true;
        t->s_fetch_taddr = 2;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);

        // Send 4. fetch
        t->s_fetch_tvalid = true;
        t->s_fetch_taddr = 3;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);

        // Send 5. fetch
        t->s_fetch_tvalid = true;
        t->s_fetch_taddr = 4;
        t->s_fetch_tlast = true;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == false);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);

        // Mem requests
        t->s_fetch_tvalid = false;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == false);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);

        // Mem requests
        t->s_fetch_tvalid = false;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == false);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);

        // Mem requests
        t->s_fetch_tvalid = false;
        t->m_mem_axi_arready = true;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == false);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Mem requests
        t->s_fetch_tvalid = false;
        t->m_mem_axi_arready = true;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == false);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0004);

        // Mem requests
        t->s_fetch_tvalid = false;
        t->s_fetch_tlast = false;
        t->m_mem_axi_arready = true;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Last mem request
        t->s_fetch_tvalid = false;
        t->m_mem_axi_arready = false;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0008);

        // Last mem request
        t->s_fetch_tvalid = false;
        t->m_mem_axi_arready = false;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0008);

        // Last mem request
        t->s_fetch_tvalid = false;
        t->m_mem_axi_arready = true;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);
    }

    // Destroy model
    delete t;
}

TEST_CASE("tlast after one clock", "[MemoryReadRequestGenerator]")
{
    VMemoryReadRequestGenerator* t = new VMemoryReadRequestGenerator();
    t->s_fetch_tlast = 0;
    t->s_fetch_tvalid = 0;
    t->m_mem_axi_arready = 1;
    rr::ut::reset(t);

    t->confAddr = 0x1000'0000;

    // Do this twice, to see, if also a second cycle is possible
    for (uint32_t i = 0; i < 2; i++)
    {
        // Send 1. fetch
        t->s_fetch_tvalid = true;
        t->s_fetch_taddr = 0;
        t->s_fetch_tlast = true;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Memory request
        t->s_fetch_tvalid = false;
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);

        // End
        rr::ut::clk(t);
        REQUIRE(t->s_fetch_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);
    }

    // Destroy model
    delete t;
}