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

// Converts a pixel stream of PIXEL_WIDTH pixels into DATA_WIDTH transactions.
// It can only upsize. Downsizing is not possible.
module FramebufferAdapter 
#(
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
)
(
    input  wire                             aclk,
    input  wire                             resetn,

    /////////////////////////
    // Read Side - Fragment Interface
    /////////////////////////

    // Fetch interface (input)
    input  wire                             s_fetch_arvalid,
    input  wire                             s_fetch_arlast,
    output wire                             s_fetch_arready,
    input  wire [ADDR_WIDTH - 1 : 0]        s_fetch_araddr,

    // Framebuffer read interface (output)
    output wire                             s_frag_rvalid,
    input  wire                             s_frag_rready,
    output wire [PIXEL_WIDTH - 1 : 0]       s_frag_rdata,
    output wire                             s_frag_rlast,

    /////////////////////////
    // Write Side - Fragment Interface
    /////////////////////////

    // Framebuffer write interface (input)
    input  wire                             s_frag_wvalid,
    input  wire                             s_frag_wlast,
    output wire                             s_frag_wready,
    input  wire [PIXEL_WIDTH - 1 : 0]       s_frag_wdata,
    input  wire [(PIXEL_WIDTH / 8) - 1 : 0] s_frag_wstrb,
    input  wire [ADDR_WIDTH - 1 : 0]        s_frag_waddr,

    /////////////////////////
    // Memory Interface - AXI Read Channel
    /////////////////////////

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

    input  wire [ID_WIDTH - 1 : 0]          m_mem_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]        m_mem_axi_rdata,
    input  wire [ 1 : 0]                    m_mem_axi_rresp,
    input  wire                             m_mem_axi_rlast,
    input  wire                             m_mem_axi_rvalid,
    output wire                             m_mem_axi_rready,

    /////////////////////////
    // Memory Interface - AXI Write Channel
    /////////////////////////

    output wire [ID_WIDTH - 1 : 0]          m_mem_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]        m_mem_axi_awaddr,
    output wire [ 7 : 0]                    m_mem_axi_awlen,
    output wire [ 2 : 0]                    m_mem_axi_awsize,
    output wire [ 1 : 0]                    m_mem_axi_awburst,
    output wire                             m_mem_axi_awlock,
    output wire [ 3 : 0]                    m_mem_axi_awcache,
    output wire [ 2 : 0]                    m_mem_axi_awprot,
    output wire                             m_mem_axi_awvalid,
    input  wire                             m_mem_axi_awready,

    output wire [DATA_WIDTH - 1 : 0]        m_mem_axi_wdata,
    output wire [STRB_WIDTH - 1 : 0]        m_mem_axi_wstrb,
    output wire                             m_mem_axi_wlast,
    output wire                             m_mem_axi_wvalid,
    input  wire                             m_mem_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]          m_mem_axi_bid,
    input  wire [ 1 : 0]                    m_mem_axi_bresp,
    input  wire                             m_mem_axi_bvalid,
    output wire                             m_mem_axi_bready
);

    generate
        if (DATA_WIDTH < PIXEL_WIDTH) begin
            $error("FramebufferAdapter: DATA_WIDTH must be >= PIXEL_WIDTH. DATA_WIDTH=%0d, PIXEL_WIDTH=%0d", DATA_WIDTH, PIXEL_WIDTH);
        end else if (DATA_WIDTH == PIXEL_WIDTH) begin
            // Pass-through mode: DATA_WIDTH == PIXEL_WIDTH
            // No conversion needed - direct connection of read and write paths

            assign s_frag_rvalid = m_mem_axi_rvalid;
            assign s_frag_rdata = m_mem_axi_rdata[PIXEL_WIDTH - 1 : 0];
            assign s_frag_rlast = m_mem_axi_rlast;
            assign m_mem_axi_rready = s_frag_rready;

            assign m_mem_axi_arid = 8'b0;
            assign m_mem_axi_araddr = s_fetch_araddr;
            assign m_mem_axi_arlen = 8'b0;
            assign m_mem_axi_arsize = $clog2(STRB_WIDTH);
            assign m_mem_axi_arburst = 2'b01;  // INCR
            assign m_mem_axi_arlock = 1'b0;
            assign m_mem_axi_arcache = 4'b0;
            assign m_mem_axi_arprot = 3'b0;
            assign m_mem_axi_arvalid = s_fetch_arvalid;
            assign s_fetch_arready = m_mem_axi_arready;

            // Broadcast the fragment write signals to both AW and W channels
            localparam USER_WIDTH_FRAG = ADDR_WIDTH + (PIXEL_WIDTH / 8);
            wire [ 1 : 0] bcast_m_axis_tvalid;
            wire [ 1 : 0] bcast_m_axis_tready;
            wire [2 * PIXEL_WIDTH - 1 : 0] bcast_m_axis_tdata;
            wire [2 * USER_WIDTH_FRAG - 1 : 0] bcast_m_axis_tuser;
            
            axis_broadcast #(
                .M_COUNT(2),
                .DATA_WIDTH(PIXEL_WIDTH),
                .KEEP_ENABLE(0),
                .LAST_ENABLE(1),
                .ID_ENABLE(0),
                .DEST_ENABLE(0),
                .USER_ENABLE(1),
                .USER_WIDTH(USER_WIDTH_FRAG)
            ) frag_write_broadcaster (
                .clk(aclk),
                .rst(~resetn),
                
                .s_axis_tdata(s_frag_wdata),
                .s_axis_tkeep(1'b1),
                .s_axis_tvalid(s_frag_wvalid),
                .s_axis_tready(s_frag_wready),
                .s_axis_tlast(s_frag_wlast),
                .s_axis_tid(8'b0),
                .s_axis_tdest(8'b0),
                .s_axis_tuser({s_frag_wstrb, s_frag_waddr}),
                
                .m_axis_tdata(bcast_m_axis_tdata),
                .m_axis_tkeep(),
                .m_axis_tvalid(bcast_m_axis_tvalid),
                .m_axis_tready(bcast_m_axis_tready),
                .m_axis_tlast(),
                .m_axis_tid(),
                .m_axis_tdest(),
                .m_axis_tuser(bcast_m_axis_tuser)
            );

            assign bcast_m_axis_tready[0] = m_mem_axi_awready;  // AW channel
            assign bcast_m_axis_tready[1] = m_mem_axi_wready;   // W channel

            wire [ADDR_WIDTH - 1 : 0]        bcast_awaddr_0 = bcast_m_axis_tuser[0 +: ADDR_WIDTH];
            wire [(PIXEL_WIDTH / 8) - 1 : 0] bcast_wstrb_0 = bcast_m_axis_tuser[ADDR_WIDTH +: (PIXEL_WIDTH / 8)];
            
            wire [PIXEL_WIDTH - 1 : 0]       bcast_wdata_1 = bcast_m_axis_tdata[PIXEL_WIDTH +: PIXEL_WIDTH];
            wire [ADDR_WIDTH - 1 : 0 ]       bcast_awaddr_1 = bcast_m_axis_tuser[USER_WIDTH_FRAG +: ADDR_WIDTH];
            wire [(PIXEL_WIDTH / 8) -1  : 0] bcast_wstrb_1 = bcast_m_axis_tuser[USER_WIDTH_FRAG + ADDR_WIDTH +: (PIXEL_WIDTH / 8)];
            assign m_mem_axi_awid = 8'b0;
            assign m_mem_axi_awaddr = bcast_awaddr_0;
            assign m_mem_axi_awlen = 8'b0;
            assign m_mem_axi_awsize = $clog2(STRB_WIDTH);
            assign m_mem_axi_awburst = 2'b01;  // INCR
            assign m_mem_axi_awlock = 1'b0;
            assign m_mem_axi_awcache = 4'b0;
            assign m_mem_axi_awprot = 3'b0;
            assign m_mem_axi_awvalid = bcast_m_axis_tvalid[0];
            
            assign m_mem_axi_wdata = bcast_wdata_1;
            assign m_mem_axi_wstrb = bcast_wstrb_1;
            assign m_mem_axi_wlast = 1'b1;
            assign m_mem_axi_wvalid = bcast_m_axis_tvalid[1];

            assign m_mem_axi_bready = 1'b1;

        end else begin
            // Upsize mode: DATA_WIDTH > PIXEL_WIDTH
            // Use FramebufferReader and FramebufferPacker for packing/unpacking

            FramebufferReader #(
                .DATA_WIDTH(DATA_WIDTH),
                .ADDR_WIDTH(ADDR_WIDTH),
                .STRB_WIDTH(STRB_WIDTH),
                .ID_WIDTH(ID_WIDTH),
                .PIXEL_WIDTH(PIXEL_WIDTH)
            ) reader (
                .aclk(aclk),
                .resetn(resetn),

                .s_fetch_tvalid(s_fetch_arvalid),
                .s_fetch_tlast(s_fetch_arlast),
                .s_fetch_tready(s_fetch_arready),
                .s_fetch_taddr(s_fetch_araddr),

                .m_frag_tvalid(s_frag_rvalid),
                .m_frag_tready(s_frag_rready),
                .m_frag_tdata(s_frag_rdata),
                .m_frag_taddr(),
                .m_frag_tlast(s_frag_rlast),

                .m_mem_axi_arid(m_mem_axi_arid),
                .m_mem_axi_araddr(m_mem_axi_araddr),
                .m_mem_axi_arlen(m_mem_axi_arlen),
                .m_mem_axi_arsize(m_mem_axi_arsize),
                .m_mem_axi_arburst(m_mem_axi_arburst),
                .m_mem_axi_arlock(m_mem_axi_arlock),
                .m_mem_axi_arcache(m_mem_axi_arcache),
                .m_mem_axi_arprot(m_mem_axi_arprot),
                .m_mem_axi_arvalid(m_mem_axi_arvalid),
                .m_mem_axi_arready(m_mem_axi_arready),

                .m_mem_axi_rid(m_mem_axi_rid),
                .m_mem_axi_rdata(m_mem_axi_rdata),
                .m_mem_axi_rresp(m_mem_axi_rresp),
                .m_mem_axi_rlast(m_mem_axi_rlast),
                .m_mem_axi_rvalid(m_mem_axi_rvalid),
                .m_mem_axi_rready(m_mem_axi_rready)
            );

            FramebufferPacker #(
                .DATA_WIDTH(DATA_WIDTH),
                .ADDR_WIDTH(ADDR_WIDTH),
                .STRB_WIDTH(STRB_WIDTH),
                .ID_WIDTH(ID_WIDTH),
                .PIXEL_WIDTH(PIXEL_WIDTH)
            ) writer (
                .aclk(aclk),
                .resetn(resetn),

                .s_frag_tvalid(s_frag_wvalid),
                .s_frag_tlast(s_frag_wlast),
                .s_frag_tready(s_frag_wready),
                .s_frag_tdata(s_frag_wdata),
                .s_frag_tstrb(s_frag_wstrb),
                .s_frag_taddr(s_frag_waddr),

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
    endgenerate
endmodule
