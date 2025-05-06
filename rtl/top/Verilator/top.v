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

    wire                        fb_swap;

    axi_ram #(
        .DATA_WIDTH(DATA_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH)
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

    RasterIX #(
        .VARIANT(VARIANT),
        .ENABLE_FRAMEBUFFER_STREAM(1),
        .FRAMEBUFFER_SIZE_IN_PIXEL_LG(FRAMEBUFFER_SIZE_IN_PIXEL_LG),
        .TEXTURE_PAGE_SIZE(4096),
        .ADDR_WIDTH(ADDR_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .DATA_WIDTH(DATA_WIDTH),
        .ENABLE_STENCIL_BUFFER(1),
        .MAX_TEXTURE_SIZE(MAX_TEXTURE_SIZE),
        .FRAMEBUFFER_SUB_PIXEL_WIDTH(6),
        .TMU_COUNT(2),
        .RASTERIZER_ENABLE_FLOAT_INTERPOLATION(0)
    ) rix (
        .aclk(aclk),
        .resetn(resetn),
        
        .s_cmd_axis_tvalid(s_cmd_axis_tvalid),
        .s_cmd_axis_tready(s_cmd_axis_tready),
        .s_cmd_axis_tlast(s_cmd_axis_tlast),
        .s_cmd_axis_tdata(s_cmd_axis_tdata),

        .m_framebuffer_axis_tvalid(m_framebuffer_axis_tvalid),
        .m_framebuffer_axis_tready(m_framebuffer_axis_tready),
        .m_framebuffer_axis_tlast(m_framebuffer_axis_tlast),
        .m_framebuffer_axis_tdata(m_framebuffer_axis_tdata),

        .swap_fb(fb_swap),
        .swap_fb_enable_vsync(),
        .fb_addr(),
        .fb_swapped(!fb_swap),

        .m_axi_awid(mem_axi_awid),
        .m_axi_awaddr(mem_axi_awaddr),
        .m_axi_awlen(mem_axi_awlen),
        .m_axi_awsize(mem_axi_awsize),
        .m_axi_awburst(mem_axi_awburst),
        .m_axi_awlock(mem_axi_awlock),
        .m_axi_awcache(mem_axi_awcache),
        .m_axi_awprot(mem_axi_awprot),
        .m_axi_awvalid(mem_axi_awvalid),
        .m_axi_awready(mem_axi_awready),
        .m_axi_wdata(mem_axi_wdata),
        .m_axi_wstrb(mem_axi_wstrb),
        .m_axi_wlast(mem_axi_wlast),
        .m_axi_wvalid(mem_axi_wvalid),
        .m_axi_wready(mem_axi_wready),
        .m_axi_bid(mem_axi_bid),
        .m_axi_bresp(mem_axi_bresp),
        .m_axi_bvalid(mem_axi_bvalid),
        .m_axi_bready(mem_axi_bready),
        .m_axi_arid(mem_axi_arid),
        .m_axi_araddr(mem_axi_araddr),
        .m_axi_arlen(mem_axi_arlen),
        .m_axi_arsize(mem_axi_arsize),
        .m_axi_arburst(mem_axi_arburst),
        .m_axi_arlock(mem_axi_arlock),
        .m_axi_arcache(mem_axi_arcache),
        .m_axi_arprot(mem_axi_arprot),
        .m_axi_arvalid(mem_axi_arvalid),
        .m_axi_arready(mem_axi_arready),
        .m_axi_rid(mem_axi_rid),
        .m_axi_rdata(mem_axi_rdata),
        .m_axi_rresp(mem_axi_rresp),
        .m_axi_rlast(mem_axi_rlast),
        .m_axi_rvalid(mem_axi_rvalid),
        .m_axi_rready(mem_axi_rready)
    );

endmodule