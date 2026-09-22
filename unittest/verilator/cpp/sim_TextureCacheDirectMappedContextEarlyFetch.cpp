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

#include <cstdint>

#include "VTextureCacheDirectMappedContextEarlyFetch.h"

namespace
{
constexpr uint8_t READ_CACHE_ENTRY = 0;
constexpr uint8_t LOAD_CACHE_LINE = 1;
constexpr uint32_t LINE_BEATS = 8;

VTextureCacheDirectMappedContextEarlyFetch* makeContext()
{
    auto* t = rr::ut::makeTop<VTextureCacheDirectMappedContextEarlyFetch>();
    t->s_tc_valid = 0;
    t->s_tc_cmd = READ_CACHE_ENTRY;
    t->s_tc_addr = 0;
    t->m_tc_ready = 1;
    t->m_axi_rid = 0;
    t->m_axi_rdata = 0;
    t->m_axi_rresp = 0;
    t->m_axi_rlast = 0;
    t->m_axi_rvalid = 0;
    rr::ut::reset(t);
    return t;
}
}

TEST_CASE("serves a read while a cache line is loading", "[TextureCacheDirectMappedContext]")
{
    auto* t = makeContext();

    t->s_tc_addr = 0;
    t->s_tc_cmd = LOAD_CACHE_LINE;
    t->s_tc_valid = 1;
    REQUIRE(t->s_tc_ready == 1);
    rr::ut::clk(t);
    CHECK(t->s_tc_ready == 1);
    CHECK(t->m_axi_rready == 1);

    t->s_tc_addr = 4;
    t->s_tc_cmd = READ_CACHE_ENTRY;

    const uint32_t data[LINE_BEATS] = {
        0x11223344, 0x55667788, 0, 0, 0, 0, 0, 0
    };
    for (uint32_t beat = 0; beat < LINE_BEATS; ++beat)
    {
        t->m_axi_rdata = data[beat];
        t->m_axi_rvalid = 1;
        t->m_axi_rlast = beat == LINE_BEATS - 1;
        REQUIRE(t->m_axi_rready == 1);
        rr::ut::clk(t);

        if (beat < 2)
        {
            CHECK(t->m_tc_valid == 0);
            CHECK(t->s_tc_ready == 0);
        }
        else if (beat == 2)
        {
            CHECK(t->m_tc_valid == 1);
            CHECK(t->m_tc_texel == 0x7788);
            CHECK(t->s_tc_ready == 1);
        }

        CHECK(t->m_axi_rready == (beat != LINE_BEATS - 1));
        if (beat == 2)
        {
            t->s_tc_valid = 0;
        }
    }

    t->m_axi_rvalid = 0;
    t->m_axi_rlast = 0;
    rr::ut::clk(t);
    CHECK(t->m_axi_rready == 0);
    CHECK(t->s_tc_ready == 1);

    t->s_tc_addr = 4;
    t->s_tc_cmd = READ_CACHE_ENTRY;
    t->s_tc_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->m_tc_texel == 0x7788);

    delete t;
}

TEST_CASE("does not advertise a full skid buffer after the final AXI beat", "[TextureCacheDirectMappedContext]")
{
    auto* t = makeContext();
    t->s_tc_addr = 0;
    t->s_tc_cmd = LOAD_CACHE_LINE;
    t->s_tc_valid = 1;
    rr::ut::clk(t);
    t->s_tc_valid = 0;

    for (uint32_t beat = 0; beat < LINE_BEATS - 1; ++beat)
    {
        t->m_axi_rdata = 0;
        t->m_axi_rvalid = 1;
        t->m_axi_rlast = 0;
        rr::ut::clk(t);
    }

    t->s_tc_addr = 28;
    t->s_tc_cmd = READ_CACHE_ENTRY;
    t->s_tc_valid = 1;
    t->m_axi_rdata = 0x11223344;
    t->m_axi_rlast = 1;
    rr::ut::clk(t);
    CHECK(t->s_tc_ready == 0);
    CHECK(t->m_tc_valid == 0);

    t->s_tc_valid = 0;
    t->m_axi_rvalid = 0;
    t->m_axi_rlast = 0;
    rr::ut::clk(t);
    CHECK(t->s_tc_ready == 1);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->m_tc_texel == 0x3344);
    delete t;
}