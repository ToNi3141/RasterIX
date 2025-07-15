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
#include "VAxisFramebufferWriter.h"

TEST_CASE("check address channel", "[VAxisFramebufferWriter]")
{
    constexpr std::size_t PIXEL_PER_BEAT = 2;

    VAxisFramebufferWriter* t = new VAxisFramebufferWriter();

    t->s_disp_axis_tvalid = true;
    rr::ut::reset(t);
    CHECK(t->fb_committed == true);

    t->commit_fb = true;
    t->fb_addr = 0x1000'0000;
    t->fb_size = 0x100;
    t->m_mem_axi_awready = true;
    t->m_mem_axi_wready = true;
    t->m_mem_axi_bready = true;
    rr::ut::clk(t);
    CHECK(t->fb_committed == false);

    t->commit_fb = false;
    rr::ut::clk(t);
    for (std::size_t i = 0; i < t->fb_size; i += 16 * PIXEL_PER_BEAT)
    {
        rr::ut::clk(t);
        CHECK(t->m_mem_axi_awvalid == true);
        CHECK(t->m_mem_axi_awaddr == (t->fb_addr + (i * PIXEL_PER_BEAT)));
        CHECK(t->m_mem_axi_awlen == 15);
        CHECK(t->m_mem_axi_awsize == 2);
        CHECK(t->m_mem_axi_awburst == 1);
        CHECK(t->fb_committed == false);
    }

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_awvalid == false);
    CHECK(t->fb_committed == false);

    // Destroy model
    delete t;
}

TEST_CASE("check mem write", "[VAxisFramebufferWriter]")
{
    constexpr std::size_t PIXEL_PER_BEAT = 2;

    VAxisFramebufferWriter* t = new VAxisFramebufferWriter();

    t->s_disp_axis_tvalid = 1;
    rr::ut::reset(t);
    CHECK(t->fb_committed == true);

    t->commit_fb = true;
    t->fb_addr = 0x1000'0000;
    t->fb_size = 0x100;
    t->m_mem_axi_awready = true;
    t->m_mem_axi_wready = true;
    t->m_mem_axi_bready = true;
    rr::ut::clk(t);
    CHECK(t->fb_committed == false);

    t->commit_fb = false;
    for (std::size_t i = 0; i < t->fb_size; i += PIXEL_PER_BEAT)
    {
        t->s_disp_axis_tdata = i;
        t->s_disp_axis_tvalid = true;
        rr::ut::clk(t);
        INFO(std::string("i ") + std::to_string(i));
        CHECK(t->m_mem_axi_wdata == i);
        CHECK(t->m_mem_axi_wvalid == true);
        CHECK(t->m_mem_axi_wlast == ((i % (16 * PIXEL_PER_BEAT)) == 30));
        CHECK(t->fb_committed == false);
    }

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_wvalid == false);
    CHECK(t->fb_committed == true);

    // Destroy model
    delete t;
}

TEST_CASE("interrupted mem write", "[VAxisFramebufferWriter]")
{
    constexpr std::size_t PIXEL_PER_BEAT = 2;

    VAxisFramebufferWriter* t = new VAxisFramebufferWriter();

    t->s_disp_axis_tvalid = true;
    rr::ut::reset(t);
    CHECK(t->fb_committed == true);

    t->commit_fb = true;
    t->fb_addr = 0x1000'0000;
    t->fb_size = 0x100;
    t->m_mem_axi_awready = true;
    t->m_mem_axi_wready = false;
    t->m_mem_axi_bready = true;
    rr::ut::clk(t);
    CHECK(t->fb_committed == false);
    CHECK(t->s_disp_axis_tready == true);

    t->commit_fb = false;

    t->s_disp_axis_tdata = 0;
    t->s_disp_axis_tvalid = true;
    t->m_mem_axi_wready = false;
    rr::ut::clk(t);
    CHECK(t->m_mem_axi_wdata == 0);
    CHECK(t->m_mem_axi_wvalid == true);
    CHECK(t->m_mem_axi_wlast == 0);
    CHECK(t->s_disp_axis_tready == true);
    CHECK(t->fb_committed == false);

    t->s_disp_axis_tdata = PIXEL_PER_BEAT;
    t->s_disp_axis_tvalid = true;
    t->m_mem_axi_wready = false;
    rr::ut::clk(t);
    CHECK(t->m_mem_axi_wdata == 0);
    CHECK(t->m_mem_axi_wvalid == true);
    CHECK(t->m_mem_axi_wlast == 0);
    CHECK(t->s_disp_axis_tready == false);
    CHECK(t->fb_committed == false);

    for (std::size_t i = 0; i < t->fb_size; i += PIXEL_PER_BEAT)
    {
        const std::size_t index = i + PIXEL_PER_BEAT;

        t->s_disp_axis_tdata = index;
        t->s_disp_axis_tvalid = true;
        t->m_mem_axi_wready = false;
        rr::ut::clk(t);
        CHECK(t->m_mem_axi_wdata == i);
        CHECK(t->m_mem_axi_wvalid == true);
        CHECK(t->m_mem_axi_wlast == ((i % (16 * PIXEL_PER_BEAT)) == 30));
        CHECK(t->s_disp_axis_tready == false);
        CHECK(t->fb_committed == false);

        t->m_mem_axi_wready = true;
        rr::ut::clk(t);
        INFO(std::string("i ") + std::to_string(i));
        CHECK(t->m_mem_axi_wdata == index);
        CHECK(t->m_mem_axi_wvalid == true);
        CHECK(t->m_mem_axi_wlast == ((index % (16 * PIXEL_PER_BEAT)) == 30));
        CHECK(t->s_disp_axis_tready == (index < (t->fb_size - PIXEL_PER_BEAT))); // It's one pixel behind here because one is in the skid buffer
        CHECK(t->fb_committed == false);
    }

    rr::ut::clk(t);
    CHECK(t->m_mem_axi_wvalid == false);
    CHECK(t->fb_committed == true);

    // Destroy model
    delete t;
}