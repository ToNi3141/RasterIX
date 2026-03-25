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

module CoalesceFiFo #(
    parameter ID_WIDTH   = 4,
    parameter ADDR_WIDTH = 32,
    parameter DATA_WIDTH = 32,
    parameter STRB_WIDTH = DATA_WIDTH / 8,
    parameter MAX_BEATS_TO_COALESCE = 8
) 
(
    input  wire                                 aclk,
    input  wire                                 resetn,

    input  wire [ID_WIDTH - 1 : 0]              s_mem_axi_awid,
    input  wire [ADDR_WIDTH - 1 : 0]            s_mem_axi_awaddr,
    input  wire [ 7 : 0]                        s_mem_axi_awlen, // How many beats are in this transaction
    input  wire [ 2 : 0]                        s_mem_axi_awsize, // The increment during one cycle. Means, 0 incs addrStart by 1, 2 by 4 and so on
    input  wire [ 1 : 0]                        s_mem_axi_awburst, // 0 fixed, 1 incr, 2 wrapping
    input  wire                                 s_mem_axi_awlock,
    input  wire [ 3 : 0]                        s_mem_axi_awcache,
    input  wire [ 2 : 0]                        s_mem_axi_awprot, 
    input  wire                                 s_mem_axi_awvalid,
    output reg                                  s_mem_axi_awready,

    input  wire [DATA_WIDTH - 1 : 0]            s_mem_axi_wdata,
    input  wire [STRB_WIDTH - 1 : 0]            s_mem_axi_wstrb,
    input  wire                                 s_mem_axi_wlast,
    input  wire                                 s_mem_axi_wvalid,
    output wire                                 s_mem_axi_wready,

    output wire [ID_WIDTH - 1 : 0]              s_mem_axi_bid,
    output wire [ 1 : 0]                        s_mem_axi_bresp,
    output wire                                 s_mem_axi_bvalid,
    input  wire                                 s_mem_axi_bready,

    // Output
    output reg  [ID_WIDTH - 1 : 0]              m_mem_axi_awid,
    output reg  [ADDR_WIDTH - 1 : 0]            m_mem_axi_awaddr,
    output reg  [ 7 : 0]                        m_mem_axi_awlen, // How many beats are in this transaction
    output wire [ 2 : 0]                        m_mem_axi_awsize, // The increment during one cycle. Means, 0 incs addrStart by 1, 2 by 4 and so on
    output wire [ 1 : 0]                        m_mem_axi_awburst, // 0 fixed, 1 incr, 2 wrapping
    output wire                                 m_mem_axi_awlock,
    output wire [ 3 : 0]                        m_mem_axi_awcache,
    output wire [ 2 : 0]                        m_mem_axi_awprot, 
    output reg                                  m_mem_axi_awvalid,
    input  reg                                  m_mem_axi_awready,

    output reg  [DATA_WIDTH - 1 : 0]            m_mem_axi_wdata,
    output reg  [STRB_WIDTH - 1 : 0]            m_mem_axi_wstrb,
    output reg                                  m_mem_axi_wlast,
    output reg                                  m_mem_axi_wvalid,
    input  wire                                 m_mem_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]              m_mem_axi_bid,
    input  wire [ 1 : 0]                        m_mem_axi_bresp,
    input  wire                                 m_mem_axi_bvalid,
    output wire                                 m_mem_axi_bready
);
    localparam DATA_WIDTH_LG = $clog2(DATA_WIDTH / 8);

    wire fifo_full;

    wire [DATA_WIDTH - 1 : 0]   fifo_wdata;
    wire [STRB_WIDTH - 1 : 0]   fifo_wstrb;
    wire                        fifo_empty;
    reg                         fifo_read;
    wire                        fifo_ready;

    reg  [DATA_WIDTH - 1: 0]    skid_wdata;
    reg  [STRB_WIDTH - 1 : 0]   skid_wstrb;
    reg                         skid_last;
    reg                         skid_valid;

    reg [ 7 : 0]                awcount;
    
    sfifo dataFiFo (
        .i_clk(aclk),
        .i_reset(!resetn),

        .i_wr(s_mem_axi_wvalid),
        .i_data({ 
            s_mem_axi_wdata,
            s_mem_axi_wstrb
        }),
        .o_full(fifo_full),
        .o_fill(),

        .i_rd(fifo_read),
        .o_data({
            fifo_wdata,
            fifo_wstrb
        }),
        .o_empty(fifo_empty)    
    );
    defparam dataFiFo.BW = DATA_WIDTH + STRB_WIDTH;
    defparam dataFiFo.LGFLEN = $clog2(MAX_BEATS_TO_COALESCE);
    defparam dataFiFo.OPT_ASYNC_READ = 0;

    assign fifo_ready = !fifo_empty;

    assign s_mem_axi_wready = !fifo_full;
    assign m_mem_axi_bready = 1'b1;
    assign m_mem_axi_awsize = DATA_WIDTH_LG[0 +: 3];
    assign m_mem_axi_awburst = 1;
    assign m_mem_axi_awlock = 0;
    assign m_mem_axi_awcache = 0;
    assign m_mem_axi_awprot = 0;

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            fifo_read <= 0;
            s_mem_axi_awready <= 0;
            m_mem_axi_awvalid <= 0;
            m_mem_axi_wvalid <= 0;
            m_mem_axi_awid <= 0;
            skid_valid <= 0;
            awcount <= 0;
        end
        else
        begin
            if (s_mem_axi_awvalid && !s_mem_axi_awready && !m_mem_axi_awvalid && !skid_valid && (awcount == 0))
            begin
                s_mem_axi_awready <= 1;
                m_mem_axi_awaddr <= s_mem_axi_awaddr;
                m_mem_axi_awlen <= s_mem_axi_awlen;
                m_mem_axi_awid <= m_mem_axi_awid + 1;
                m_mem_axi_awvalid <= 1;

                awcount <= s_mem_axi_awlen + 1;

                if (s_mem_axi_awburst != 1)
                begin
                    $error("Only incrementing burst is supported");
                    $finish;
                end

                fifo_read <= 1;
            end

            if (s_mem_axi_awready)
            begin
                s_mem_axi_awready <= 0;
            end

            if (m_mem_axi_awvalid && m_mem_axi_awready)
            begin
                m_mem_axi_awvalid <= 0;
            end

            if ((awcount != 0) || skid_valid)
            begin
                if (!m_mem_axi_wvalid || (m_mem_axi_wvalid && m_mem_axi_wready))
                begin
                    if (skid_valid)
                    begin
                        m_mem_axi_wdata <= skid_wdata;
                        m_mem_axi_wstrb <= skid_wstrb;
                        m_mem_axi_wlast <= skid_last;
                        m_mem_axi_wvalid <= 1;
                        skid_valid <= 0;
                        fifo_read <= (awcount > 0);
                    end
                    else
                    begin
                        m_mem_axi_wdata <= fifo_wdata;
                        m_mem_axi_wstrb <= fifo_wstrb;
                        m_mem_axi_wlast <= (awcount == 1);
                        m_mem_axi_wvalid <= fifo_ready;
                        if (fifo_ready)
                        begin
                            awcount <= awcount - 1;
                        end
                        fifo_read <= (awcount > 1);
                    end
                end
                else if (m_mem_axi_wvalid && !m_mem_axi_wready && fifo_ready && !skid_valid)
                begin
                    skid_wdata <= fifo_wdata;
                    skid_wstrb <= fifo_wstrb;
                    skid_last <= (awcount == 1);
                    skid_valid <= 1;
                    fifo_read <= 0;
                    awcount <= awcount - 1;
                end
            end
            else
            begin
                if (m_mem_axi_wready && m_mem_axi_wvalid)
                begin
                    m_mem_axi_wvalid <= 0;
                end
            end
        end
    end

endmodule