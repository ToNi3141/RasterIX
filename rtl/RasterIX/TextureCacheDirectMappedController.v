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
    // Direct mapped cache. The cache is split into two modules:
    //    - Cache Controller: Controls the cache misses and loading of cache lines from memory
    //    - Cache Context: Stores the actual cache lines, loads the data from the cache and provides it to is master interface.
    // 1. Cache miss:
    //    - Slave port stalls
    //    - Memory request is issued
    //    - Cache load command is send to the context
    //    - Cache access command is send to the context
    // 2. Cache hit:
    //    - Cache access command is send immediately to the context
    //    - Do skid buffering if necessary
    // 3. Invalidate complete cache:
    //    - Remove in a loop all tags. This can take several clock cycles.
    // Note: This cache is read only. No write back strategies are required.
    //       This is a direct mapped cache, also no cache replacement policies are needed.

    localparam READ_CACHE_ENTRY = 1'b0;
    localparam LOAD_CACHE_LINE = 1'b1;

    function [TAG_WIDTH - 1 : 0] getTagFromAddress;
        input [ADDR_WIDTH - 1 : 0] addr;
        begin
            getTagFromAddress = addr[$clog2(CACHE_LINE_SIZE) + $clog2(CACHE_LINES) +: TAG_WIDTH];
        end
    endfunction

    function [INDEX_WIDTH - 1 : 0] getIndexFromAddress;
        input [ADDR_WIDTH - 1 : 0] addr;
        begin
            getIndexFromAddress = addr[$clog2(CACHE_LINE_SIZE) +: $clog2(CACHE_LINES)];
        end
    endfunction

    function [ADDR_WIDTH - 1 : 0] getBaseAddress;
        input [ADDR_WIDTH - 1 : 0] addr;
        begin
            getBaseAddress = addr & { {(ADDR_WIDTH - $clog2(CACHE_LINE_SIZE)) { 1'b1 }},
                                      {$clog2(CACHE_LINE_SIZE) { 1'b0 }} };
        end
    endfunction

    function [0 : 0] tagMatch;
        input [TAG_ENTRY_WIDTH - 1 : 0] tag_entry;
        input [ADDR_WIDTH - 1 : 0] addr;
        begin
            tagMatch = (tag_entry[0 +: TAG_WIDTH] == getTagFromAddress(addr)) && tag_entry[TAG_ENTRY_WIDTH - 1];
        end
    endfunction

    assign m_axi_arid = 0;
    assign m_axi_arlen = (CACHE_LINE_SIZE / (DATA_WIDTH / 8)) - 1;
    assign m_axi_arsize = ARSIZE[0 +: 3];
    assign m_axi_arburst = 2'b01; // INCR burst type
    assign m_axi_arlock = 0;
    assign m_axi_arcache = 0;
    assign m_axi_arprot = 0;

    reg [TAG_ENTRY_WIDTH - 1 : 0]   r_tag_entires [0 : CACHE_LINES - 1];

    reg                             r_invalidate;
    reg [INDEX_WIDTH - 1 : 0]       r_i;

    reg                             r_skid_valid;
    reg  [ADDR_WIDTH - 1 : 0]       r_skid_addr;
    reg                             r_load_line_pending;

    wire [ADDR_WIDTH - 1 : 0]       w_addr;


    assign w_addr = (r_skid_valid) ? r_skid_addr
                                   : s_tc_addr;

    

    always @(posedge aclk) 
    begin
        if (!resetn) 
        begin
            r_invalidate <= 1'b1;
            r_i <= { INDEX_WIDTH { 1'b0 } };
            r_skid_valid <= 1'b0;
            r_load_line_pending <= 1'b0;
            s_tc_ready <= 1'b0;
            m_tc_valid <= 1'b0;
            m_axi_arvalid <= 1'b0;
        end 
        else 
        begin
            if (invalidate || r_invalidate)
            begin
                r_invalidate <= 1'b1;
                if (r_i != { INDEX_WIDTH { 1'b1 } })
                begin
                    r_i <= r_i + 1'b1;
                    r_tag_entires[r_i] <= { TAG_ENTRY_WIDTH { 1'b0 } };
                end
                else
                begin
                    r_tag_entires[r_i] <= { TAG_ENTRY_WIDTH { 1'b0 } };
                    r_invalidate <= 1'b0;
                    r_i <= { INDEX_WIDTH { 1'b0 } };
                    s_tc_ready <= !r_skid_valid;
                end
                
                if (s_tc_valid && s_tc_ready)
                begin
                    r_skid_valid <= 1'b1;
                    r_skid_addr <= s_tc_addr;
                    s_tc_ready <= 1'b0;
                end
                if (m_tc_valid && m_tc_ready)
                begin
                    m_tc_valid <= 1'b0;
                end
            end
            else if (r_load_line_pending && m_tc_valid && m_tc_ready)
            begin
                r_load_line_pending <= 1'b0;
                m_tc_valid <= 1'b0;
            end
            else if ((s_tc_valid || r_skid_valid) && (!m_tc_valid || m_tc_ready) && !m_axi_arvalid)
            begin            
                m_tc_valid <= 1'b1;
                m_tc_addr <= w_addr;
                m_tc_cmd <= READ_CACHE_ENTRY;
                r_skid_valid <= 1'b0;

                s_tc_ready <= 1'b1;

                // Check for cache miss
                if (!tagMatch(r_tag_entires[getIndexFromAddress(w_addr)], w_addr)) 
                begin
                    m_tc_cmd <= LOAD_CACHE_LINE;
                    
                    r_skid_valid <= 1'b1;
                    r_skid_addr <= w_addr;
                    r_load_line_pending <= 1'b1;
                    s_tc_ready <= 1'b0;
                    
                    m_axi_arvalid <= 1'b1;
                    m_axi_araddr <= getBaseAddress(w_addr);
                    
                    r_tag_entires[getIndexFromAddress(w_addr)] <= { 1'b1, getTagFromAddress(w_addr) };
                end
            end
            else if (m_tc_valid && !m_tc_ready && s_tc_valid && s_tc_ready)
            begin
                r_skid_addr <= w_addr;
                r_skid_valid <= 1'b1;
                s_tc_ready <= 1'b0;
            end
            else if (!s_tc_valid && !r_skid_valid && (!m_tc_valid || m_tc_ready))
            begin
                m_tc_valid <= 1'b0;
                s_tc_ready <= 1'b1;
                r_skid_valid <= 1'b0;
            end
        end

        if (m_axi_arvalid && m_axi_arready)
        begin
            m_axi_arvalid <= 1'b0;
        end
    end

endmodule 