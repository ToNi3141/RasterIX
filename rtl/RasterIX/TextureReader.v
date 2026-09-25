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

// Texture reader which reads a texture from the memory.
// It implements a MMU for translating virtual texture addresses to physical memory addresses.
// It implements a cache to optimize performance
// Pipelined: yes
// Depth: heavily depends on cache hits and memory latency
module TextureReader #(
    parameter DATA_WIDTH = 32,
    parameter TEXEL_WIDTH = 16,

    parameter ID_WIDTH = 4,
    parameter ADDR_WIDTH = 32,
    parameter PAGE_SIZE = 2048,
    parameter CACHE_SIZE = 1024,

    localparam STREAM_WIDTH = 32,
    localparam TEX_ADDR_WIDTH = 17,
    localparam BYTE_ADDR_WIDTH = TEX_ADDR_WIDTH + 1,
    localparam FIFO_DEPTH_LG = 5 // Maximum number of texel requests
)
(
    input  wire                             aclk,
    input  wire                             resetn,
    input  wire                             enable,
    input  wire                             nearest,

    // Texture read address channel
    input  wire                             s_tr_valid,
    output wire                             s_tr_ready,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    s_tr_addr_00,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    s_tr_addr_01,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    s_tr_addr_10,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    s_tr_addr_11,

    // Texture read texel channel
    output wire                             m_tr_valid,
    input  wire                             m_tr_ready,
    output wire [TEXEL_WIDTH - 1 : 0]       m_tr_texel_00,
    output wire [TEXEL_WIDTH - 1 : 0]       m_tr_texel_01,
    output wire [TEXEL_WIDTH - 1 : 0]       m_tr_texel_10,
    output wire [TEXEL_WIDTH - 1 : 0]       m_tr_texel_11,

    // Page table interface
    input  wire                             s_axis_tvalid,
    output wire                             s_axis_tready,
    input  wire                             s_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]      s_axis_tdata,

    // AXI memory interface
    output wire [ID_WIDTH - 1 : 0]          m_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_axi_araddr,
    output wire [ 7 : 0]                    m_axi_arlen,
    output wire [ 2 : 0]                    m_axi_arsize,
    output wire [ 1 : 0]                    m_axi_arburst,
    output wire                             m_axi_arlock,
    output wire [ 3 : 0]                    m_axi_arcache,
    output wire [ 2 : 0]                    m_axi_arprot,
    output wire                             m_axi_arvalid,
    input  wire                             m_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]          m_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]        m_axi_rdata,
    input  wire [ 1 : 0]                    m_axi_rresp,
    input  wire                             m_axi_rlast,
    input  wire                             m_axi_rvalid,
    output wire                             m_axi_rready
);
    wire [ 1 : 0]                   ttcmTexelPos;
    wire [BYTE_ADDR_WIDTH - 1 : 0]  ttcmAddr;
    wire                            ttcmCmd;
    wire                            ttcmValid;
    wire                            ttcmReady;
    TextureReaderController #(
        .TEX_ADDR_WIDTH(TEX_ADDR_WIDTH),
        .TEXEL_WIDTH(TEXEL_WIDTH),
        .ID_WIDTH(ID_WIDTH)
    ) textureTexelContextManager_inst (
        .aclk(aclk),
        .resetn(resetn),

        .invalidate(s_axis_tvalid), // As soon as a new page table is set, invalidate the cache
        .nearest(nearest),

        .s_tr_texel_00(s_tr_addr_00),
        .s_tr_texel_01(s_tr_addr_01),
        .s_tr_texel_10(s_tr_addr_10),
        .s_tr_texel_11(s_tr_addr_11),
        .s_tr_valid(s_tr_valid),
        .s_tr_ready(s_tr_ready),

        .m_trc_texel_pos(ttcmTexelPos),
        .m_trc_cmd(ttcmCmd),
        .m_trc_valid(ttcmValid),
        .m_trc_ready(ttcmReady),
        .m_trc_addr(ttcmAddr)
    );

    wire [ 1 : 0]                   bcTexelPos0;
    wire [BYTE_ADDR_WIDTH - 1 : 0]  bcAddr0;
    wire                            bcCmd0;
    wire                            bcValid0;
    wire                            bcReady0;
    wire [ 1 : 0]                   bcTexelPos1;
    wire [BYTE_ADDR_WIDTH - 1 : 0]  bcAddr1;
    wire                            bcCmd1;
    wire                            bcValid1;
    wire                            bcReady1;
    wire [ID_WIDTH - 1 : 0]         cacheAxiId;
    wire [BYTE_ADDR_WIDTH - 1 : 0]  cacheAxiAddr;
    wire [ 7 : 0]                   cacheAxiLen;
    wire [ 2 : 0]                   cacheAxiSize;
    wire [ 1 : 0]                   cacheAxiBurst;
    wire                            cacheAxiLock;
    wire [ 3 : 0]                   cacheAxiCache;
    wire [ 2 : 0]                   cacheAxiProt;
    wire                            cacheAxiValid;
    wire                            cacheAxiReady;
    wire [TEXEL_WIDTH - 1 : 0]      cacheTexel;
    wire                            cacheValid;
    wire                            cacheReady;
    axis_broadcast #(
        .M_COUNT(2),
        .DATA_WIDTH(2 + BYTE_ADDR_WIDTH + 1),
        .KEEP_ENABLE(0),
        .LAST_ENABLE(1),
        .ID_ENABLE(0),
        .DEST_ENABLE(0),
        .USER_ENABLE(0)
    ) axis_broadcast_inst (
        .clk(aclk),
        .rst(!resetn),

        .s_axis_tdata({
            ttcmTexelPos,
            ttcmAddr,
            ttcmCmd
        }),
        .s_axis_tkeep(~0),
        .s_axis_tvalid(ttcmValid),
        .s_axis_tready(ttcmReady),
        .s_axis_tlast(1),
        .s_axis_tid(0),
        .s_axis_tdest(0),
        .s_axis_tuser(0),

        .m_axis_tdata({
            bcTexelPos1,
            bcAddr1,
            bcCmd1,
            bcTexelPos0,
            bcAddr0,
            bcCmd0
        }),
        .m_axis_tkeep(),
        .m_axis_tvalid({ bcValid1, bcValid0 }),
        .m_axis_tready({ bcReady1, bcReady0 }),
        .m_axis_tlast(),
        .m_axis_tid(),
        .m_axis_tdest(),
        .m_axis_tuser()
    );

    TextureCacheDirectMapped #(
        .TEXEL_WIDTH(TEXEL_WIDTH),
        .DATA_WIDTH(DATA_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .ADDR_WIDTH(BYTE_ADDR_WIDTH),
        .CACHE_SIZE(CACHE_SIZE),
        .ENABLE_EARLY_FETCH(1),
        .COMMAND_FIFO_DEPTH_POW2(FIFO_DEPTH_LG),
        .AXI_R_FIFO_DEPTH_POW2(FIFO_DEPTH_LG)
    ) textureCacheDirectMapped_inst (
        .aclk(aclk),
        .resetn(resetn),
        .invalidate(s_axis_tvalid),
        .enable(enable),

        .s_tc_addr(bcAddr0),
        .s_tc_valid(bcValid0),
        .s_tc_ready(bcReady0),

        .m_tc_texel(cacheTexel),
        .m_tc_valid(cacheValid),
        .m_tc_ready(cacheReady),

        .m_axi_arid(cacheAxiId),
        .m_axi_araddr(cacheAxiAddr),
        .m_axi_arlen(cacheAxiLen),
        .m_axi_arsize(cacheAxiSize),
        .m_axi_arburst(cacheAxiBurst),
        .m_axi_arlock(cacheAxiLock),
        .m_axi_arcache(cacheAxiCache),
        .m_axi_arprot(cacheAxiProt),
        .m_axi_arvalid(cacheAxiValid),
        .m_axi_arready(cacheAxiReady),

        .m_axi_rid(m_axi_rid),
        .m_axi_rdata(m_axi_rdata),
        .m_axi_rresp(m_axi_rresp),
        .m_axi_rlast(m_axi_rlast),
        .m_axi_rvalid(m_axi_rvalid),
        .m_axi_rready(m_axi_rready)
    );

    TextureMMU #(
        .TEX_ADDR_WIDTH(BYTE_ADDR_WIDTH),
        .PAGE_SIZE(PAGE_SIZE),
        .ID_WIDTH(ID_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH)
    ) textureMMU_inst (
        .aclk(aclk),
        .resetn(resetn),

        .s_axis_tvalid(s_axis_tvalid),
        .s_axis_tready(s_axis_tready),
        .s_axis_tlast(s_axis_tlast),
        .s_axis_tdata(s_axis_tdata),

        .s_axi_araddr(cacheAxiAddr),
        .s_axi_arid(cacheAxiId),
        .s_axi_arlen(cacheAxiLen),
        .s_axi_arsize(cacheAxiSize),
        .s_axi_arburst(cacheAxiBurst),
        .s_axi_arlock(cacheAxiLock),
        .s_axi_arcache(cacheAxiCache),
        .s_axi_arprot(cacheAxiProt),
        .s_axi_arvalid(cacheAxiValid),
        .s_axi_arready(cacheAxiReady),

        .m_axi_arid(m_axi_arid),
        .m_axi_araddr(m_axi_araddr),
        .m_axi_arlen(m_axi_arlen),
        .m_axi_arsize(m_axi_arsize),
        .m_axi_arburst(m_axi_arburst),
        .m_axi_arlock(m_axi_arlock),
        .m_axi_arcache(m_axi_arcache),
        .m_axi_arprot(m_axi_arprot),
        .m_axi_arvalid(m_axi_arvalid),
        .m_axi_arready(m_axi_arready)
    );

    wire [TEXEL_WIDTH - 1 : 0]   fifoTexel;
    wire [ 1 : 0]                fifoTexelPos;
    wire                         fifoCmd;
    wire                         fifoValid;
    wire                         fifoReady;
    StreamConcatFifo #(
        .STREAM0_WIDTH(TEXEL_WIDTH),
        .STREAM1_WIDTH(2 + 1),
        .STREAM2_WIDTH(1),
        .STREAM3_WIDTH(1),

        .FIFO_DEPTH0_POW2(FIFO_DEPTH_LG),
        .FIFO_DEPTH1_POW2(FIFO_DEPTH_LG),
        .FIFO_DEPTH2_POW2(0),
        .FIFO_DEPTH3_POW2(0)
    ) stream_concat_fifo_inst (
        .aclk(aclk),
        .resetn(resetn),

        .s_stream0_tenable(1'b1),
        .s_stream0_tvalid(cacheValid),
        .s_stream0_tdata(cacheTexel),
        .s_stream0_tready(cacheReady),

        .s_stream1_tenable(1'b1),
        .s_stream1_tvalid(bcValid1),
        .s_stream1_tdata({ 
            bcTexelPos1,
            bcCmd1
        }),
        .s_stream1_tready(bcReady1),

        .s_stream2_tenable(1'b0),
        .s_stream2_tvalid(1'b0),
        .s_stream2_tdata(1'b0),
        .s_stream2_tready(),

        .s_stream3_tenable(1'b0),
        .s_stream3_tvalid(1'b0),
        .s_stream3_tdata(1'b0),
        .s_stream3_tready(),

        .m_stream_tvalid(fifoValid),
        .m_stream_tdata({
            1'b0,
            1'b0,
            fifoTexelPos,
            fifoCmd,
            fifoTexel
        }),
        .m_stream_tready(fifoReady)
    );

    TextureReaderContext #(
        .TEXEL_WIDTH(TEXEL_WIDTH)
    ) textureTexelContext_inst ( 
        .aclk(aclk),
        .resetn(resetn),
        
        .s_trc_texel_pos(fifoTexelPos),
        .s_trc_texel(fifoTexel),
        .s_trc_cmd(fifoCmd),
        .s_trc_valid(fifoValid),
        .s_trc_ready(fifoReady),

        .m_tr_texel_00(m_tr_texel_00),
        .m_tr_texel_01(m_tr_texel_01),
        .m_tr_texel_10(m_tr_texel_10),
        .m_tr_texel_11(m_tr_texel_11),
        .m_tr_valid(m_tr_valid),
        .m_tr_ready(m_tr_ready)
    );

endmodule 