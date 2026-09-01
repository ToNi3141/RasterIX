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


module TextureMemory #(
    parameter STREAM_WIDTH = 32,
    parameter TEXEL_WIDTH = 16,

    parameter ID_WIDTH = 4,
    parameter ADDR_WIDTH = 32,
    parameter PAGE_SIZE = 2048,

    localparam TEX_ADDR_WIDTH = 17
)
(
    input  wire                             aclk,
    input  wire                             resetn,

    // Texture read address channel
    input  wire                             texelAddrValid,
    output wire                             texelAddrReady,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    texelAddr00,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    texelAddr01,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    texelAddr10,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    texelAddr11,

    // Texture read texel channel
    output wire                             texelOutputValid,
    input  wire                             texelOutputReady,
    output wire [TEXEL_WIDTH - 1 : 0]       texelOutput00,
    output wire [TEXEL_WIDTH - 1 : 0]       texelOutput01,
    output wire [TEXEL_WIDTH - 1 : 0]       texelOutput10,
    output wire [TEXEL_WIDTH - 1 : 0]       texelOutput11,

    // Page table interface
    input  wire                             s_axis_tvalid,
    output wire                             s_axis_tready,
    input  wire                             s_axis_tlast,
    input  wire [ADDR_WIDTH - 1 : 0]        s_axis_tdata,

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
    input  wire [TEXEL_WIDTH - 1 : 0]       m_axi_rdata,
    input  wire [ 1 : 0]                    m_axi_rresp,
    input  wire                             m_axi_rlast,
    input  wire                             m_axi_rvalid,
    output wire                             m_axi_rready
);
    wire [ 1 : 0]                   ttcm_texel_pos;
    wire [TEX_ADDR_WIDTH - 1 : 0]   ttcm_araddr;
    wire                            ttcm_cmd;
    wire                            ttcm_valid;
    wire                            ttcm_ready;
    TextureTexelContextManager #(
        .TEX_ADDR_WIDTH(TEX_ADDR_WIDTH)
    ) textureTexelContextManager_inst (
        .aclk(aclk),
        .resetn(resetn),

        .invalidate(s_axis_tvalid), // As soon as a new page table is set, invalidate the cache

        .s_ar_texel00(texelAddr00),
        .s_ar_texel01(texelAddr01),
        .s_ar_texel10(texelAddr10),
        .s_ar_texel11(texelAddr11),
        .s_ar_valid(texelAddrValid),
        .s_ar_ready(texelAddrReady),

        .m_texel_pos(ttcm_texel_pos),
        .m_cmd(ttcm_cmd),
        .m_valid(ttcm_valid),
        .m_ready(ttcm_ready),
        .m_araddr(ttcm_araddr)
    );

    wire [ 1 : 0]                   bc_texel_pos_0;
    wire [TEX_ADDR_WIDTH - 1 : 0]   bc_araddr_0;
    wire                            bc_cmd_0;
    wire                            bc_valid_0;
    wire                            bc_ready_0;
    wire [ 1 : 0]                   bc_texel_pos_1;
    wire [TEX_ADDR_WIDTH - 1 : 0]   bc_araddr_1;
    wire                            bc_cmd_1;
    wire                            bc_valid_1;
    wire                            bc_ready_1;
    axis_broadcast #(
        .M_COUNT(2),
        .DATA_WIDTH(2 + TEX_ADDR_WIDTH + 1),
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

    wire [TEXEL_WIDTH - 1 : 0] mmu_texel;
    wire                       mmu_valid;
    wire                       mmu_ready;
    TextureMMU #(
        .TEX_ADDR_WIDTH(TEX_ADDR_WIDTH),
        .PAGE_SIZE(PAGE_SIZE),
        .DATA_WIDTH(TEXEL_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH)
    ) textureMMU_inst (
        .aclk(aclk),
        .resetn(resetn),

        .s_axis_tvalid(s_axis_tvalid),
        .s_axis_tready(s_axis_tready),
        .s_axis_tlast(s_axis_tlast),
        .s_axis_tdata(s_axis_tdata),

        .s_araddr(bc_araddr_0),
        .s_arvalid(bc_valid_0),
        .s_arready(bc_ready_0),

        .s_rdata(mmu_texel),
        .s_rvalid(mmu_valid),
        .s_rready(mmu_ready),

        .m_axi_arid(m_axi_arid),
        .m_axi_araddr(m_axi_araddr),
        .m_axi_arlen(m_axi_arlen),
        .m_axi_arsize(m_axi_arsize),
        .m_axi_arburst(m_axi_arburst),
        .m_axi_arlock(m_axi_arlock),
        .m_axi_arcache(m_axi_arcache),
        .m_axi_arprot(m_axi_arprot),
        .m_axi_arvalid(m_axi_arvalid),
        .m_axi_arready(m_axi_arready),

        .m_axi_rid(m_axi_rid),
        .m_axi_rdata(m_axi_rdata),
        .m_axi_rresp(m_axi_rresp),
        .m_axi_rlast(m_axi_rlast),
        .m_axi_rvalid(m_axi_rvalid),
        .m_axi_rready(m_axi_rready)
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
        .s_stream0_tvalid(mmu_valid),
        .s_stream0_tdata(mmu_texel),
        .s_stream0_tready(mmu_ready),

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

    TextureTexelContext #(
        .TEXEL_WIDTH(TEXEL_WIDTH)
    ) textureTexelContext_inst ( 
        .aclk(aclk),
        .resetn(resetn),
        
        .s_texel_pos(fifo_texel_pos),
        .s_texel(fifo_texel),
        .s_cmd(fifo_cmd),
        .s_valid(fifo_valid),
        .s_ready(fifo_ready),

        .m_texel00(texelOutput00),
        .m_texel01(texelOutput01),
        .m_texel10(texelOutput10),
        .m_texel11(texelOutput11),
        .m_valid(texelOutputValid),
        .m_ready(texelOutputReady)
    );

endmodule 