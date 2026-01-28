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
// It takes from the fb_addr the address where to read the framebuffer.
// A read is started when swap_fb is set to high and is acknowledged
// by a transition from fb_swapped from 0 to 1. 
// Node: An acknowledged framebuffer does not mean that it is fully
// transferred. It just means, that this module starts to transfer
// it to the display.
module AxisFramebufferReader #(
    // Width of the display stream
    parameter DISPLAY_STREAM_WIDTH = 16,

    // Width of the axi interfaces
    parameter DATA_WIDTH = 32,
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of wstrb (width of data bus in words)
    parameter STRB_WIDTH = 4,
    // Width of ID signal
    parameter ID_WIDTH = 8,
    // When this parameter is set, the swap is acknowledged when the 
    // framebuffer is completely streamed
    parameter BLOCKING = 1,

    localparam FB_SIZE_IN_PIXEL_LG = 20
) (
    input  wire                                 aclk,
    input  wire                                 resetn,

    input  wire                                 swap_fb,
    input  wire [ADDR_WIDTH - 1 : 0]            fb_addr,
    input  wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]   fb_size,
    output reg                                  fb_swapped,

    // Display port
    output wire                                 m_disp_axis_tvalid,
    input  wire                                 m_disp_axis_tready,
    output wire                                 m_disp_axis_tlast,
    output wire [DISPLAY_STREAM_WIDTH - 1 : 0]  m_disp_axis_tdata,

    // Memory port
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
    reg                         st0_axis_tvalid;
    wire                        st0_axis_tready;
    reg                         st0_axis_tlast;
    reg [DATA_WIDTH - 1 : 0]    st0_axis_tdata;

    wire                        disp_axis_tvalid;
    wire                        disp_axis_tready;
    wire                        disp_axis_tlast;
    wire [DATA_WIDTH - 1 : 0]   disp_axis_tdata;

    localparam STATE_CMD = 0;
    localparam STATE_ADDR = 1;
    localparam STATE_WAIT_DSE = 2;
    localparam STATE_NOP = 3;
    reg [ 1 : 0] state;

    FrameStreamingCore #(
        .STREAM_WIDTH(DATA_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .STRB_WIDTH(STRB_WIDTH),
        .ID_WIDTH(ID_WIDTH)
    ) fte (
        .aclk(aclk),
        .resetn(resetn),
        
        .m_st0_axis_tvalid(disp_axis_tvalid),
        .m_st0_axis_tready(disp_axis_tready),
        .m_st0_axis_tlast(disp_axis_tlast),
        .m_st0_axis_tdata(disp_axis_tdata),
        .s_st0_axis_tvalid(st0_axis_tvalid),
        .s_st0_axis_tready(st0_axis_tready),
        .s_st0_axis_tlast(st0_axis_tlast),
        .s_st0_axis_tdata(st0_axis_tdata),

        .m_st1_axis_tvalid(),
        .m_st1_axis_tready(0),
        .m_st1_axis_tlast(),
        .m_st1_axis_tdata(),
        .s_st1_axis_tvalid(0),
        .s_st1_axis_tready(),
        .s_st1_axis_tlast(0),
        .s_st1_axis_tdata(0),

        .m_mem_axi_awid(),
        .m_mem_axi_awaddr(),
        .m_mem_axi_awlen(), 
        .m_mem_axi_awsize(),
        .m_mem_axi_awburst(),
        .m_mem_axi_awlock(),
        .m_mem_axi_awcache(),
        .m_mem_axi_awprot(), 
        .m_mem_axi_awvalid(),
        .m_mem_axi_awready(0),

        .m_mem_axi_wdata(),
        .m_mem_axi_wstrb(),
        .m_mem_axi_wlast(),
        .m_mem_axi_wvalid(),
        .m_mem_axi_wready(0),

        .m_mem_axi_bid(0),
        .m_mem_axi_bresp(0),
        .m_mem_axi_bvalid(0),
        .m_mem_axi_bready(),

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

    axis_adapter #(
        .S_DATA_WIDTH(DATA_WIDTH), 
        .S_KEEP_ENABLE(1), 
        .S_KEEP_WIDTH(DATA_WIDTH / 16),
        .M_DATA_WIDTH(DISPLAY_STREAM_WIDTH), 
        .M_KEEP_ENABLE(1),
        .M_KEEP_WIDTH(DISPLAY_STREAM_WIDTH / 16),
        .ID_ENABLE(0), 
        .DEST_ENABLE(0),
        .USER_ENABLE(0)
    ) adapter (
        .clk(aclk),
        .rst(!resetn),

        .s_axis_tdata(disp_axis_tdata),
        .s_axis_tkeep(~0),
        .s_axis_tvalid(disp_axis_tvalid),
        .s_axis_tready(disp_axis_tready),
        .s_axis_tlast(disp_axis_tlast),

        .m_axis_tdata(m_disp_axis_tdata),
        .m_axis_tkeep(),
        .m_axis_tvalid(m_disp_axis_tvalid),
        .m_axis_tready(m_disp_axis_tready),
        .m_axis_tlast(m_disp_axis_tlast)
    );

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            state <= STATE_CMD;
            fb_swapped <= 1;
            st0_axis_tvalid <= 0;
            st0_axis_tlast <= 0;
        end
        else
        begin
            case (state)
                STATE_CMD:
                begin
                    if (swap_fb)
                    begin
                        st0_axis_tdata <= { 2'h1, 2'h3, 7'h0, fb_size, 1'h0 }; // fb_size is the size of the FB in 16 bit pixel
                        st0_axis_tvalid <= 1;
                        st0_axis_tlast <= 0;
                        fb_swapped <= 0;
                        state <= STATE_ADDR;
                    end
                end
                STATE_ADDR: 
                begin
                    if (st0_axis_tready)
                    begin
                        st0_axis_tdata <= fb_addr;
                        st0_axis_tlast <= 1;
                        if (BLOCKING == 0)
                        begin
                            fb_swapped <= 1; // Early acknowledge to the framebuffer
                        end
                        state <= STATE_WAIT_DSE;
                    end
                end
                STATE_WAIT_DSE:
                begin
                    if ((BLOCKING == 1) && st0_axis_tready)
                    begin
                        // Use a NOP as fence
                        st0_axis_tdata <= 0; // NOP
                        st0_axis_tlast <= 1;
                        state <= STATE_NOP;
                    end

                    if ((BLOCKING == 0) && st0_axis_tready)
                    begin
                        st0_axis_tvalid <= 0;
                        st0_axis_tlast <= 0;
                        fb_swapped <= 1;
                        state <= STATE_CMD;
                    end
                end
                STATE_NOP:
                begin
                    if (st0_axis_tready)
                    begin
                        st0_axis_tvalid <= 0;
                        st0_axis_tlast <= 0;
                        fb_swapped <= 1;
                        state <= STATE_CMD;
                    end
                end
                default: 
                begin
                end
            endcase
        end
    end
endmodule