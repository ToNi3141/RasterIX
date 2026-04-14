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

// Reads a contiguous memory region via a 16-bit AXI read interface and
// streams each 16-bit word out on an AXI-Stream master port.
//
// A synchronous FIFO decouples the memory interface from the stream
// consumer. Bursts are issued one at a time: a new burst is only
// started once the FIFO fill level drops below FIFO_THRESHOLD, ensuring
// the FIFO never overflows. Non-power-of-2 transfer sizes are handled by
// capping the last burst to the remaining beat count.
//
// Usage:
//   1. Assert cmd_start for one cycle while providing cmd_addr and cmd_size.
//   2. The module streams cmd_size 16-bit words to m_axis, asserting
//      m_axis_tlast on the final word.
//   3. cmd_done is asserted once all words have been delivered to the
//      stream consumer.
module AxisMemoryReader #(
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of ID signal
    parameter ID_WIDTH = 8,
    // log2 of the FIFO depth (FIFO has 2^FIFO_DEPTH_LG entries of 16 bits)
    parameter FIFO_DEPTH_LG = 5,
    // Number of beats per AXI burst. Must satisfy:
    //   BEATS_PER_BURST <= 2^FIFO_DEPTH_LG
    parameter BEATS_PER_BURST = 16,

    localparam DATA_WIDTH = 16,
    localparam FIFO_SIZE = (1 << FIFO_DEPTH_LG),
    // Issue next burst when fill drops to or below this level
    localparam FIFO_THRESHOLD = FIFO_SIZE - BEATS_PER_BURST,
    localparam FB_SIZE_IN_PIXEL_LG = 20
) (
    input  wire                                     aclk,
    input  wire                                     resetn,

    // Command interface
    input  wire                                     cmd_start,     // Pulse high for one cycle to start
    input  wire [ADDR_WIDTH - 1 : 0]                cmd_addr,      // Base address
    input  wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]       cmd_size,      // Transfer size in 16-bit words
    output reg                                      cmd_done,      // High when all words delivered to stream

    // AXI-Stream master
    output wire                                     m_axis_tvalid,
    input  wire                                     m_axis_tready,
    output wire                                     m_axis_tlast,
    output wire [DATA_WIDTH - 1 : 0]                m_axis_tdata,

    // AXI4 read address channel
    output wire [ID_WIDTH - 1 : 0]                  m_mem_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]                m_mem_axi_araddr,
    output wire [ 7 : 0]                            m_mem_axi_arlen,
    output wire [ 2 : 0]                            m_mem_axi_arsize,
    output wire [ 1 : 0]                            m_mem_axi_arburst,
    output wire                                     m_mem_axi_arlock,
    output wire [ 3 : 0]                            m_mem_axi_arcache,
    output wire [ 2 : 0]                            m_mem_axi_arprot,
    output reg                                      m_mem_axi_arvalid,
    input  wire                                     m_mem_axi_arready,

    // AXI4 read data channel
    input  wire [ID_WIDTH - 1 : 0]                  m_mem_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]                m_mem_axi_rdata,
    input  wire [ 1 : 0]                            m_mem_axi_rresp,
    input  wire                                     m_mem_axi_rlast,
    input  wire                                     m_mem_axi_rvalid,
    output wire                                     m_mem_axi_rready
);
    // -----------------------------------------------------------------------
    // FIFO
    // -----------------------------------------------------------------------
    wire                        fifo_wr;
    wire [DATA_WIDTH - 1 : 0]   fifo_wdata;
    wire                        fifo_full;
    wire [FIFO_DEPTH_LG : 0]    fifo_fill;
    wire                        fifo_rd;
    wire [DATA_WIDTH - 1 : 0]   fifo_rdata;
    wire                        fifo_empty;

    sfifo #(
        .BW(DATA_WIDTH),
        .LGFLEN(FIFO_DEPTH_LG),
        .OPT_ASYNC_READ(1'b1),
        .OPT_WRITE_ON_FULL(1'b0),
        .OPT_READ_ON_EMPTY(1'b0)
    ) data_fifo (
        .i_clk(aclk),
        .i_reset(!resetn),

        .i_wr(fifo_wr),
        .i_data(fifo_wdata),
        .o_full(fifo_full),
        .o_fill(fifo_fill),

        .i_rd(fifo_rd),
        .o_data(fifo_rdata),
        .o_empty(fifo_empty)
    );

    // Write AXI read data directly into the FIFO.
    // Back-pressure: stop accepting memory beats when FIFO is full.
    assign fifo_wr          = m_mem_axi_rvalid & ~fifo_full;
    assign fifo_wdata       = m_mem_axi_rdata;
    assign m_mem_axi_rready = ~fifo_full;

    // Forward FIFO output to AXI-Stream consumer.
    // Data is only offered during an active transfer (state != IDLE).
    assign fifo_rd       = m_axis_tready & ~fifo_empty & (state != STATE_IDLE);
    assign m_axis_tvalid = ~fifo_empty & (state != STATE_IDLE);
    assign m_axis_tdata  = fifo_rdata;
    // tlast is high on the beat where beats_to_output reaches 1 (the last word).
    assign m_axis_tlast  = (beats_to_output == { {(FB_SIZE_IN_PIXEL_LG - 1){1'b0}}, 1'b1 })
                           & ~fifo_empty
                           & (state != STATE_IDLE);

    // -----------------------------------------------------------------------
    // AXI AR channel – driven from registered signals
    // -----------------------------------------------------------------------
    reg [ADDR_WIDTH - 1 : 0]  ar_addr_r;
    reg [ 7 : 0]               ar_len_r;

    assign m_mem_axi_arid    = { ID_WIDTH { 1'b0 } };
    assign m_mem_axi_araddr  = ar_addr_r;
    assign m_mem_axi_arlen   = ar_len_r;
    assign m_mem_axi_arsize  = 3'b001;    // 2^1 = 2 bytes = 16-bit per beat
    assign m_mem_axi_arburst = 2'b01;     // INCR
    assign m_mem_axi_arlock  = 1'b0;
    assign m_mem_axi_arcache = 4'b0011;
    assign m_mem_axi_arprot  = 3'b000;

    // -----------------------------------------------------------------------
    // FSM
    // -----------------------------------------------------------------------
    localparam STATE_IDLE     = 3'd0;
    localparam STATE_CHECK    = 3'd1;  // decide whether to issue next burst
    localparam STATE_BURST_AR = 3'd2;  // wait for AR channel handshake
    localparam STATE_WAIT_R   = 3'd3;  // count incoming R-channel beats
    localparam STATE_DRAIN    = 3'd4;  // all AR requests issued, drain FIFO

    reg [ 2 : 0]                        state;
    // Beats whose AR request has not yet been issued
    reg [FB_SIZE_IN_PIXEL_LG - 1 : 0]   beats_remaining;
    // Beats not yet consumed by the AXI-Stream consumer
    reg [FB_SIZE_IN_PIXEL_LG - 1 : 0]   beats_to_output;
    // Remaining beats of the current in-flight burst
    reg [ 7 : 0]                         burst_len_r;

    wire mem_beat = m_mem_axi_rvalid & m_mem_axi_rready;

    initial
    begin
        cmd_done          = 1;
        m_mem_axi_arvalid = 0;
    end

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            state             <= STATE_IDLE;
            cmd_done          <= 1;
            m_mem_axi_arvalid <= 0;
            ar_addr_r         <= 0;
            ar_len_r          <= 0;
            beats_remaining   <= 0;
            beats_to_output   <= 0;
            burst_len_r       <= 0;
        end
        else
        begin
            case (state)
                // -----------------------------------------------------------------
                STATE_IDLE:
                begin
                    if (cmd_start)
                    begin
                        cmd_done        <= 0;
                        beats_remaining <= cmd_size;
                        beats_to_output <= cmd_size;
                        ar_addr_r       <= cmd_addr;
                        state           <= STATE_CHECK;
                    end
                end

                // -----------------------------------------------------------------
                // Check whether to launch the next burst or transition to DRAIN.
                // Also track stream-consumer reads in this state.
                STATE_CHECK:
                begin
                    if (fifo_rd)
                        beats_to_output <= beats_to_output - 1;

                    if (beats_remaining == 0)
                    begin
                        state <= STATE_DRAIN;
                    end
                    else if (fifo_fill <= FIFO_THRESHOLD[FIFO_DEPTH_LG : 0])
                    begin
                        // Cap burst length to remaining beats (handles non-aligned sizes)
                        if (beats_remaining > BEATS_PER_BURST)
                        begin
                            burst_len_r <= BEATS_PER_BURST[7 : 0];
                            ar_len_r    <= BEATS_PER_BURST[7 : 0] - 8'h1;
                        end
                        else
                        begin
                            burst_len_r <= beats_remaining[7 : 0];
                            ar_len_r    <= beats_remaining[7 : 0] - 8'h1;
                        end
                        m_mem_axi_arvalid <= 1;
                        state             <= STATE_BURST_AR;
                    end
                    // else: FIFO not below threshold yet – stay in CHECK
                end

                // -----------------------------------------------------------------
                // Wait for the AR-channel handshake.
                STATE_BURST_AR:
                begin
                    if (fifo_rd)
                        beats_to_output <= beats_to_output - 1;

                    if (m_mem_axi_arready && m_mem_axi_arvalid)
                    begin
                        m_mem_axi_arvalid <= 0;
                        beats_remaining   <= beats_remaining
                                            - { {(FB_SIZE_IN_PIXEL_LG - 8){1'b0}}, burst_len_r };
                        // Advance address: burst_len_r beats × 2 bytes each
                        ar_addr_r         <= ar_addr_r
                                            + ({ {(ADDR_WIDTH - 9){1'b0}}, burst_len_r, 1'b0 });
                        state             <= STATE_WAIT_R;
                    end
                end

                // -----------------------------------------------------------------
                // Count incoming R-channel beats. When the burst completes,
                // go back to CHECK to decide on the next burst.
                STATE_WAIT_R:
                begin
                    if (fifo_rd)
                        beats_to_output <= beats_to_output - 1;

                    if (mem_beat)
                    begin
                        burst_len_r <= burst_len_r - 8'h1;
                        if (burst_len_r == 8'h1)
                            state <= STATE_CHECK;
                    end
                end

                // -----------------------------------------------------------------
                // All AR requests issued. Drain remaining FIFO data to the consumer.
                STATE_DRAIN:
                begin
                    if (fifo_rd)
                    begin
                        beats_to_output <= beats_to_output - 1;
                        if (beats_to_output == { {(FB_SIZE_IN_PIXEL_LG - 1){1'b0}}, 1'b1 })
                        begin
                            cmd_done <= 1;
                            state    <= STATE_IDLE;
                        end
                    end
                    else if (beats_to_output == 0)
                    begin
                        // Reached zero without a final fifo_rd (e.g. consumer was fast)
                        cmd_done <= 1;
                        state    <= STATE_IDLE;
                    end
                end

                // -----------------------------------------------------------------
                default:
                begin
                    state <= STATE_IDLE;
                end
            endcase
        end
    end
endmodule
