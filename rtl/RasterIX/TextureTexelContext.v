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

module TextureTexelContext #(
    parameter TEXEL_WIDTH = 16
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    // Interface to the texture texel cache
    input  wire [ 1 : 0]                s_texel_pos,
    input  wire [TEXEL_WIDTH - 1 : 0]   s_texel,
    input  wire                         s_cmd, // 1 = store and sample, 0 = store only 
    input  wire                         s_valid,
    output reg                          s_ready,
    
    // Texture Read
    output reg  [TEXEL_WIDTH - 1 : 0]   m_texel00,
    output reg  [TEXEL_WIDTH - 1 : 0]   m_texel01,
    output reg  [TEXEL_WIDTH - 1 : 0]   m_texel10,
    output reg  [TEXEL_WIDTH - 1 : 0]   m_texel11,
    output reg                          m_valid,
    input  wire                         m_ready
);
    localparam CMD_STORE_AND_SAMPLE = 1;
    localparam CMD_STORE_ONLY       = 0;

    reg  [TEXEL_WIDTH - 1 : 0] r_texel00;
    reg  [TEXEL_WIDTH - 1 : 0] r_texel01;
    reg  [TEXEL_WIDTH - 1 : 0] r_texel10;
    reg  [TEXEL_WIDTH - 1 : 0] r_texel11;

    reg                        r_skid_valid;
    reg  [ 1 : 0]              r_skid_texel_pos;
    reg  [TEXEL_WIDTH - 1 : 0] r_skid_texel;
    reg                        r_skid_cmd;

    wire [ 1 : 0]              w_texel_pos = r_skid_valid ? r_skid_texel_pos : s_texel_pos;
    wire [TEXEL_WIDTH - 1 : 0] w_texel     = r_skid_valid ? r_skid_texel     : s_texel;
    wire                       w_cmd       = r_skid_valid ? r_skid_cmd       : s_cmd;
    wire                       w_valid     = r_skid_valid ? 1'b1             : s_valid;

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            m_valid      <= 1'b0;
            s_ready      <= 1'b1;
            r_skid_valid <= 1'b0;
        end
        else
        begin
            if (!m_valid || (m_valid && m_ready))
            begin
                if (s_valid || r_skid_valid)
                begin
                    case (w_texel_pos)
                        2'b00: r_texel00 = w_texel;
                        2'b01: r_texel01 = w_texel;
                        2'b10: r_texel10 = w_texel;
                        2'b11: r_texel11 = w_texel;
                    endcase

                    if (w_cmd == CMD_STORE_AND_SAMPLE)
                    begin
                        m_texel00 <= r_texel00;
                        m_texel01 <= r_texel01;
                        m_texel10 <= r_texel10;
                        m_texel11 <= r_texel11;
                        m_valid   <= 1'b1;
                    end
                    else
                    begin
                        m_valid <= 1'b0;
                    end

                    if (r_skid_valid)
                    begin
                        r_skid_valid <= 1'b0;
                        s_ready      <= 1'b1;
                    end
                end
                else
                begin
                    m_valid <= 1'b0;
                end
            end
            else
            begin
                if (!r_skid_valid)
                begin
                    r_skid_texel_pos <= s_texel_pos;
                    r_skid_texel     <= s_texel;
                    r_skid_cmd       <= s_cmd;
                    r_skid_valid     <= s_valid;
                    s_ready          <= !s_valid;
                end
            end
        end
    end
endmodule 