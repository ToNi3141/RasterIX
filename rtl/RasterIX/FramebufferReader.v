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

// This module is used to read pixels from the memory.
// High level module which combines the read requesting of pixel vectors
// and the serialization of the vector in one module.
// It is the counter part to the FramebufferWriter.
// Performance: 1 pixel per cycle
module FramebufferReader #(
    // Width of the axi interfaces
    parameter DATA_WIDTH = 32,
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of wstrb (width of data bus in words)
    parameter STRB_WIDTH = (DATA_WIDTH / 8),
    // Width of ID signal
    parameter ID_WIDTH = 8,

    // Size of the pixels
    parameter PIXEL_WIDTH = 16

) (
    input  wire                             aclk,
    input  wire                             resetn,

    /////////////////////////
    // Fragment Interface
    /////////////////////////

    // Fetch interface
    input  wire                             s_fetch_tvalid,
    input  wire                             s_fetch_tlast,
    output wire                             s_fetch_tready,
    input  wire [ADDR_WIDTH - 1 : 0]        s_fetch_taddr,

    // Framebuffer Interface
    output wire                             m_frag_tvalid,
    input  wire                             m_frag_tready,
    output wire [PIXEL_WIDTH - 1 : 0]       m_frag_tdata,
    output wire [ADDR_WIDTH - 1 : 0]        m_frag_taddr,
    output wire                             m_frag_tlast,

    /////////////////////////
    // Memory Interface
    /////////////////////////

    // Address channel
    output wire [ID_WIDTH - 1 : 0]          m_mem_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_mem_axi_araddr,
    output wire [ 7 : 0]                    m_mem_axi_arlen,
    output wire [ 2 : 0]                    m_mem_axi_arsize,
    output wire [ 1 : 0]                    m_mem_axi_arburst,
    output wire                             m_mem_axi_arlock,
    output wire [ 3 : 0]                    m_mem_axi_arcache,
    output wire [ 2 : 0]                    m_mem_axi_arprot,
    output wire                             m_mem_axi_arvalid,
    input  wire                             m_mem_axi_arready,

    // Data channel
    input  wire [ID_WIDTH - 1 : 0]          m_mem_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]        m_mem_axi_rdata,
    input  wire [ 1 : 0]                    m_mem_axi_rresp,
    input  wire                             m_mem_axi_rlast,
    input  wire                             m_mem_axi_rvalid,
    output wire                             m_mem_axi_rready
);
    localparam FETCH_FIFO_LEN = $clog2(64);

    wire [ 1 : 0]                   bc_fetch_tvalid;
    wire [ 1 : 0]                   bc_fetch_tlast;
    wire [ 1 : 0]                   bc_fetch_tready;
    wire [(ADDR_WIDTH * 2) - 1 : 0] bc_fetch_tdest;

    wire                            fifo_fetch_tvalid;
    wire                            fifo_fetch_tlast;
    wire                            fifo_fetch_tready;
    wire [ADDR_WIDTH - 1 : 0]       fifo_fetch_tdest;
    wire                            fifo_fetch_tempty;
    wire [ADDR_WIDTH + 1 - 1 : 0]   fifo_fetch_tdata;
    wire                            fifo_full;
    assign fifo_fetch_tvalid = !fifo_fetch_tempty;
    assign fifo_fetch_tlast = fifo_fetch_tdata[ADDR_WIDTH +: 1];
    assign fifo_fetch_tdest = fifo_fetch_tdata[0 +: ADDR_WIDTH];

    assign bc_fetch_tready[1] = !fifo_full; 

    axis_broadcast #(
        .M_COUNT(2),
        .DATA_WIDTH(ADDR_WIDTH),
        .KEEP_ENABLE(0),
        .LAST_ENABLE(1),
        .ID_ENABLE(0),
        .DEST_ENABLE(0),
        .USER_ENABLE(0)
    ) axisBroadcast (
        .clk(aclk),
        .rst(!resetn),

        .s_axis_tdata(s_fetch_taddr),
        .s_axis_tlast(s_fetch_tlast),
        .s_axis_tvalid(s_fetch_tvalid),
        .s_axis_tready(s_fetch_tready),
        .s_axis_tid(),
        .s_axis_tdest(),
        .s_axis_tuser(),
        .s_axis_tkeep(),

        .m_axis_tdata(bc_fetch_tdest),
        .m_axis_tvalid(bc_fetch_tvalid),
        .m_axis_tready(bc_fetch_tready),
        .m_axis_tlast(bc_fetch_tlast),
        .m_axis_tid(),
        .m_axis_tdest(),
        .m_axis_tuser(),
        .m_axis_tkeep()
    );

    sfifo #(
        .BW(ADDR_WIDTH + 1),
        .LGFLEN(FETCH_FIFO_LEN)
    ) fetchAddrFifo (
        .i_clk(aclk),
        .i_reset(!resetn),

        .i_wr(bc_fetch_tvalid[1]),
        .i_data({ bc_fetch_tlast[1], bc_fetch_tdest[ADDR_WIDTH +: ADDR_WIDTH] }),
        .o_full(fifo_full),
        .o_fill(),

        .i_rd(fifo_fetch_tready),
        .o_data(fifo_fetch_tdata),
        .o_empty(fifo_fetch_tempty)
    );

    MemoryReadRequestGenerator #(
        .DATA_WIDTH(DATA_WIDTH), 
        .ADDR_WIDTH(ADDR_WIDTH), 
        .ID_WIDTH(ID_WIDTH)
    ) mrrg (
        .aclk(aclk),
        .resetn(resetn),
        
        .s_fetch_tvalid(bc_fetch_tvalid[0]),
        .s_fetch_tlast(bc_fetch_tlast[0]),
        .s_fetch_tready(bc_fetch_tready[0]),
        .s_fetch_taddr(bc_fetch_tdest[0 +: ADDR_WIDTH]),

        .m_mem_axi_arid(m_mem_axi_arid),
        .m_mem_axi_araddr(m_mem_axi_araddr),
        .m_mem_axi_arlen(m_mem_axi_arlen),
        .m_mem_axi_arsize(m_mem_axi_arsize),
        .m_mem_axi_arburst(m_mem_axi_arburst),
        .m_mem_axi_arlock(m_mem_axi_arlock),
        .m_mem_axi_arcache(m_mem_axi_arcache),
        .m_mem_axi_arprot(m_mem_axi_arprot),
        .m_mem_axi_arvalid(m_mem_axi_arvalid),
        .m_mem_axi_arready(m_mem_axi_arready)
    );

    FramebufferSerializer #(
        .DATA_WIDTH(DATA_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .PIXEL_WIDTH(PIXEL_WIDTH)
    ) fbs (
        .aclk(aclk),
        .resetn(resetn),

        .m_frag_tvalid(m_frag_tvalid),
        .m_frag_tready(m_frag_tready),
        .m_frag_tdata(m_frag_tdata),
        .m_frag_taddr(m_frag_taddr),
        .m_frag_tlast(m_frag_tlast),

        .s_fetch_tvalid(fifo_fetch_tvalid),
        .s_fetch_tlast(fifo_fetch_tlast),
        .s_fetch_tready(fifo_fetch_tready),
        .s_fetch_taddr(fifo_fetch_tdest),

        .m_mem_axi_rid(m_mem_axi_rid),
        .m_mem_axi_rdata(m_mem_axi_rdata),
        .m_mem_axi_rresp(m_mem_axi_rresp),
        .m_mem_axi_rlast(m_mem_axi_rlast),
        .m_mem_axi_rvalid(m_mem_axi_rvalid),
        .m_mem_axi_rready(m_mem_axi_rready)
    );

endmodule