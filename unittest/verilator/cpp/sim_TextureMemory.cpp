#include "general.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

#include "VTextureMemory.h"

namespace
{

constexpr std::uint32_t PAGE_BASE { 0x1000'0000 };
constexpr std::size_t MAX_WAIT_CYCLES { 128 };

using TexelAddresses = std::array<std::uint32_t, 4>;
using Texels = std::array<std::uint16_t, 4>;

class TextureMemoryFixture
{
public:
    TextureMemoryFixture()
        : textureMemory { rr::ut::makeTop<VTextureMemory>() }
    {
        textureMemory->texelAddrValid = 0;
        textureMemory->texelAddr00 = 0;
        textureMemory->texelAddr01 = 0;
        textureMemory->texelAddr10 = 0;
        textureMemory->texelAddr11 = 0;
        textureMemory->texelOutputReady = 0;

        textureMemory->s_axis_tvalid = 0;
        textureMemory->s_axis_tlast = 0;
        textureMemory->s_axis_tdata = 0;

        textureMemory->m_axi_arready = 0;
        textureMemory->m_axi_rid = 0;
        textureMemory->m_axi_rdata = 0;
        textureMemory->m_axi_rresp = 0;
        textureMemory->m_axi_rlast = 0;
        textureMemory->m_axi_rvalid = 0;

        rr::ut::reset(textureMemory);
    }

    ~TextureMemoryFixture()
    {
        delete textureMemory;
    }

    void loadPageTable(std::uint32_t pageBase)
    {
        REQUIRE(textureMemory->s_axis_tready == 1);
        textureMemory->s_axis_tvalid = 1;
        textureMemory->s_axis_tlast = 1;
        textureMemory->s_axis_tdata = pageBase;
        rr::ut::clk(textureMemory);

        textureMemory->s_axis_tvalid = 0;
        textureMemory->s_axis_tlast = 0;
        textureMemory->s_axis_tdata = 0;
    }

    void submitRequest(const TexelAddresses& addresses)
    {
        driveRequest(addresses);
        textureMemory->texelAddrValid = 1;

        std::size_t cycles = 0;
        while (!textureMemory->texelAddrReady && cycles++ < MAX_WAIT_CYCLES)
        {
            rr::ut::clk(textureMemory);
        }
        REQUIRE(textureMemory->texelAddrReady == 1);
        rr::ut::clk(textureMemory);

        textureMemory->texelAddrValid = 0;
    }

    void acceptRead(std::uint32_t expectedAddress)
    {
        textureMemory->m_axi_arready = 0;

        std::size_t cycles = 0;
        while (!textureMemory->m_axi_arvalid && cycles++ < MAX_WAIT_CYCLES)
        {
            rr::ut::clk(textureMemory);
        }
        REQUIRE(textureMemory->m_axi_arvalid == 1);
        REQUIRE(textureMemory->m_axi_araddr == expectedAddress);

        textureMemory->m_axi_arready = 1;
        rr::ut::clk(textureMemory);
        textureMemory->m_axi_arready = 0;
    }

    void sendReadResponse(std::uint16_t texel)
    {
        textureMemory->m_axi_rdata = texel;
        textureMemory->m_axi_rlast = 1;
        textureMemory->m_axi_rvalid = 1;

        std::size_t cycles = 0;
        while (!textureMemory->m_axi_rready && cycles++ < MAX_WAIT_CYCLES)
        {
            rr::ut::clk(textureMemory);
        }
        REQUIRE(textureMemory->m_axi_rready == 1);
        rr::ut::clk(textureMemory);

        textureMemory->m_axi_rvalid = 0;
        textureMemory->m_axi_rlast = 0;
    }

    void completeRequest(const TexelAddresses& addresses, const Texels& texels)
    {
        for (std::size_t texelIndex = 0; texelIndex < addresses.size(); ++texelIndex)
        {
            acceptRead(PAGE_BASE + addresses[texelIndex]);
            sendReadResponse(texels[texelIndex]);
        }
    }

    void waitForOutput()
    {
        std::size_t cycles = 0;
        while (!textureMemory->texelOutputValid && cycles++ < MAX_WAIT_CYCLES)
        {
            rr::ut::clk(textureMemory);
        }
        REQUIRE(textureMemory->texelOutputValid == 1);
    }

    void checkOutput(const Texels& expected) const
    {
        CHECK(textureMemory->texelOutput00 == expected[0]);
        CHECK(textureMemory->texelOutput01 == expected[1]);
        CHECK(textureMemory->texelOutput10 == expected[2]);
        CHECK(textureMemory->texelOutput11 == expected[3]);
    }

    void driveRequest(const TexelAddresses& addresses)
    {
        textureMemory->texelAddr00 = addresses[0];
        textureMemory->texelAddr01 = addresses[1];
        textureMemory->texelAddr10 = addresses[2];
        textureMemory->texelAddr11 = addresses[3];
    }

    VTextureMemory* textureMemory;
};

std::uint16_t texelForAddress(std::uint32_t physicalAddress)
{
    return static_cast<std::uint16_t>(0x4000 + physicalAddress - PAGE_BASE);
}

} // namespace

TEST_CASE("Read texels through TextureMemory", "[TextureMemory]")
{
    TextureMemoryFixture fixture;
    fixture.loadPageTable(PAGE_BASE);

    const TexelAddresses addresses { 0x10, 0x20, 0x30, 0x40 };
    const Texels texels { 0x1010, 0x2020, 0x3030, 0x4040 };

    fixture.submitRequest(addresses);
    fixture.completeRequest(addresses, texels);
    fixture.waitForOutput();
    fixture.checkOutput(texels);

    fixture.textureMemory->texelOutputReady = 1;
    rr::ut::clk(fixture.textureMemory);
    CHECK(fixture.textureMemory->texelOutputValid == 0);
}

TEST_CASE("Hold an AXI read address while the address channel stalls", "[TextureMemory]")
{
    TextureMemoryFixture fixture;
    fixture.loadPageTable(PAGE_BASE);

    const TexelAddresses addresses { 0x11, 0x21, 0x31, 0x41 };
    const Texels texels { 0x1111, 0x2121, 0x3131, 0x4141 };
    fixture.submitRequest(addresses);

    std::size_t cycles = 0;
    while (!fixture.textureMemory->m_axi_arvalid && cycles++ < MAX_WAIT_CYCLES)
    {
        rr::ut::clk(fixture.textureMemory);
    }
    REQUIRE(fixture.textureMemory->m_axi_arvalid == 1);

    const auto stalledAddress = fixture.textureMemory->m_axi_araddr;
    CHECK(stalledAddress == PAGE_BASE + addresses[0]);
    for (std::size_t stallCycle = 0; stallCycle < 4; ++stallCycle)
    {
        rr::ut::clk(fixture.textureMemory);
        CHECK(fixture.textureMemory->m_axi_arvalid == 1);
        CHECK(fixture.textureMemory->m_axi_araddr == stalledAddress);
        CHECK(fixture.textureMemory->texelOutputValid == 0);
    }

    fixture.acceptRead(PAGE_BASE + addresses[0]);
    fixture.sendReadResponse(texels[0]);
    for (std::size_t texelIndex = 1; texelIndex < addresses.size(); ++texelIndex)
    {
        fixture.acceptRead(PAGE_BASE + addresses[texelIndex]);
        fixture.sendReadResponse(texels[texelIndex]);
    }

    fixture.waitForOutput();
    fixture.checkOutput(texels);
}

TEST_CASE("Backpressure AXI read responses while texel output stalls", "[TextureMemory]")
{
    TextureMemoryFixture fixture;
    fixture.loadPageTable(PAGE_BASE);

    std::size_t requestIndex = 0;
    TexelAddresses request { 0, 1, 2, 3 };
    fixture.driveRequest(request);
    fixture.textureMemory->texelAddrValid = 1;
    fixture.textureMemory->m_axi_arready = 1;

    bool responseBackpressured = false;
    constexpr std::size_t MAX_SATURATION_CYCLES { 512 };
    for (std::size_t cycle = 0; cycle < MAX_SATURATION_CYCLES; ++cycle)
    {
        const bool requestAccepted
            = fixture.textureMemory->texelAddrValid && fixture.textureMemory->texelAddrReady;
        const bool addressAccepted
            = fixture.textureMemory->m_axi_arvalid && fixture.textureMemory->m_axi_arready;
        const bool responseAccepted
            = fixture.textureMemory->m_axi_rvalid && fixture.textureMemory->m_axi_rready;

        if (fixture.textureMemory->m_axi_rvalid && !fixture.textureMemory->m_axi_rready)
        {
            responseBackpressured = true;
            break;
        }

        const auto acceptedAddress = fixture.textureMemory->m_axi_araddr;
        rr::ut::clk(fixture.textureMemory);

        if (responseAccepted)
        {
            fixture.textureMemory->m_axi_rvalid = 0;
            fixture.textureMemory->m_axi_rlast = 0;
            fixture.textureMemory->m_axi_arready = 1;
        }
        if (addressAccepted)
        {
            fixture.textureMemory->m_axi_rvalid = 1;
            fixture.textureMemory->m_axi_rlast = 1;
            fixture.textureMemory->m_axi_rdata = texelForAddress(acceptedAddress);
            fixture.textureMemory->m_axi_arready = 0;
        }
        if (requestAccepted)
        {
            ++requestIndex;
            const auto firstAddress = static_cast<std::uint32_t>(requestIndex * 4);
            request = { firstAddress, firstAddress + 1, firstAddress + 2, firstAddress + 3 };
            fixture.driveRequest(request);
        }
    }

    REQUIRE(responseBackpressured);
    REQUIRE(fixture.textureMemory->texelOutputValid == 1);
    const Texels stalledOutput {
        static_cast<std::uint16_t>(fixture.textureMemory->texelOutput00),
        static_cast<std::uint16_t>(fixture.textureMemory->texelOutput01),
        static_cast<std::uint16_t>(fixture.textureMemory->texelOutput10),
        static_cast<std::uint16_t>(fixture.textureMemory->texelOutput11)
    };
    const auto stalledResponse = fixture.textureMemory->m_axi_rdata;

    fixture.textureMemory->texelAddrValid = 0;
    fixture.textureMemory->m_axi_arready = 0;
    for (std::size_t stallCycle = 0; stallCycle < 4; ++stallCycle)
    {
        rr::ut::clk(fixture.textureMemory);
        CHECK(fixture.textureMemory->m_axi_rready == 0);
        CHECK(fixture.textureMemory->m_axi_rvalid == 1);
        CHECK(fixture.textureMemory->m_axi_rdata == stalledResponse);
        fixture.checkOutput(stalledOutput);
    }

    fixture.textureMemory->texelOutputReady = 1;
    std::size_t releaseCycles = 0;
    while (!fixture.textureMemory->m_axi_rready && releaseCycles++ < MAX_WAIT_CYCLES)
    {
        rr::ut::clk(fixture.textureMemory);
    }
    REQUIRE(fixture.textureMemory->m_axi_rready == 1);
    CHECK(fixture.textureMemory->m_axi_rdata == stalledResponse);
    rr::ut::clk(fixture.textureMemory);
}