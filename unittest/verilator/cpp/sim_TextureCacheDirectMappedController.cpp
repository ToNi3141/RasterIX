#include "general.hpp"

#include <cstdint>
#include <vector>

#include "VTextureCacheDirectMappedController.h"

namespace
{
constexpr uint32_t CACHE_SIZE = 1024;
constexpr uint32_t CACHE_LINE_SIZE = 32;
constexpr uint8_t READ_CACHE_ENTRY = 0;
constexpr uint8_t LOAD_CACHE_LINE = 1;

void waitForReady(VTextureCacheDirectMappedController* t)
{
    t->invalidate = 0;
    t->s_tc_valid = 0;
    t->m_tc_ready = 1;
    t->m_axi_arready = 1;
    for (unsigned cycle = 0; cycle < 64 && !t->s_tc_ready; cycle++)
    {
        rr::ut::clk(t);
    }
    REQUIRE(t->s_tc_ready == 1);
}

void checkAxiRequest(VTextureCacheDirectMappedController* t, uint32_t address)
{
    REQUIRE(t->m_axi_arvalid == 1);
    REQUIRE(t->m_axi_araddr == (address & ~(CACHE_LINE_SIZE - 1)));
    REQUIRE(t->m_axi_arid == 0);
    REQUIRE(t->m_axi_arlen == (CACHE_LINE_SIZE / 2) - 1);
    REQUIRE(t->m_axi_arsize == 1);
    REQUIRE(t->m_axi_arburst == 1);
    REQUIRE(t->m_axi_arlock == 0);
    REQUIRE(t->m_axi_arcache == 0);
    REQUIRE(t->m_axi_arprot == 0);
}

void fillCache(
    VTextureCacheDirectMappedController* t,
    uint32_t address)
{
    t->s_tc_addr = address;
    t->s_tc_valid = 1;
    REQUIRE(t->s_tc_ready == 1);
    REQUIRE(t->m_axi_arready == 1);
    rr::ut::clk(t);

    REQUIRE(t->m_tc_valid == 1);
    REQUIRE(t->m_tc_cmd == LOAD_CACHE_LINE);
    REQUIRE(t->m_tc_addr == address);
    checkAxiRequest(t, address);
    REQUIRE(t->s_tc_ready == 0);

    REQUIRE(t->m_axi_arvalid == 1);
    rr::ut::clk(t);
    REQUIRE(t->m_axi_arvalid == 0);
    REQUIRE(t->m_tc_valid == 0);
    REQUIRE(t->s_tc_ready == 0);

    rr::ut::clk(t);
    REQUIRE(t->m_tc_valid == 1);
    REQUIRE(t->m_tc_cmd == READ_CACHE_ENTRY);
    REQUIRE(t->m_tc_addr == address);
    REQUIRE(t->s_tc_ready == 1);

    t->s_tc_valid = 0;
    rr::ut::clk(t);
    REQUIRE(t->m_tc_valid == 0);
    REQUIRE(t->s_tc_ready == 1);
}

VTextureCacheDirectMappedController* makeController()
{
    auto* t = rr::ut::makeTop<VTextureCacheDirectMappedController>();
    rr::ut::reset(t);
    waitForReady(t);
    return t;
}
}
TEST_CASE("Cold access emits line load, entry access, and one AXI request",
    "[TextureCacheDirectMappedController]")
{
    auto* t = makeController();
    fillCache(t, 45);
    delete t;
}
TEST_CASE("Warm cache produces one entry command per cycle",
    "[TextureCacheDirectMappedController]")
{
    auto* t = makeController();
    fillCache(t, 0);
    fillCache(t, 32);

    t->m_axi_arready = 1;
    t->m_tc_ready = 1;
    const std::array<uint32_t, 4> addresses = { 1, 2, 33, 34 };
    for (const auto address : addresses)
    {
        t->s_tc_addr = address;
        t->s_tc_valid = 1;
        REQUIRE(t->s_tc_ready == 1);
        rr::ut::clk(t);
        CHECK(t->m_tc_valid == 1);
        CHECK(t->m_tc_cmd == READ_CACHE_ENTRY);
        CHECK(t->m_tc_addr == address);
        CHECK(t->s_tc_ready == 1);
        CHECK(t->m_axi_arvalid == 0);
    }
    t->s_tc_valid = 0;
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 0);
    CHECK(t->s_tc_ready == 1);

    delete t;
}
TEST_CASE("Master backpressure holds a cache command stable",
    "[TextureCacheDirectMappedController]")
{
    auto* t = makeController();
    fillCache(t, 0);

    t->m_tc_ready = 0;
    t->s_tc_addr = 1;
    t->s_tc_valid = 1;
    REQUIRE(t->s_tc_ready == 1);
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->m_tc_cmd == READ_CACHE_ENTRY);
    CHECK(t->m_tc_addr == 1);

    t->s_tc_valid = 0;
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->m_tc_cmd == READ_CACHE_ENTRY);
    CHECK(t->m_tc_addr == 1);
    CHECK(t->s_tc_ready == 1);

    t->m_tc_ready = 1;
    REQUIRE(t->m_tc_valid == 1);
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 0);
    CHECK(t->s_tc_ready == 1);
    delete t;
}

TEST_CASE("AXI backpressure blocks the next cache command",
    "[TextureCacheDirectMappedController]")
{
    auto* t = makeController();
    REQUIRE(t->s_tc_ready == 1);

    t->m_axi_arready = 0;
    t->s_tc_addr = 0;
    t->s_tc_valid = 1;
    rr::ut::clk(t);
    REQUIRE(t->m_axi_arvalid == 1);
    REQUIRE(t->m_tc_cmd == LOAD_CACHE_LINE);
    REQUIRE(t->s_tc_ready == 0);

    t->s_tc_addr = 2;
    t->s_tc_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_axi_arvalid == 1);
    CHECK(t->m_axi_araddr == 0);
    CHECK(t->m_tc_valid == 0);
    CHECK(t->s_tc_ready == 0);

    t->m_axi_arready = 1;
    rr::ut::clk(t);
    CHECK(t->m_axi_arvalid == 0);
    CHECK(t->m_tc_valid == 0);
    CHECK(t->s_tc_ready == 0);

    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->m_tc_cmd == READ_CACHE_ENTRY);
    CHECK(t->m_tc_addr == 0);
    CHECK(t->s_tc_ready == 1);

    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->m_tc_cmd == READ_CACHE_ENTRY);
    CHECK(t->m_tc_addr == 2);
    CHECK(t->s_tc_ready == 1);

    delete t;
}

TEST_CASE("skid buffer retains request order across invalidation",
    "[TextureCacheDirectMappedController]")
{
    auto* t = makeController();
    fillCache(t, 0);

    t->m_tc_ready = 0;
    t->s_tc_addr = 1;
    t->s_tc_valid = 1;
    REQUIRE(t->s_tc_ready == 1);
    rr::ut::clk(t);
    REQUIRE(t->m_tc_valid == 1);
    CHECK(t->m_tc_addr == 1);

    t->s_tc_addr = 2;
    rr::ut::clk(t);
    REQUIRE(t->s_tc_ready == 0);

    t->s_tc_addr = 3;
    t->invalidate = 1;
    rr::ut::clk(t);
    t->invalidate = 0;
    for (uint32_t index = 1; index < CACHE_SIZE / CACHE_LINE_SIZE; ++index)
    {
        rr::ut::clk(t);
    }

    CHECK(t->s_tc_ready == 0);

    t->m_tc_ready = 1;
    rr::ut::clk(t);
    REQUIRE(t->m_tc_valid == 1);
    CHECK(t->m_tc_cmd == LOAD_CACHE_LINE);
    CHECK(t->m_tc_addr == 2);
    CHECK(t->s_tc_ready == 0);

    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 0);
    CHECK(t->s_tc_ready == 0);

    rr::ut::clk(t);
    REQUIRE(t->m_tc_valid == 1);
    CHECK(t->m_tc_cmd == READ_CACHE_ENTRY);
    CHECK(t->m_tc_addr == 2);
    CHECK(t->s_tc_ready == 1);

    rr::ut::clk(t);
    REQUIRE(t->m_tc_valid == 1);
    CHECK(t->m_tc_cmd == READ_CACHE_ENTRY);
    CHECK(t->m_tc_addr == 3);

    t->s_tc_valid = 0;
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 0);
    delete t;
}

TEST_CASE("Two cold requests produce two line loads and two entry accesses",
    "[TextureCacheDirectMappedController]")
{
    auto* t = makeController();
    t->m_tc_ready = 1;
    t->m_axi_arready = 1;

    t->s_tc_addr = 0;
    t->s_tc_valid = 1;
    REQUIRE(t->s_tc_ready == 1);
    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->m_tc_cmd == LOAD_CACHE_LINE);
    CHECK(t->m_tc_addr == 0);
    checkAxiRequest(t, 0);
    CHECK(t->s_tc_ready == 0);

    t->s_tc_addr = CACHE_SIZE;
    rr::ut::clk(t);
    CHECK(t->m_axi_arvalid == 0);
    CHECK(t->m_tc_valid == 0);
    CHECK(t->s_tc_ready == 0);

    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->m_tc_cmd == READ_CACHE_ENTRY);
    CHECK(t->m_tc_addr == 0);
    CHECK(t->s_tc_ready == 1);

    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->m_tc_cmd == LOAD_CACHE_LINE);
    CHECK(t->m_tc_addr == CACHE_SIZE);
    checkAxiRequest(t, CACHE_SIZE);
    CHECK(t->s_tc_ready == 0);
    t->s_tc_valid = 0;

    rr::ut::clk(t);
    CHECK(t->m_axi_arvalid == 0);
    CHECK(t->m_tc_valid == 0);

    rr::ut::clk(t);
    CHECK(t->m_tc_valid == 1);
    CHECK(t->m_tc_cmd == READ_CACHE_ENTRY);
    CHECK(t->m_tc_addr == CACHE_SIZE);
    CHECK(t->s_tc_ready == 1);

    delete t;
}

TEST_CASE("Conflicting direct-map addresses replace the tag",
    "[TextureCacheDirectMappedController]")
{
    auto* t = makeController();
    fillCache(t, 0);
    fillCache(t, CACHE_SIZE);
    fillCache(t, 0);
    delete t;
}

TEST_CASE("Invalidation clears the final cache tag",
    "[TextureCacheDirectMappedController]")
{
    auto* t = makeController();
    constexpr uint32_t CACHE_LINES = CACHE_SIZE / CACHE_LINE_SIZE;
    for (uint32_t index = 0; index < CACHE_LINES; ++index)
    {
        fillCache(t, index * CACHE_LINE_SIZE);
    }

    t->invalidate = 1;
    rr::ut::clk(t);
    t->invalidate = 0;
    for (uint32_t index = 1; index < CACHE_LINES; ++index)
    {
        rr::ut::clk(t);
    }
    REQUIRE(t->s_tc_ready == 1);

    for (uint32_t index = 0; index < CACHE_LINES; ++index)
    {
        fillCache(t, index * CACHE_LINE_SIZE);
    }
    delete t;
}

TEST_CASE("Every cache index can be filled and reread as a hit",
    "[TextureCacheDirectMappedController]")
{
    auto* t = makeController();
    for (uint32_t index = 0; index < CACHE_SIZE / CACHE_LINE_SIZE; ++index)
    {
        fillCache(t, index * CACHE_LINE_SIZE);
    }

    t->m_tc_ready = 1;
    t->m_axi_arready = 1;
    for (uint32_t index = 0; index < CACHE_SIZE / CACHE_LINE_SIZE; ++index)
    {
        t->s_tc_addr = index * CACHE_LINE_SIZE;
        t->s_tc_valid = 1;
        rr::ut::clk(t);
        CHECK(t->m_tc_valid == 1);
        CHECK(t->m_tc_cmd == READ_CACHE_ENTRY);
        CHECK(t->m_axi_arvalid == 0);
    }
    t->s_tc_valid = 0;
    delete t;
}