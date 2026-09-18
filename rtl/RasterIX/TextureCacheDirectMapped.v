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

module TextureCacheDirectMapped #(
    parameter TEXEL_WIDTH = 16,

    parameter CACHE_SIZE = 1024,
    parameter CACHE_LINE_SIZE = 32,
    parameter ENABLE_EARLY_FETCH = 1,

    parameter DATA_WIDTH = 32,
    parameter ID_WIDTH = 4,
    parameter ADDR_WIDTH = 18,

    parameter COMMAND_FIFO_DEPTH_POW2 = 5,
    parameter AXI_R_FIFO_DEPTH_POW2 = 5
)
(
    input wire                              aclk,
    input wire                              resetn,

    input wire                              invalidate,

    input wire [ADDR_WIDTH - 1 : 0]         s_tc_addr,
    input wire                              s_tc_valid,
    output wire                             s_tc_ready,

    output wire [TEXEL_WIDTH - 1 : 0]       m_tc_texel,
    output wire                             m_tc_valid,
    input wire                              m_tc_ready,

    output wire [ID_WIDTH - 1 : 0]          m_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_axi_araddr,
    output wire [7 : 0]                     m_axi_arlen,
    output wire [2 : 0]                     m_axi_arsize,
    output wire [1 : 0]                     m_axi_arburst,
    output wire                             m_axi_arlock,
    output wire [3 : 0]                     m_axi_arcache,
    output wire [2 : 0]                     m_axi_arprot,
    output wire                             m_axi_arvalid,
    input wire                              m_axi_arready,

    input wire [ID_WIDTH - 1 : 0]           m_axi_rid,
    input wire [DATA_WIDTH - 1 : 0]         m_axi_rdata,
    input wire [1 : 0]                      m_axi_rresp,
    input wire                              m_axi_rlast,
    input wire                              m_axi_rvalid,
    output wire                             m_axi_rready
);
    localparam COMMAND_WIDTH = 1 + ADDR_WIDTH;
    localparam AXI_R_WIDTH = ID_WIDTH + DATA_WIDTH + 2 + 1;

    wire                               controller_valid;
    wire                               controller_ready;
    wire                               controller_cmd;
    wire [ADDR_WIDTH - 1 : 0]          controller_addr;

    wire                               context_s_ready;
    wire                               context_m_axi_rready;

    wire                               command_fifo_full;
    wire                               command_fifo_empty;
    wire                               command_fifo_cmd;
    wire [ADDR_WIDTH - 1 : 0]          command_fifo_addr;

    wire                               axi_r_fifo_full;
    wire                               axi_r_fifo_empty;
    wire [ID_WIDTH - 1 : 0]            axi_r_fifo_rid;
    wire [DATA_WIDTH - 1 : 0]          axi_r_fifo_rdata;
    wire [1 : 0]                       axi_r_fifo_rresp;
    wire                               axi_r_fifo_rlast;

    assign m_axi_rready = AXI_R_FIFO_DEPTH_POW2 == 0 ? context_m_axi_rready : !axi_r_fifo_full;

    TextureCacheDirectMappedController #(
        .ADDR_WIDTH(ADDR_WIDTH),
        .CACHE_SIZE(CACHE_SIZE),
        .CACHE_LINE_SIZE(CACHE_LINE_SIZE),
        .DATA_WIDTH(DATA_WIDTH),
        .ID_WIDTH(ID_WIDTH)
    ) controller (
        .aclk(aclk),
        .resetn(resetn),
        .invalidate(invalidate),
        .s_tc_addr(s_tc_addr),
        .s_tc_valid(s_tc_valid),
        .s_tc_ready(s_tc_ready),
        .m_tc_valid(controller_valid),
        .m_tc_ready(controller_ready),
        .m_tc_cmd(controller_cmd),
        .m_tc_addr(controller_addr),
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

    TextureCacheDirectMappedContext #(
        .ADDR_WIDTH(ADDR_WIDTH),
        .TEXEL_WIDTH(TEXEL_WIDTH),
        .CACHE_SIZE(CACHE_SIZE),
        .CACHE_LINE_SIZE(CACHE_LINE_SIZE),
        .ENABLE_EARLY_FETCH(ENABLE_EARLY_FETCH),
        .DATA_WIDTH(DATA_WIDTH),
        .ID_WIDTH(ID_WIDTH)
    ) cache_context (
        .aclk(aclk),
        .resetn(resetn),
        .s_tc_valid(!command_fifo_empty),
        .s_tc_ready(context_s_ready),
        .s_tc_cmd(command_fifo_cmd),
        .s_tc_addr(command_fifo_addr),
        .m_tc_texel(m_tc_texel),
        .m_tc_valid(m_tc_valid),
        .m_tc_ready(m_tc_ready),
        .m_axi_rid(axi_r_fifo_rid),
        .m_axi_rdata(axi_r_fifo_rdata),
        .m_axi_rresp(axi_r_fifo_rresp),
        .m_axi_rlast(axi_r_fifo_rlast),
        .m_axi_rvalid(!axi_r_fifo_empty),
        .m_axi_rready(context_m_axi_rready)
    );

    assign controller_ready = COMMAND_FIFO_DEPTH_POW2 == 0 ? context_s_ready : !command_fifo_full;

    generate
        if (COMMAND_FIFO_DEPTH_POW2 == 0)
        begin
            assign command_fifo_cmd = controller_cmd;
            assign command_fifo_addr = controller_addr;
            assign command_fifo_empty = !controller_valid;
            assign command_fifo_full = 1'b0;
        end
        else
        begin
            sfifo #(
                .BW(COMMAND_WIDTH),
                .LGFLEN(COMMAND_FIFO_DEPTH_POW2),
                .OPT_ASYNC_READ(0),
                .OPT_WRITE_ON_FULL(0),
                .OPT_READ_ON_EMPTY(0)
            ) command_fifo (
                .i_clk(aclk),
                .i_reset(!resetn),
                .i_wr(controller_valid && controller_ready),
                .i_data({ 
                    controller_addr, 
                    controller_cmd 
                }),
                .o_full(command_fifo_full),
                .o_fill(),
                .i_rd(!command_fifo_empty && context_s_ready),
                .o_data({ 
                    command_fifo_addr, 
                    command_fifo_cmd 
                }),
                .o_empty(command_fifo_empty)
            );
        end
    endgenerate

    generate
        if (AXI_R_FIFO_DEPTH_POW2 == 0)
        begin
            assign { 
                axi_r_fifo_rlast, 
                axi_r_fifo_rresp,
                axi_r_fifo_rdata, 
                axi_r_fifo_rid 
            } = { 
                m_axi_rlast, 
                m_axi_rresp, 
                m_axi_rdata, 
                m_axi_rid 
            };
            assign axi_r_fifo_empty = !m_axi_rvalid;
            assign axi_r_fifo_full = 1'b0;
        end
        else
        begin
            sfifo #(
                .BW(AXI_R_WIDTH),
                .LGFLEN(AXI_R_FIFO_DEPTH_POW2),
                .OPT_ASYNC_READ(0),
                .OPT_WRITE_ON_FULL(0),
                .OPT_READ_ON_EMPTY(0)
            ) axi_r_fifo (
                .i_clk(aclk),
                .i_reset(!resetn),
                .i_wr(m_axi_rvalid && m_axi_rready),
                .i_data({ 
                    m_axi_rlast, 
                    m_axi_rresp, 
                    m_axi_rdata, 
                    m_axi_rid 
                }),
                .o_full(axi_r_fifo_full),
                .o_fill(),
                .i_rd(!axi_r_fifo_empty && context_m_axi_rready),
                .o_data({ 
                    axi_r_fifo_rlast, 
                    axi_r_fifo_rresp,
                    axi_r_fifo_rdata, 
                    axi_r_fifo_rid 
                }),
                .o_empty(axi_r_fifo_empty)
            );
        end
    endgenerate
endmodule
