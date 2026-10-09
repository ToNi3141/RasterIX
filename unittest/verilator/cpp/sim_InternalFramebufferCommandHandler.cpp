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
// Include model header, generated from Verilating "top.v"
#include "VInternalFramebufferCommandHandler.h"

static constexpr std::size_t NUMBER_OF_PIXELS_PER_BEAT = 2;
static constexpr std::size_t AXI_BURST_BEATS = 16;

static std::size_t getTransferBeats(const std::size_t pixelCount)
{
    constexpr std::size_t pixelsPerBurst = NUMBER_OF_PIXELS_PER_BEAT * AXI_BURST_BEATS;
    const std::size_t paddedPixels = ((pixelCount + pixelsPerBurst - 1) / pixelsPerBurst) * pixelsPerBurst;
    return paddedPixels / NUMBER_OF_PIXELS_PER_BEAT;
}

static uint8_t getPixelValidMask(const std::size_t pixelOffset, const std::size_t pixelCount)
{
    const uint8_t lane0 = pixelOffset < pixelCount ? 0b01 : 0;
    const uint8_t lane1 = (pixelOffset + 1) < pixelCount ? 0b10 : 0;
    return lane0 | lane1;
}

TEST_CASE("commit", "[VInternalFramebufferCommandHandler]")
{
    VInternalFramebufferCommandHandler* t = rr::ut::makeTop<VInternalFramebufferCommandHandler>();

    rr::ut::reset(t);

    t->m_aready = 0;

    t->apply = 1;
    t->cmdCommit = 1;
    t->cmdSize = 220;
    t->cmdAddr = 0x1000'0000;

    t->m_axis_tready = 1; // Set ready signal to 1 to allow data transfer
    t->readDataPort = 0;
    rr::ut::clk(t);
    CHECK(t->readAddrPort == 0);
    CHECK(t->m_axis_tvalid == 0);
    CHECK(t->m_avalid == 1);
    CHECK(t->m_aaddr == t->cmdAddr);
    CHECK(t->m_abeats == getTransferBeats(t->cmdSize));
    CHECK(t->m_arnw == 1);

    rr::ut::clk(t);
    CHECK(t->readAddrPort == 1);
    CHECK(t->m_axis_tvalid == 0);
    CHECK(t->m_avalid == 1);
    CHECK(t->m_aaddr == t->cmdAddr);
    CHECK(t->m_abeats == getTransferBeats(t->cmdSize));
    CHECK(t->m_arnw == 1);

    t->apply = 0;
    const std::size_t numberOfBeats = getTransferBeats(t->cmdSize);
    for (std::size_t i = 0; i < numberOfBeats; i++)
    {
        t->readDataPort = i; // Simulate reading data from memory
        rr::ut::clk(t);
        CHECK(t->readAddrPort == i + 2);
        CHECK(t->applied == 0);

        CHECK(t->m_axis_tdata == i);
        CHECK(t->m_axis_tstrb == getPixelValidMask(i * NUMBER_OF_PIXELS_PER_BEAT, t->cmdSize));
        CHECK(t->m_axis_tvalid == 1);
        CHECK(t->m_axis_tlast == (i == numberOfBeats - 1));

        CHECK(t->m_avalid == 1);
        CHECK(t->m_aaddr == t->cmdAddr);
        CHECK(t->m_abeats == getTransferBeats(t->cmdSize));
    }

    // Additional clocks to check, that applied stays 0 as long as m_aready is not asserted
    rr::ut::clk(t);
    CHECK(t->m_axis_tvalid == 0);
    CHECK(t->m_avalid == 1);
    CHECK(t->applied == 0);

    t->m_aready = 1;
    rr::ut::clk(t);
    CHECK(t->m_axis_tvalid == 0);
    CHECK(t->m_avalid == 0);
    CHECK(t->applied == 0);

    rr::ut::clk(t);
    CHECK(t->m_axis_tvalid == 0);
    CHECK(t->m_avalid == 0);
    CHECK(t->applied == 1);

    // Destroy model
    delete t;
}

TEST_CASE("commit pixel strobe handles exact-size tail and burst padding", "[VInternalFramebufferCommandHandler]")
{
    VInternalFramebufferCommandHandler* t = rr::ut::makeTop<VInternalFramebufferCommandHandler>();

    rr::ut::reset(t);

    t->apply = 1;
    t->cmdCommit = 1;
    t->cmdSize = 3; // Exact size ends part-way through a pixel beat.
    t->m_axis_tready = 1;
    t->readDataPort = 0;

    rr::ut::clk(t);
    t->apply = 0;
    rr::ut::clk(t);
    CHECK(t->m_axis_tvalid == 0);

    const std::size_t numberOfBeats = getTransferBeats(t->cmdSize);
    CHECK(t->m_abeats == numberOfBeats);
    for (std::size_t i = 0; i < numberOfBeats; i++)
    {
        t->readDataPort = i;
        rr::ut::clk(t);
        CHECK(t->m_axis_tvalid == 1);
        CHECK(t->m_axis_tdata == i);
        CHECK(t->m_axis_tstrb == getPixelValidMask(i * NUMBER_OF_PIXELS_PER_BEAT, t->cmdSize));
        CHECK(t->m_axis_tlast == (i == numberOfBeats - 1));
    }

    delete t;
}

TEST_CASE("commit with interrupted stream", "[VInternalFramebufferCommandHandler]")
{
    VInternalFramebufferCommandHandler* t = rr::ut::makeTop<VInternalFramebufferCommandHandler>();

    rr::ut::reset(t);

    t->apply = 1;
    t->cmdCommit = 1;
    t->cmdSize = 200;

    t->m_axis_tready = 1; // Set ready signal to 1 to allow data transfer
    t->readDataPort = 1;
    rr::ut::clk(t);
    CHECK(t->readAddrPort == 0);
    CHECK(t->m_axis_tvalid == 0);

    t->m_axis_tready = 0;
    t->readDataPort = 0;
    rr::ut::clk(t);
    CHECK(t->readAddrPort == 1);
    CHECK(t->m_axis_tvalid == 0);

    t->m_axis_tready = 0;
    t->readDataPort = 2;
    rr::ut::clk(t);
    CHECK(t->readAddrPort == 2);
    CHECK(t->m_axis_tvalid == 1);
    CHECK(t->m_axis_tdata == 2);

    const std::size_t numberOfBeats = getTransferBeats(t->cmdSize);
    for (std::size_t i = 0; i < numberOfBeats - 1; i++)
    {
        t->m_axis_tready = 0; // Simulate that the stream is interrupted
        t->readDataPort = i + 3;
        rr::ut::clk(t);
        CHECK(t->readAddrPort == i + 2);
        CHECK(t->applied == 0);
        CHECK(t->m_axis_tdata == i + 2);
        CHECK(t->m_axis_tstrb == getPixelValidMask(i * NUMBER_OF_PIXELS_PER_BEAT, t->cmdSize));
        CHECK(t->m_axis_tvalid == 1);
        CHECK(t->m_axis_tlast == (i == numberOfBeats - 1));
        t->apply = 0;

        t->m_axis_tready = 1;
        if (!t->m_axis_tlast)
        {
            rr::ut::clk(t);
            CHECK(t->readAddrPort == i + 3);
            CHECK(t->applied == 0);
            CHECK(t->m_axis_tdata == i + 3);
            CHECK(t->m_axis_tstrb == getPixelValidMask((i + 1) * NUMBER_OF_PIXELS_PER_BEAT, t->cmdSize));
            CHECK(t->m_axis_tvalid == 1);
            CHECK(t->m_axis_tlast == ((i + 1) == numberOfBeats - 1));
        }
    }

    t->m_aready = 1;
    rr::ut::clk(t);
    CHECK(t->m_axis_tvalid == 0);

    rr::ut::clk(t);
    CHECK(t->applied == 1);

    // Destroy model
    delete t;
}

TEST_CASE("read", "[VInternalFramebufferCommandHandler]")
{
    VInternalFramebufferCommandHandler* t = rr::ut::makeTop<VInternalFramebufferCommandHandler>();

    rr::ut::reset(t);

    t->m_aready = 0;

    t->apply = 1;
    t->cmdRead = 1;
    t->cmdSize = 0x100;
    t->cmdAddr = 0x1000'0000;

    t->m_axis_tready = 1; // Set ready signal to 1 to allow data transfer
    rr::ut::clk(t);
    CHECK(t->writeAddrPort == 0);
    CHECK(t->s_axis_tready == 1);
    CHECK(t->m_avalid == 1);
    CHECK(t->m_aaddr == t->cmdAddr);
    CHECK(t->m_abeats == getTransferBeats(t->cmdSize));
    CHECK(t->m_arnw == 0);

    rr::ut::clk(t);
    CHECK(t->writeAddrPort == 0);
    CHECK(t->s_axis_tready == 1);
    CHECK(t->m_avalid == 1);
    CHECK(t->m_aaddr == t->cmdAddr);
    CHECK(t->m_abeats == getTransferBeats(t->cmdSize));
    CHECK(t->m_arnw == 0);

    t->apply = 0;
    for (std::size_t i = 0; i < t->cmdSize; i += NUMBER_OF_PIXELS_PER_BEAT)
    {
        t->s_axis_tvalid = 1; // Simulate that the AXIS stream is valid
        t->s_axis_tdata = i;
        t->s_axis_tlast = (i + NUMBER_OF_PIXELS_PER_BEAT) >= t->cmdSize;
        rr::ut::clk(t);
        CHECK(t->writeAddrPort == i / NUMBER_OF_PIXELS_PER_BEAT);
        CHECK(t->writeEnablePort == 1);
        CHECK(t->writeDataPort == i);
        CHECK(t->writeMaskPort == 0b11);
        CHECK(t->s_axis_tready == !t->s_axis_tlast);
        CHECK(t->applied == 0);

        CHECK(t->m_avalid == 1);
        CHECK(t->m_aaddr == t->cmdAddr);
        CHECK(t->m_abeats == getTransferBeats(t->cmdSize));
    }

    // Additional clocks to check, that applied stays 0 as long as m_aready is not asserted
    rr::ut::clk(t);
    CHECK(t->m_axis_tvalid == 0);
    CHECK(t->m_avalid == 1);
    CHECK(t->applied == 0);
    CHECK(t->writeEnablePort == 0);
    CHECK(t->s_axis_tready == 0);

    t->m_aready = 1;
    rr::ut::clk(t);
    CHECK(t->m_axis_tvalid == 0);
    CHECK(t->m_avalid == 0);
    CHECK(t->applied == 0);
    CHECK(t->writeEnablePort == 0);
    CHECK(t->s_axis_tready == 0);

    rr::ut::clk(t);
    CHECK(t->m_axis_tvalid == 0);
    CHECK(t->m_avalid == 0);
    CHECK(t->applied == 1);
    CHECK(t->writeEnablePort == 0);
    CHECK(t->s_axis_tready == 0);

    // Destroy model
    delete t;
}

TEST_CASE("read pixel-valid mask handles burst padding beyond exact size", "[VInternalFramebufferCommandHandler]")
{
    VInternalFramebufferCommandHandler* t = rr::ut::makeTop<VInternalFramebufferCommandHandler>();

    rr::ut::reset(t);
    t->m_aready = 0;
    t->apply = 1;
    t->cmdRead = 1;
    t->cmdSize = 3;
    t->cmdAddr = 0x1000'0000;

    rr::ut::clk(t);
    CHECK(t->m_abeats == getTransferBeats(t->cmdSize));
    CHECK(t->m_abeats == AXI_BURST_BEATS);

    t->apply = 0;
    const std::size_t transferPixels = getTransferBeats(t->cmdSize) * NUMBER_OF_PIXELS_PER_BEAT;
    for (std::size_t pixelOffset = 0; pixelOffset < transferPixels; pixelOffset += NUMBER_OF_PIXELS_PER_BEAT)
    {
        t->s_axis_tvalid = 1;
        t->s_axis_tdata = pixelOffset;
        t->s_axis_tlast = (pixelOffset + NUMBER_OF_PIXELS_PER_BEAT) >= transferPixels;
        rr::ut::clk(t);

        CHECK(t->writeAddrPort == pixelOffset / NUMBER_OF_PIXELS_PER_BEAT);
        CHECK(t->writeEnablePort == 1);
        CHECK(t->writeDataPort == pixelOffset);
        CHECK(t->writeMaskPort == getPixelValidMask(pixelOffset, t->cmdSize));
        CHECK(t->s_axis_tready == !t->s_axis_tlast);
    }

    rr::ut::clk(t);
    CHECK(t->writeEnablePort == 0);
    t->m_aready = 1;
    rr::ut::clk(t);
    rr::ut::clk(t);
    CHECK(t->applied == 1);

    delete t;
}