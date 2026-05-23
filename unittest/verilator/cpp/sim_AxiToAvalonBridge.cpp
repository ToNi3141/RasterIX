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

#include "VAxiToAvalonBridge.h"

namespace
{

void initInputs(VAxiToAvalonBridge* t)
{
    t->s_axi_awid = 0;
    t->s_axi_awaddr = 0;
    t->s_axi_awlen = 0;
    t->s_axi_awsize = 1; // 16-bit
    t->s_axi_awburst = 1; // INCR
    t->s_axi_awvalid = 0;

    t->s_axi_wdata = 0;
    t->s_axi_wstrb = 0x3;
    t->s_axi_wlast = 0;
    t->s_axi_wvalid = 0;

    t->s_axi_bready = 0;

    t->s_axi_arid = 0;
    t->s_axi_araddr = 0;
    t->s_axi_arlen = 0;
    t->s_axi_arsize = 1; // 16-bit
    t->s_axi_arburst = 1; // INCR
    t->s_axi_arvalid = 0;

    t->s_axi_rready = 1;

    t->avm_readdata = 0;
    t->avm_readdatavalid = 0;
    t->avm_waitrequest = 0;
}

} // namespace

// ===================================================================
// Post-reset state (after rr::ut::reset()):
//   The reset() helper runs one live clock (resetn=1) with all inputs idle.
//   During that clock R_IDLE sees grant=READ and arvalid=0 → grant←WRITE.
//   Result: grant=WRITE, arready=0, awready=0.
//
//   Writes can therefore start immediately after reset.
//   Reads require one idle clock first so W_IDLE hands grant back to READ.
// ===================================================================

// Cycle-accurate sequence (no waitrequest):
//
//  After reset:   grant=WRITE, arready=0, awready=0
//  clk 1: W_IDLE, awvalid=1 & wvalid=1 → awready=1, wready=1, W_INIT
//  clk 2: W_INIT, wlast=1 → avm_write=1, bvalid=1 (bid=awid), grant→READ, W_RESP
//          awready deasserted; arvalid queued
//  clk 3: R_IDLE, arvalid=1 → arready=1, avm_read=1, R_REQUEST
//          bvalid still=1 (bready=0)
//  clk 4: bready=1 → W_RESP→W_IDLE, bvalid=0
//          R_REQUEST→R_COLLECT, arready=0

TEST_CASE("write hands over to read on last beat; bresp completes off-grant", "[VAxiToAvalonBridge]")
{
    VAxiToAvalonBridge* t = rr::ut::makeTop<VAxiToAvalonBridge>();
    initInputs(t);
    rr::ut::reset(t);

    // After reset: grant=WRITE, arready=0, awready=0
    CHECK(t->s_axi_arready == 0);
    CHECK(t->s_axi_awready == 0);

    // -- clk 1: W_IDLE with grant=WRITE; present both AW and first W beat --
    t->s_axi_awvalid = 1;
    t->s_axi_awid = 0x12;
    t->s_axi_awaddr = 0x1000;
    t->s_axi_awlen = 0;
    t->s_axi_wvalid = 1;
    t->s_axi_wdata = 0x55aa;
    t->s_axi_wstrb = 0x3;
    t->s_axi_wlast = 0; // wlast will be presented in W_INIT (clk 2)
    rr::ut::clk(t); // W_IDLE→W_INIT
    CHECK(t->s_axi_awready == 1);
    CHECK(t->s_axi_wready == 1);

    // -- clk 2: W_INIT latches data; wlast=1 → W_RESP, grant→READ --
    t->s_axi_awvalid = 0;
    t->s_axi_wlast = 1;
    t->s_axi_bready = 0; // keep B pending to test off-grant response
    t->s_axi_arvalid = 1;
    t->s_axi_arid = 0x34;
    t->s_axi_araddr = 0x2000;
    t->s_axi_arlen = 0;
    rr::ut::clk(t); // W_INIT→W_RESP, grant→READ
    CHECK(t->s_axi_bvalid == 1);
    CHECK(t->s_axi_bid == 0x12);
    CHECK(t->s_axi_awready == 0);

    // -- clk 3: R_IDLE sees grant still WRITE because B is pending; read not yet started --
    rr::ut::clk(t);
    CHECK(t->s_axi_arready == 0);
    CHECK(t->avm_read == 0);
    CHECK(t->s_axi_bvalid == 1); // B still pending (bready=0)

    // -- clk 4: complete B handshake; it will clear s_axi_bvalid this cycle,
    // but grant flips one cycle later due to non-blocking updates --
    t->s_axi_wvalid = 0;
    t->s_axi_wlast = 0;
    t->s_axi_bready = 1;
    rr::ut::clk(t);
    CHECK(t->s_axi_bvalid == 0);
    CHECK(t->s_axi_arready == 0);

    // -- clk 5-6: grant flips in one cycle, read accepted on the following cycle
    rr::ut::clk(t); // allow grant to update
    rr::ut::clk(t); // R_IDLE now accepts pending AR
    CHECK(t->s_axi_arready == 1);
    CHECK(t->avm_read == 1);
    CHECK(t->avm_address == (0x2000 >> 1));
    CHECK(t->avm_burstcount == 1);

    delete t;
}

// Two-beat write burst where the second beat is delayed by the Avalon waitrequest
TEST_CASE("two-beat write with avm wait on second beat", "[VAxiToAvalonBridge]")
{
    VAxiToAvalonBridge* t = rr::ut::makeTop<VAxiToAvalonBridge>();
    initInputs(t);
    rr::ut::reset(t);

    // Present AW (awlen=1 -> two beats) and first W beat in W_IDLE
    t->s_axi_awvalid = 1;
    t->s_axi_awid = 0x5;
    t->s_axi_awaddr = 0x3000;
    t->s_axi_awlen = 1; // two beats
    t->s_axi_wvalid = 1;
    t->s_axi_wdata = 0xAAAA;
    t->s_axi_wstrb = 0x3;
    t->s_axi_wlast = 0; // first beat
    t->s_axi_bready = 0; // don't accept response yet

    // Cycle 1: W_IDLE -> W_INIT
    rr::ut::clk(t);
    CHECK(t->s_axi_awready == 1);
    CHECK(t->s_axi_wready == 1);
    CHECK(t->avm_write == 0);

    // Cycle 2: W_INIT -> begin AVM transfer of beat 0
    rr::ut::clk(t);
    CHECK(t->avm_write == 1);
    CHECK(t->avm_writedata == 0xAAAA);
    CHECK(t->avm_byteenable == 0x3);
    CHECK(t->avm_address == (0x3000 >> 1));
    CHECK(t->avm_burstcount == 2);
    CHECK(t->s_axi_awready == 0);

    // Now present second beat on W_DATA but force Avalon to assert waitrequest
    t->s_axi_awvalid = 0;
    t->s_axi_wdata = 0xBBBB;
    t->s_axi_wlast = 1; // final beat
    t->avm_waitrequest = 1; // Avalon stalls after first beat
    // Cycle 3: attempt to transfer beat 1 but AVM stalls
    rr::ut::clk(t);
    // Exact pattern after clock 3
    CHECK(t->avm_write == 1); // ongoing AVM transfer
    CHECK(t->avm_writedata == 0xAAAA); // still the first beat on the bus
    CHECK(t->s_axi_wready == 0); // AXI wready deasserted due to skid

    // Cycle 4: deassert waitrequest so skid fires and beat 1 moves to AVM
    t->avm_waitrequest = 0;
    rr::ut::clk(t);
    // Exact pattern after clock 4
    CHECK(t->avm_write == 1);
    CHECK(t->avm_writedata == 0xBBBB);
    CHECK(t->s_axi_bvalid == 1);
    CHECK(t->s_axi_bid == 0x5);

    // Cycle 5: accept B and finish
    t->s_axi_bready = 1;
    rr::ut::clk(t);
    CHECK(t->s_axi_bvalid == 0);

    delete t;
}

// Cycle-accurate sequence (no waitrequest):
//
//  After reset:   grant=WRITE, arready=0
//  clk 1: W_IDLE, awvalid=0 → grant→READ  (idle clock to flip grant)
//  clk 2: R_IDLE, arvalid=1 → arready=1, avm_read=1, R_REQUEST
//  clk 3: R_REQUEST→R_COLLECT, readdatavalid=1 → data in FIFO (bypass)
//          arready=0, rvalid=1, rdata=0xabcd, rlast=1
//  clk 4: R_COLLECT exits (r_beat_cnt=1 >= 0); FIFO consumed → rvalid=0

TEST_CASE("word address conversion and single-beat read response", "[VAxiToAvalonBridge]")
{
    VAxiToAvalonBridge* t = rr::ut::makeTop<VAxiToAvalonBridge>();
    initInputs(t);
    rr::ut::reset(t);

    // After reset: grant=WRITE, arready=0
    CHECK(t->s_axi_arready == 0);
    CHECK(t->avm_read == 0);

    // -- clk 1: idle clock so W_IDLE hands grant back to READ --
    rr::ut::clk(t);

    // -- clk 2: R_IDLE accepts AR --
    t->s_axi_arvalid = 1;
    t->s_axi_arid = 0x3;
    t->s_axi_araddr = 0x0124;
    t->s_axi_arlen = 0;
    rr::ut::clk(t); // R_IDLE→R_REQUEST
    CHECK(t->avm_read == 1);
    CHECK(t->avm_address == (0x0124 >> 1)); // byte → word
    CHECK(t->avm_burstcount == 1);
    CHECK(t->s_axi_arready == 1);

    // -- clk 3: R_REQUEST→R_COLLECT, readdatavalid arrives same cycle --
    t->s_axi_arvalid = 0;
    t->avm_readdata = 0xabcd;
    t->avm_readdatavalid = 1;
    rr::ut::clk(t);
    CHECK(t->s_axi_arready == 0);
    CHECK(t->s_axi_rvalid == 1);
    CHECK(t->s_axi_rid == 0x3);
    CHECK(t->s_axi_rdata == 0xabcd);
    CHECK(t->s_axi_rlast == 1);
    CHECK(t->s_axi_rresp == 0);

    // -- clk 4: R_COLLECT exits; FIFO consumed → rvalid=0 --
    t->avm_readdatavalid = 0;
    rr::ut::clk(t);
    CHECK(t->s_axi_rvalid == 0);

    delete t;
}

// Multi-beat burst read test: verifies rlast is asserted only on the final beat
// and that all beats are transferred correctly with back-to-back readdatavalid.
//
// Burst of 4 beats (arlen=3).
// The sfifo (OPT_ASYNC_READ=0) uses a bypass register: data written to an
// empty FIFO appears at o_data on the very next clock (bypass_valid path).
// For subsequent beats with simultaneous read+write the bypass also fires,
// so each beat is visible exactly one cycle after readdatavalid.
TEST_CASE("multi-beat read burst with rlast on final beat", "[VAxiToAvalonBridge]")
{
    VAxiToAvalonBridge* t = rr::ut::makeTop<VAxiToAvalonBridge>();
    initInputs(t);
    rr::ut::reset(t);

    // After reset: grant=WRITE, arready=0
    CHECK(t->s_axi_arready == 0);

    // -- clk 1: idle clock so W_IDLE hands grant back to READ --
    rr::ut::clk(t);

    // -- clk 2: AR handshake, burst of 4 --
    t->s_axi_arvalid = 1;
    t->s_axi_arid = 0x7;
    t->s_axi_araddr = 0x100;
    t->s_axi_arlen = 3; // 4 beats
    rr::ut::clk(t); // R_IDLE→R_REQUEST
    CHECK(t->avm_read == 1);
    CHECK(t->avm_burstcount == 4);
    CHECK(t->s_axi_arready == 1);

    // -- clk 3: R_REQUEST→R_COLLECT (no waitrequest), no data yet --
    t->s_axi_arvalid = 0;
    t->avm_readdatavalid = 0;
    rr::ut::clk(t);
    CHECK(t->s_axi_arready == 0);
    CHECK(t->s_axi_rvalid == 0);

    // -- clk 4: beat 0; FIFO empty → bypass; visible after this clock --
    t->avm_readdatavalid = 1;
    t->avm_readdata = 0x1111;
    rr::ut::clk(t);
    CHECK(t->s_axi_rvalid == 1);
    CHECK(t->s_axi_rdata == 0x1111);
    CHECK(t->s_axi_rlast == 0);

    // -- clk 5: beat 1 --
    t->avm_readdata = 0x2222;
    rr::ut::clk(t);
    CHECK(t->s_axi_rvalid == 1);
    CHECK(t->s_axi_rdata == 0x2222);
    CHECK(t->s_axi_rlast == 0);

    // -- clk 6: beat 2 --
    t->avm_readdata = 0x3333;
    rr::ut::clk(t);
    CHECK(t->s_axi_rvalid == 1);
    CHECK(t->s_axi_rdata == 0x3333);
    CHECK(t->s_axi_rlast == 0);

    // -- clk 7: beat 3 (last); r_last=1 tagged in FIFO; R_COLLECT exits --
    t->avm_readdata = 0x4444;
    rr::ut::clk(t);
    CHECK(t->s_axi_rvalid == 1);
    CHECK(t->s_axi_rdata == 0x4444);
    CHECK(t->s_axi_rlast == 1);
    CHECK(t->s_axi_rid == 0x7);

    // -- clk 8: last beat consumed, FIFO drains → rvalid=0 --
    t->avm_readdatavalid = 0;
    rr::ut::clk(t);
    CHECK(t->s_axi_rvalid == 0);

    delete t;
}

// Test that readdatavalid arriving simultaneously with waitrequest deassertion
// (R_REQUEST→R_COLLECT transition) is not lost. This is the typical behavior
// of pipelined memories like HyperRAM.
TEST_CASE("readdatavalid on same cycle as R_REQUEST to R_COLLECT transition", "[VAxiToAvalonBridge]")
{
    VAxiToAvalonBridge* t = rr::ut::makeTop<VAxiToAvalonBridge>();
    initInputs(t);
    rr::ut::reset(t);

    // After reset: grant=WRITE, arready=0
    CHECK(t->s_axi_arready == 0);

    // -- clk 1: idle clock so W_IDLE hands grant back to READ --
    rr::ut::clk(t);

    // -- clk 2: AR handshake, burst of 4; waitrequest stalls --
    t->s_axi_arvalid = 1;
    t->s_axi_arid = 0x5;
    t->s_axi_araddr = 0x200;
    t->s_axi_arlen = 3; // 4 beats
    t->avm_waitrequest = 1;
    rr::ut::clk(t); // R_IDLE→R_REQUEST
    CHECK(t->avm_read == 1);
    CHECK(t->s_axi_arready == 1);

    // -- clk 3: waitrequest deasserts AND readdatavalid arrives same cycle --
    t->s_axi_arvalid = 0;
    t->avm_waitrequest = 0;
    t->avm_readdatavalid = 1;
    t->avm_readdata = 0xAAAA;
    rr::ut::clk(t); // R_REQUEST→R_COLLECT, beat 0 written to FIFO (bypass)
    CHECK(t->s_axi_arready == 0);
    CHECK(t->s_axi_rvalid == 1);
    CHECK(t->s_axi_rdata == 0xAAAA);
    CHECK(t->s_axi_rlast == 0);

    // -- clk 4: beat 1 --
    t->avm_readdata = 0xBBBB;
    rr::ut::clk(t);
    CHECK(t->s_axi_rvalid == 1);
    CHECK(t->s_axi_rdata == 0xBBBB);
    CHECK(t->s_axi_rlast == 0);

    // -- clk 5: beat 2 --
    t->avm_readdata = 0xCCCC;
    rr::ut::clk(t);
    CHECK(t->s_axi_rvalid == 1);
    CHECK(t->s_axi_rdata == 0xCCCC);
    CHECK(t->s_axi_rlast == 0);

    // -- clk 6: beat 3 (last) --
    t->avm_readdata = 0xDDDD;
    rr::ut::clk(t);
    CHECK(t->s_axi_rvalid == 1);
    CHECK(t->s_axi_rdata == 0xDDDD);
    CHECK(t->s_axi_rlast == 1);

    // -- clk 7: last beat consumed → rvalid=0 --
    t->avm_readdatavalid = 0;
    rr::ut::clk(t);
    CHECK(t->s_axi_rvalid == 0);

    delete t;
}
// Test that data is not lost when rready is deasserted while Avalon streams data.
// This simulates real AXI interconnect backpressure behavior.
TEST_CASE("no data loss when rready stalls during burst", "[VAxiToAvalonBridge]")
{
    VAxiToAvalonBridge* t = rr::ut::makeTop<VAxiToAvalonBridge>();
    initInputs(t);
    rr::ut::reset(t);

    // -- clk 1: idle clock so W_IDLE hands grant back to READ --
    rr::ut::clk(t);

    // -- clk 2: AR handshake, burst of 4 --
    t->s_axi_arvalid = 1;
    t->s_axi_arid = 0x9;
    t->s_axi_araddr = 0x400;
    t->s_axi_arlen = 3; // 4 beats
    rr::ut::clk(t); // R_IDLE→R_REQUEST
    CHECK(t->avm_read == 1);

    // -- clk 3: R_REQUEST→R_COLLECT, no data yet --
    t->s_axi_arvalid = 0;
    rr::ut::clk(t);

    // -- Deassert rready: AXI master is not ready to accept data --
    t->s_axi_rready = 0;

    // -- clk 4-7: Avalon streams all 4 beats while rready=0 --
    t->avm_readdatavalid = 1;
    t->avm_readdata = 0x1111;
    rr::ut::clk(t);
    t->avm_readdata = 0x2222;
    rr::ut::clk(t);
    t->avm_readdata = 0x3333;
    rr::ut::clk(t);
    t->avm_readdata = 0x4444;
    rr::ut::clk(t);
    t->avm_readdatavalid = 0;

    // All data buffered in FIFO; head (0x1111) visible via rd_data register
    CHECK(t->s_axi_rvalid == 1);
    CHECK(t->s_axi_rdata == 0x1111);
    CHECK(t->s_axi_rlast == 0);

    // -- Reassert rready and drain all 4 beats --
    t->s_axi_rready = 1;
    rr::ut::clk(t); // consume beat 0; rd_data advances to beat 1
    CHECK(t->s_axi_rvalid == 1);
    CHECK(t->s_axi_rdata == 0x2222);
    CHECK(t->s_axi_rlast == 0);

    rr::ut::clk(t); // consume beat 1; rd_data advances to beat 2
    CHECK(t->s_axi_rvalid == 1);
    CHECK(t->s_axi_rdata == 0x3333);
    CHECK(t->s_axi_rlast == 0);

    rr::ut::clk(t); // consume beat 2; rd_data advances to beat 3
    CHECK(t->s_axi_rvalid == 1);
    CHECK(t->s_axi_rdata == 0x4444);
    CHECK(t->s_axi_rlast == 1); // LAST!

    rr::ut::clk(t); // consume beat 3; FIFO empty
    CHECK(t->s_axi_rvalid == 0);

    delete t;
}