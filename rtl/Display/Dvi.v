// Rasterix
// https://github.com/ToNi3141/Rasterix
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

`timescale 1ns / 1ps

module Dvi #(
    FB_1_ADDR = 32'h01E00000,
    FB_2_ADDR = 32'h01C00000
) (
    input  wire             aclkLogic,
    input  wire             aclk,
    input  wire             aclk5x,
    input  wire             resetn,

    input  wire             swap,
    output reg              swapped,

    output wire [3 : 0]     m_mem_axi_arid,
    output wire [31 : 0]    m_mem_axi_araddr,
    output wire [ 7 : 0]    m_mem_axi_arlen,
    output wire [ 2 : 0]    m_mem_axi_arsize,
    output wire [ 1 : 0]    m_mem_axi_arburst,
    output wire             m_mem_axi_arlock,
    output wire [ 3 : 0]    m_mem_axi_arcache,
    output wire [ 2 : 0]    m_mem_axi_arprot,
    output wire             m_mem_axi_arvalid,
    input  wire             m_mem_axi_arready,

    input  wire [7 : 0]     m_mem_axi_rid,
    input  wire [31 : 0]    m_mem_axi_rdata,
    input  wire [ 1 : 0]    m_mem_axi_rresp,
    input  wire             m_mem_axi_rlast,
    input  wire             m_mem_axi_rvalid,
    output wire             m_mem_axi_rready,

    output wire             dvi_red,
    output wire             dvi_green,
    output wire             dvi_blue,
    output wire             dvi_clock
);
    assign m_mem_axi_arsize = 2;
    assign m_mem_axi_arlock = 0;
    assign m_mem_axi_arcache = 0;
    assign m_mem_axi_arprot = 0;
    wire [31 : 0] araddr;
    reg  [31 : 0] addr;
    assign m_mem_axi_araddr = araddr + addr;

    dvi_framebuffer #(
        .VIDEO_WIDTH(1024),
        .VIDEO_HEIGHT(600),
        .VIDEO_REFRESH(60),
        .VIDEO_ENABLE(1),
        .VIDEO_X2_MODE(0),
        .VIDEO_FB_RAM(0)
    ) dvi (
        .clk_i(aclk),
        .rst_i(!resetn),
        .clk_x5_i(aclk5x),
        .cfg_awvalid_i(0),
        .cfg_awaddr_i(0),
        .cfg_wvalid_i(0),
        .cfg_wdata_i(0),
        .cfg_wstrb_i(0),
        .cfg_bready_i(0),
        .cfg_arvalid_i(0),
        .cfg_araddr_i(0),
        .cfg_rready_i(0),
        .outport_awready_i(0),
        .outport_wready_i(0),
        .outport_bvalid_i(0),
        .outport_bresp_i(0),
        .outport_bid_i(0),
        .outport_arready_i(m_mem_axi_arready),
        .outport_rvalid_i(m_mem_axi_rvalid),
        .outport_rdata_i(m_mem_axi_rdata),
        .outport_rresp_i(m_mem_axi_rresp),
        .outport_rid_i(m_mem_axi_rid[0 +: 4]),
        .outport_rlast_i(m_mem_axi_rlast),
        .cfg_awready_o(),
        .cfg_wready_o(),
        .cfg_bvalid_o(),
        .cfg_bresp_o(),
        .cfg_arready_o(),
        .cfg_rvalid_o(),
        .cfg_rdata_o(),
        .cfg_rresp_o(),
        .intr_o(),
        .outport_awvalid_o(),
        .outport_awaddr_o(),
        .outport_awid_o(),
        .outport_awlen_o(),
        .outport_awburst_o(),
        .outport_wvalid_o(),
        .outport_wdata_o(),
        .outport_wstrb_o(),
        .outport_wlast_o(),
        .outport_bready_o(),

        .outport_arvalid_o(m_mem_axi_arvalid),
        .outport_araddr_o(araddr),
        .outport_arid_o(m_mem_axi_arid),
        .outport_arlen_o(m_mem_axi_arlen),
        .outport_arburst_o(m_mem_axi_arburst),
        .outport_rready_o(m_mem_axi_rready),
        .dvi_red_o(dvi_red),
        .dvi_green_o(dvi_green),
        .dvi_blue_o(dvi_blue),
        .dvi_clock_o(dvi_clock)
    );

    // Quick and dirty double buffer algorithm for the DVI.
    // Note: It should use the configuration interface of the DVI core and
    // also there is no clock domain crossing here ...
    // But for now it works good enough.
    always @(posedge aclkLogic)
    begin
        if (!resetn)
        begin
            addr <= FB_1_ADDR;
            swapped <= 1;
        end
        else
        begin
            if (swap && swapped)
            begin
                if (addr == FB_1_ADDR)
                begin
                    addr <= FB_2_ADDR;
                end
                else
                begin
                    addr <= FB_1_ADDR;
                end
                swapped <= 0;
            end
            if (!swap)
            begin
                swapped <= 1;
            end
        end
    end

endmodule