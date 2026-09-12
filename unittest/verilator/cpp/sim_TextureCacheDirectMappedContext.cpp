#include "general.hpp"

#include <array>
#include <cstdint>

#include "VTextureCacheDirectMappedContext.h"

namespace
{
constexpr uint32_t CACHE_LINE_SIZE = 32;
constexpr uint32_t DATA_BYTES = 4;
constexpr uint32_t LINE_BEATS = CACHE_LINE_SIZE / DATA_BYTES;
constexpr uint8_t READ_CACHE_ENTRY = 0;
constexpr uint8_t LOAD_CACHE_LINE = 1;

VTextureCacheDirectMappedContext* makeContext()
{
    auto* t = rr::ut::makeTop<VTextureCacheDirectMappedContext>();
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

void loadLine(VTextureCacheDirectMappedContext* t, uint32_t address, const std::array<uint32_t, LINE_BEATS>& data)
{
    t->s_tc_addr = address;
    t->s_tc_cmd = LOAD_CACHE_LINE;
    t->s_tc_valid = 1;
    REQUIRE(t->s_tc_ready == 1);
    rr::ut::clk(t);
    CHECK(t->s_tc_ready == 0);
    CHECK(t->m_axi_rready == 1);
    CHECK(t->m_tc_valid == 0);

    for (uint32_t beat = 0; beat < LINE_BEATS; ++beat)
    {
        t->m_axi_rdata = data[beat];
        t->m_axi_rvalid = 1;
        t->m_axi_rlast = beat == LINE_BEATS - 1;
        REQUIRE(t->m_axi_rready == 1);
        rr::ut::clk(t);
        CHECK(t->m_axi_rready == (beat != LINE_BEATS - 1));
    }

    t->m_axi_rvalid = 0;
    t->m_axi_rlast = 0;
    t->s_tc_valid = 0;
    rr::ut::clk(t);
    CHECK(t->s_tc_ready == 1);
}
}

TEST_CASE("loads the configured number of AXI words", "[TextureCacheDirectMappedContext]")
{
    auto* t = makeContext();
    loadLine(t, 0, { 0x00010002, 0x00030004, 0x00050006, 0x00070008, 0x0009000a, 0x000b000c, 0x000d000e, 0x000f0010 });
    delete t;
}

TEST_CASE("narrows a cached data word to either texel lane", "[TextureCacheDirectMappedContext]")
{
    auto* t = makeContext();
    loadLine(t, 0, { 0x11223344, 0, 0, 0, 0, 0, 0, 0 });

    t->m_tc_ready = 1;
    t->s_tc_valid = 1;
    t->s_tc_cmd = READ_CACHE_ENTRY;

    t->s_tc_addr = 0;
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->m_tc_texel == 0x3344);

    t->s_tc_valid = 1;
    t->s_tc_addr = 2;
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->m_tc_texel == 0x1122);

    t->s_tc_valid = 0;
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 0);
    delete t;
}

TEST_CASE("backpressures the source while the output is busy", "[TextureCacheDirectMappedContext]")
{
    auto* t = makeContext();
    loadLine(t, 0, { 0x11223344, 0, 0, 0, 0, 0, 0, 0 });

    t->m_tc_ready = 0;
    t->s_tc_addr = 0;
    t->s_tc_cmd = READ_CACHE_ENTRY;
    t->s_tc_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->m_tc_texel == 0x3344);
    CHECK(t->s_tc_ready == 1);

    t->s_tc_addr = 2;
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->s_tc_ready == 0);

    t->s_tc_valid = 0;
    t->m_tc_ready = 1;
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->m_tc_texel == 0x1122);
    CHECK(t->s_tc_ready == 1);

    t->m_tc_ready = 0;
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);

    t->m_tc_ready = 1;
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 0);

    delete t;
}

TEST_CASE("skid buffer preserves a queued cache-line load", "[TextureCacheDirectMappedContext]")
{
    auto* t = makeContext();
    loadLine(t, 0, { 0x11223344, 0, 0, 0, 0, 0, 0, 0 });

    t->m_tc_ready = 0;
    t->s_tc_addr = 0;
    t->s_tc_cmd = READ_CACHE_ENTRY;
    t->s_tc_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->m_tc_texel == 0x3344);

    t->s_tc_addr = 64;
    t->s_tc_cmd = LOAD_CACHE_LINE;
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->s_tc_ready == 0);
    CHECK(t->m_tc_texel == 0x3344);

    t->s_tc_valid = 0;
    t->m_tc_ready = 1;
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 0);
    CHECK(t->m_axi_rready == 1);
    CHECK(t->s_tc_ready == 0);

    const std::array<uint32_t, LINE_BEATS> data = {
        0xaabbccdd, 0, 0, 0, 0, 0, 0, 0
    };
    for (uint32_t beat = 0; beat < LINE_BEATS; ++beat)
    {
        t->m_axi_rdata = data[beat];
        t->m_axi_rvalid = 1;
        t->m_axi_rlast = beat == LINE_BEATS - 1;
        rr::ut::clk(t);
    }
    t->m_axi_rvalid = 0;
    t->m_axi_rlast = 0;
    rr::ut::clk(t);
    CHECK(t->s_tc_ready == 1);

    t->s_tc_addr = 64;
    t->s_tc_cmd = READ_CACHE_ENTRY;
    t->s_tc_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->m_tc_texel == 0xccdd);
    delete t;
}
