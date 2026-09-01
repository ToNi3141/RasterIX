#include "general.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

#include "VTextureMMU.h"

namespace
{

constexpr std::uint32_t PAGE_SIZE { 2048 };
constexpr std::size_t PAGE_TABLE_ENTRIES { 64 };

void initialize(VTextureMMU* textureMmu)
{
    textureMmu->s_axis_tvalid = 0;
    textureMmu->s_axis_tlast = 0;
    textureMmu->s_axis_tdata = 0;

    textureMmu->s_araddr = 0;
    textureMmu->s_arvalid = 0;
    textureMmu->s_rready = 0;

    textureMmu->m_axi_arready = 0;
    textureMmu->m_axi_rid = 0;
    textureMmu->m_axi_rdata = 0;
    textureMmu->m_axi_rresp = 0;
    textureMmu->m_axi_rlast = 0;
    textureMmu->m_axi_rvalid = 0;

    rr::ut::reset(textureMmu);
}

template <std::size_t EntryCount>
void loadPageTable(VTextureMMU* textureMmu, const std::array<std::uint32_t, EntryCount>& pageTable)
{
    REQUIRE(textureMmu->s_axis_tready == 1);

    textureMmu->s_axis_tvalid = 1;
    for (std::size_t entryIndex = 0; entryIndex < EntryCount; ++entryIndex)
    {
        textureMmu->s_axis_tdata = pageTable[entryIndex];
        textureMmu->s_axis_tlast = entryIndex + 1 == EntryCount;
        rr::ut::clk(textureMmu);
    }

    textureMmu->s_axis_tvalid = 0;
    textureMmu->s_axis_tlast = 0;
    textureMmu->s_axis_tdata = 0;
    textureMmu->eval();
}

std::array<std::uint32_t, PAGE_TABLE_ENTRIES> makePageTable(std::uint32_t baseAddress)
{
    std::array<std::uint32_t, PAGE_TABLE_ENTRIES> pageTable {};
    for (std::size_t entryIndex = 0; entryIndex < PAGE_TABLE_ENTRIES; ++entryIndex)
    {
        pageTable[entryIndex] = baseAddress + static_cast<std::uint32_t>(entryIndex * PAGE_SIZE);
    }
    return pageTable;
}

template <std::size_t EntryCount>
std::uint32_t translate(const std::array<std::uint32_t, EntryCount>& pageTable,
    std::uint32_t virtualAddress)
{
    const auto pageIndex = virtualAddress / PAGE_SIZE;
    const auto pageOffset = virtualAddress % PAGE_SIZE;
    return pageTable[pageIndex] + pageOffset;
}

std::uint32_t translate(VTextureMMU* textureMmu, std::uint32_t virtualAddress)
{
    textureMmu->s_araddr = virtualAddress;
    textureMmu->s_arvalid = 1;
    rr::ut::clk(textureMmu);
    return textureMmu->m_axi_araddr;
}

} // namespace

TEST_CASE("Translate one page-table entry", "[TextureMMU]")
{
    VTextureMMU* textureMmu = rr::ut::makeTop<VTextureMMU>();
    initialize(textureMmu);

    const std::array<std::uint32_t, 1> pageTable { 0x1000'0000 };
    loadPageTable(textureMmu, pageTable);

    constexpr std::uint32_t virtualAddress { 0x00345 };
    textureMmu->s_arvalid = 1;
    textureMmu->s_araddr = virtualAddress;
    textureMmu->m_axi_arready = 1;
    rr::ut::clk(textureMmu);
    CHECK(textureMmu->m_axi_araddr == translate(pageTable, virtualAddress));
    CHECK(textureMmu->m_axi_arvalid == 1);
    CHECK(textureMmu->s_arready == 1);

    delete textureMmu;
}

TEST_CASE("Translate every page-table entry", "[TextureMMU]")
{
    VTextureMMU* textureMmu = rr::ut::makeTop<VTextureMMU>();
    initialize(textureMmu);

    const auto pageTable = makePageTable(0x2000'0000);
    loadPageTable(textureMmu, pageTable);
    textureMmu->m_axi_arready = 1;

    for (std::size_t pageIndex = 0; pageIndex < PAGE_TABLE_ENTRIES; ++pageIndex)
    {
        const auto virtualAddress = static_cast<std::uint32_t>(pageIndex * PAGE_SIZE) + 0x345;
        CHECK(translate(textureMmu, virtualAddress) == translate(pageTable, virtualAddress));
    }

    delete textureMmu;
}

TEST_CASE("Overwrite the page table", "[TextureMMU]")
{
    VTextureMMU* textureMmu = rr::ut::makeTop<VTextureMMU>();
    initialize(textureMmu);

    const auto originalPageTable = makePageTable(0x3000'0000);
    const auto replacementPageTable = makePageTable(0x4000'0000);
    loadPageTable(textureMmu, originalPageTable);
    textureMmu->m_axi_arready = 1;

    constexpr std::uint32_t virtualAddress { (7 * PAGE_SIZE) + 0x345 };
    CHECK(translate(textureMmu, virtualAddress) == translate(originalPageTable, virtualAddress));

    loadPageTable(textureMmu, replacementPageTable);
    for (std::size_t pageIndex = 0; pageIndex < PAGE_TABLE_ENTRIES; ++pageIndex)
    {
        const auto replacementVirtualAddress = static_cast<std::uint32_t>(pageIndex * PAGE_SIZE) + 0x345;
        CHECK(translate(textureMmu, replacementVirtualAddress)
            == translate(replacementPageTable, replacementVirtualAddress));
    }

    delete textureMmu;
}