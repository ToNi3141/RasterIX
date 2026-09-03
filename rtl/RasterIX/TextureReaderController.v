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

`include "PixelUtil.vh"

module TextureReaderController #(
    parameter TEX_ADDR_WIDTH = 17
)
(
    input  wire                             aclk,
    input  wire                             resetn,

    // Control
    input  wire                             invalidate,

    // Texture Read
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    s_ar_texel00,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    s_ar_texel01,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    s_ar_texel10,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    s_ar_texel11,
    input  wire                             s_ar_valid,
    output reg                              s_ar_ready,

    // Interface to the texture texel cache
    output reg  [ 1 : 0]                    m_texel_pos,
    output reg                              m_cmd, // 1 = store and sample, 0 = store only 
    output reg                              m_valid,
    input  wire                             m_ready,
    output reg  [TEX_ADDR_WIDTH - 1 : 0]    m_araddr
);
// The following cases have to be handled:
// 1. Cache miss: The texel must be cached from memory before it can be used.
//    r_cmd[2] is set to 0 as long as not at least three texel samples have been cached.
// 2. Cache hit: The texel can be used directly from the cache.
//    Still producing a read request but should be handled immediately from the cache.
//    Reason: The texture texel cache will concatenate the the stream from the
//    Cache and from this module. So it requires a data on both streams simultaneously.
// 3. Invalidate: The cache must be invalidated when requested sets all addresses to ~0 (all bits set).

    localparam INVALID_TEXEL_ADDR = { TEX_ADDR_WIDTH { 1'b1 } };
    localparam CMD_STORE_AND_SAMPLE = 1;
    localparam CMD_STORE_ONLY       = 0;

    reg [TEX_ADDR_WIDTH - 1 : 0] r_texel00;
    reg [TEX_ADDR_WIDTH - 1 : 0] r_texel01;
    reg [TEX_ADDR_WIDTH - 1 : 0] r_texel10;
    reg [TEX_ADDR_WIDTH - 1 : 0] r_texel11;

    reg                          r_skid_valid;
    reg [TEX_ADDR_WIDTH - 1 : 0] r_texel00_skid;
    reg [TEX_ADDR_WIDTH - 1 : 0] r_texel01_skid;
    reg [TEX_ADDR_WIDTH - 1 : 0] r_texel10_skid;
    reg [TEX_ADDR_WIDTH - 1 : 0] r_texel11_skid;

    wire [TEX_ADDR_WIDTH - 1 : 0] r_texel00_next = (r_skid_valid) ? r_texel00_skid : s_ar_texel00;
    wire [TEX_ADDR_WIDTH - 1 : 0] r_texel01_next = (r_skid_valid) ? r_texel01_skid : s_ar_texel01;
    wire [TEX_ADDR_WIDTH - 1 : 0] r_texel10_next = (r_skid_valid) ? r_texel10_skid : s_ar_texel10;
    wire [TEX_ADDR_WIDTH - 1 : 0] r_texel11_next = (r_skid_valid) ? r_texel11_skid : s_ar_texel11;

    wire [3 : 0] texel_match = { 
        (r_texel11_next == r_texel11), 
        (r_texel10_next == r_texel10), 
        (r_texel01_next == r_texel01), 
        (r_texel00_next == r_texel00) 
    };

    always @(posedge aclk)
    begin
        if (!resetn || invalidate) 
        begin
            r_texel00 <= INVALID_TEXEL_ADDR;
            r_texel01 <= INVALID_TEXEL_ADDR;
            r_texel10 <= INVALID_TEXEL_ADDR;
            r_texel11 <= INVALID_TEXEL_ADDR;

            if (!resetn)
            begin
                s_ar_ready <= 1;
                r_skid_valid <= 0;
                m_valid <= 0;
            end
        end 
        else 
        begin
            if ((s_ar_valid || r_skid_valid) && (!m_valid || m_ready))
            begin
                m_valid <= 1;

                case (texel_match)
                    // All texels or at least one does not match
                    // One texel loading is always for free
                    4'b1111, 
                    4'b1110,
                    4'b1101,
                    4'b1011,
                    4'b0111:
                    begin
                        m_cmd <= CMD_STORE_AND_SAMPLE;
                        s_ar_ready <= 1;
                        r_skid_valid <= 0;
                    end
                    // More than one does not match. Now we need a stall
                    default:
                    begin
                        m_cmd <= CMD_STORE_ONLY;
                        s_ar_ready <= 0;
                        r_skid_valid <= 1;

                        if (!r_skid_valid)
                        begin
                            r_texel00_skid <= s_ar_texel00;
                            r_texel01_skid <= s_ar_texel01;
                            r_texel10_skid <= s_ar_texel10;
                            r_texel11_skid <= s_ar_texel11;
                        end
                    end
                endcase

                // Check which texels do not match and set the corresponding address
                // to load them.
                // TODO: Only check all 4 texel when texture filtering is enabled
                if (!texel_match[0])
                begin
                    m_araddr <= r_texel00_next;
                    m_texel_pos <= 2'b00;
                    r_texel00 <= r_texel00_next;
                end
                else if (!texel_match[1])
                begin
                    m_araddr <= r_texel01_next;
                    m_texel_pos <= 2'b01;
                    r_texel01 <= r_texel01_next;
                end
                else if (!texel_match[2])
                begin
                    m_araddr <= r_texel10_next;
                    m_texel_pos <= 2'b10;
                    r_texel10 <= r_texel10_next;
                end
                else if (!texel_match[3])
                begin
                    m_araddr <= r_texel11_next;
                    m_texel_pos <= 2'b11;
                    r_texel11 <= r_texel11_next;
                end
            end
            else if (m_valid && !m_ready && s_ar_valid && s_ar_ready)
            begin
                r_texel00_skid <= s_ar_texel00;
                r_texel01_skid <= s_ar_texel01;
                r_texel10_skid <= s_ar_texel10;
                r_texel11_skid <= s_ar_texel11;
                r_skid_valid <= 1;
                s_ar_ready <= 0;
            end
            else if (!s_ar_valid && !r_skid_valid && (!m_valid || m_ready))
            begin
                m_valid <= 0;
                s_ar_ready <= 1;
                r_skid_valid <= 0;
            end
        end
    end
endmodule 