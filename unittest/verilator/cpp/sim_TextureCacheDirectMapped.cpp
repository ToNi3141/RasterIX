#include "general.hpp"

#include <array>
#include <cstdint>

#include "VTextureCacheDirectMapped.h"

namespace
{
constexpr uint32_t CACHE_LINE_SIZE = 32;
constexpr uint32_t DATA_BYTES = 4;
constexpr uint32_t LINE_BEATS = CACHE_LINE_SIZE / DATA_BYTES;

VTextureCacheDirectMapped* makeCache()
{
    auto* t = rr::ut::makeTop<VTextureCacheDirectMapped>();
    t->invalidate = 0;
    t->s_araddr = 0;
    t->s_arvalid = 0;
    t->m_ready = 1;
    t->m_axi_arready = 1;
    t->m_axi_rid = 0;
    t->m_axi_rdata = 0;
    t->m_axi_rresp = 0;
    t->m_axi_rlast = 0;
    t->m_axi_rvalid = 0;
    rr::ut::reset(t);
    for (unsigned cycle = 0; cycle < 64 && !t->s_arready; ++cycle)
    {
        rr::ut::clk(t);
    }
    REQUIRE(t->s_arready == 1);
    return t;
}

void request(VTextureCacheDirectMapped* t, uint32_t address)
{
    t->s_araddr = address;
    t->s_arvalid = 1;
    while (!t->s_arready)
    {
        rr::ut::clk(t);
    }
    rr::ut::clk(t);
    t->s_arvalid = 0;
}

void checkAxiReadRequest(VTextureCacheDirectMapped* t, uint32_t address)
{
    REQUIRE(t->m_axi_arvalid == 1);
    CHECK(t->m_axi_araddr == address);
    CHECK(t->m_axi_arlen == LINE_BEATS - 1);
    CHECK(t->m_axi_arsize == 2);
    CHECK(t->m_axi_arburst == 1);
}

void provideLine(VTextureCacheDirectMapped* t, const std::array<uint32_t, LINE_BEATS>& data)
{
    for (uint32_t beat = 0; beat < LINE_BEATS; ++beat)
    {
        t->m_axi_rdata = data[beat];
        t->m_axi_rvalid = 1;
        t->m_axi_rlast = beat == LINE_BEATS - 1;
        while (!t->m_axi_rready)
        {
            rr::ut::clk(t);
        }
        rr::ut::clk(t);
    }
    t->m_axi_rvalid = 0;
    t->m_axi_rlast = 0;
}

void warmLine(VTextureCacheDirectMapped* t)
{
    request(t, 0);
    provideLine(t, { 0x11223344, 0, 0, 0, 0, 0, 0, 0 });
    for (unsigned cycle = 0; cycle < 16 && !t->m_valid; ++cycle)
    {
        rr::ut::clk(t);
    }
    REQUIRE(t->m_valid == 1);
    rr::ut::clk(t);
}
}

TEST_CASE("loads a cache line and returns the requested texel", "[TextureCacheDirectMapped]")
{
    auto* t = makeCache();
    t->m_axi_arready = 0;
    request(t, 0);

    checkAxiReadRequest(t, 0);

    t->m_axi_arready = 1;
    rr::ut::clk(t);
    provideLine(t, { 0x11223344, 0, 0, 0, 0, 0, 0, 0 });

    for (unsigned cycle = 0; cycle < 16 && !t->m_valid; ++cycle)
    {
        rr::ut::clk(t);
    }
    REQUIRE(t->m_valid == 1);
    CHECK(t->m_texel == 0x3344);

    delete t;
}

TEST_CASE("stalls the slave after the command FIFO fills", "[TextureCacheDirectMapped]")
{
    auto* t = makeCache();
    warmLine(t);
    t->m_ready = 0;
    t->s_araddr = 0;
    t->s_arvalid = 1;

    bool stalled = false;
    // 37 because: 33 (fifo + skid) + 2 (context + skid) + 2 (controller + skid)
    for (unsigned requestIndex = 0; requestIndex < 37; ++requestIndex)
    {
        if (!t->s_arready)
        {
            stalled = true;
            break;
        }
        rr::ut::clk(t);
    }
    CHECK(stalled);
    CHECK(t->s_arready == 0);

    t->s_arvalid = 0;
    t->m_ready = 1;
    for (unsigned cycle = 0; cycle < 64 && !t->s_arready; ++cycle)
    {
        rr::ut::clk(t);
    }
    CHECK(t->s_arready == 1);
    delete t;
}

TEST_CASE("stalls AXI R when the cache output is blocked", "[TextureCacheDirectMapped]")
{
    auto* t = makeCache();
    t->m_ready = 0;
    t->m_axi_rvalid = 1;
    t->m_axi_rdata = 0xabcdef01;

    for (unsigned beat = 0; beat < 32; ++beat)
    {
        REQUIRE(t->m_axi_rready == 1);
        t->m_axi_rlast = beat % (LINE_BEATS - 1);
        rr::ut::clk(t);
    }
    CHECK(t->m_axi_rready == 0);

    t->m_axi_rvalid = 0;
    t->m_axi_rlast = 0;
    delete t;
}
