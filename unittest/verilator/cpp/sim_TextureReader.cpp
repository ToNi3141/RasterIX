#include "general.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

#include "VTextureReader.h"

namespace
{

constexpr std::uint32_t PAGE_BASE { 0x1000'0000 };
constexpr std::uint32_t CACHE_LINE_SIZE { 32 };
constexpr std::size_t MAX_WAIT_CYCLES { 128 };

constexpr std::array<std::uint32_t, 4> TEXEL_ADDRESSES { 0x10, 0x11, 0x12, 0x13 };
constexpr std::array<std::uint16_t, 4> TEXELS { 0xabcd, 0x1234, 0x5678, 0x9abc };

}

TEST_CASE("reads a texel through TextureReader", "[TextureReader]")
{
    auto* textureMemory = rr::ut::makeTop<VTextureReader>();
    textureMemory->s_tr_valid = 0;
    textureMemory->m_tr_ready = 0;
    textureMemory->s_axis_tvalid = 0;
    textureMemory->m_axi_arready = 0;
    textureMemory->m_axi_rvalid = 0;
    rr::ut::reset(textureMemory);

    textureMemory->s_axis_tvalid = 1;
    textureMemory->s_axis_tlast = 1;
    textureMemory->s_axis_tdata = PAGE_BASE;
    rr::ut::clk(textureMemory);
    textureMemory->s_axis_tvalid = 0;
    textureMemory->s_axis_tlast = 0;

    textureMemory->s_tr_addr_00 = TEXEL_ADDRESSES[0];
    textureMemory->s_tr_addr_01 = TEXEL_ADDRESSES[1];
    textureMemory->s_tr_addr_10 = TEXEL_ADDRESSES[2];
    textureMemory->s_tr_addr_11 = TEXEL_ADDRESSES[3];
    textureMemory->s_tr_valid = 1;
    std::size_t cycles = 0;
    while (!textureMemory->s_tr_ready && cycles++ < MAX_WAIT_CYCLES)
    {
        rr::ut::clk(textureMemory);
    }
    REQUIRE(textureMemory->s_tr_ready);
    rr::ut::clk(textureMemory);
    textureMemory->s_tr_valid = 0;

    cycles = 0;
    while (!textureMemory->m_axi_arvalid && cycles++ < MAX_WAIT_CYCLES)
    {
        rr::ut::clk(textureMemory);
    }
    REQUIRE(textureMemory->m_axi_arvalid);
    CHECK(textureMemory->m_axi_araddr == PAGE_BASE + (TEXEL_ADDRESSES[0] << 1));

    textureMemory->m_axi_arready = 1;
    rr::ut::clk(textureMemory);
    textureMemory->m_axi_arready = 0;

    for (std::size_t beat = 0; beat < CACHE_LINE_SIZE / sizeof(std::uint32_t); ++beat)
    {
        textureMemory->m_axi_rdata = 0;
        if (beat == 0)
        {
            textureMemory->m_axi_rdata = static_cast<std::uint32_t>(TEXELS[0])
                | (static_cast<std::uint32_t>(TEXELS[1]) << 16);
        }
        else if (beat == 1)
        {
            textureMemory->m_axi_rdata = static_cast<std::uint32_t>(TEXELS[2])
                | (static_cast<std::uint32_t>(TEXELS[3]) << 16);
        }
        textureMemory->m_axi_rlast = beat == (CACHE_LINE_SIZE / sizeof(std::uint32_t)) - 1;
        textureMemory->m_axi_rvalid = 1;
        cycles = 0;
        while (!textureMemory->m_axi_rready && cycles++ < MAX_WAIT_CYCLES)
        {
            rr::ut::clk(textureMemory);
        }
        REQUIRE(textureMemory->m_axi_rready);
        rr::ut::clk(textureMemory);
    }
    textureMemory->m_axi_rvalid = 0;
    textureMemory->m_axi_rlast = 0;

    cycles = 0;
    while (!textureMemory->m_tr_valid && cycles++ < MAX_WAIT_CYCLES)
    {
        rr::ut::clk(textureMemory);
    }
    REQUIRE(textureMemory->m_tr_valid);
    CHECK(textureMemory->m_tr_texel_00 == TEXELS[0]);
    CHECK(textureMemory->m_tr_texel_01 == TEXELS[1]);
    CHECK(textureMemory->m_tr_texel_10 == TEXELS[2]);
    CHECK(textureMemory->m_tr_texel_11 == TEXELS[3]);

    delete textureMemory;
}
