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

// Mixes four colors together with the following formular 
//  mixedColor = (colorA * colorB) + (colorC * colorD)
// Each color is a vector of four sub colors.
// The mixedColor will be saturated.
// It expects signed sub pixels.
// Pipelined: yes
// Depth: 2 cycles
module ColorMixerSigned #(
    parameter SUB_PIXEL_WIDTH = 9,
    parameter SUB_PIXEL_CALC_PRECISION = SUB_PIXEL_WIDTH,
    localparam NUMBER_OF_SUB_PIXEL = 4,
    localparam PIXEL_WIDTH = SUB_PIXEL_WIDTH * NUMBER_OF_SUB_PIXEL
) (
    input wire                          aclk,
    input wire                          resetn,
    input wire                          ce,

    input wire  [PIXEL_WIDTH - 1 : 0]   colorA,
    input wire  [PIXEL_WIDTH - 1 : 0]   colorB,
    input wire  [PIXEL_WIDTH - 1 : 0]   colorC,
    input wire  [PIXEL_WIDTH - 1 : 0]   colorD,

    // mixedColor = (colorA * colorB) + (colorC * colorD)
    // Colors will be saturated
    output reg  [PIXEL_WIDTH - 1 : 0]   mixedColor
);
    localparam SIGN_WIDTH = 1;
    localparam SUB_PX_PRECISION_UNSIGNED_WIDTH = SUB_PIXEL_CALC_PRECISION - SIGN_WIDTH;
    localparam SUB_PX_PRECISION_WIDTH_2X = (SUB_PX_PRECISION_UNSIGNED_WIDTH * 2) + SIGN_WIDTH;
    localparam SUB_PX_PRECISION_WIDTH_2X_WITH_CARRY = SUB_PX_PRECISION_WIDTH_2X + 1;
    localparam SUB_PX_PRECISION_WIDTH_DIFF = SUB_PIXEL_WIDTH - SUB_PIXEL_CALC_PRECISION;
    localparam SUB_PIXEL_0_POS = (SUB_PIXEL_WIDTH * 0) + SUB_PX_PRECISION_WIDTH_DIFF;
    localparam SUB_PIXEL_1_POS = (SUB_PIXEL_WIDTH * 1) + SUB_PX_PRECISION_WIDTH_DIFF;
    localparam SUB_PIXEL_2_POS = (SUB_PIXEL_WIDTH * 2) + SUB_PX_PRECISION_WIDTH_DIFF;
    localparam SUB_PIXEL_3_POS = (SUB_PIXEL_WIDTH * 3) + SUB_PX_PRECISION_WIDTH_DIFF;


    localparam signed [SUB_PX_PRECISION_WIDTH_2X - 1 : 0] ONE_DOT_ZERO = { 1'b0, { SUB_PX_PRECISION_UNSIGNED_WIDTH { 1'b0 } }, { SUB_PX_PRECISION_UNSIGNED_WIDTH { 1'b1 } } };
    `ReduceAndSaturateSigned(ReduceAndSaturateSigned, SUB_PX_PRECISION_WIDTH_2X_WITH_CARRY, SUB_PIXEL_CALC_PRECISION)

    reg signed [SUB_PX_PRECISION_WIDTH_2X - 1 : 0] V00;
    reg signed [SUB_PX_PRECISION_WIDTH_2X - 1 : 0] V01;
    reg signed [SUB_PX_PRECISION_WIDTH_2X - 1 : 0] V02;
    reg signed [SUB_PX_PRECISION_WIDTH_2X - 1 : 0] V03;
    reg signed [SUB_PX_PRECISION_WIDTH_2X - 1 : 0] V10;
    reg signed [SUB_PX_PRECISION_WIDTH_2X - 1 : 0] V11;
    reg signed [SUB_PX_PRECISION_WIDTH_2X - 1 : 0] V12;
    reg signed [SUB_PX_PRECISION_WIDTH_2X - 1 : 0] V13;
    always @(posedge aclk)
    if (ce) begin : Blending
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] ca0;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] ca1;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] ca2;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] ca3;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] cb0;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] cb1;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] cb2;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] cb3;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] cc0;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] cc1;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] cc2;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] cc3;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] cd0;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] cd1;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] cd2;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] cd3;

        ca0 = colorA[SUB_PIXEL_0_POS +: SUB_PIXEL_CALC_PRECISION];
        ca1 = colorA[SUB_PIXEL_1_POS +: SUB_PIXEL_CALC_PRECISION];
        ca2 = colorA[SUB_PIXEL_2_POS +: SUB_PIXEL_CALC_PRECISION];
        ca3 = colorA[SUB_PIXEL_3_POS +: SUB_PIXEL_CALC_PRECISION];

        cb0 = colorB[SUB_PIXEL_0_POS +: SUB_PIXEL_CALC_PRECISION];
        cb1 = colorB[SUB_PIXEL_1_POS +: SUB_PIXEL_CALC_PRECISION];
        cb2 = colorB[SUB_PIXEL_2_POS +: SUB_PIXEL_CALC_PRECISION];
        cb3 = colorB[SUB_PIXEL_3_POS +: SUB_PIXEL_CALC_PRECISION];

        cc0 = colorC[SUB_PIXEL_0_POS +: SUB_PIXEL_CALC_PRECISION];
        cc1 = colorC[SUB_PIXEL_1_POS +: SUB_PIXEL_CALC_PRECISION];
        cc2 = colorC[SUB_PIXEL_2_POS +: SUB_PIXEL_CALC_PRECISION];
        cc3 = colorC[SUB_PIXEL_3_POS +: SUB_PIXEL_CALC_PRECISION];

        cd0 = colorD[SUB_PIXEL_0_POS +: SUB_PIXEL_CALC_PRECISION];
        cd1 = colorD[SUB_PIXEL_1_POS +: SUB_PIXEL_CALC_PRECISION];
        cd2 = colorD[SUB_PIXEL_2_POS +: SUB_PIXEL_CALC_PRECISION];
        cd3 = colorD[SUB_PIXEL_3_POS +: SUB_PIXEL_CALC_PRECISION];

        V00 <= (ca0 * cb0);
        V01 <= (ca1 * cb1);
        V02 <= (ca2 * cb2);
        V03 <= (ca3 * cb3);

        V10 <= (cc0 * cd0);
        V11 <= (cc1 * cd1);
        V12 <= (cc2 * cd2);
        V13 <= (cc3 * cd3);
    end

    always @(posedge aclk)
    if (ce) begin : Result
        reg signed [SUB_PX_PRECISION_WIDTH_2X_WITH_CARRY - 1 : 0] c0;
        reg signed [SUB_PX_PRECISION_WIDTH_2X_WITH_CARRY - 1 : 0] c1;
        reg signed [SUB_PX_PRECISION_WIDTH_2X_WITH_CARRY - 1 : 0] c2;
        reg signed [SUB_PX_PRECISION_WIDTH_2X_WITH_CARRY - 1 : 0] c3;

        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] cs0;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] cs1;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] cs2;
        reg signed [SUB_PIXEL_CALC_PRECISION - 1 : 0] cs3;

        reg signed [(SUB_PIXEL_CALC_PRECISION * 2) - 1 - 1 : 0] cd0;
        reg signed [(SUB_PIXEL_CALC_PRECISION * 2) - 1 - 1 : 0] cd1;
        reg signed [(SUB_PIXEL_CALC_PRECISION * 2) - 1 - 1 : 0] cd2;
        reg signed [(SUB_PIXEL_CALC_PRECISION * 2) - 1 - 1 : 0] cd3;

        c0 = ((V00 + V10) + ONE_DOT_ZERO) >>> SUB_PX_PRECISION_UNSIGNED_WIDTH;
        c1 = ((V01 + V11) + ONE_DOT_ZERO) >>> SUB_PX_PRECISION_UNSIGNED_WIDTH;
        c2 = ((V02 + V12) + ONE_DOT_ZERO) >>> SUB_PX_PRECISION_UNSIGNED_WIDTH;
        c3 = ((V03 + V13) + ONE_DOT_ZERO) >>> SUB_PX_PRECISION_UNSIGNED_WIDTH;

        cs3 = ReduceAndSaturateSigned(c3);
        cs2 = ReduceAndSaturateSigned(c2);
        cs1 = ReduceAndSaturateSigned(c1);
        cs0 = ReduceAndSaturateSigned(c0);

        cd3 = { cs3, cs3[0 +: SUB_PIXEL_CALC_PRECISION - 1] }; // -1 is because of the sign. It is not used in the repetition
        cd2 = { cs2, cs2[0 +: SUB_PIXEL_CALC_PRECISION - 1] };
        cd1 = { cs1, cs1[0 +: SUB_PIXEL_CALC_PRECISION - 1] };
        cd0 = { cs0, cs0[0 +: SUB_PIXEL_CALC_PRECISION - 1] };

        mixedColor <= {
            cd3[((SUB_PIXEL_CALC_PRECISION * 2) - 1) - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
            cd2[((SUB_PIXEL_CALC_PRECISION * 2) - 1) - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
            cd1[((SUB_PIXEL_CALC_PRECISION * 2) - 1) - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
            cd0[((SUB_PIXEL_CALC_PRECISION * 2) - 1) - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH]
        };
    end
endmodule