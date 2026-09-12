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


module TextureReader #(
    parameter DATA_WIDTH = 32,
    parameter TEXEL_WIDTH = 16,

    parameter ID_WIDTH = 4,
    parameter ADDR_WIDTH = 32,
    parameter PAGE_SIZE = 2048,

    localparam STREAM_WIDTH = 32,
    localparam TEX_ADDR_WIDTH = 17,
    localparam BYTE_ADDR_WIDTH = TEX_ADDR_WIDTH + 1
)
(
    input  wire                             aclk,
    input  wire                             resetn,

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
    wire [ 1 : 0]                   ttcm_texel_pos;
    wire [BYTE_ADDR_WIDTH - 1 : 0]  ttcm_araddr;
    wire                            ttcm_cmd;
    wire                            ttcm_valid;
    wire                            ttcm_ready;
    wire [ID_WIDTH - 1 : 0]         ttcm_arid;
    wire [ 7 : 0]                   ttcm_arlen;
    wire [ 2 : 0]                   ttcm_arsize;
    wire [ 1 : 0]                   ttcm_arburst;
    wire                            ttcm_arlock;
    wire [ 3 : 0]                   ttcm_arcache;
    wire [ 2 : 0]                   ttcm_arprot;
    TextureReaderController #(
        .TEX_ADDR_WIDTH(TEX_ADDR_WIDTH),
        .TEXEL_WIDTH(TEXEL_WIDTH),
        .ID_WIDTH(ID_WIDTH)
    ) textureTexelContextManager_inst (
        .aclk(aclk),
        .resetn(resetn),

        .invalidate(s_axis_tvalid), // As soon as a new page table is set, invalidate the cache

        .s_tr_texel_00(s_tr_addr_00),
        .s_tr_texel_01(s_tr_addr_01),
        .s_tr_texel_10(s_tr_addr_10),
        .s_tr_texel_11(s_tr_addr_11),
        .s_tr_valid(s_tr_valid),
        .s_tr_ready(s_tr_ready),

        .m_tr_texel_pos(ttcm_texel_pos),
        .m_tr_cmd(ttcm_cmd),
        .m_tr_valid(ttcm_valid),
        .m_tr_ready(ttcm_ready),
        .m_tr_addr(ttcm_araddr),
        .m_arid(ttcm_arid),
        .m_arlen(ttcm_arlen),
        .m_arsize(ttcm_arsize),
        .m_arburst(ttcm_arburst),
        .m_arlock(ttcm_arlock),
        .m_arcache(ttcm_arcache),
        .m_arprot(ttcm_arprot)
    );

    wire [ 1 : 0]                   bc_texel_pos_0;
    wire [BYTE_ADDR_WIDTH - 1 : 0]  bc_araddr_0;
    wire                            bc_cmd_0;
    wire                            bc_valid_0;
    wire                            bc_ready_0;
    wire [ 1 : 0]                   bc_texel_pos_1;
    wire [BYTE_ADDR_WIDTH - 1 : 0]  bc_araddr_1;
    wire                            bc_cmd_1;
    wire                            bc_valid_1;
    wire                            bc_ready_1;
    wire [ID_WIDTH - 1 : 0]         cache_axi_arid;
    wire [BYTE_ADDR_WIDTH - 1 : 0]  cache_axi_araddr;
    wire [ 7 : 0]                   cache_axi_arlen;
    wire [ 2 : 0]                   cache_axi_arsize;
    wire [ 1 : 0]                   cache_axi_arburst;
    wire                            cache_axi_arlock;
    wire [ 3 : 0]                   cache_axi_arcache;
    wire [ 2 : 0]                   cache_axi_arprot;
    wire                            cache_axi_arvalid;
    wire                            cache_axi_arready;
    wire [TEXEL_WIDTH - 1 : 0]      cache_texel;
    wire                            cache_valid;
    wire                            cache_ready;
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
            ttcm_texel_pos,
            ttcm_araddr, 
            ttcm_cmd
        }),
        .s_axis_tkeep(~0),
        .s_axis_tvalid(ttcm_valid),
        .s_axis_tready(ttcm_ready),
        .s_axis_tlast(1),
        .s_axis_tid(0),
        .s_axis_tdest(0),
        .s_axis_tuser(0),

        .m_axis_tdata({
            bc_texel_pos_1,
            bc_araddr_1,
            bc_cmd_1,
            bc_texel_pos_0,
            bc_araddr_0,
            bc_cmd_0
        }),
        .m_axis_tkeep(),
        .m_axis_tvalid({ bc_valid_1, bc_valid_0 }),
        .m_axis_tready({ bc_ready_1, bc_ready_0 }),
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
        .CACHE_SIZE(1024)
    ) textureCacheDirectMapped_inst (
        .aclk(aclk),
        .resetn(resetn),
        .invalidate(s_axis_tvalid),

        .s_tc_addr(bc_araddr_0),
        .s_tc_valid(bc_valid_0),
        .s_tc_ready(bc_ready_0),

        .m_tc_texel(cache_texel),
        .m_tc_valid(cache_valid),
        .m_tc_ready(cache_ready),

        .m_axi_arid(cache_axi_arid),
        .m_axi_araddr(cache_axi_araddr),
        .m_axi_arlen(cache_axi_arlen),
        .m_axi_arsize(cache_axi_arsize),
        .m_axi_arburst(cache_axi_arburst),
        .m_axi_arlock(cache_axi_arlock),
        .m_axi_arcache(cache_axi_arcache),
        .m_axi_arprot(cache_axi_arprot),
        .m_axi_arvalid(cache_axi_arvalid),
        .m_axi_arready(cache_axi_arready),

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

        .s_axi_araddr(cache_axi_araddr),
        .s_axi_arid(cache_axi_arid),
        .s_axi_arlen(cache_axi_arlen),
        .s_axi_arsize(cache_axi_arsize),
        .s_axi_arburst(cache_axi_arburst),
        .s_axi_arlock(cache_axi_arlock),
        .s_axi_arcache(cache_axi_arcache),
        .s_axi_arprot(cache_axi_arprot),
        .s_axi_arvalid(cache_axi_arvalid),
        .s_axi_arready(cache_axi_arready),

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

    wire [TEXEL_WIDTH - 1 : 0]   fifo_texel;
    wire [ 1 : 0]                fifo_texel_pos;
    wire                         fifo_cmd;
    wire                         fifo_valid;
    wire                         fifo_ready;
    StreamConcatFifo #(
        .STREAM0_WIDTH(TEXEL_WIDTH),
        .STREAM1_WIDTH(2 + 1),
        .STREAM2_WIDTH(1),
        .STREAM3_WIDTH(1),

        .FIFO_DEPTH0_POW2(5),
        .FIFO_DEPTH1_POW2(5),
        .FIFO_DEPTH2_POW2(0),
        .FIFO_DEPTH3_POW2(0)
    ) stream_concat_fifo_inst (
        .aclk(aclk),
        .resetn(resetn),

        .s_stream0_tenable(1'b1),
        .s_stream0_tvalid(cache_valid),
        .s_stream0_tdata(cache_texel),
        .s_stream0_tready(cache_ready),

        .s_stream1_tenable(1'b1),
        .s_stream1_tvalid(bc_valid_1),
        .s_stream1_tdata({ 
            bc_texel_pos_1, 
            bc_cmd_1 
        }),
        .s_stream1_tready(bc_ready_1),

        .s_stream2_tenable(1'b0),
        .s_stream2_tvalid(1'b0),
        .s_stream2_tdata(1'b0),
        .s_stream2_tready(),

        .s_stream3_tenable(1'b0),
        .s_stream3_tvalid(1'b0),
        .s_stream3_tdata(1'b0),
        .s_stream3_tready(),

        .m_stream_tvalid(fifo_valid),
        .m_stream_tdata({
            1'b0,
            1'b0,
            fifo_texel_pos, 
            fifo_cmd,
            fifo_texel
        }),
        .m_stream_tready(fifo_ready)
    );

    TextureReaderContext #(
        .TEXEL_WIDTH(TEXEL_WIDTH)
    ) textureTexelContext_inst ( 
        .aclk(aclk),
        .resetn(resetn),
        
        .s_tr_texel_pos(fifo_texel_pos),
        .s_tr_texel(fifo_texel),
        .s_tr_cmd(fifo_cmd),
        .s_tr_valid(fifo_valid),
        .s_tr_ready(fifo_ready),

        .m_tr_texel_00(m_tr_texel_00),
        .m_tr_texel_01(m_tr_texel_01),
        .m_tr_texel_10(m_tr_texel_10),
        .m_tr_texel_11(m_tr_texel_11),
        .m_tr_valid(m_tr_valid),
        .m_tr_ready(m_tr_ready)
    );

endmodule 