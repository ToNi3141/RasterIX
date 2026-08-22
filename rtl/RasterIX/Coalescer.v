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

// Coalesces AXI transfers consolidating small AXI transactions into one big.
// Write requests are temporary stored in an internal FiFo until a 
// new coalesced write transfer is triggered.
// Same counts for the read channel.
module Coalescer #(
    parameter ID_WIDTH   = 4,
    parameter ADDR_WIDTH = 32,
    parameter DATA_WIDTH = 32,
    parameter STRB_WIDTH = DATA_WIDTH / 8,
    parameter MAX_BEATS_TO_COALESCE = 8
) 
(
    input  wire                                 aclk,
    input  wire                                 resetn,

    // Slave Memory Interface
    input  wire [ID_WIDTH - 1 : 0]              s_mem_axi_awid,
    input  wire [ADDR_WIDTH - 1 : 0]            s_mem_axi_awaddr,
    input  wire [ 7 : 0]                        s_mem_axi_awlen, 
    input  wire [ 2 : 0]                        s_mem_axi_awsize, 
    input  wire [ 1 : 0]                        s_mem_axi_awburst, 
    input  wire                                 s_mem_axi_awlock,
    input  wire [ 3 : 0]                        s_mem_axi_awcache,
    input  wire [ 2 : 0]                        s_mem_axi_awprot, 
    input  wire                                 s_mem_axi_awvalid,
    output wire                                 s_mem_axi_awready,

    input  wire [DATA_WIDTH - 1 : 0]            s_mem_axi_wdata,
    input  wire [STRB_WIDTH - 1 : 0]            s_mem_axi_wstrb,
    input  wire                                 s_mem_axi_wlast,
    input  wire                                 s_mem_axi_wvalid,
    output wire                                 s_mem_axi_wready,

    output wire [ID_WIDTH - 1 : 0]              s_mem_axi_bid,
    output wire [ 1 : 0]                        s_mem_axi_bresp,
    output wire                                 s_mem_axi_bvalid,
    input  wire                                 s_mem_axi_bready,

    input  wire [ID_WIDTH - 1 : 0]              s_mem_axi_arid,
    input  wire [ADDR_WIDTH - 1 : 0]            s_mem_axi_araddr,
    input  wire [ 7 : 0]                        s_mem_axi_arlen,
    input  wire [ 2 : 0]                        s_mem_axi_arsize,
    input  wire [ 1 : 0]                        s_mem_axi_arburst,
    input  wire                                 s_mem_axi_arlock,
    input  wire [ 3 : 0]                        s_mem_axi_arcache,
    input  wire [ 2 : 0]                        s_mem_axi_arprot,
    input  wire                                 s_mem_axi_arvalid,
    output wire                                 s_mem_axi_arready,

    output wire [ID_WIDTH - 1 : 0]              s_mem_axi_rid,
    output wire [DATA_WIDTH - 1 : 0]            s_mem_axi_rdata,
    output wire [ 1 : 0]                        s_mem_axi_rresp,
    output wire                                 s_mem_axi_rlast,
    output wire                                 s_mem_axi_rvalid,
    input  wire                                 s_mem_axi_rready,

    // Master Memory Interface
    output wire [ID_WIDTH - 1 : 0]              m_mem_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]            m_mem_axi_awaddr,
    output wire [ 7 : 0]                        m_mem_axi_awlen, 
    output wire [ 2 : 0]                        m_mem_axi_awsize, 
    output wire [ 1 : 0]                        m_mem_axi_awburst, 
    output wire                                 m_mem_axi_awlock,
    output wire [ 3 : 0]                        m_mem_axi_awcache,
    output wire [ 2 : 0]                        m_mem_axi_awprot, 
    output wire                                 m_mem_axi_awvalid,
    input  wire                                 m_mem_axi_awready,

    output wire [DATA_WIDTH - 1 : 0]            m_mem_axi_wdata,
    output wire [STRB_WIDTH - 1 : 0]            m_mem_axi_wstrb,
    output wire                                 m_mem_axi_wlast,
    output wire                                 m_mem_axi_wvalid,
    input  wire                                 m_mem_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]              m_mem_axi_bid,
    input  wire [ 1 : 0]                        m_mem_axi_bresp,
    input  wire                                 m_mem_axi_bvalid,
    output wire                                 m_mem_axi_bready,

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

    assign s_mem_axi_rdata = m_mem_axi_rdata;
    assign s_mem_axi_rid = m_mem_axi_rid;
    assign s_mem_axi_rresp = m_mem_axi_rresp;
    assign s_mem_axi_rlast = 1; // We always return one beat, so this is always the last beat
    assign s_mem_axi_rvalid = m_mem_axi_rvalid;
    assign m_mem_axi_rready = s_mem_axi_rready;

    generate
        if (MAX_BEATS_TO_COALESCE > 1)
        begin : gen_coalesce
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

            // Read Channel
            CoalesceAddrGen #(
                .ID_WIDTH(ID_WIDTH),
                .ADDR_WIDTH(ADDR_WIDTH),
                .DATA_WIDTH(DATA_WIDTH),
                .MAX_BEATS_TO_COALESCE(MAX_BEATS_TO_COALESCE)
            ) coalesce_addr_read_gen_inst (
                .aclk(aclk),
                .resetn(resetn),

                .s_mem_axi_axid(s_mem_axi_arid),
                .s_mem_axi_axaddr(s_mem_axi_araddr),
                .s_mem_axi_axlen(s_mem_axi_arlen),
                .s_mem_axi_axsize(s_mem_axi_arsize),
                .s_mem_axi_axburst(s_mem_axi_arburst),
                .s_mem_axi_axlock(s_mem_axi_arlock),
                .s_mem_axi_axcache(s_mem_axi_arcache),
                .s_mem_axi_axprot(s_mem_axi_arprot),
                .s_mem_axi_axvalid(s_mem_axi_arvalid),
                .s_mem_axi_axready(s_mem_axi_arready),

                .m_mem_axi_axid(m_mem_axi_arid),
                .m_mem_axi_axaddr(m_mem_axi_araddr),
                .m_mem_axi_axlen(m_mem_axi_arlen),
                .m_mem_axi_axsize(m_mem_axi_arsize),
                .m_mem_axi_axburst(m_mem_axi_arburst),
                .m_mem_axi_axlock(m_mem_axi_arlock),
                .m_mem_axi_axcache(m_mem_axi_arcache),
                .m_mem_axi_axprot(m_mem_axi_arprot),
                .m_mem_axi_axvalid(m_mem_axi_arvalid),
                .m_mem_axi_axready(m_mem_axi_arready)
            );

            // Write Channel
            CoalesceAddrGen #(
                .ID_WIDTH(ID_WIDTH),
                .ADDR_WIDTH(ADDR_WIDTH),
                .DATA_WIDTH(DATA_WIDTH),
                .MAX_BEATS_TO_COALESCE(MAX_BEATS_TO_COALESCE)
            ) coalesce_addr_write_gen_inst (
                .aclk(aclk),
                .resetn(resetn),

                .s_mem_axi_axid(s_mem_axi_awid),
                .s_mem_axi_axaddr(s_mem_axi_awaddr),
                .s_mem_axi_axlen(s_mem_axi_awlen),
                .s_mem_axi_axsize(s_mem_axi_awsize),
                .s_mem_axi_axburst(s_mem_axi_awburst),
                .s_mem_axi_axlock(s_mem_axi_awlock),
                .s_mem_axi_axcache(s_mem_axi_awcache),
                .s_mem_axi_axprot(s_mem_axi_awprot),
                .s_mem_axi_axvalid(s_mem_axi_awvalid),
                .s_mem_axi_axready(s_mem_axi_awready),

                .m_mem_axi_axid(mem_axi_awid),
                .m_mem_axi_axaddr(mem_axi_awaddr),
                .m_mem_axi_axlen(mem_axi_awlen),
                .m_mem_axi_axsize(mem_axi_awsize),
                .m_mem_axi_axburst(mem_axi_awburst),
                .m_mem_axi_axlock(mem_axi_awlock),
                .m_mem_axi_axcache(mem_axi_awcache),
                .m_mem_axi_axprot(mem_axi_awprot),
                .m_mem_axi_axvalid(mem_axi_awvalid),
                .m_mem_axi_axready(mem_axi_awready)
            );

            CoalesceFiFo #(
                .ID_WIDTH(ID_WIDTH),
                .ADDR_WIDTH(ADDR_WIDTH),
                .DATA_WIDTH(DATA_WIDTH),
                .STRB_WIDTH(STRB_WIDTH),
                .MAX_BEATS_TO_COALESCE(MAX_BEATS_TO_COALESCE)
            ) coalesce_fifo_write_inst (
                .aclk(aclk),
                .resetn(resetn),

                .s_mem_axi_awid(mem_axi_awid),
                .s_mem_axi_awaddr(mem_axi_awaddr),
                .s_mem_axi_awlen(mem_axi_awlen),
                .s_mem_axi_awsize(mem_axi_awsize),
                .s_mem_axi_awburst(mem_axi_awburst),
                .s_mem_axi_awlock(mem_axi_awlock),
                .s_mem_axi_awcache(mem_axi_awcache),
                .s_mem_axi_awprot(mem_axi_awprot),
                .s_mem_axi_awvalid(mem_axi_awvalid),
                .s_mem_axi_awready(mem_axi_awready),

                .s_mem_axi_wdata(s_mem_axi_wdata),
                .s_mem_axi_wstrb(s_mem_axi_wstrb),
                .s_mem_axi_wlast(s_mem_axi_wlast),
                .s_mem_axi_wvalid(s_mem_axi_wvalid),
                .s_mem_axi_wready(s_mem_axi_wready),

                .s_mem_axi_bid(s_mem_axi_bid),
                .s_mem_axi_bresp(s_mem_axi_bresp),
                .s_mem_axi_bvalid(s_mem_axi_bvalid),
                .s_mem_axi_bready(s_mem_axi_bready),

                .m_mem_axi_awid(m_mem_axi_awid),
                .m_mem_axi_awaddr(m_mem_axi_awaddr),
                .m_mem_axi_awlen(m_mem_axi_awlen),
                .m_mem_axi_awsize(m_mem_axi_awsize),
                .m_mem_axi_awburst(m_mem_axi_awburst),
                .m_mem_axi_awlock(m_mem_axi_awlock),
                .m_mem_axi_awcache(m_mem_axi_awcache),
                .m_mem_axi_awprot(m_mem_axi_awprot),
                .m_mem_axi_awvalid(m_mem_axi_awvalid),
                .m_mem_axi_awready(m_mem_axi_awready),

                .m_mem_axi_wdata(m_mem_axi_wdata),
                .m_mem_axi_wstrb(m_mem_axi_wstrb),
                .m_mem_axi_wlast(m_mem_axi_wlast),
                .m_mem_axi_wvalid(m_mem_axi_wvalid),
                .m_mem_axi_wready(m_mem_axi_wready),

                .m_mem_axi_bid(m_mem_axi_bid),
                .m_mem_axi_bresp(m_mem_axi_bresp),
                .m_mem_axi_bvalid(m_mem_axi_bvalid),
                .m_mem_axi_bready(m_mem_axi_bready)
            );
        end
        else
        begin : gen_bypass
            // Bypass coalescing entirely: connect slave AW/W/B/AR channels directly to master
            assign m_mem_axi_awid = s_mem_axi_awid;
            assign m_mem_axi_awaddr = s_mem_axi_awaddr;
            assign m_mem_axi_awlen = s_mem_axi_awlen;
            assign m_mem_axi_awsize = s_mem_axi_awsize;
            assign m_mem_axi_awburst = s_mem_axi_awburst;
            assign m_mem_axi_awlock = s_mem_axi_awlock;
            assign m_mem_axi_awcache = s_mem_axi_awcache;
            assign m_mem_axi_awprot = s_mem_axi_awprot;
            assign m_mem_axi_awvalid = s_mem_axi_awvalid;
            assign s_mem_axi_awready = m_mem_axi_awready;

            assign m_mem_axi_wdata = s_mem_axi_wdata;
            assign m_mem_axi_wstrb = s_mem_axi_wstrb;
            assign m_mem_axi_wlast = s_mem_axi_wlast;
            assign m_mem_axi_wvalid = s_mem_axi_wvalid;
            assign s_mem_axi_wready = m_mem_axi_wready;

            assign s_mem_axi_bid = m_mem_axi_bid;
            assign s_mem_axi_bresp = m_mem_axi_bresp;
            assign s_mem_axi_bvalid = m_mem_axi_bvalid;
            assign m_mem_axi_bready = s_mem_axi_bready;

            assign m_mem_axi_arid = s_mem_axi_arid;
            assign m_mem_axi_araddr = s_mem_axi_araddr;
            assign m_mem_axi_arlen = s_mem_axi_arlen;
            assign m_mem_axi_arsize = s_mem_axi_arsize;
            assign m_mem_axi_arburst = s_mem_axi_arburst;
            assign m_mem_axi_arlock = s_mem_axi_arlock;
            assign m_mem_axi_arcache = s_mem_axi_arcache;
            assign m_mem_axi_arprot = s_mem_axi_arprot;
            assign m_mem_axi_arvalid = s_mem_axi_arvalid;
            assign s_mem_axi_arready = m_mem_axi_arready;
        end
    endgenerate

endmodule