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

module TextureClamp #(
    parameter TEXEL_WIDTH = 16,
    parameter USER_WIDTH = 1
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    input  wire                         s_valid,
    output wire                         s_ready,
    input  wire [USER_WIDTH - 1 : 0]    s_user,
    input  wire [TEXEL_WIDTH - 1 : 0]   s_texel00,
    input  wire [TEXEL_WIDTH - 1 : 0]   s_texel01,
    input  wire [TEXEL_WIDTH - 1 : 0]   s_texel10,
    input  wire [TEXEL_WIDTH - 1 : 0]   s_texel11,
    input  wire [15 : 0]                s_texelSubCoordS,
    input  wire [15 : 0]                s_texelSubCoordT,
    input  wire                         s_clampU,
    input  wire                         s_clampV,

    input  wire                         m_ready,
    output reg                          m_valid,
    output reg  [USER_WIDTH - 1 : 0]    m_user,
    output reg  [TEXEL_WIDTH - 1 : 0]   m_texel00,
    output reg  [TEXEL_WIDTH - 1 : 0]   m_texel01,
    output reg  [TEXEL_WIDTH - 1 : 0]   m_texel10,
    output reg  [TEXEL_WIDTH - 1 : 0]   m_texel11,
    output reg  [15 : 0]                m_texelSubCoordS,
    output reg  [15 : 0]                m_texelSubCoordT
);
    assign s_ready = m_ready;

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            m_valid <= 0;
        end
        else if (m_ready)
        begin
            m_texel00 <= s_texel00;
            m_texel01 <= (s_clampU) ? s_texel00
                                    : s_texel01;
            m_texel10 <= (s_clampV) ? s_texel00
                                    : s_texel10;
            m_texel11 <= (s_clampU) ? (s_clampV) ? s_texel00
                                                 : s_texel10
                                    : (s_clampV) ? s_texel01
                                                 : s_texel11;

            m_texelSubCoordS <= s_texelSubCoordS;
            m_texelSubCoordT <= s_texelSubCoordT;
            m_valid <= s_valid;
            m_user <= s_user;
        end
    end
endmodule
