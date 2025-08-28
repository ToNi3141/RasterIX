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

module top #(
    parameter DATA_WIDTH = 32
)
(
    input  wire                             aclk,
    input  wire                             resetn,

    // AXI Stream command interface
    input  wire                             s_cmd_axis_tvalid,
    output wire                             s_cmd_axis_tready,
    input  wire                             s_cmd_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  s_cmd_axis_tdata,

    output wire                             m_cmd_resp_axis_tvalid,
    input  wire                             m_cmd_resp_axis_tready,
    output wire                             m_cmd_resp_axis_tlast,
    output wire [CMD_STREAM_WIDTH - 1 : 0]  m_cmd_resp_axis_tdata,

    // AXI Stream framebuffer
    output wire                             m_framebuffer_axis_tvalid,
    input  wire                             m_framebuffer_axis_tready,
    output wire                             m_framebuffer_axis_tlast,
    output wire [CMD_STREAM_WIDTH - 1 : 0]  m_framebuffer_axis_tdata
);
`define STR(x) `"x`"
    parameter VARIANT = `STR(`VARIANT);
    parameter FRAMEBUFFER_SIZE_IN_PIXEL_LG = `FRAMEBUFFER_SIZE_IN_PIXEL_LG;
    parameter MAX_TEXTURE_SIZE = 256;
    parameter CMD_STREAM_WIDTH = 32;

    initial
    begin
        $dumpfile("rr.vcd");
        $dumpvars();
    end

    localparam ID_WIDTH = 8;
    localparam ADDR_WIDTH = 25;
    localparam STRB_WIDTH = DATA_WIDTH / 8;
    wire [ID_WIDTH - 1 : 0]     mem_axi_awid;
    wire [ADDR_WIDTH - 1 : 0]   mem_axi_awaddr;
    wire [ 7 : 0]               mem_axi_awlen;
    wire [ 2 : 0]               mem_axi_awsize;
    wire [ 1 : 0]               mem_axi_awburst;
    wire                        mem_axi_awlock;
    wire [ 3 : 0]               mem_axi_awcache;
    wire [ 2 : 0]               mem_axi_awprot;
    wire                        mem_axi_awvalid;
    wire                        mem_axi_awready;
    wire [DATA_WIDTH - 1 : 0]   mem_axi_wdata;
    wire [STRB_WIDTH - 1 : 0]   mem_axi_wstrb;
    wire                        mem_axi_wlast;
    wire                        mem_axi_wvalid;
    wire                        mem_axi_wready;
    wire [ID_WIDTH - 1 : 0]     mem_axi_bid;
    wire [ 1 : 0]               mem_axi_bresp;
    wire                        mem_axi_bvalid;
    wire                        mem_axi_bready;
    wire [ID_WIDTH - 1 : 0]     mem_axi_arid;
    wire [ADDR_WIDTH - 1 : 0]   mem_axi_araddr;
    wire [ 7 : 0]               mem_axi_arlen;
    wire [ 2 : 0]               mem_axi_arsize;
    wire [ 1 : 0]               mem_axi_arburst;
    wire                        mem_axi_arlock;
    wire [ 3 : 0]               mem_axi_arcache;
    wire [ 2 : 0]               mem_axi_arprot;
    wire                        mem_axi_arvalid;
    wire                        mem_axi_arready;
    wire [ID_WIDTH - 1 : 0]     mem_axi_rid;
    wire [DATA_WIDTH - 1 : 0]   mem_axi_rdata;
    wire [ 1 : 0]               mem_axi_rresp;
    wire                        mem_axi_rlast;
    wire                        mem_axi_rvalid;
    wire                        mem_axi_rready;

    localparam NRS = 2;
    localparam ID_WIDTH_LOC = ID_WIDTH - 1;
    wire [(NRS * ID_WIDTH_LOC) - 1 : 0]     xbar_axi_awid;
    wire [(NRS * ADDR_WIDTH) - 1 : 0]       xbar_axi_awaddr;
    wire [(NRS * 8) - 1 : 0]                xbar_axi_awlen; 
    wire [(NRS * 3) - 1 : 0]                xbar_axi_awsize;
    wire [(NRS * 2) - 1 : 0]                xbar_axi_awburst;
    wire [NRS - 1 : 0]                      xbar_axi_awlock;
    wire [(NRS * 4) - 1 : 0]                xbar_axi_awcache;
    wire [(NRS * 3) - 1 : 0]                xbar_axi_awprot; 
    wire [NRS - 1 : 0]                      xbar_axi_awvalid;
    wire [NRS - 1 : 0]                      xbar_axi_awready;

    wire [(NRS * DATA_WIDTH) - 1 : 0]       xbar_axi_wdata;
    wire [(NRS * STRB_WIDTH) - 1 : 0]       xbar_axi_wstrb;
    wire [NRS - 1 : 0]                      xbar_axi_wlast;
    wire [NRS - 1 : 0]                      xbar_axi_wvalid;
    wire [NRS - 1 : 0]                      xbar_axi_wready;

    wire [(NRS * ID_WIDTH_LOC) - 1 : 0]     xbar_axi_bid;
    wire [(NRS * 2) - 1 : 0]                xbar_axi_bresp;
    wire [NRS - 1 : 0]                      xbar_axi_bvalid;
    wire [NRS - 1 : 0]                      xbar_axi_bready;

    wire [(NRS * ID_WIDTH_LOC) - 1 : 0]     xbar_axi_arid;
    wire [(NRS * ADDR_WIDTH) - 1 : 0]       xbar_axi_araddr;
    wire [(NRS * 8) - 1 : 0]                xbar_axi_arlen;
    wire [(NRS * 3) - 1 : 0]                xbar_axi_arsize;
    wire [(NRS * 2) - 1 : 0]                xbar_axi_arburst;
    wire [NRS - 1 : 0]                      xbar_axi_arlock;
    wire [(NRS * 4) - 1 : 0]                xbar_axi_arcache;
    wire [(NRS * 3) - 1 : 0]                xbar_axi_arprot;
    wire [NRS - 1 : 0]                      xbar_axi_arvalid;
    wire [NRS - 1 : 0]                      xbar_axi_arready;

    wire [(NRS * ID_WIDTH_LOC) - 1 : 0]     xbar_axi_rid;
    wire [(NRS * DATA_WIDTH) - 1 : 0]       xbar_axi_rdata;
    wire [(NRS * 2) - 1 : 0]                xbar_axi_rresp;
    wire [NRS - 1 : 0]                      xbar_axi_rlast;
    wire [NRS - 1 : 0]                      xbar_axi_rvalid;
    wire [NRS - 1 : 0]                      xbar_axi_rready;

    localparam FB_SIZE_IN_PIXEL_LG = 20;
    wire                                swap_fb;
    wire [ADDR_WIDTH - 1 : 0]           fb_addr;
    wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]  fb_size;
    wire                                fb_swapped;

    axi_ram #(
        .DATA_WIDTH(DATA_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .ID_WIDTH(ID_WIDTH)
    ) ram (
        .rst(!resetn),
        .clk(aclk),

        .s_axi_awid(mem_axi_awid),
        .s_axi_awaddr(mem_axi_awaddr),
        .s_axi_awlen(mem_axi_awlen),
        .s_axi_awsize(mem_axi_awsize),
        .s_axi_awburst(mem_axi_awburst),
        .s_axi_awlock(mem_axi_awlock),
        .s_axi_awcache(mem_axi_awcache),
        .s_axi_awprot(mem_axi_awprot),
        .s_axi_awvalid(mem_axi_awvalid),
        .s_axi_awready(mem_axi_awready),
        .s_axi_wdata(mem_axi_wdata),
        .s_axi_wstrb(mem_axi_wstrb),
        .s_axi_wlast(mem_axi_wlast),
        .s_axi_wvalid(mem_axi_wvalid),
        .s_axi_wready(mem_axi_wready),
        .s_axi_bid(mem_axi_bid),
        .s_axi_bresp(mem_axi_bresp),
        .s_axi_bvalid(mem_axi_bvalid),
        .s_axi_bready(mem_axi_bready),
        .s_axi_arid(mem_axi_arid),
        .s_axi_araddr(mem_axi_araddr),
        .s_axi_arlen(mem_axi_arlen),
        .s_axi_arsize(mem_axi_arsize),
        .s_axi_arburst(mem_axi_arburst),
        .s_axi_arlock(mem_axi_arlock),
        .s_axi_arcache(mem_axi_arcache),
        .s_axi_arprot(mem_axi_arprot),
        .s_axi_arvalid(mem_axi_arvalid),
        .s_axi_arready(mem_axi_arready),
        .s_axi_rid(mem_axi_rid),
        .s_axi_rdata(mem_axi_rdata),
        .s_axi_rresp(mem_axi_rresp),
        .s_axi_rlast(mem_axi_rlast),
        .s_axi_rvalid(mem_axi_rvalid),
        .s_axi_rready(mem_axi_rready)
    );

    axi_crossbar #(
        .DATA_WIDTH(DATA_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .S_ID_WIDTH(ID_WIDTH_LOC),
        .S_COUNT(NRS),
        .M_COUNT(1),
        .M_ID_WIDTH(ID_WIDTH),
        .M_ADDR_WIDTH(ADDR_WIDTH[0 +: 32]),
        .S_THREADS({ NRS { 32'd1 } }),
        .M_ISSUE(32'd1)
    ) mainXBar (
        .clk(aclk),
        .rst(!resetn),

        .s_axi_awid(xbar_axi_awid),
        .s_axi_awaddr(xbar_axi_awaddr),
        .s_axi_awlen(xbar_axi_awlen),
        .s_axi_awsize(xbar_axi_awsize),
        .s_axi_awburst(xbar_axi_awburst),
        .s_axi_awlock(xbar_axi_awlock),
        .s_axi_awcache(xbar_axi_awcache),
        .s_axi_awprot(xbar_axi_awprot),
        .s_axi_awqos(0),
        .s_axi_awuser(0),
        .s_axi_awvalid(xbar_axi_awvalid),
        .s_axi_awready(xbar_axi_awready),

        .s_axi_wdata(xbar_axi_wdata),
        .s_axi_wstrb(xbar_axi_wstrb),
        .s_axi_wlast(xbar_axi_wlast),
        .s_axi_wuser(0),
        .s_axi_wvalid(xbar_axi_wvalid),
        .s_axi_wready(xbar_axi_wready),

        .s_axi_bid(xbar_axi_bid),
        .s_axi_bresp(xbar_axi_bresp),
        .s_axi_buser(),
        .s_axi_bvalid(xbar_axi_bvalid),
        .s_axi_bready(xbar_axi_bready),

        .s_axi_arid(xbar_axi_arid),
        .s_axi_araddr(xbar_axi_araddr),
        .s_axi_arlen(xbar_axi_arlen),
        .s_axi_arsize(xbar_axi_arsize),
        .s_axi_arburst(xbar_axi_arburst),
        .s_axi_arlock(xbar_axi_arlock),
        .s_axi_arcache(xbar_axi_arcache),
        .s_axi_arprot(xbar_axi_arprot),
        .s_axi_arqos(0),
        .s_axi_aruser(0),
        .s_axi_arvalid(xbar_axi_arvalid),
        .s_axi_arready(xbar_axi_arready),

        .s_axi_rid(xbar_axi_rid),
        .s_axi_rdata(xbar_axi_rdata),
        .s_axi_rresp(xbar_axi_rresp),
        .s_axi_rlast(xbar_axi_rlast),
        .s_axi_ruser(),
        .s_axi_rvalid(xbar_axi_rvalid),
        .s_axi_rready(xbar_axi_rready),

        .m_axi_awid(mem_axi_awid),
        .m_axi_awaddr(mem_axi_awaddr),
        .m_axi_awlen(mem_axi_awlen),
        .m_axi_awsize(mem_axi_awsize),
        .m_axi_awburst(mem_axi_awburst),
        .m_axi_awlock(mem_axi_awlock),
        .m_axi_awcache(mem_axi_awcache),
        .m_axi_awprot(mem_axi_awprot),
        .m_axi_awqos(),
        .m_axi_awregion(),
        .m_axi_awuser(),
        .m_axi_awvalid(mem_axi_awvalid),
        .m_axi_awready(mem_axi_awready),

        .m_axi_wdata(mem_axi_wdata),
        .m_axi_wstrb(mem_axi_wstrb),
        .m_axi_wlast(mem_axi_wlast),
        .m_axi_wuser(),
        .m_axi_wvalid(mem_axi_wvalid),
        .m_axi_wready(mem_axi_wready),

        .m_axi_bid(mem_axi_bid),
        .m_axi_bresp(mem_axi_bresp),
        .m_axi_buser(0),
        .m_axi_bvalid(mem_axi_bvalid),
        .m_axi_bready(mem_axi_bready),

        .m_axi_arid(mem_axi_arid),
        .m_axi_araddr(mem_axi_araddr),
        .m_axi_arlen(mem_axi_arlen),
        .m_axi_arsize(mem_axi_arsize),
        .m_axi_arburst(mem_axi_arburst),
        .m_axi_arlock(mem_axi_arlock),
        .m_axi_arcache(mem_axi_arcache),
        .m_axi_arqos(),
        .m_axi_arregion(),
        .m_axi_aruser(),
        .m_axi_arprot(mem_axi_arprot),
        .m_axi_arvalid(mem_axi_arvalid),
        .m_axi_arready(mem_axi_arready),

        .m_axi_rid(mem_axi_rid),
        .m_axi_rdata(mem_axi_rdata),
        .m_axi_rresp(mem_axi_rresp),
        .m_axi_rlast(mem_axi_rlast),
        .m_axi_ruser(0),
        .m_axi_rvalid(mem_axi_rvalid),
        .m_axi_rready(mem_axi_rready)
    );

    RasterIX #(
        .VARIANT(VARIANT),
        .FRAMEBUFFER_SIZE_IN_PIXEL_LG(FRAMEBUFFER_SIZE_IN_PIXEL_LG),
        .TEXTURE_PAGE_SIZE(4096),
        .ADDR_WIDTH(ADDR_WIDTH),
        .ID_WIDTH(ID_WIDTH_LOC),
        .DATA_WIDTH(DATA_WIDTH),
        .ENABLE_STENCIL_BUFFER(1),
        .ENABLE_DEPTH_BUFFER(1),
        .ENABLE_TEXTURE_FILTERING(1),
        .ENABLE_FOG(1),
        .MAX_TEXTURE_SIZE(MAX_TEXTURE_SIZE),
        .FRAMEBUFFER_SUB_PIXEL_WIDTH(6),
        .SUB_PIXEL_CALC_PRECISION(8),
        .TMU_COUNT(2),
        .RASTERIZER_ENABLE_FLOAT_INTERPOLATION(0)
    ) rix (
        .aclk(aclk),
        .resetn(resetn),
        
        .s_cmd_axis_tvalid(s_cmd_axis_tvalid),
        .s_cmd_axis_tready(s_cmd_axis_tready),
        .s_cmd_axis_tlast(s_cmd_axis_tlast),
        .s_cmd_axis_tdata(s_cmd_axis_tdata),

        .m_cmd_resp_axis_tvalid(m_cmd_resp_axis_tvalid),
        .m_cmd_resp_axis_tready(m_cmd_resp_axis_tready),
        .m_cmd_resp_axis_tlast(m_cmd_resp_axis_tlast),
        .m_cmd_resp_axis_tdata(m_cmd_resp_axis_tdata),

        .swap_fb(swap_fb),
        .swap_fb_enable_vsync(),
        .fb_addr(fb_addr),
        .fb_size(fb_size),
        .fb_swapped(fb_swapped),

        .m_axi_awid(xbar_axi_awid[0 +: ID_WIDTH_LOC]),
        .m_axi_awaddr(xbar_axi_awaddr[0 +: ADDR_WIDTH]),
        .m_axi_awlen(xbar_axi_awlen[0 +: 8]),
        .m_axi_awsize(xbar_axi_awsize[0 +: 3]),
        .m_axi_awburst(xbar_axi_awburst[0 +: 2]),
        .m_axi_awlock(xbar_axi_awlock[0]),
        .m_axi_awcache(xbar_axi_awcache[0 +: 4]),
        .m_axi_awprot(xbar_axi_awprot[0 +: 3]),
        .m_axi_awvalid(xbar_axi_awvalid[0]),
        .m_axi_awready(xbar_axi_awready[0]),
        .m_axi_wdata(xbar_axi_wdata[0 +: DATA_WIDTH]),
        .m_axi_wstrb(xbar_axi_wstrb[0 +: STRB_WIDTH]),
        .m_axi_wlast(xbar_axi_wlast[0]),
        .m_axi_wvalid(xbar_axi_wvalid[0]),
        .m_axi_wready(xbar_axi_wready[0]),
        .m_axi_bid(xbar_axi_bid[0 +: ID_WIDTH_LOC]),
        .m_axi_bresp(xbar_axi_bresp[0 +: 2]),
        .m_axi_bvalid(xbar_axi_bvalid[0]),
        .m_axi_bready(xbar_axi_bready[0]),
        .m_axi_arid(xbar_axi_arid[0 +: ID_WIDTH_LOC]),
        .m_axi_araddr(xbar_axi_araddr[0 +: ADDR_WIDTH]),
        .m_axi_arlen(xbar_axi_arlen[0 +: 8]),
        .m_axi_arsize(xbar_axi_arsize[0 +: 3]),
        .m_axi_arburst(xbar_axi_arburst[0 +: 2]),
        .m_axi_arlock(xbar_axi_arlock[0]),
        .m_axi_arcache(xbar_axi_arcache[0 +: 4]),
        .m_axi_arprot(xbar_axi_arprot[0 +: 3]),
        .m_axi_arvalid(xbar_axi_arvalid[0]),
        .m_axi_arready(xbar_axi_arready[0]),
        .m_axi_rid(xbar_axi_rid[0 +: ID_WIDTH_LOC]),
        .m_axi_rdata(xbar_axi_rdata[0 +: DATA_WIDTH]),
        .m_axi_rresp(xbar_axi_rresp[0 +: 2]),
        .m_axi_rlast(xbar_axi_rlast[0]),
        .m_axi_rvalid(xbar_axi_rvalid[0]),
        .m_axi_rready(xbar_axi_rready[0])
    );

    AxisFramebufferReader #(
        .DISPLAY_STREAM_WIDTH(CMD_STREAM_WIDTH),
        .DATA_WIDTH(DATA_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .STRB_WIDTH(STRB_WIDTH),
        .ID_WIDTH(ID_WIDTH_LOC),
        .BLOCKING(0)
    ) axisFramebufferReader (
        .aclk(aclk),
        .resetn(resetn),

        .swap_fb(swap_fb),
        .fb_addr(fb_addr),
        .fb_size(fb_size),
        .fb_swapped(fb_swapped),

        .m_disp_axis_tvalid(m_framebuffer_axis_tvalid),
        .m_disp_axis_tready(m_framebuffer_axis_tready),
        .m_disp_axis_tlast(m_framebuffer_axis_tlast),
        .m_disp_axis_tdata(m_framebuffer_axis_tdata),

        .m_mem_axi_arid(xbar_axi_arid[1 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_mem_axi_araddr(xbar_axi_araddr[1 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_mem_axi_arlen(xbar_axi_arlen[1 * 8 +: 8]),
        .m_mem_axi_arsize(xbar_axi_arsize[1 * 3 +: 3]),
        .m_mem_axi_arburst(xbar_axi_arburst[1 * 2 +: 2]),
        .m_mem_axi_arlock(xbar_axi_arlock[1 * 1 +: 1]),
        .m_mem_axi_arcache(xbar_axi_arcache[1 * 4 +: 4]),
        .m_mem_axi_arprot(xbar_axi_arprot[1 * 3 +: 3]),
        .m_mem_axi_arvalid(xbar_axi_arvalid[1 * 1 +: 1]),
        .m_mem_axi_arready(xbar_axi_arready[1 * 1 +: 1]),

        .m_mem_axi_rid(xbar_axi_rid[1 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_mem_axi_rdata(xbar_axi_rdata[1 * DATA_WIDTH +: DATA_WIDTH]),
        .m_mem_axi_rresp(xbar_axi_rresp[1 * 2 +: 2]),
        .m_mem_axi_rlast(xbar_axi_rlast[1 * 1 +: 1]),
        .m_mem_axi_rvalid(xbar_axi_rvalid[1 * 1 +: 1]),
        .m_mem_axi_rready(xbar_axi_rready[1 * 1 +: 1])
    );

    reg tmpOne = 1;
    reg tmpZero = 0;
    assign xbar_axi_awid[1 * ID_WIDTH_LOC +: ID_WIDTH_LOC] = { ID_WIDTH_LOC { tmpZero } };
    assign xbar_axi_awaddr[1 * ADDR_WIDTH +: ADDR_WIDTH] = { ADDR_WIDTH { tmpZero } };
    assign xbar_axi_awlen[1 * 8 +: 8] = { 8 { tmpZero } };
    assign xbar_axi_awsize[1 * 3 +: 3] = { 3 { tmpZero } };
    assign xbar_axi_awburst[1 * 2 +: 2] = { 2 { tmpZero } };
    assign xbar_axi_awlock[1 * 1 +: 1] = tmpZero;
    assign xbar_axi_awcache[1 * 4 +: 4] = { 4 { tmpZero } };
    assign xbar_axi_awprot[1 * 3 +: 3] = { 3 { tmpZero } };
    assign xbar_axi_awvalid[1 * 1 +: 1] = tmpZero;
    assign xbar_axi_wdata[1 * DATA_WIDTH +: DATA_WIDTH] = { DATA_WIDTH { tmpZero } };
    assign xbar_axi_wstrb[1 * STRB_WIDTH +: STRB_WIDTH] = { STRB_WIDTH { tmpZero } };
    assign xbar_axi_wlast[1 * 1 +: 1] = tmpZero;
    assign xbar_axi_wvalid[1 * 1 +: 1] = tmpZero;
    assign xbar_axi_bready[1 * 1 +: 1] = tmpZero;

endmodule