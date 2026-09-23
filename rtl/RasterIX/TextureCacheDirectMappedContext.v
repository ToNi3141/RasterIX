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

// Direct-mapped texture cache context which manages the cache and handles read requests.
// This is the counterpart to the TextureCacheDirectMappedController which manages the cache.
// Pipelined: yes
// Depth: 1
module TextureCacheDirectMappedContext #(
    parameter TEXEL_WIDTH = 16,

    parameter CACHE_SIZE = 1024,
    parameter CACHE_LINE_SIZE = 32,
    localparam CACHE_LINES = CACHE_SIZE / CACHE_LINE_SIZE,
    // This option enables fetching of cache lines while they are loaded.
    // This requires dual port RAM. Is this option disabled, the cache waits
    // until the while cache line is loaded before serving new requests.
    // In this case, single port memory is sufficient.
    parameter ENABLE_EARLY_FETCH = 0,

    parameter DATA_WIDTH = 32,
    parameter ID_WIDTH = 4,
    parameter ADDR_WIDTH = 18,

    localparam LG_DATA_BYTES = $clog2(DATA_WIDTH / 8),
    localparam LG_CACHE_WORDS = $clog2(CACHE_SIZE / (DATA_WIDTH / 8)),
    localparam TAG_WIDTH = ADDR_WIDTH - $clog2(CACHE_LINE_SIZE) - $clog2(CACHE_LINES)
)
(
    input  wire                             aclk,
    input  wire                             resetn,

    // Input interface
    input  wire                             s_tc_valid,
    output reg                              s_tc_ready,
    input  wire                             s_tc_cmd,
    input  wire [ADDR_WIDTH - 1 : 0]        s_tc_addr,

    output reg  [TEXEL_WIDTH - 1 : 0]       m_tc_texel,
    output reg                              m_tc_valid,
    input  wire                             m_tc_ready,

    // AXI input interface
    input  wire [ID_WIDTH - 1 : 0]          m_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]        m_axi_rdata,
    input  wire [ 1 : 0]                    m_axi_rresp,
    input  wire                             m_axi_rlast,
    input  wire                             m_axi_rvalid,
    output reg                              m_axi_rready
);
    localparam READ_CACHE_ENTRY = 1'b0;
    localparam LOAD_CACHE_LINE = 1'b1;

    function [ADDR_WIDTH - 1 : 0] getCacheGroupAddress;
        input [ADDR_WIDTH - 1 : 0] addr;
        begin
            getCacheGroupAddress = { 
                { (ADDR_WIDTH - $clog2(CACHE_LINE_SIZE) - $clog2(CACHE_LINES)) { 1'b0 } }, 
                addr[$clog2(CACHE_LINE_SIZE) +: $clog2(CACHE_LINES)], 
                { ( $clog2(CACHE_LINE_SIZE)) { 1'b0 } } 
            };
        end
    endfunction

    function [ADDR_WIDTH - 1 : 0] getByteAddress;
        input [ADDR_WIDTH - 1 : 0] addr;
        begin
            getByteAddress = {
                { (ADDR_WIDTH - $clog2(CACHE_LINE_SIZE)) { 1'b0 } }, 
                addr[0 +: $clog2(CACHE_LINE_SIZE)]
            }; 
        end
        
    endfunction

    function [LG_CACHE_WORDS - 1 : 0] getWordAddress;
        input [ADDR_WIDTH - 1 : 0] addr;
        begin
            getWordAddress = addr[LG_DATA_BYTES +: LG_CACHE_WORDS];
        end
    endfunction

    function [TEXEL_WIDTH - 1 : 0] getTexel;
        input [ADDR_WIDTH - 1 : 0] addr;
        localparam TEXEL_OFFSET_WIDTH = (DATA_WIDTH / TEXEL_WIDTH) > 1
                                      ? $clog2(DATA_WIDTH / TEXEL_WIDTH)
                                      : 1;
        reg [DATA_WIDTH - 1 : 0] word;
        reg [DATA_WIDTH - 1 : 0] shifted_word;
        integer texel_shift;
        begin
            word = r_cache_memory[getWordAddress(addr)];
            if (DATA_WIDTH == TEXEL_WIDTH)
            begin
                texel_shift = 0;
            end
            else
            begin
                texel_shift = TEXEL_WIDTH * addr[$clog2(TEXEL_WIDTH / 8) +: TEXEL_OFFSET_WIDTH];
            end
            shifted_word = word >> texel_shift;
            getTexel = shifted_word[0 +: TEXEL_WIDTH];
        end
    endfunction

    reg  [DATA_WIDTH - 1 : 0]       r_cache_memory [0 : (CACHE_SIZE / (DATA_WIDTH / 8)) - 1];

    reg                             r_skid_valid;
    reg                             r_skid_cmd;
    reg  [ADDR_WIDTH - 1 : 0]       r_skid_addr;

    reg  [ADDR_WIDTH - 1 : 0]       r_i;
    reg  [ADDR_WIDTH - 1 : 0]       r_axi_addr;

    wire                            w_cmd = r_skid_valid ? r_skid_cmd : s_tc_cmd;
    wire [ADDR_WIDTH - 1 : 0]       w_addr = r_skid_valid ? r_skid_addr : s_tc_addr;

    always @(posedge aclk) 
    begin
        if (!resetn) 
        begin
            r_skid_valid <= 1'b0;
            s_tc_ready   <= 1'b1;
            m_tc_valid   <= 1'b0;
            m_axi_rready <= 1'b0;
        end 
        else 
        begin
            if ((ENABLE_EARLY_FETCH || !m_axi_rready) && 
                (!m_tc_valid || (m_tc_valid && m_tc_ready)))
            begin
                if (s_tc_valid || r_skid_valid)
                begin
                    if (r_skid_valid)
                    begin
                        r_skid_valid <= 1'b0;
                        s_tc_ready   <= 1'b1;
                    end
                    
                    if (w_cmd == LOAD_CACHE_LINE)
                    begin
                        if (m_axi_rready)
                        begin
                            r_skid_addr  <= w_addr;
                            r_skid_cmd   <= w_cmd;
                            r_skid_valid <= 1'b1;
                            m_tc_valid   <= 1'b0;
                            s_tc_ready   <= 1'b0;
                        end
                        else
                        begin
                            m_axi_rready <= 1'b1;
                            r_i          <= { ADDR_WIDTH { 1'b0 } };
                            r_axi_addr   <= w_addr;
                            m_tc_valid   <= 1'b0;
                        end
                    end
                    else if (w_cmd == READ_CACHE_ENTRY)
                    begin
                        if (m_axi_rready && (getWordAddress(r_i) <= getWordAddress(getByteAddress(w_addr))))
                        begin
                            r_skid_addr  <= w_addr;
                            r_skid_cmd   <= w_cmd;
                            r_skid_valid <= 1'b1;
                            s_tc_ready   <= 1'b0;
                            m_tc_valid   <= 1'b0;
                        end
                        else
                        begin
                            m_tc_texel <= getTexel(w_addr);
                            m_tc_valid <= 1'b1;
                        end
                    end
                end
                else
                begin
                    m_tc_valid <= 1'b0;
                end
            end
            else
            begin
                if (!r_skid_valid)
                begin
                    r_skid_addr  <= s_tc_addr;
                    r_skid_cmd   <= s_tc_cmd;
                    r_skid_valid <= s_tc_valid;
                    s_tc_ready   <= !s_tc_valid;
                end
            end

            if (m_axi_rready && m_axi_rvalid)
            begin
                r_i <= r_i + (DATA_WIDTH / 8);
                r_cache_memory[getWordAddress(getCacheGroupAddress(r_axi_addr) + r_i)] <= m_axi_rdata;
                if (r_i == (CACHE_LINE_SIZE - (DATA_WIDTH / 8)))
                begin
                    m_axi_rready <= 1'b0;
                end
            end
        end
    end

endmodule 