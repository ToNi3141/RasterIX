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

// This module acts as a framebuffer reader for the displays.
// It takes from fb_addr the address where to read the framebuffer.
// A read is started when swap_fb is set to high and is acknowledged
// by a transition from fb_swapped from 0 to 1.
// Note: An acknowledged framebuffer does not mean that it is fully
// transferred. It just means that this module starts to transfer
// it to the display (BLOCKING=0) or that the transfer is fully
// complete (BLOCKING=1).
//
// The display stream is always 16-bit (RGB565). The AXI memory port width
// is set by DATA_WIDTH. An internal axi_adapter_rd upsizes the 16-bit
// AxisMemoryReader read channel to DATA_WIDTH.
// An internal FIFO (depth 2^FIFO_DEPTH_LG) decouples the memory read path
// from the display stream. Bursts are issued one at a time; a new burst is
// only started after the FIFO fill drops below its threshold.
module AxisFramebufferReader #(
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of the AXI memory data bus (the internal reader is always 16-bit;
    // an adapter upsizes to this width for the external port)
    parameter DATA_WIDTH = 32,
    // Width of AXI ID signal
    parameter ID_WIDTH = 8,
    // When 1: fb_swapped is asserted only after the full transfer is complete.
    // When 0: fb_swapped is asserted immediately after the transfer starts.
    parameter BLOCKING = 1,
    // log2 of the internal FIFO depth
    parameter FIFO_DEPTH_LG = 8,
    // Beats per AXI read burst (must be <= 2^FIFO_DEPTH_LG)
    parameter BEATS_PER_BURST = 16,

    localparam INTERNAL_DATA_WIDTH = 16,
    localparam FB_SIZE_IN_PIXEL_LG = 20
) (
    input  wire                                 aclk,
    input  wire                                 resetn,

    input  wire                                 swap_fb,
    input  wire [ADDR_WIDTH - 1 : 0]            fb_addr,
    input  wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]   fb_size,
    output reg                                  fb_swapped,

    // Display AXI-Stream master (always 16-bit RGB565 pixels)
    output wire                                     m_disp_axis_tvalid,
    input  wire                                     m_disp_axis_tready,
    output wire                                     m_disp_axis_tlast,
    output wire [INTERNAL_DATA_WIDTH - 1 : 0]       m_disp_axis_tdata,

    // AXI4 read-only memory master (DATA_WIDTH wide)
    output wire [ID_WIDTH - 1 : 0]              m_mem_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]            m_mem_axi_araddr,
    output wire [ 7 : 0]                        m_mem_axi_arlen,
    output wire [ 2 : 0]                        m_mem_axi_arsize,
    output wire [ 1 : 0]                        m_mem_axi_arburst,
    output wire                                 m_mem_axi_arlock,
    output wire [ 3 : 0]                        m_mem_axi_arcache,
    output wire [ 2 : 0]                        m_mem_axi_arprot,
    output wire                                 m_mem_axi_arvalid,
    input  wire                                 m_mem_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]              m_mem_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]            m_mem_axi_rdata,
    input  wire [ 1 : 0]                        m_mem_axi_rresp,
    input  wire                                 m_mem_axi_rlast,
    input  wire                                 m_mem_axi_rvalid,
    output wire                                 m_mem_axi_rready
);
    wire cmd_done;
    reg  cmd_start;

    // Internal 16-bit AXI wires between AxisMemoryReader and axi_adapter_rd
    wire [ID_WIDTH - 1 : 0]                 mem16_axi_arid;
    wire [ADDR_WIDTH - 1 : 0]               mem16_axi_araddr;
    wire [ 7 : 0]                           mem16_axi_arlen;
    wire [ 2 : 0]                           mem16_axi_arsize;
    wire [ 1 : 0]                           mem16_axi_arburst;
    wire                                    mem16_axi_arlock;
    wire [ 3 : 0]                           mem16_axi_arcache;
    wire [ 2 : 0]                           mem16_axi_arprot;
    wire                                    mem16_axi_arvalid;
    wire                                    mem16_axi_arready;
    wire [ID_WIDTH - 1 : 0]                 mem16_axi_rid;
    wire [INTERNAL_DATA_WIDTH - 1 : 0]      mem16_axi_rdata;
    wire [ 1 : 0]                           mem16_axi_rresp;
    wire                                    mem16_axi_rlast;
    wire                                    mem16_axi_rvalid;
    wire                                    mem16_axi_rready;

    AxisMemoryReader #(
        .ADDR_WIDTH(ADDR_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .FIFO_DEPTH_LG(FIFO_DEPTH_LG),
        .BEATS_PER_BURST(BEATS_PER_BURST)
    ) memReader (
        .aclk(aclk),
        .resetn(resetn),

        .cmd_start(cmd_start),
        .cmd_addr(fb_addr),
        .cmd_size(fb_size),
        .cmd_done(cmd_done),

        .m_axis_tvalid(m_disp_axis_tvalid),
        .m_axis_tready(m_disp_axis_tready),
        .m_axis_tlast(m_disp_axis_tlast),
        .m_axis_tdata(m_disp_axis_tdata),

        .m_mem_axi_arid(mem16_axi_arid),
        .m_mem_axi_araddr(mem16_axi_araddr),
        .m_mem_axi_arlen(mem16_axi_arlen),
        .m_mem_axi_arsize(mem16_axi_arsize),
        .m_mem_axi_arburst(mem16_axi_arburst),
        .m_mem_axi_arlock(mem16_axi_arlock),
        .m_mem_axi_arcache(mem16_axi_arcache),
        .m_mem_axi_arprot(mem16_axi_arprot),
        .m_mem_axi_arvalid(mem16_axi_arvalid),
        .m_mem_axi_arready(mem16_axi_arready),

        .m_mem_axi_rid(mem16_axi_rid),
        .m_mem_axi_rdata(mem16_axi_rdata),
        .m_mem_axi_rresp(mem16_axi_rresp),
        .m_mem_axi_rlast(mem16_axi_rlast),
        .m_mem_axi_rvalid(mem16_axi_rvalid),
        .m_mem_axi_rready(mem16_axi_rready)
    );

    // Upsize the 16-bit AxisMemoryReader AXI port to DATA_WIDTH
    axi_adapter_rd #(
        .ADDR_WIDTH(ADDR_WIDTH),
        .S_DATA_WIDTH(INTERNAL_DATA_WIDTH),
        .M_DATA_WIDTH(DATA_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .CONVERT_BURST(1),
        .CONVERT_NARROW_BURST(0)
    ) memAxiAdapter (
        .clk(aclk),
        .rst(!resetn),

        .s_axi_arid(mem16_axi_arid),
        .s_axi_araddr(mem16_axi_araddr),
        .s_axi_arlen(mem16_axi_arlen),
        .s_axi_arsize(mem16_axi_arsize),
        .s_axi_arburst(mem16_axi_arburst),
        .s_axi_arlock(mem16_axi_arlock),
        .s_axi_arcache(mem16_axi_arcache),
        .s_axi_arprot(mem16_axi_arprot),
        .s_axi_arqos(0),
        .s_axi_arregion(0),
        .s_axi_aruser(0),
        .s_axi_arvalid(mem16_axi_arvalid),
        .s_axi_arready(mem16_axi_arready),

        .s_axi_rid(mem16_axi_rid),
        .s_axi_rdata(mem16_axi_rdata),
        .s_axi_rresp(mem16_axi_rresp),
        .s_axi_rlast(mem16_axi_rlast),
        .s_axi_ruser(),
        .s_axi_rvalid(mem16_axi_rvalid),
        .s_axi_rready(mem16_axi_rready),

        .m_axi_arid(m_mem_axi_arid),
        .m_axi_araddr(m_mem_axi_araddr),
        .m_axi_arlen(m_mem_axi_arlen),
        .m_axi_arsize(m_mem_axi_arsize),
        .m_axi_arburst(m_mem_axi_arburst),
        .m_axi_arlock(m_mem_axi_arlock),
        .m_axi_arcache(m_mem_axi_arcache),
        .m_axi_arprot(m_mem_axi_arprot),
        .m_axi_arqos(),
        .m_axi_arregion(),
        .m_axi_aruser(),
        .m_axi_arvalid(m_mem_axi_arvalid),
        .m_axi_arready(m_mem_axi_arready),

        .m_axi_rid(m_mem_axi_rid),
        .m_axi_rdata(m_mem_axi_rdata),
        .m_axi_rresp(m_mem_axi_rresp),
        .m_axi_rlast(m_mem_axi_rlast),
        .m_axi_ruser(0),
        .m_axi_rvalid(m_mem_axi_rvalid),
        .m_axi_rready(m_mem_axi_rready)
    );

    // -----------------------------------------------------------------------
    // Swap-buffer handshake FSM
    // -----------------------------------------------------------------------
    localparam STATE_IDLE   = 1'b0;
    localparam STATE_ACTIVE = 1'b1;
    reg state;

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            state      <= STATE_IDLE;
            fb_swapped <= 1;
            cmd_start  <= 0;
        end
        else
        begin
            cmd_start <= 0; // default: one-shot pulse

            case (state)
                STATE_IDLE:
                begin
                    if (swap_fb)
                    begin
                        fb_swapped <= 0;
                        cmd_start  <= 1;
                        state      <= STATE_ACTIVE;
                    end
                end

                STATE_ACTIVE:
                begin
                    // Non-blocking: acknowledge in the first cycle after start
                    if (BLOCKING == 0)
                        fb_swapped <= 1;

                    if (cmd_done)
                    begin
                        // Blocking: acknowledge only when transfer is fully done
                        if (BLOCKING == 1)
                            fb_swapped <= 1;
                        state <= STATE_IDLE;
                    end
                end

                default:
                begin
                    state <= STATE_IDLE;
                end
            endcase
        end
    end
endmodule
