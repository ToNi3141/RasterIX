// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2023 ToNi3141

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

// This module filters the texture. It gets 4 pixel from the texture buffer inclusive 
// the sub pixel information where the sample lies exactly.
// Pipelined: yes
// Depth: 4 cycles
module TextureFilter #(
    parameter USER_WIDTH = 1,
    localparam SUB_PIXEL_WIDTH = 8,
    parameter SUB_PIXEL_CALC_PRECISION = SUB_PIXEL_WIDTH,
    localparam PIXEL_WIDTH = SUB_PIXEL_WIDTH * 4
) 
(
    input  wire                         aclk,
    input  wire                         resetn,

    input  wire                         enable,

    output wire                         s_ready,
    input  wire                         s_valid,
    input  wire [USER_WIDTH - 1 : 0]    s_user,
    input  wire [PIXEL_WIDTH - 1 : 0]   s_texel00,
    input  wire [PIXEL_WIDTH - 1 : 0]   s_texel01,
    input  wire [PIXEL_WIDTH - 1 : 0]   s_texel10,
    input  wire [PIXEL_WIDTH - 1 : 0]   s_texel11,
    input  wire [15 : 0]                s_texelSubCoordS,
    input  wire [15 : 0]                s_texelSubCoordT,

    input  wire                         m_ready,
    output wire                         m_valid,
    output wire [USER_WIDTH - 1 : 0]    m_user,
    output wire [PIXEL_WIDTH - 1 : 0]   m_texel
);
    wire ce;
    assign ce = m_ready;
    assign s_ready = m_ready;

    wire [15 : 0]               intensityDelayedT;
    wire [PIXEL_WIDTH - 1 : 0]  mixedColorS0;
    wire [PIXEL_WIDTH - 1 : 0]  mixedColorS1;
    wire [15 : 0]               intensityS;
    wire [15 : 0]               intensityT;
    wire [PIXEL_WIDTH - 1 : 0]  filteredTexel;
    wire [PIXEL_WIDTH - 1 : 0]  unfilteredTexel;

    assign intensityS = 16'hffff - s_texelSubCoordS;
    assign intensityT = 16'hffff - s_texelSubCoordT;

    ColorInterpolator #(
        .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION)
    ) interpolatorS0 (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),

        .intensity(intensityS),
        .colorA(s_texel00),
        .colorB(s_texel01),
        .mixedColor(mixedColorS0)
    );

    ColorInterpolator #(
        .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION)
    ) interpolatorS1 (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),

        .intensity(intensityS),
        .colorA(s_texel10),
        .colorB(s_texel11),
        .mixedColor(mixedColorS1)
    );

    ColorInterpolator #(
        .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION)
    ) interpolatorT (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),

        .intensity(intensityDelayedT),
        .colorA(mixedColorS0),
        .colorB(mixedColorS1),
        .mixedColor(filteredTexel)
    );

    // Delay intensityT by 2 clocks (to calculate the S interpolations)
    ValueDelay #(
        .VALUE_SIZE(16), 
        .DELAY(2)
    ) 
    intensityTDelay (
        .clk(aclk), 
        .ce(ce),
        .in(intensityT), 
        .out(intensityDelayedT)
    );

    ValueDelay #(
        .VALUE_SIZE(1 + USER_WIDTH + PIXEL_WIDTH), 
        .DELAY(4)
    ) 
    validDelay (
        .clk(aclk), 
        .ce(ce),
        .in({
            s_valid,
            s_user,
            s_texel00 // Use texel00 as unfiltered texel
        }), 
        .out({
            m_valid,
            m_user,
            unfilteredTexel
        })
    );

    assign m_texel = (enable) ? filteredTexel : unfilteredTexel;

endmodule