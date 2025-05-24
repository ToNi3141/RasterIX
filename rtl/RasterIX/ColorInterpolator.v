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

`include "PixelUtil.vh"

// Interpolates a color with 4 sub color elements together like RGBA4444
// The used caluclation: mixedColor = (intensity * colorA) + ((1.0 - intensity) * colorB)
// Pipelined: yes
// Depth: 2 cycles
module ColorInterpolator #(
    parameter SUB_PIXEL_WIDTH = 8,
    parameter SUB_PIXEL_CALC_PRECISION = SUB_PIXEL_WIDTH,
    localparam NUMBER_OF_SUB_PIXEL = 4,
    localparam PIXEL_WIDTH = SUB_PIXEL_WIDTH * NUMBER_OF_SUB_PIXEL
)
(
    input  wire                         aclk,
    input  wire                         resetn,
    input  wire                         ce,

    input  wire [15 : 0]                intensity,
    input  wire [PIXEL_WIDTH - 1 : 0]   colorA,
    input  wire [PIXEL_WIDTH - 1 : 0]   colorB,

    output wire [PIXEL_WIDTH - 1 : 0]   mixedColor
);
    localparam SUB_PIXEL_0_POS = SUB_PIXEL_WIDTH * 0;
    localparam SUB_PIXEL_1_POS = SUB_PIXEL_WIDTH * 1;
    localparam SUB_PIXEL_2_POS = SUB_PIXEL_WIDTH * 2;
    localparam SUB_PIXEL_3_POS = SUB_PIXEL_WIDTH * 3;

    localparam [SUB_PIXEL_WIDTH - 1 : 0] ONE_DOT_ZERO = { SUB_PIXEL_WIDTH { 1'b1 } };

    ColorMixer #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
        .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION)
    ) colorMixer (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),

        .colorA(colorA),
        .colorB({
            intensity[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
            intensity[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
            intensity[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
            intensity[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH]
        }),
        .colorC(colorB),
        .colorD({
            ONE_DOT_ZERO - intensity[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
            ONE_DOT_ZERO - intensity[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
            ONE_DOT_ZERO - intensity[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
            ONE_DOT_ZERO - intensity[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH]
        }),

        .mixedColor(mixedColor)
    );
endmodule