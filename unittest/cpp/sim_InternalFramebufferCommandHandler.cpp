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
#include "VInternalFramebufferCommandHandler.h"

static constexpr std::size_t NUMBER_OF_PIXELS_PER_BEAT = 2;

static bool isInScissorArea(
    const std::size_t x,
    const std::size_t y,
    const std::size_t scissorStartX,
    const std::size_t scissorStartY,
    const std::size_t scissorEndX,
    const std::size_t scissorEndY,
    const std::size_t yOffset,
    const std::size_t yResolution)
{
    // Note: The Y coordinate is flipped. The Framebuffer's Y coordinate starts at the top left, while the openGL Y coordinate starts at the bottom left.
    std::size_t tY = yOffset + yResolution - 1 - y; // Apply Y offset and flip Y coordinate
    return (x >= scissorStartX && x < scissorEndX) && (tY >= scissorStartY && tY < scissorEndY);
}

static uint8_t getScissorMask(
    const std::size_t x,
    const std::size_t y,
    const std::size_t scissorStartX,
    const std::size_t scissorStartY,
    const std::size_t scissorEndX,
    const std::size_t scissorEndY,
    const std::size_t yOffset,
    const std::size_t yResolution)
{
    const uint8_t maskScissor1 = isInScissorArea(
                                     x,
                                     y,
                                     scissorStartX,
                                     scissorStartY,
                                     scissorEndX,
                                     scissorEndY,
                                     yOffset,
                                     yResolution)
        ? 0b0000'1111
        : 0b0000'0000;
    const uint8_t maskScissor0 = isInScissorArea(x + 1,
                                     y,
                                     scissorStartX,
                                     scissorStartY,
                                     scissorEndX,
                                     scissorEndY,
                                     yOffset,
                                     yResolution)
        ? 0b1111'0000
        : 0b0000'0000;
    const uint8_t maskScissor = maskScissor0 | maskScissor1;
    return maskScissor;
}

TEST_CASE("memset", "[VInternalFramebufferCommandHandler]")
{
    VInternalFramebufferCommandHandler* t = new VInternalFramebufferCommandHandler();

    rr::ut::reset(t);

    t->confClearColor = 0xaabbccdd; // Set clear color
    t->confEnableScissor = 0; // Disable scissor test
    t->confScissorStartX = 0;
    t->confScissorStartY = 0;
    t->confScissorEndX = 0;
    t->confScissorEndY = 0;
    t->confYOffset = 0;
    t->confXResolution = 20;
    t->confYResolution = 10;
    t->confMask = 0b1111; // Enable all sub-pixels

    t->apply = 1;
    t->cmdMemset = 1; // Trigger memset command
    t->cmdSize = t->confXResolution * t->confYResolution; // Set size to fill
    rr::ut::clk(t);

    for (std::size_t i = 0; i < (t->confXResolution * t->confYResolution) / NUMBER_OF_PIXELS_PER_BEAT; i++)
    {
        rr::ut::clk(t);
        CHECK(t->writeEnablePort == 1);
        CHECK(t->writeDataPort == 0xaabbccdd'aabbccdd);
        CHECK(t->writeAddrPort == i);
        CHECK(t->writeMaskPort == 0b1111'1111);
        CHECK(t->applied == 0);
        t->apply = 0;
    }

    rr::ut::clk(t);
    CHECK(t->writeEnablePort == 0);

    rr::ut::clk(t);
    CHECK(t->applied == 1);

    // Destroy model
    delete t;
}

TEST_CASE("memset with scissor and offset", "[VInternalFramebufferCommandHandler]")
{
    VInternalFramebufferCommandHandler* t = new VInternalFramebufferCommandHandler();

    rr::ut::reset(t);

    t->confClearColor = 0xaabbccdd; // Set clear color
    t->confEnableScissor = 1; // Disable scissor test
    t->confScissorStartX = 5;
    t->confScissorStartY = 15;
    t->confScissorEndX = 11;
    t->confScissorEndY = 20;
    t->confYOffset = 10;
    t->confXResolution = 20;
    t->confYResolution = 10;
    t->confMask = 0b1111; // Enable all sub-pixels

    t->apply = 1;
    t->cmdMemset = 1; // Trigger memset command
    t->cmdSize = t->confXResolution * t->confYResolution; // Set size to fill
    rr::ut::clk(t);

    for (std::size_t y = 0; y < t->confYResolution; y++)
    {
        for (std::size_t x = 0; x < t->confXResolution; x += NUMBER_OF_PIXELS_PER_BEAT)
        {
            // Calculate index in memory
            std::size_t i = (y * t->confXResolution + x) / NUMBER_OF_PIXELS_PER_BEAT;
            rr::ut::clk(t);
            CHECK(t->writeEnablePort == 1);
            CHECK(t->writeDataPort == 0xaabbccdd'aabbccdd);
            CHECK(t->writeAddrPort == i);
            CHECK(t->writeMaskPort == getScissorMask(x, y, t->confScissorStartX, t->confScissorStartY, t->confScissorEndX, t->confScissorEndY, t->confYOffset, t->confYResolution));
            CHECK(t->applied == 0);
            t->apply = 0;
        }
    }

    rr::ut::clk(t);
    CHECK(t->writeEnablePort == 0);

    rr::ut::clk(t);
    CHECK(t->applied == 1);

    // Destroy model
    delete t;
}

TEST_CASE("commit", "[VInternalFramebufferCommandHandler]")
{
    VInternalFramebufferCommandHandler* t = new VInternalFramebufferCommandHandler();

    rr::ut::reset(t);

    t->m_aready = 0;

    t->confEnableScissor = 0; // Disable scissor test
    t->confYOffset = 0;
    t->confXResolution = 20;
    t->confYResolution = 10;

    t->apply = 1;
    t->cmdCommit = 1;
    t->cmdSize = (t->confXResolution * t->confYResolution) + t->confXResolution;
    t->cmdAddr = 0x1000'0000;

    t->m_axis_tready = 1; // Set ready signal to 1 to allow data transfer
    t->readDataPort = 0;
    rr::ut::clk(t);
    CHECK(t->readAddrPort == 0);
    CHECK(t->m_axis_tvalid == 0);
    CHECK(t->m_avalid == 1);
    CHECK(t->m_aaddr == t->cmdAddr);
    CHECK(t->m_abytes == (t->cmdSize * 2));
    CHECK(t->m_arnw == 1);

    rr::ut::clk(t);
    CHECK(t->readAddrPort == 1);
    CHECK(t->m_axis_tvalid == 0);
    CHECK(t->m_avalid == 1);
    CHECK(t->m_aaddr == t->cmdAddr);
    CHECK(t->m_abytes == (t->cmdSize * 2));
    CHECK(t->m_arnw == 1);

    t->apply = 0;
    const std::size_t numberOfBeats = ((t->confXResolution * t->confYResolution) / NUMBER_OF_PIXELS_PER_BEAT);
    for (std::size_t y = 0; y < t->confYResolution + 1; y++) // + to simulate an additional line (in respect to the cmdSize)
    {
        for (std::size_t x = 0; x < t->confXResolution; x += NUMBER_OF_PIXELS_PER_BEAT)
        {
            std::size_t i = (y * t->confXResolution + x) / NUMBER_OF_PIXELS_PER_BEAT;

            t->readDataPort = i; // Simulate reading data from memory
            rr::ut::clk(t);
            CHECK(t->readAddrPort == i + 2);
            CHECK(t->applied == 0);

            CHECK(t->m_axis_tdata == i);
            CHECK(t->m_axis_tstrb == getScissorMask(x, y, 0, 0, t->confXResolution, t->confYResolution, 0, t->confYResolution));
            CHECK(t->m_axis_tvalid == 1);
            CHECK(t->m_axis_tlast == (i >= (t->cmdSize / NUMBER_OF_PIXELS_PER_BEAT) - 1));

            CHECK(t->m_avalid == 1);
            CHECK(t->m_aaddr == t->cmdAddr);
            CHECK(t->m_abytes == (t->cmdSize * 2));
        }
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

TEST_CASE("commit with interrupted stream", "[VInternalFramebufferCommandHandler]")
{
    VInternalFramebufferCommandHandler* t = new VInternalFramebufferCommandHandler();

    rr::ut::reset(t);

    t->confEnableScissor = 0; // Disable scissor test
    t->confYOffset = 0;
    t->confXResolution = 20;
    t->confYResolution = 10;

    t->apply = 1;
    t->cmdCommit = 1;
    t->cmdSize = t->confXResolution * t->confYResolution;

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

    const std::size_t numberOfBeats = ((t->confXResolution * t->confYResolution) / NUMBER_OF_PIXELS_PER_BEAT);
    for (std::size_t i = 0; i < numberOfBeats - 1; i++)
    {
        t->m_axis_tready = 0; // Simulate that the stream is interrupted
        t->readDataPort = i + 3;
        rr::ut::clk(t);
        CHECK(t->readAddrPort == i + 2);
        CHECK(t->applied == 0);
        CHECK(t->m_axis_tdata == i + 2);
        CHECK(t->m_axis_tstrb == 0b1111'1111);
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
            CHECK(t->m_axis_tstrb == 0b1111'1111);
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
    VInternalFramebufferCommandHandler* t = new VInternalFramebufferCommandHandler();

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
    CHECK(t->m_abytes == (t->cmdSize * 2));
    CHECK(t->m_arnw == 0);

    rr::ut::clk(t);
    CHECK(t->writeAddrPort == 0);
    CHECK(t->s_axis_tready == 1);
    CHECK(t->m_avalid == 1);
    CHECK(t->m_aaddr == t->cmdAddr);
    CHECK(t->m_abytes == (t->cmdSize * 2));
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
        CHECK(t->writeMaskPort == 0b1111'1111);
        CHECK(t->s_axis_tready == !t->s_axis_tlast);
        CHECK(t->applied == 0);

        CHECK(t->m_avalid == 1);
        CHECK(t->m_aaddr == t->cmdAddr);
        CHECK(t->m_abytes == (t->cmdSize * 2));
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