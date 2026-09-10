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
    parameter TEX_ADDR_WIDTH = 17,
    parameter TEXEL_WIDTH = 16,

    parameter CACHE_SIZE = 1024,
    parameter CACHE_LINE_SIZE = 32,

    parameter DATA_WIDTH = 32,
    parameter ID_WIDTH = 4,
    parameter ADDR_WIDTH = 32,

    parameter COMMAND_FIFO_DEPTH_POW2 = 5,
    parameter AXI_R_FIFO_DEPTH_POW2 = 5
)
(
    input wire                              aclk,
    input wire                              resetn,

    input wire                              invalidate,

    input wire [TEX_ADDR_WIDTH - 1 : 0]     s_araddr,
    input wire                              s_arvalid,
    output wire                             s_arready,

    output wire [TEXEL_WIDTH - 1 : 0]       m_texel,
    output wire                             m_valid,
    input wire                              m_ready,

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
    localparam COMMAND_WIDTH = 1 + TEX_ADDR_WIDTH;
    localparam AXI_R_WIDTH = ID_WIDTH + DATA_WIDTH + 2 + 1;

    wire                               controller_valid;
    wire                               controller_ready;
    wire                               controller_cmd;
    wire [TEX_ADDR_WIDTH - 1 : 0]      controller_addr;

    wire                               context_valid;
    wire                               context_ready;
    wire                               context_cmd;
    wire [TEX_ADDR_WIDTH - 1 : 0]      context_addr;
    wire                               context_s_ready;
    wire                               context_m_axi_rready;

    wire [COMMAND_WIDTH - 1 : 0]       command_fifo_data_in;
    wire [COMMAND_WIDTH - 1 : 0]       command_fifo_data_out;
    wire                               command_fifo_full;
    wire                               command_fifo_empty;
    wire                               command_fifo_write;
    wire                               command_fifo_read;

    wire [AXI_R_WIDTH - 1 : 0]         axi_r_fifo_data_in;
    wire [AXI_R_WIDTH - 1 : 0]         axi_r_fifo_data_out;
    wire                               axi_r_fifo_full;
    wire                               axi_r_fifo_empty;
    wire                               axi_r_fifo_write;
    wire                               axi_r_fifo_read;
    wire                               m_axi_rvalid_to_context;
    wire [ID_WIDTH - 1 : 0]            m_axi_rid_to_context;
    wire [DATA_WIDTH - 1 : 0]          m_axi_rdata_to_context;
    wire [1 : 0]                       m_axi_rresp_to_context;
    wire                               m_axi_rlast_to_context;
    wire                               m_axi_rready_to_context;

    assign command_fifo_data_in = { controller_addr, controller_cmd };
    assign { context_addr, context_cmd } = command_fifo_data_out;
    assign command_fifo_write = controller_valid && controller_ready;
    assign command_fifo_read = context_valid && context_ready;

    assign axi_r_fifo_data_in = { m_axi_rlast, m_axi_rresp, m_axi_rdata, m_axi_rid };
    assign { m_axi_rlast_to_context, m_axi_rresp_to_context,
             m_axi_rdata_to_context, m_axi_rid_to_context } = axi_r_fifo_data_out;
    assign axi_r_fifo_write = m_axi_rvalid && m_axi_rready;
    assign axi_r_fifo_read = m_axi_rvalid_to_context && m_axi_rready_to_context;

    assign context_valid = !command_fifo_empty;
    assign context_ready = context_s_ready;
    assign m_axi_rvalid_to_context = !axi_r_fifo_empty;
    assign m_axi_rready_to_context = context_m_axi_rready;

    assign m_axi_rready = AXI_R_FIFO_DEPTH_POW2 == 0 ? context_m_axi_rready : !axi_r_fifo_full;

    TextureCacheDirectMappedController #(
        .TEX_ADDR_WIDTH(TEX_ADDR_WIDTH),
        .CACHE_SIZE(CACHE_SIZE),
        .CACHE_LINE_SIZE(CACHE_LINE_SIZE),
        .DATA_WIDTH(DATA_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH)
    ) controller (
        .aclk(aclk),
        .resetn(resetn),
        .invalidate(invalidate),
        .s_araddr(s_araddr),
        .s_arvalid(s_arvalid),
        .s_arready(s_arready),
        .m_valid(controller_valid),
        .m_ready(controller_ready),
        .m_cmd(controller_cmd),
        .m_addr(controller_addr),
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
        .TEX_ADDR_WIDTH(TEX_ADDR_WIDTH),
        .TEXEL_WIDTH(TEXEL_WIDTH),
        .CACHE_SIZE(CACHE_SIZE),
        .CACHE_LINE_SIZE(CACHE_LINE_SIZE),
        .DATA_WIDTH(DATA_WIDTH),
        .ID_WIDTH(ID_WIDTH)
    ) cache_context (
        .aclk(aclk),
        .resetn(resetn),
        .s_valid(context_valid),
        .s_ready(context_s_ready),
        .s_cmd(context_cmd),
        .s_addr(context_addr),
        .m_texel(m_texel),
        .m_valid(m_valid),
        .m_ready(m_ready),
        .m_axi_rid(m_axi_rid_to_context),
        .m_axi_rdata(m_axi_rdata_to_context),
        .m_axi_rresp(m_axi_rresp_to_context),
        .m_axi_rlast(m_axi_rlast_to_context),
        .m_axi_rvalid(m_axi_rvalid_to_context),
        .m_axi_rready(context_m_axi_rready)
    );

    assign controller_ready = COMMAND_FIFO_DEPTH_POW2 == 0 ? context_ready : !command_fifo_full;

    generate
        if (COMMAND_FIFO_DEPTH_POW2 == 0)
        begin
            assign command_fifo_data_out = command_fifo_data_in;
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
                .i_wr(command_fifo_write),
                .i_data(command_fifo_data_in),
                .o_full(command_fifo_full),
                .o_fill(),
                .i_rd(command_fifo_read),
                .o_data(command_fifo_data_out),
                .o_empty(command_fifo_empty)
            );
        end
    endgenerate

    generate
        if (AXI_R_FIFO_DEPTH_POW2 == 0)
        begin
            assign axi_r_fifo_data_out = axi_r_fifo_data_in;
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
                .i_wr(axi_r_fifo_write),
                .i_data(axi_r_fifo_data_in),
                .o_full(axi_r_fifo_full),
                .o_fill(),
                .i_rd(axi_r_fifo_read),
                .o_data(axi_r_fifo_data_out),
                .o_empty(axi_r_fifo_empty)
            );
        end
    endgenerate
endmodule
