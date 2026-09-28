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

// Direct-mapped texture cache controller which manages the cache.
// It checks for cache hits and misses and loads new data into the 
// context or overwrites data from the context (TextureCacheDirectMappedContext).
// Pipelined: yes
// Depth: 1
module TextureCacheDirectMappedController #(
    parameter CACHE_SIZE = 1024,
    parameter CACHE_LINE_SIZE = 32,
    localparam CACHE_LINES = CACHE_SIZE / CACHE_LINE_SIZE,

    parameter DATA_WIDTH = 16,
    parameter ID_WIDTH = 4,
    parameter ADDR_WIDTH = 18,

    localparam TAG_WIDTH = ADDR_WIDTH - $clog2(CACHE_LINE_SIZE) - $clog2(CACHE_LINES),
    localparam TAG_ENTRY_WIDTH = TAG_WIDTH + 1,
    localparam INDEX_WIDTH = $clog2(CACHE_LINES),
    localparam ARSIZE = $clog2(DATA_WIDTH / 8)
)
(
    input  wire                             aclk,
    input  wire                             resetn,

    input  wire                             invalidate,
    input  wire                             enable,

    // Input interface
    input  wire [ADDR_WIDTH - 1 : 0]        s_tc_addr,
    input  wire                             s_tc_valid,
    output reg                              s_tc_ready,

    output reg                              m_tc_valid,
    input  wire                             m_tc_ready,
    output reg                              m_tc_cmd,
    output reg  [ADDR_WIDTH - 1 : 0]        m_tc_addr,

    // Output interface
    output wire [ID_WIDTH - 1 : 0]          m_axi_arid,
    output reg  [ADDR_WIDTH - 1 : 0]        m_axi_araddr,
    output wire [ 7 : 0]                    m_axi_arlen,
    output wire [ 2 : 0]                    m_axi_arsize,
    output wire [ 1 : 0]                    m_axi_arburst,
    output wire                             m_axi_arlock,
    output wire [ 3 : 0]                    m_axi_arcache,
    output wire [ 2 : 0]                    m_axi_arprot,
    output reg                              m_axi_arvalid,
    input  wire                             m_axi_arready
);
    localparam READ_CACHE_ENTRY = 1'b0;
    localparam LOAD_CACHE_LINE = 1'b1;

    function [TAG_WIDTH - 1 : 0] GetTagFromAddress;
        input [ADDR_WIDTH - 1 : 0] addr;
        begin
            GetTagFromAddress = addr[$clog2(CACHE_LINE_SIZE) + $clog2(CACHE_LINES) +: TAG_WIDTH];
        end
    endfunction

    function [INDEX_WIDTH - 1 : 0] GetIndexFromAddress;
        input [ADDR_WIDTH - 1 : 0] addr;
        begin
            GetIndexFromAddress = addr[$clog2(CACHE_LINE_SIZE) +: $clog2(CACHE_LINES)];
        end
    endfunction

    function [ADDR_WIDTH - 1 : 0] GetBaseAddress;
        input [ADDR_WIDTH - 1 : 0] addr;
        begin
            GetBaseAddress = addr & { {(ADDR_WIDTH - $clog2(CACHE_LINE_SIZE)) { 1'b1 }},
                                      {$clog2(CACHE_LINE_SIZE) { 1'b0 }} };
        end
    endfunction

    function [0 : 0] TagMatch;
        input [TAG_ENTRY_WIDTH - 1 : 0] tagEntry;
        input [ADDR_WIDTH - 1 : 0] addr;
        begin
            TagMatch = !enable || ((tagEntry[0 +: TAG_WIDTH] == GetTagFromAddress(addr)) && tagEntry[TAG_ENTRY_WIDTH - 1]);
        end
    endfunction

    assign m_axi_arid = 0;
    assign m_axi_arlen = (CACHE_LINE_SIZE / (DATA_WIDTH / 8)) - 1;
    assign m_axi_arsize = ARSIZE[0 +: 3];
    assign m_axi_arburst = 2'b01; // INCR burst type
    assign m_axi_arlock = 0;
    assign m_axi_arcache = 0;
    assign m_axi_arprot = 0;

    reg [TAG_ENTRY_WIDTH - 1 : 0]   rTagEntries [0 : CACHE_LINES - 1];

    reg                             rInvalidate;
    reg [INDEX_WIDTH - 1 : 0]       rI;

    reg                             rSkidValid;
    reg  [ADDR_WIDTH - 1 : 0]       rSkidAddr;
    reg                             rLoadLinePending;

    wire [ADDR_WIDTH - 1 : 0]       wAddr;

    assign wAddr = (rSkidValid) ? rSkidAddr
                                : s_tc_addr;

    always @(posedge aclk) 
    begin
        if (!resetn) 
        begin
            rInvalidate <= 1'b1;
            rI <= { INDEX_WIDTH { 1'b0 } };
            rSkidValid <= 1'b0;
            rLoadLinePending <= 1'b0;
            s_tc_ready <= 1'b0;
            m_tc_valid <= 1'b0;
            m_axi_arvalid <= 1'b0;
        end 
        else 
        begin
            if (invalidate || rInvalidate)
            begin
                rInvalidate <= 1'b1;
                if (rI != { INDEX_WIDTH { 1'b1 } })
                begin
                    rI <= rI + 1'b1;
                    rTagEntries[rI] <= { TAG_ENTRY_WIDTH { 1'b0 } };
                end
                else
                begin
                    rTagEntries[rI] <= { TAG_ENTRY_WIDTH { 1'b0 } };
                    rInvalidate <= 1'b0;
                    rI <= { INDEX_WIDTH { 1'b0 } };
                    s_tc_ready <= !rSkidValid;
                end
                
                if (s_tc_valid && s_tc_ready)
                begin
                    rSkidValid <= 1'b1;
                    rSkidAddr <= s_tc_addr;
                    s_tc_ready <= 1'b0;
                end
                if (m_tc_valid && m_tc_ready)
                begin
                    m_tc_valid <= 1'b0;
                end
            end
            else if (rLoadLinePending && m_tc_valid && m_tc_ready)
            begin
                rLoadLinePending <= 1'b0;
                m_tc_valid <= 1'b0;
            end
            else if ((s_tc_valid || rSkidValid) && (!m_tc_valid || m_tc_ready) && !m_axi_arvalid)
            begin            
                m_tc_valid <= 1'b1;
                m_tc_addr <= wAddr;
                m_tc_cmd <= READ_CACHE_ENTRY;
                rSkidValid <= 1'b0;

                s_tc_ready <= 1'b1;

                if (!TagMatch(rTagEntries[GetIndexFromAddress(wAddr)], wAddr))
                begin
                    m_tc_cmd <= LOAD_CACHE_LINE;
                    
                    rSkidValid <= 1'b1;
                    rSkidAddr <= wAddr;
                    rLoadLinePending <= 1'b1;
                    s_tc_ready <= 1'b0;
                    
                    m_axi_arvalid <= 1'b1;
                    m_axi_araddr <= GetBaseAddress(wAddr);
                    
                    rTagEntries[GetIndexFromAddress(wAddr)] <= { 1'b1, GetTagFromAddress(wAddr) };
                end
            end
            else if (m_tc_valid && !m_tc_ready && s_tc_valid && s_tc_ready)
            begin
                rSkidAddr <= wAddr;
                rSkidValid <= 1'b1;
                s_tc_ready <= 1'b0;
            end
            else if (!s_tc_valid && !rSkidValid && (!m_tc_valid || m_tc_ready))
            begin
                m_tc_valid <= 1'b0;
                s_tc_ready <= 1'b1;
                rSkidValid <= 1'b0;
            end
        end

        if (m_axi_arvalid && m_axi_arready)
        begin
            m_axi_arvalid <= 1'b0;
        end
    end

endmodule 