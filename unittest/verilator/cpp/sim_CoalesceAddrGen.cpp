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
#include "VCoalesceAddrGen.h"

TEST_CASE("Timeout", "[VCoalesceAddrGen]")
{
    VCoalesceAddrGen* t = new VCoalesceAddrGen();

    rr::ut::reset(t);

    t->s_mem_axi_axburst = 1; // INCR
    t->s_mem_axi_axvalid = 1;
    t->s_mem_axi_axaddr = 0x1000;
    t->s_mem_axi_axlen = 0;
    t->s_mem_axi_axsize = 2; // 4 bytes
    t->m_mem_axi_axready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 0);

    for (std::size_t i = 0; i < 5; i++)
    {
        t->s_mem_axi_axvalid = 0;
        rr::ut::clk(t);
        REQUIRE(t->s_mem_axi_axready == 1);
        REQUIRE(t->m_mem_axi_axvalid == 0);
    }

    t->s_mem_axi_axvalid = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 1);
    REQUIRE(t->m_mem_axi_axaddr == 0x1000);
    REQUIRE(t->m_mem_axi_axlen == 0);

    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 0);

    // Destroy model
    delete t;
}

TEST_CASE("Boundary check", "[VCoalesceAddrGen]")
{
    VCoalesceAddrGen* t = new VCoalesceAddrGen();

    rr::ut::reset(t);

    t->s_mem_axi_axburst = 1; // INCR
    t->s_mem_axi_axvalid = 1;
    t->s_mem_axi_axaddr = 0x1000;
    t->s_mem_axi_axlen = 0;
    t->s_mem_axi_axsize = 2; // 4 bytes
    t->m_mem_axi_axready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 0);

    t->s_mem_axi_axburst = 1; // INCR
    t->s_mem_axi_axvalid = 1;
    t->s_mem_axi_axaddr = 0x2000;
    t->s_mem_axi_axlen = 0;
    t->s_mem_axi_axsize = 2; // 4 bytes
    t->m_mem_axi_axready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 1);
    REQUIRE(t->m_mem_axi_axaddr == 0x1000);
    REQUIRE(t->m_mem_axi_axlen == 0);

    for (std::size_t i = 0; i < 5; i++)
    {
        t->s_mem_axi_axvalid = 0;
        rr::ut::clk(t);
        REQUIRE(t->s_mem_axi_axready == 1);
        REQUIRE(t->m_mem_axi_axvalid == 0);
    }

    t->s_mem_axi_axvalid = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 1);
    REQUIRE(t->m_mem_axi_axaddr == 0x2000);
    REQUIRE(t->m_mem_axi_axlen == 0);

    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 0);

    // Destroy model
    delete t;
}

TEST_CASE("Address Order", "[VCoalesceAddrGen]")
{
    VCoalesceAddrGen* t = new VCoalesceAddrGen();

    rr::ut::reset(t);

    t->s_mem_axi_axburst = 1; // INCR
    t->s_mem_axi_axvalid = 1;
    t->s_mem_axi_axaddr = 0x1004;
    t->s_mem_axi_axlen = 0;
    t->s_mem_axi_axsize = 2; // 4 bytes
    t->m_mem_axi_axready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 0);

    t->s_mem_axi_axburst = 1; // INCR
    t->s_mem_axi_axvalid = 1;
    t->s_mem_axi_axaddr = 0x1000;
    t->s_mem_axi_axlen = 0;
    t->s_mem_axi_axsize = 2; // 4 bytes
    t->m_mem_axi_axready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 1);
    REQUIRE(t->m_mem_axi_axaddr == 0x1004);
    REQUIRE(t->m_mem_axi_axlen == 0);

    for (std::size_t i = 0; i < 5; i++)
    {
        t->s_mem_axi_axvalid = 0;
        rr::ut::clk(t);
        REQUIRE(t->s_mem_axi_axready == 1);
        REQUIRE(t->m_mem_axi_axvalid == 0);
    }

    t->s_mem_axi_axvalid = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 1);
    REQUIRE(t->m_mem_axi_axaddr == 0x1000);
    REQUIRE(t->m_mem_axi_axlen == 0);

    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 0);

    // Destroy model
    delete t;
}

TEST_CASE("Max Beats", "[VCoalesceAddrGen]")
{
    VCoalesceAddrGen* t = new VCoalesceAddrGen();

    rr::ut::reset(t);

    for (std::size_t i = 0; i < 8; i++)
    {
        t->s_mem_axi_axburst = 1; // INCR
        t->s_mem_axi_axvalid = 1;
        t->s_mem_axi_axaddr = 0x1000 + (i * 4);
        t->s_mem_axi_axlen = 0;
        t->s_mem_axi_axsize = 2; // 4 bytes
        t->m_mem_axi_axready = 1;
        rr::ut::clk(t);
        REQUIRE(t->s_mem_axi_axready == 1);
        REQUIRE(t->m_mem_axi_axvalid == 0);
    }

    t->s_mem_axi_axvalid = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 1);
    REQUIRE(t->m_mem_axi_axaddr == 0x1000);
    REQUIRE(t->m_mem_axi_axlen == 7);

    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 0);

    // Destroy model
    delete t;
}

TEST_CASE("Create two transaction", "[VCoalesceAddrGen]")
{
    VCoalesceAddrGen* t = new VCoalesceAddrGen();

    rr::ut::reset(t);

    t->s_mem_axi_axburst = 1; // INCR
    t->s_mem_axi_axvalid = 1;
    t->s_mem_axi_axaddr = 0x1000;
    t->s_mem_axi_axlen = 0;
    t->s_mem_axi_axsize = 2; // 4 bytes
    t->m_mem_axi_axready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 0);

    t->s_mem_axi_axburst = 1; // INCR
    t->s_mem_axi_axvalid = 1;
    t->s_mem_axi_axaddr = 0x1004;
    t->s_mem_axi_axlen = 0;
    t->s_mem_axi_axsize = 2; // 4 bytes
    t->m_mem_axi_axready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 0);

    for (std::size_t i = 0; i < 5; i++)
    {
        t->s_mem_axi_axvalid = 0;
        rr::ut::clk(t);
        REQUIRE(t->s_mem_axi_axready == 1);
        REQUIRE(t->m_mem_axi_axvalid == 0);
    }

    t->s_mem_axi_axvalid = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 1);
    REQUIRE(t->m_mem_axi_axaddr == 0x1000);
    REQUIRE(t->m_mem_axi_axlen == 1);

    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 0);

    // Destroy model
    delete t;
}

TEST_CASE("Stall", "[VCoalesceAddrGen]")
{
    VCoalesceAddrGen* t = new VCoalesceAddrGen();

    rr::ut::reset(t);

    t->s_mem_axi_axburst = 1; // INCR
    t->s_mem_axi_axvalid = 1;
    t->s_mem_axi_axaddr = 0x1000;
    t->s_mem_axi_axlen = 0;
    t->s_mem_axi_axsize = 2; // 4 bytes
    t->m_mem_axi_axready = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 0);

    t->s_mem_axi_axburst = 1; // INCR
    t->s_mem_axi_axvalid = 1;
    t->s_mem_axi_axaddr = 0x2000;
    t->s_mem_axi_axlen = 0;
    t->s_mem_axi_axsize = 2; // 4 bytes
    t->m_mem_axi_axready = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 1);
    REQUIRE(t->m_mem_axi_axaddr == 0x1000);
    REQUIRE(t->m_mem_axi_axlen == 0);

    t->s_mem_axi_axburst = 1; // INCR
    t->s_mem_axi_axvalid = 1;
    t->s_mem_axi_axaddr = 0x3000;
    t->s_mem_axi_axlen = 0;
    t->s_mem_axi_axsize = 2; // 4 bytes
    t->m_mem_axi_axready = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 0);
    REQUIRE(t->m_mem_axi_axvalid == 1);
    REQUIRE(t->m_mem_axi_axaddr == 0x1000);
    REQUIRE(t->m_mem_axi_axlen == 0);

    t->s_mem_axi_axburst = 1; // INCR
    t->s_mem_axi_axvalid = 1;
    t->s_mem_axi_axaddr = 0x3000;
    t->s_mem_axi_axlen = 0;
    t->s_mem_axi_axsize = 2; // 4 bytes
    t->m_mem_axi_axready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 1);
    REQUIRE(t->m_mem_axi_axaddr == 0x2000);
    REQUIRE(t->m_mem_axi_axlen == 0);

    t->s_mem_axi_axburst = 1; // INCR
    t->s_mem_axi_axvalid = 1;
    t->s_mem_axi_axaddr = 0x4000;
    t->s_mem_axi_axlen = 0;
    t->s_mem_axi_axsize = 2; // 4 bytes
    t->m_mem_axi_axready = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 0);
    REQUIRE(t->m_mem_axi_axvalid == 1);
    REQUIRE(t->m_mem_axi_axaddr == 0x2000);
    REQUIRE(t->m_mem_axi_axlen == 0);

    t->s_mem_axi_axburst = 1; // INCR
    t->s_mem_axi_axvalid = 1;
    t->s_mem_axi_axaddr = 0x4000;
    t->s_mem_axi_axlen = 0;
    t->s_mem_axi_axsize = 2; // 4 bytes
    t->m_mem_axi_axready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 1);
    REQUIRE(t->m_mem_axi_axaddr == 0x3000);
    REQUIRE(t->m_mem_axi_axlen == 0);

    t->s_mem_axi_axvalid = 0;
    t->m_mem_axi_axready = 0;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 1);
    REQUIRE(t->m_mem_axi_axaddr == 0x3000);
    REQUIRE(t->m_mem_axi_axlen == 0);

    // Note: The 0x4000 is still pending because no abort condition accured jet like a boundary crossing or timeout.
    // It should be sufficient for the test.
    t->s_mem_axi_axvalid = 0;
    t->m_mem_axi_axready = 1;
    rr::ut::clk(t);
    REQUIRE(t->s_mem_axi_axready == 1);
    REQUIRE(t->m_mem_axi_axvalid == 0);

    // Destroy model
    delete t;
}