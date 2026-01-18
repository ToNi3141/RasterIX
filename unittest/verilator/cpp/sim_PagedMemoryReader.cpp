// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

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
#include "VPagedMemoryReader.h"

TEST_CASE("Read data from memory and stream it", "[VPagedMemoryReader]")
{
    static constexpr std::size_t BEATS { 256 };
    static constexpr std::size_t PAGE_SIZE { 2048 };
    static constexpr std::size_t TRANSFER_SIZE { BEATS * 4 };

    VPagedMemoryReader* t = new VPagedMemoryReader();
    auto testMemoryAddressGeneration = [t]()
    {
        // Send page to load
        t->s_axis_tvalid = 1;
        t->s_axis_tdata = 0x1000'0000;
        rr::ut::clk(t);
        CHECK(t->s_axis_tready == 0);
        CHECK(t->m_mem_axi_arvalid == 0);
        CHECK(t->m_axis_tvalid == 0);

        for (std::size_t i = 0; i < (PAGE_SIZE / TRANSFER_SIZE); i++)
        {
            // The PagedMemoryReader responses with a memory request
            rr::ut::clk(t);
            CHECK(t->m_mem_axi_arvalid == 1);
            CHECK(t->m_mem_axi_araddr == (0x1000'0000 + (i * TRANSFER_SIZE)));
            CHECK(t->m_axis_tvalid == 0);
            CHECK(t->s_axis_tready == 0);

            // Answer the memory response
            t->m_mem_axi_arready = 1;
            rr::ut::clk(t);
            CHECK(t->m_mem_axi_arvalid == 0);
            CHECK(t->s_axis_tready == 0);
        }

        // Check for last transfer
        t->m_mem_axi_arready = 0;
        rr::ut::clk(t);
    };

    t->s_axis_tvalid = 0;
    t->s_axis_tlast = 0;

    t->m_mem_axi_arready = 0;
    t->m_mem_axi_rvalid = 0;
    t->m_mem_axi_rlast = 0;

    // Reset hardware
    rr::ut::reset(t);
    CHECK(t->m_axis_tvalid == 0);
    CHECK(t->s_axis_tready == 1);

    // Test first transfer
    testMemoryAddressGeneration();
    CHECK(t->s_axis_tready == 1);

    // Test last transfer
    t->s_axis_tlast = 1;
    testMemoryAddressGeneration();
    CHECK(t->s_axis_tready == 0);

    // The memory data is now streamed to the axis interface
    for (std::size_t i = 0; i < ((PAGE_SIZE * 2) / 4) - 1; i++)
    {
        t->m_mem_axi_rlast = ((i % BEATS) == (BEATS - 1));
        t->m_mem_axi_rvalid = 1;
        t->m_mem_axi_rdata = i;
        rr::ut::clk(t);
        CHECK(t->m_axis_tvalid == 1);
        CHECK(t->m_axis_tlast == 0);
        CHECK(t->m_axis_tdata == i);
        // Make sure that the page interface is still stalling
        CHECK(t->s_axis_tready == 0);
    }

    // Last beat is streamed
    t->m_mem_axi_rlast = 1;
    t->m_mem_axi_rvalid = 1;
    t->m_mem_axi_rdata = 123;
    rr::ut::clk(t);
    CHECK(t->m_axis_tvalid == 1);
    CHECK(t->m_axis_tlast == 1);
    CHECK(t->m_axis_tdata == 123);
    // Make sure that the page interface is still stalling
    CHECK(t->s_axis_tready == 0);

    // Last beat is streamed
    t->m_mem_axi_rlast = 0;
    t->m_mem_axi_rvalid = 0;
    t->m_mem_axi_rdata = 0;
    rr::ut::clk(t);
    CHECK(t->m_axis_tvalid == 0);
    // The page interface is now ready again to receive new data
    CHECK(t->s_axis_tready == 1);

    // Destroy model
    delete t;
}
