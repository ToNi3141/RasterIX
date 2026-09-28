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

// Direct-mapped texture cache which interfaces with the AXI memory system.
// This cache is read only. It has no write back policy implemented.
// Pipelined: yes
// Depth: 1
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
    input wire                              enable,

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

    wire                        controllerValid;
    wire                        controllerReady;
    wire                        controllerCmd;
    wire [ADDR_WIDTH - 1 : 0]   controllerAddr;

    wire                        contextSlaveReady;
    wire                        contextMasterReady;

    wire                        commandFifoFull;
    wire                        commandFifoEmpty;
    wire                        commandFifoCmd;
    wire [ADDR_WIDTH - 1 : 0]   commandFifoAddr;

    wire                        readFifoFull;
    wire                        readFifoEmpty;
    wire [ID_WIDTH - 1 : 0]     readFifoId;
    wire [DATA_WIDTH - 1 : 0]   readFifoData;
    wire [1 : 0]                readFifoResponse;
    wire                        readFifoLast;

    assign m_axi_rready = AXI_R_FIFO_DEPTH_POW2 == 0 ? contextMasterReady : !readFifoFull;

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
        .enable(enable),
        .s_tc_addr(s_tc_addr),
        .s_tc_valid(s_tc_valid),
        .s_tc_ready(s_tc_ready),
        .m_tc_valid(controllerValid),
        .m_tc_ready(controllerReady),
        .m_tc_cmd(controllerCmd),
        .m_tc_addr(controllerAddr),
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
        .s_tc_valid(!commandFifoEmpty),
        .s_tc_ready(contextSlaveReady),
        .s_tc_cmd(commandFifoCmd),
        .s_tc_addr(commandFifoAddr),
        .m_tc_texel(m_tc_texel),
        .m_tc_valid(m_tc_valid),
        .m_tc_ready(m_tc_ready),
        .m_axi_rid(readFifoId),
        .m_axi_rdata(readFifoData),
        .m_axi_rresp(readFifoResponse),
        .m_axi_rlast(readFifoLast),
        .m_axi_rvalid(!readFifoEmpty),
        .m_axi_rready(contextMasterReady)
    );

    assign controllerReady = COMMAND_FIFO_DEPTH_POW2 == 0 ? contextSlaveReady : !commandFifoFull;

    generate
        if (COMMAND_FIFO_DEPTH_POW2 == 0)
        begin
            assign commandFifoCmd = controllerCmd;
            assign commandFifoAddr = controllerAddr;
            assign commandFifoEmpty = !controllerValid;
            assign commandFifoFull = 1'b0;
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
                .i_wr(controllerValid && controllerReady),
                .i_data({ 
                    controllerAddr,
                    controllerCmd
                }),
                .o_full(commandFifoFull),
                .o_fill(),
                .i_rd(!commandFifoEmpty && contextSlaveReady),
                .o_data({ 
                    commandFifoAddr,
                    commandFifoCmd
                }),
                .o_empty(commandFifoEmpty)
            );
        end
    endgenerate

    generate
        if (AXI_R_FIFO_DEPTH_POW2 == 0)
        begin
            assign { 
                readFifoLast,
                readFifoResponse,
                readFifoData,
                readFifoId
            } = { 
                m_axi_rlast, 
                m_axi_rresp, 
                m_axi_rdata, 
                m_axi_rid 
            };
            assign readFifoEmpty = !m_axi_rvalid;
            assign readFifoFull = 1'b0;
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
                .o_full(readFifoFull),
                .o_fill(),
                .i_rd(!readFifoEmpty && contextMasterReady),
                .o_data({ 
                    readFifoLast,
                    readFifoResponse,
                    readFifoData,
                    readFifoId
                }),
                .o_empty(readFifoEmpty)
            );
        end
    endgenerate
endmodule
