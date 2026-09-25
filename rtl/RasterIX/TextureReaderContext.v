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

// Texture reader context which manages the texture texel cache and handles read requests.
// Counterpart is the TextureReaderController which manages the cache.
// Pipelined: yes
// Depth: 1
module TextureReaderContext #(
    parameter TEXEL_WIDTH = 16
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    // Interface to the TextureReaderController
    input  wire [ 1 : 0]                s_trc_texel_pos,
    input  wire [TEXEL_WIDTH - 1 : 0]   s_trc_texel,
    input  wire                         s_trc_cmd, // 1 = store and sample, 0 = store only
    input  wire                         s_trc_valid,
    output reg                          s_trc_ready,
    
    // Texture Read
    output reg  [TEXEL_WIDTH - 1 : 0]   m_tr_texel_00,
    output reg  [TEXEL_WIDTH - 1 : 0]   m_tr_texel_01,
    output reg  [TEXEL_WIDTH - 1 : 0]   m_tr_texel_10,
    output reg  [TEXEL_WIDTH - 1 : 0]   m_tr_texel_11,
    output reg                          m_tr_valid,
    input  wire                         m_tr_ready
);
    localparam CMD_STORE_AND_SAMPLE = 1;
    localparam CMD_STORE_ONLY       = 0;

    reg  [TEXEL_WIDTH - 1 : 0] rTexel00;
    reg  [TEXEL_WIDTH - 1 : 0] rTexel01;
    reg  [TEXEL_WIDTH - 1 : 0] rTexel10;
    reg  [TEXEL_WIDTH - 1 : 0] rTexel11;

    reg                        rSkidValid;
    reg  [ 1 : 0]              rSkidTexelPos;
    reg  [TEXEL_WIDTH - 1 : 0] rSkidTexel;
    reg                        rSkidCmd;

    wire [ 1 : 0]              wTexelPos = rSkidValid ? rSkidTexelPos : s_trc_texel_pos;
    wire [TEXEL_WIDTH - 1 : 0] wTexel    = rSkidValid ? rSkidTexel    : s_trc_texel;
    wire                       wCmd      = rSkidValid ? rSkidCmd      : s_trc_cmd;
    wire                       wValid    = rSkidValid ? 1'b1          : s_trc_valid;

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            m_tr_valid <= 1'b0;
            s_trc_ready <= 1'b1;
            rSkidValid <= 1'b0;
        end
        else
        begin
            if (!m_tr_valid || (m_tr_valid && m_tr_ready))
            begin
                if (s_trc_valid || rSkidValid)
                begin
                    case (wTexelPos)
                        2'b00: rTexel00 = wTexel;
                        2'b01: rTexel01 = wTexel;
                        2'b10: rTexel10 = wTexel;
                        2'b11: rTexel11 = wTexel;
                    endcase

                    if (wCmd == CMD_STORE_AND_SAMPLE)
                    begin
                        m_tr_texel_00 <= rTexel00;
                        m_tr_texel_01 <= rTexel01;
                        m_tr_texel_10 <= rTexel10;
                        m_tr_texel_11 <= rTexel11;
                        m_tr_valid   <= 1'b1;
                    end
                    else
                    begin
                        m_tr_valid <= 1'b0;
                    end

                    if (rSkidValid)
                    begin
                        rSkidValid <= 1'b0;
                        s_trc_ready <= 1'b1;
                    end
                end
                else
                begin
                    m_tr_valid <= 1'b0;
                end
            end
            else
            begin
                if (!rSkidValid)
                begin
                    rSkidTexelPos <= s_trc_texel_pos;
                    rSkidTexel    <= s_trc_texel;
                    rSkidCmd      <= s_trc_cmd;
                    rSkidValid    <= s_trc_valid;
                    s_trc_ready    <= !s_trc_valid;
                end
            end
        end
    end
endmodule 