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

// Receives a stream from a framebuffer and writes it into the memory
module AxisFramebufferWriter #(
    // Width of the axi interfaces
    parameter DATA_WIDTH = 32,
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of wstrb (width of data bus in words)
    parameter STRB_WIDTH = 4,
    // Width of ID signal
    parameter ID_WIDTH = 8,

    localparam FB_SIZE_IN_PIXEL_LG = 20,
    localparam PIXEL_SIZE = 16
) (
    input  wire                                 aclk,
    input  wire                                 resetn,

    input  wire                                 commit_fb,
    input  wire [ADDR_WIDTH - 1 : 0]            fb_addr,
    input  wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]   fb_size,
    output reg                                  fb_committed,

    // Display port
    input  wire                                 s_disp_axis_tvalid,
    output reg                                  s_disp_axis_tready,
    input  wire                                 s_disp_axis_tlast,
    input  wire [DATA_WIDTH - 1 : 0]            s_disp_axis_tdata,
    input  wire [STRB_WIDTH - 1 : 0]            s_disp_axis_tstrb,

    // Memory port
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
    localparam AWLEN = 15;
    localparam AWSIZE = $clog2(DATA_WIDTH / 8);

    reg  [FB_SIZE_IN_PIXEL_LG - 1 : 0]  dataSizeInBytes;
    reg  [FB_SIZE_IN_PIXEL_LG - 1 : 0]  counter;
    wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]  counterNext = counter + (DATA_WIDTH / PIXEL_SIZE);
    wire                                lastSignal = ((counterNext >> (AWSIZE - $clog2(PIXEL_SIZE / 8))) & AWLEN) == 0;
    wire                                transferEnd = counterNext > dataSizeInBytes;

    assign m_mem_axi_bready = 1;

    reg  startAddressGeneration;
    wire addressGenerationDone;

    reg  [DATA_WIDTH - 1 : 0]   wdataSkid;
    reg  [STRB_WIDTH - 1 : 0]   wstrbSkid;
    reg                         wlastSkid;
    reg                         wvalidSkid;
    reg                         skidValid;

    LinearAddressGenerator #(
        .ADDR_WIDTH(ADDR_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .AxLEN_BEATS_PER_TRANSFER(AWLEN),
        .AxSIZE_BYTES_PER_BEAT(AWSIZE)
    ) addressGenerator (
        .aclk(aclk),
        .resetn(resetn),

        .start(startAddressGeneration),
        .done(addressGenerationDone),

        .startAddr(fb_addr),
        .dataSizeInBytes({ 11'h0, fb_size, 1'b0 }),

        .axid(m_mem_axi_awid),
        .axaddr(m_mem_axi_awaddr),
        .axlen(m_mem_axi_awlen),
        .axsize(m_mem_axi_awsize),
        .axburst(m_mem_axi_awburst),
        .axlock(m_mem_axi_awlock),
        .axcache(m_mem_axi_awcache),
        .axprot(m_mem_axi_awprot),
        .axvalid(m_mem_axi_awvalid),
        .axready(m_mem_axi_awready)
    );

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            fb_committed <= 1;
            startAddressGeneration <= 0;
            s_disp_axis_tready <= 0;
            m_mem_axi_wvalid <= 0;
            skidValid <= 0;
        end
        else
        begin
            if (commit_fb && addressGenerationDone)
            begin
                fb_committed <= 0;
                counter <= 0;
                s_disp_axis_tready <= 1;
                dataSizeInBytes <= fb_size;
                startAddressGeneration <= 1;
            end

            if (!fb_committed)
            begin
                if (skidValid && m_mem_axi_wready)
                begin
                    m_mem_axi_wdata <= wdataSkid; 
                    m_mem_axi_wvalid <= wvalidSkid; 
                    m_mem_axi_wstrb <= wstrbSkid; 
                    m_mem_axi_wlast <= wlastSkid;
                    skidValid <= 0;
                    s_disp_axis_tready <= !transferEnd; 
                end
                else if (!m_mem_axi_wvalid || m_mem_axi_wready)
                begin
                    m_mem_axi_wdata <= s_disp_axis_tdata;
                    m_mem_axi_wvalid <= s_disp_axis_tvalid;
                    m_mem_axi_wstrb <= s_disp_axis_tstrb;
                    m_mem_axi_wlast <= lastSignal;
                    s_disp_axis_tready <= !transferEnd;
                    if (s_disp_axis_tvalid)
                        counter <= counterNext;
                end
                else if (!skidValid && !m_mem_axi_wready && m_mem_axi_wvalid && s_disp_axis_tvalid)
                begin
                    wdataSkid <= s_disp_axis_tdata;
                    wvalidSkid <= s_disp_axis_tvalid;
                    wstrbSkid <= s_disp_axis_tstrb;
                    wlastSkid <= lastSignal;
                    s_disp_axis_tready <= 0;
                    skidValid <= 1;
                    counter <= counterNext;
                end

                if (transferEnd && m_mem_axi_wready && m_mem_axi_wvalid && !skidValid)
                begin
                    fb_committed <= 1;
                    m_mem_axi_wvalid <= 0;
                    s_disp_axis_tready <= 0;
                end
            end

            if (startAddressGeneration && !addressGenerationDone)
            begin
                startAddressGeneration <= 0;
            end
        end
    end
endmodule