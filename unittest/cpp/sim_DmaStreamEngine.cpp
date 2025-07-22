// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2023 ToNi3141

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

// #define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
// #include "../Unittests/3rdParty/catch.hpp"

#include "general.hpp"

// Include model header, generated from Verilating "top.v"
#include "VDmaStreamEngine.h"

void reset(VDmaStreamEngine& t)
{
    t.resetn = 0;

    t.m_st0_axis_tready = 0;

    t.s_st0_axis_tvalid = 0;
    t.s_st0_axis_tlast = 0;
    t.s_st0_axis_tdata = 0;

    rr::ut::clk(&t);

    t.resetn = 1;
    rr::ut::clk(&t);
}

void configPhase(VDmaStreamEngine& t, uint32_t command, uint32_t addr)
{
    // COMMAND ///////////////////////////
    // Outputs
    REQUIRE(t.m_mem_axi_awvalid == 0);
    REQUIRE(t.m_mem_axi_wvalid == 0);
    REQUIRE(t.m_mem_axi_arvalid == 0);
    REQUIRE(t.m_mem_axi_rready == 0);
    REQUIRE(t.m_st0_axis_tvalid == 0);
    REQUIRE(t.s_st0_axis_tready == 1);
    REQUIRE(t.m_st1_axis_tvalid == 0);
    REQUIRE(t.s_st1_axis_tready == 0);

    // Inputs
    // Input command
    t.s_st0_axis_tdata = command;
    t.s_st0_axis_tvalid = 1;

    // Process command
    rr::ut::clk(&t);

    // COMMAND ADDR //////////////////////
    // Outputs
    REQUIRE(t.m_mem_axi_awvalid == 0);
    REQUIRE(t.m_mem_axi_wvalid == 0);
    REQUIRE(t.m_mem_axi_arvalid == 0);
    REQUIRE(t.m_mem_axi_rready == 0);
    REQUIRE(t.m_st0_axis_tvalid == 0);
    REQUIRE(t.s_st0_axis_tready == 1);
    REQUIRE(t.m_st1_axis_tvalid == 0);
    REQUIRE(t.s_st1_axis_tready == 0);

    // Inputs
    // Input command
    t.s_st0_axis_tdata = addr;
    t.s_st0_axis_tvalid = 1;

    // Process command
    rr::ut::clk(&t);
}

TEST_CASE("Mux data simple (st0 -> st0)", "[Stream]")
{
    VDmaStreamEngine* top = new VDmaStreamEngine();
    VDmaStreamEngine& t = *top;

    reset(t);
    static constexpr uint32_t OP = 0x5000'0000;
    static constexpr uint32_t SIZE = 0x0000'0004;
    static constexpr uint32_t COMMAND = OP | SIZE; // Stream 4 bytes from one stream interface to another

    // Tell the command parser that we are ready to receive data
    t.m_st0_axis_tready = 1;

    // Run this test two times, so that we know that we can start a new transfer after the old one
    for (uint32_t i = 0; i < 2; i++)
    {
        // COMMAND ///////////////////////////
        configPhase(t, COMMAND, 0);

        // STREAM ////////////////////////////
        // Input port asserts the ready signal. Valid is deasserted because no valid data are available
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 1);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_st0_axis_tlast == 0);

        t.m_st0_axis_tready = 1; // Signal ready to the master port (the port which is simulated in this test)

        // Set first 4 bytes of data
        t.s_st0_axis_tdata = 0xaa00ff55;
        t.s_st0_axis_tvalid = 1;

        // Process first 4 bytes of data
        rr::ut::clk(top);

        // STREAM ////////////////////////////
        // Assert tvalid to mark available data. Deassert tready (last byte) and assert tlast.
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 1);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_st0_axis_tlast == 1);

        REQUIRE(t.m_st0_axis_tdata == 0xaa00ff55);

        t.s_st0_axis_tvalid = 1; // Keep tvalid asserted to check that it does not pull further data.

        rr::ut::clk(top);

        // IDLE /////////////////////////////
        // Deassert everything (no read or write should be in progress now)
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_st0_axis_tlast == 0);

        rr::ut::clk(top);
    }

    // Final model cleanup
    top->final();

    // Destroy model
    delete top;
}

TEST_CASE("Mux data simple (st0 -> st1)", "[Stream]")
{
    VDmaStreamEngine* top = new VDmaStreamEngine();
    VDmaStreamEngine& t = *top;

    reset(t);
    static constexpr uint32_t OP = 0x9000'0000;
    static constexpr uint32_t SIZE = 0x0000'0004;
    static constexpr uint32_t COMMAND = OP | SIZE; // Stream 4 bytes from one stream interface to another

    // Tell the command parser that we are ready to receive data
    t.m_st0_axis_tready = 1;

    // Run this test two times, so that we know that we can start a new transfer after the old one
    for (uint32_t i = 0; i < 2; i++)
    {
        // COMMAND ///////////////////////////
        configPhase(t, COMMAND, 0);

        // STREAM ////////////////////////////
        // Input port asserts the ready signal. Valid is deasserted because no valid data are available
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 1);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_st1_axis_tlast == 0);

        t.m_st1_axis_tready = 1; // Signal ready to the master port (the port which is simulated in this test)

        // Set first 4 bytes of data
        t.s_st0_axis_tdata = 0xaa00ff55;
        t.s_st0_axis_tvalid = 1;

        // Process first 4 bytes of data
        rr::ut::clk(top);

        // STREAM ////////////////////////////
        // Assert tvalid to mark available data. Deassert tready (last byte) and assert tlast.
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 1);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_st1_axis_tlast == 1);

        REQUIRE(t.m_st1_axis_tdata == 0xaa00ff55);

        t.s_st0_axis_tvalid = 1; // Keep tvalid asserted to check that it does not pull further data.

        rr::ut::clk(top);

        // IDLE /////////////////////////////
        // Deassert everything (no read or write should be in progress now)
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_st0_axis_tlast == 0);

        rr::ut::clk(top);
    }

    // Final model cleanup
    top->final();

    // Destroy model
    delete top;
}

TEST_CASE("Mux data simple (st1 -> st1)", "[Stream]")
{
    VDmaStreamEngine* top = new VDmaStreamEngine();
    VDmaStreamEngine& t = *top;

    reset(t);
    static constexpr uint32_t OP = 0xA000'0000;
    static constexpr uint32_t SIZE = 0x0000'0004;
    static constexpr uint32_t COMMAND = OP | SIZE; // Stream 4 bytes from one stream interface to another

    // Tell the command parser that we are ready to receive data
    t.m_st0_axis_tready = 1;

    // Run this test two times, so that we know that we can start a new transfer after the old one
    for (uint32_t i = 0; i < 2; i++)
    {
        // COMMAND ///////////////////////////
        configPhase(t, COMMAND, 0);

        // STREAM ////////////////////////////
        // Input port asserts the ready signal. Valid is deasserted because no valid data are available
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 1);

        REQUIRE(t.m_st1_axis_tlast == 0);

        t.m_st1_axis_tready = 1; // Signal ready to the master port (the port which is simulated in this test)

        // Set first 4 bytes of data
        t.s_st1_axis_tdata = 0xaa00ff55;
        t.s_st1_axis_tvalid = 1;

        // Process first 4 bytes of data
        rr::ut::clk(top);

        // STREAM ////////////////////////////
        // Assert tvalid to mark available data. Deassert tready (last byte) and assert tlast.
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 1);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_st1_axis_tlast == 1);

        REQUIRE(t.m_st1_axis_tdata == 0xaa00ff55);

        t.s_st1_axis_tvalid = 1; // Keep tvalid asserted to check that it does not pull further data.

        rr::ut::clk(top);

        // IDLE /////////////////////////////
        // Deassert everything (no read or write should be in progress now)
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_st0_axis_tlast == 0);

        rr::ut::clk(top);
    }

    // Final model cleanup
    top->final();

    // Destroy model
    delete top;
}

TEST_CASE("Mux data simple (st1 -> st0)", "[Stream]")
{
    VDmaStreamEngine* top = new VDmaStreamEngine();
    VDmaStreamEngine& t = *top;

    reset(t);
    static constexpr uint32_t OP = 0x6000'0000;
    static constexpr uint32_t SIZE = 0x0000'0004;
    static constexpr uint32_t COMMAND = OP | SIZE; // Stream 4 bytes from one stream interface to another

    // Tell the command parser that we are ready to receive data
    t.m_st0_axis_tready = 1;

    // Run this test two times, so that we know that we can start a new transfer after the old one
    for (uint32_t i = 0; i < 2; i++)
    {
        // COMMAND ///////////////////////////
        configPhase(t, COMMAND, 0);

        // STREAM ////////////////////////////
        // Input port asserts the ready signal. Valid is deasserted because no valid data are available
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 1);

        REQUIRE(t.m_st1_axis_tlast == 0);

        t.m_st0_axis_tready = 1; // Signal ready to the master port (the port which is simulated in this test)

        // Set first 4 bytes of data
        t.s_st1_axis_tdata = 0xaa00ff55;
        t.s_st1_axis_tvalid = 1;

        // Process first 4 bytes of data
        rr::ut::clk(top);

        // STREAM ////////////////////////////
        // Assert tvalid to mark available data. Deassert tready (last byte) and assert tlast.
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 1);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_st0_axis_tlast == 1);

        REQUIRE(t.m_st0_axis_tdata == 0xaa00ff55);

        t.s_st1_axis_tvalid = 1; // Keep tvalid asserted to check that it does not pull further data.

        rr::ut::clk(top);

        // IDLE /////////////////////////////
        // Deassert everything (no read or write should be in progress now)
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_st0_axis_tlast == 0);

        rr::ut::clk(top);
    }

    // Final model cleanup
    top->final();

    // Destroy model
    delete top;
}

TEST_CASE("Stream data interrupted from master (st0 -> st0)", "[Stream]")
{
    VDmaStreamEngine* top = new VDmaStreamEngine();
    VDmaStreamEngine& t = *top;

    reset(t);
    static constexpr uint32_t OP = 0x5000'0000;
    static constexpr uint32_t SIZE = 0x0000'0008;
    static constexpr uint32_t COMMAND = OP | SIZE; // Stream 8 bytes from one stream interface to another

    // Run this test two times, so that we know that we can start a new transfer after the old one
    for (uint32_t i = 0; i < 2; i++)
    {
        // COMMAND ///////////////////////////
        configPhase(t, COMMAND, 0);

        // STREAM ////////////////////////////
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.m_st0_axis_tlast == 0);

        REQUIRE(t.s_st0_axis_tready == 1);

        // Inputs
        // Set first 4 bytes of data
        t.s_st0_axis_tdata = 0xaa00ff55;

        // Set master port to not ready
        t.m_st0_axis_tready = 0;

        // Process first 4 bytes of data
        rr::ut::clk(top);

        // STREAM ////////////////////////////
        // Read first 4 bytes from master interface
        REQUIRE(t.m_st0_axis_tvalid == 1);
        REQUIRE(t.m_st0_axis_tdata == 0xaa00ff55);
        REQUIRE(t.m_st0_axis_tlast == 0);

        // Master port is not ready, but we can fetch data
        REQUIRE(t.s_st0_axis_tready == 1);

        // Set the next 4 bytes of data
        t.s_st0_axis_tdata = 0xff00ff00;

        // Set master to not ready because we have no data right now
        t.m_st0_axis_tready = 0;

        rr::ut::clk(top);

        // STREAM ////////////////////////////
        // Read first 4 bytes from master interface
        REQUIRE(t.m_st0_axis_tvalid == 1);
        REQUIRE(t.m_st0_axis_tdata == 0xaa00ff55);
        REQUIRE(t.m_st0_axis_tlast == 0);

        // Master port is not ready, so we interrupting the stream on the slave
        REQUIRE(t.s_st0_axis_tready == 0);

        // Set the next 4 bytes of data
        t.s_st0_axis_tdata = 0xff00ff00;

        // Set master to ready
        t.m_st0_axis_tready = 1;

        rr::ut::clk(top);

        // STREAM ////////////////////////////
        // Read first 4 bytes from master interface
        REQUIRE(t.m_st0_axis_tvalid == 1);
        REQUIRE(t.m_st0_axis_tdata == 0xff00ff00);
        REQUIRE(t.m_st0_axis_tlast == 1);

        REQUIRE(t.s_st0_axis_tready == 0);

        // Set master port to not ready
        t.m_st0_axis_tready = 0;

        rr::ut::clk(top);

        // STREAM ////////////////////////////
        // Read first 4 bytes from master interface
        REQUIRE(t.m_st0_axis_tvalid == 1);
        REQUIRE(t.m_st0_axis_tdata == 0xff00ff00);
        REQUIRE(t.m_st0_axis_tlast == 1);

        REQUIRE(t.s_st0_axis_tready == 0);

        // Set master to ready
        t.m_st0_axis_tready = 1;

        rr::ut::clk(top);

        // IDLE /////////////////////////////

        // Last cycle and waiting for new command
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.m_st0_axis_tlast == 0);

        REQUIRE(t.s_st0_axis_tready == 0);

        rr::ut::clk(top);
    }

    // Final model cleanup
    top->final();

    // Destroy model
    delete top;
}

TEST_CASE("Stream data interrupted from slave (st0 -> st0)", "[Stream]")
{
    VDmaStreamEngine* top = new VDmaStreamEngine();
    VDmaStreamEngine& t = *top;

    reset(t);
    static constexpr uint32_t OP = 0x5000'0000;
    static constexpr uint32_t SIZE = 0x0000'0008;
    static constexpr uint32_t COMMAND = OP | SIZE; // Stream 8 bytes from one stream interface to another

    // Run this test two times, so that we know that we can start a new transfer after the old one
    for (uint32_t i = 0; i < 2; i++)
    {
        // COMMAND ///////////////////////////
        // Master port will always be ready
        t.m_st0_axis_tready = 1;

        configPhase(t, COMMAND, 0);

        // STREAM ////////////////////////////
        // Outputs
        // No valid data because slave has no data
        REQUIRE(t.m_st0_axis_tvalid == 0);

        // Telling the slave we can receive data
        REQUIRE(t.s_st0_axis_tready == 1);

        // Inputs
        // Set first 4 bytes of data
        t.s_st0_axis_tdata = 0xaa00ff55;

        // Set slave port to not ready
        t.s_st0_axis_tvalid = 0;

        // Process first 4 bytes of data
        rr::ut::clk(top);

        // STREAM ////////////////////////////
        // Outputs
        // No valid data because slave has no data
        REQUIRE(t.m_st0_axis_tvalid == 0);

        // Telling the slave we can receive data
        REQUIRE(t.s_st0_axis_tready == 1);

        // Set the next 4 bytes of data
        t.s_st0_axis_tdata = 0xaa00ff55;

        // Set slave port to ready
        t.s_st0_axis_tvalid = 1;

        rr::ut::clk(top);

        // STREAM ////////////////////////////
        // Outputs
        // Read first 4 bytes from master interface
        REQUIRE(t.m_st0_axis_tvalid == 1);
        REQUIRE(t.m_st0_axis_tdata == 0xaa00ff55);
        REQUIRE(t.m_st0_axis_tlast == 0);

        // Telling the slave we can receive data
        REQUIRE(t.s_st0_axis_tready == 1);

        // Set slave port to not ready
        t.s_st0_axis_tvalid = 0;

        rr::ut::clk(top);

        // STREAM ////////////////////////////
        // Outputs
        // No valid data because slave has no data
        REQUIRE(t.m_st0_axis_tvalid == 0);

        // Telling the slave we can receive data
        REQUIRE(t.s_st0_axis_tready == 1);

        // Set the next 4 bytes of data
        t.s_st0_axis_tdata = 0xff00ff00;

        // Set slave port to ready
        t.s_st0_axis_tvalid = 1;

        rr::ut::clk(top);

        // STREAM ////////////////////////////
        // Read first 4 bytes from master interface
        REQUIRE(t.m_st0_axis_tvalid == 1);
        REQUIRE(t.m_st0_axis_tdata == 0xff00ff00);
        REQUIRE(t.m_st0_axis_tlast == 1);

        REQUIRE(t.s_st0_axis_tready == 0);

        rr::ut::clk(top);

        // IDLE /////////////////////////////

        // Last cycle and waiting for new command
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.m_st0_axis_tlast == 0);

        REQUIRE(t.s_st0_axis_tready == 0);

        rr::ut::clk(top);
    }

    // Final model cleanup
    top->final();

    // Destroy model
    delete top;
}

TEST_CASE("Store chunk of data simple (st0 -> mem)", "[Memory]")
{
    VDmaStreamEngine* top = new VDmaStreamEngine();
    VDmaStreamEngine& t = *top;

    reset(t);
    static constexpr uint32_t OP = 0xd000'0000;
    static constexpr uint32_t SIZE = 0x0000'0100;
    static constexpr uint32_t COMMAND = OP | SIZE; // Stream 256 bytes from one stream interface to another
    static constexpr uint32_t BEATS_PER_TRANSFER = 128 / 4;

    // Tell the command parser that we are ready to receive data
    t.m_st0_axis_tready = 1;
    t.m_mem_axi_awready = 1;

    t.m_mem_axi_wready = 0;

    // Run this test two times, so that we know that we can start a new transfer after the old one
    for (uint32_t i = 0; i < 2; i++)
    {
        // COMMAND ///////////////////////////
        configPhase(t, COMMAND, 0x8000'0000);

        // First test the address channel (data and address are independent)
        // STORE ADDR ////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 1);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        t.s_st0_axis_tvalid = 0;

        rr::ut::clk(top);

        // STORE ADDR ////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 1);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 1);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_mem_axi_awaddr == 0x8000'0000);

        rr::ut::clk(top);

        // STORE ADDR ////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 1);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 1);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_mem_axi_awaddr == 0x8000'0080);

        rr::ut::clk(top);

        // STORE ADDR ////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 1);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        rr::ut::clk(top);

        // Now check the data channel
        // STORE DATA ////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 1);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        t.m_mem_axi_wready = 1;

        t.s_st0_axis_tdata = 0;
        t.s_st0_axis_tvalid = 1;

        rr::ut::clk(top);

        // STORE DATA ////////////////////////
        // Output contains the data from all elemets except the last
        for (uint32_t i = 1; i < BEATS_PER_TRANSFER; i++)
        {
            REQUIRE(t.m_mem_axi_awvalid == 0);
            REQUIRE(t.m_mem_axi_wvalid == 1);
            REQUIRE(t.m_mem_axi_arvalid == 0);
            REQUIRE(t.m_mem_axi_rready == 0);
            REQUIRE(t.m_st0_axis_tvalid == 0);
            REQUIRE(t.s_st0_axis_tready == 1);
            REQUIRE(t.m_st1_axis_tvalid == 0);
            REQUIRE(t.s_st1_axis_tready == 0);

            REQUIRE(t.m_mem_axi_wdata == i - 1);
            REQUIRE(t.m_mem_axi_wlast == 0);

            t.s_st0_axis_tdata = i;

            rr::ut::clk(top);
        }

        // Second transfer
        // STORE DATA ////////////////////////
        // Check the last element and initialize the next transfer
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 1);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 1);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_mem_axi_wdata == 31);
        REQUIRE(t.m_mem_axi_wlast == 1);

        t.s_st0_axis_tdata = 0;

        rr::ut::clk(top);

        // STORE DATA ////////////////////////
        // Output contains the data from all elemets except the last
        for (uint32_t i = 1; i < BEATS_PER_TRANSFER; i++)
        {
            REQUIRE(t.m_mem_axi_awvalid == 0);
            REQUIRE(t.m_mem_axi_wvalid == 1);
            REQUIRE(t.m_mem_axi_arvalid == 0);
            REQUIRE(t.m_mem_axi_rready == 0);
            REQUIRE(t.m_st0_axis_tvalid == 0);
            REQUIRE(t.s_st0_axis_tready == 1);
            REQUIRE(t.m_st1_axis_tvalid == 0);
            REQUIRE(t.s_st1_axis_tready == 0);

            REQUIRE(t.m_mem_axi_wdata == i - 1);
            REQUIRE(t.m_mem_axi_wlast == 0);

            t.s_st0_axis_tdata = i;

            rr::ut::clk(top);
        }

        // Second transfer
        // STORE DATA ////////////////////////
        // Check the last element
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 1);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_mem_axi_wdata == 31);
        REQUIRE(t.m_mem_axi_wlast == 1);

        rr::ut::clk(top);

        // IDLE /////////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        rr::ut::clk(top);
    }

    // Final model cleanup
    top->final();

    // Destroy model
    delete top;
}

TEST_CASE("Store chunk of data simple (st1 -> mem)", "[Memory]")
{
    VDmaStreamEngine* top = new VDmaStreamEngine();
    VDmaStreamEngine& t = *top;

    reset(t);
    static constexpr uint32_t OP = 0xe000'0000;
    static constexpr uint32_t SIZE = 0x0000'0100;
    static constexpr uint32_t COMMAND = OP | SIZE; // Stream 256 bytes from one stream interface to another
    static constexpr uint32_t BEATS_PER_TRANSFER = 128 / 4;

    // Tell the command parser that we are ready to receive data
    t.m_st0_axis_tready = 1;
    t.m_mem_axi_awready = 1;

    t.m_mem_axi_wready = 0;

    // Run this test two times, so that we know that we can start a new transfer after the old one
    for (uint32_t i = 0; i < 2; i++)
    {
        // COMMAND ///////////////////////////
        configPhase(t, COMMAND, 0x100);

        // First test the address channel (data and address are independent)
        // STORE ADDR ////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 1);

        t.s_st1_axis_tvalid = 0;

        rr::ut::clk(top);

        // STORE ADDR ////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 1);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 1);

        REQUIRE(t.m_mem_axi_awaddr == 0x100);

        rr::ut::clk(top);

        // STORE ADDR ////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 1);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 1);

        REQUIRE(t.m_mem_axi_awaddr == 0x180);

        rr::ut::clk(top);

        // STORE ADDR ////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 1);

        rr::ut::clk(top);

        // Now check the data channel
        // STORE DATA ////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 1);

        t.m_mem_axi_wready = 1;

        t.s_st1_axis_tdata = 0;
        t.s_st1_axis_tvalid = 1;

        rr::ut::clk(top);

        // STORE DATA ////////////////////////
        // Output contains the data from all elemets except the last
        for (uint32_t i = 1; i < BEATS_PER_TRANSFER; i++)
        {
            REQUIRE(t.m_mem_axi_awvalid == 0);
            REQUIRE(t.m_mem_axi_wvalid == 1);
            REQUIRE(t.m_mem_axi_arvalid == 0);
            REQUIRE(t.m_mem_axi_rready == 0);
            REQUIRE(t.m_st0_axis_tvalid == 0);
            REQUIRE(t.s_st0_axis_tready == 0);
            REQUIRE(t.m_st1_axis_tvalid == 0);
            REQUIRE(t.s_st1_axis_tready == 1);

            REQUIRE(t.m_mem_axi_wdata == i - 1);
            REQUIRE(t.m_mem_axi_wlast == 0);

            t.s_st1_axis_tdata = i;

            rr::ut::clk(top);
        }

        // Second transfer
        // STORE DATA ////////////////////////
        // Check the last element and initialize the next transfer
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 1);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 1);

        REQUIRE(t.m_mem_axi_wdata == 31);
        REQUIRE(t.m_mem_axi_wlast == 1);

        t.s_st1_axis_tdata = 0;

        rr::ut::clk(top);

        // STORE DATA ////////////////////////
        // Output contains the data from all elemets except the last
        for (uint32_t i = 1; i < BEATS_PER_TRANSFER; i++)
        {
            REQUIRE(t.m_mem_axi_awvalid == 0);
            REQUIRE(t.m_mem_axi_wvalid == 1);
            REQUIRE(t.m_mem_axi_arvalid == 0);
            REQUIRE(t.m_mem_axi_rready == 0);
            REQUIRE(t.m_st0_axis_tvalid == 0);
            REQUIRE(t.s_st0_axis_tready == 0);
            REQUIRE(t.m_st1_axis_tvalid == 0);
            REQUIRE(t.s_st1_axis_tready == 1);

            REQUIRE(t.m_mem_axi_wdata == i - 1);
            REQUIRE(t.m_mem_axi_wlast == 0);

            t.s_st1_axis_tdata = i;

            rr::ut::clk(top);
        }

        // Second transfer
        // STORE DATA ////////////////////////
        // Check the last element
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 1);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_mem_axi_wdata == 31);
        REQUIRE(t.m_mem_axi_wlast == 1);

        rr::ut::clk(top);

        // IDLE /////////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        rr::ut::clk(top);
    }

    // Final model cleanup
    top->final();

    // Destroy model
    delete top;
}

TEST_CASE("Load chunk data simple (mem -> st0)", "[Memory]")
{
    VDmaStreamEngine* top = new VDmaStreamEngine();
    VDmaStreamEngine& t = *top;

    reset(t);
    static constexpr uint32_t OP = 0x7000'0000;
    static constexpr uint32_t SIZE = 0x0000'0100;
    static constexpr uint32_t COMMAND = OP | SIZE; // Stream 256 bytes from one stream interface to another
    static constexpr uint32_t BEATS_PER_TRANSFER = 128 / 4;

    // Tell the command parser that we are ready to receive data
    t.m_st0_axis_tready = 1;
    t.m_mem_axi_arready = 1;

    t.m_mem_axi_rready = 0;

    // Run this test two times, so that we know that we can start a new transfer after the old one
    for (uint32_t i = 0; i < 1; i++)
    {
        // COMMAND ///////////////////////////
        configPhase(t, COMMAND, 0x8000'0000);

        // First test the address channel (data and address are independent)
        // LOAD ADDR /////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 1);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        t.m_st0_axis_tready = 0;

        rr::ut::clk(top);

        // LOAD ADDR /////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 1);
        REQUIRE(t.m_mem_axi_rready == 1);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_mem_axi_araddr == 0x8000'0000);

        rr::ut::clk(top);

        // LOAD ADDR /////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 1);
        REQUIRE(t.m_mem_axi_rready == 1);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_mem_axi_araddr == 0x8000'0080);

        rr::ut::clk(top);

        // LOAD ADDR /////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 1);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        rr::ut::clk(top);

        // Now check the data channel
        // LOAD DATA /////////////////////////
        // Initial value
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 1);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        t.m_st0_axis_tready = 1;

        t.m_mem_axi_rvalid = 1;
        t.m_mem_axi_rdata = 0;

        rr::ut::clk(top);

        // LOAD DATA /////////////////////////
        // Output contains the data from all elemets except the last
        for (uint32_t i = 1; i < BEATS_PER_TRANSFER * 2; i++)
        {
            REQUIRE(t.m_mem_axi_awvalid == 0);
            REQUIRE(t.m_mem_axi_wvalid == 0);
            REQUIRE(t.m_mem_axi_arvalid == 0);
            REQUIRE(t.m_mem_axi_rready == 1);
            REQUIRE(t.m_st0_axis_tvalid == 1);
            REQUIRE(t.s_st0_axis_tready == 0);
            REQUIRE(t.m_st1_axis_tvalid == 0);
            REQUIRE(t.s_st1_axis_tready == 0);

            REQUIRE(t.m_st0_axis_tdata == i - 1);
            REQUIRE(t.m_st0_axis_tlast == 0);

            t.m_mem_axi_rdata = i;

            rr::ut::clk(top);
        }

        // LOAD DATA /////////////////////////
        // Check the last element
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 1);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_st0_axis_tdata == 63);
        REQUIRE(t.m_st0_axis_tlast == 1);

        rr::ut::clk(top);

        // IDLE /////////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        rr::ut::clk(top);
    }

    // Final model cleanup
    top->final();

    // Destroy model
    delete top;
}

TEST_CASE("Load chunk data simple (mem -> st1)", "[Memory]")
{
    VDmaStreamEngine* top = new VDmaStreamEngine();
    VDmaStreamEngine& t = *top;

    reset(t);
    static constexpr uint32_t OP = 0xb000'0000;
    static constexpr uint32_t SIZE = 0x0000'0100;
    static constexpr uint32_t COMMAND = OP | SIZE; // Stream 256 bytes from one stream interface to another
    static constexpr uint32_t BEATS_PER_TRANSFER = 128 / 4;

    // Tell the command parser that we are ready to receive data
    t.m_st0_axis_tready = 1;
    t.m_mem_axi_arready = 1;

    t.m_mem_axi_rready = 0;

    // Run this test two times, so that we know that we can start a new transfer after the old one
    for (uint32_t i = 0; i < 1; i++)
    {
        // COMMAND ///////////////////////////
        configPhase(t, COMMAND, 0x8000'0000);

        // First test the address channel (data and address are independent)
        // LOAD ADDR /////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 1);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        t.m_st1_axis_tready = 0;

        rr::ut::clk(top);

        // LOAD ADDR /////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 1);
        REQUIRE(t.m_mem_axi_rready == 1);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_mem_axi_araddr == 0x8000'0000);

        rr::ut::clk(top);

        // LOAD ADDR /////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 1);
        REQUIRE(t.m_mem_axi_rready == 1);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_mem_axi_araddr == 0x8000'0080);

        rr::ut::clk(top);

        // LOAD ADDR /////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 1);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        rr::ut::clk(top);

        // Now check the data channel
        // LOAD DATA /////////////////////////
        // Initial value
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 1);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        t.m_st1_axis_tready = 1;

        t.m_mem_axi_rvalid = 1;
        t.m_mem_axi_rdata = 0;

        rr::ut::clk(top);

        // LOAD DATA /////////////////////////
        // Output contains the data from all elemets except the last
        for (uint32_t i = 1; i < BEATS_PER_TRANSFER * 2; i++)
        {
            REQUIRE(t.m_mem_axi_awvalid == 0);
            REQUIRE(t.m_mem_axi_wvalid == 0);
            REQUIRE(t.m_mem_axi_arvalid == 0);
            REQUIRE(t.m_mem_axi_rready == 1);
            REQUIRE(t.m_st0_axis_tvalid == 0);
            REQUIRE(t.s_st0_axis_tready == 0);
            REQUIRE(t.m_st1_axis_tvalid == 1);
            REQUIRE(t.s_st1_axis_tready == 0);

            REQUIRE(t.m_st1_axis_tdata == i - 1);
            REQUIRE(t.m_st1_axis_tlast == 0);

            t.m_mem_axi_rdata = i;

            rr::ut::clk(top);
        }

        // LOAD DATA /////////////////////////
        // Check the last element
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 1);
        REQUIRE(t.s_st1_axis_tready == 0);

        REQUIRE(t.m_st1_axis_tdata == 63);
        REQUIRE(t.m_st1_axis_tlast == 1);

        rr::ut::clk(top);

        // IDLE /////////////////////////////
        REQUIRE(t.m_mem_axi_awvalid == 0);
        REQUIRE(t.m_mem_axi_wvalid == 0);
        REQUIRE(t.m_mem_axi_arvalid == 0);
        REQUIRE(t.m_mem_axi_rready == 0);
        REQUIRE(t.m_st0_axis_tvalid == 0);
        REQUIRE(t.s_st0_axis_tready == 0);
        REQUIRE(t.m_st1_axis_tvalid == 0);
        REQUIRE(t.s_st1_axis_tready == 0);

        rr::ut::clk(top);
    }

    // Final model cleanup
    top->final();

    // Destroy model
    delete top;
}

TEST_CASE("Stream chunk of data int simple (st0 -> int, int -> st0)", "[Stream]")
{
    VDmaStreamEngine* top = new VDmaStreamEngine();
    VDmaStreamEngine& t = *top;

    reset(t);
    static constexpr uint32_t SIZE_ST0_TO_INT = 0x0000'0004;
    static constexpr uint32_t OP_ST0_TO_INT = 0x1000'0000;
    static constexpr uint32_t COMMAND_ST0_TO_INT = OP_ST0_TO_INT | SIZE_ST0_TO_INT; // Stream 8 bytes from one stream interface to another

    static constexpr uint32_t SIZE_INT_TO_ST0 = 0x0000'0100;
    static constexpr uint32_t OP_INT_TO_ST0 = 0x4000'0000;
    static constexpr uint32_t COMMAND_INT_TO_ST0 = OP_INT_TO_ST0 | SIZE_INT_TO_ST0; // Stream 8 bytes from one stream interface to another

    // Tell the command parser that we are ready to receive data
    t.m_st0_axis_tready = 1;

    // Run this test two times, so that we know that we can start a new transfer after the old one
    for (uint32_t i = 0; i < 2; i++)
    {
        // COMMAND ///////////////////////////
        configPhase(t, COMMAND_ST0_TO_INT, 0);

        // STREAM ////////////////////////////
        // Outputs
        REQUIRE(t.s_st0_axis_tready == 1);

        // Inputs
        // Set 4 bytes of data
        t.s_st0_axis_tlast = 1;
        t.s_st0_axis_tvalid = 1;
        t.s_st0_axis_tdata = 0xaa00ff55;

        // Process 4 bytes of data
        rr::ut::clk(top);

        // STREAM ////////////////////////////
        t.s_st0_axis_tlast = 0;
        t.s_st0_axis_tvalid = 0;
        REQUIRE(t.s_st0_axis_tready == 0);

        rr::ut::clk(top);

        // IDLE /////////////////////////////
        rr::ut::clk(top);

        // COMMAND ///////////////////////////
        configPhase(t, COMMAND_INT_TO_ST0, 0);

        // STREAM ////////////////////////////
        // Outputs
        REQUIRE(t.m_st0_axis_tvalid == 0);

        // Inputs
        // Set 4 bytes of data
        t.m_st0_axis_tready = 1;

        // Process 4 bytes of data
        rr::ut::clk(top);

        for (uint32_t i = 0; i < 0x100 / 4; i++)
        {
            // STREAM ////////////////////////////
            REQUIRE(t.m_st0_axis_tvalid == 1);
            REQUIRE(t.m_st0_axis_tlast == (i + 1 == (0x100 / 4)));
            REQUIRE(t.m_st0_axis_tdata == 0xaa00ff55);

            t.m_st0_axis_tready = 1;

            rr::ut::clk(top);
        }

        // IDLE /////////////////////////////
        rr::ut::clk(top);
    }

    // Final model cleanup
    top->final();

    // Destroy model
    delete top;
}