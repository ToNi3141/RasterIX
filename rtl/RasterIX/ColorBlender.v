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

// Blends two colors together. The following functions are supported:
// SFactor:
//  ZERO = 0
//  ONE = 1
//  DST_COLOR = 2
//  ONE_MINUS_DST_COLOR = 4
//  SRC_ALPHA = 6
//  ONE_MINUS_SRC_ALPHA = 7
//  DST_ALPHA = 8
//  ONE_MINUS_DST_ALPHA = 9
//  SRC_ALPHA_SATURATE = 10
// DFactor
//  ZERO = 0
//  ONE = 1
//  SRC_COLOR = 3
//  ONE_MINUS_SRC_COLOR = 5
//  SRC_ALPHA = 6
//  ONE_MINUS_SRC_ALPHA = 7
//  DST_ALPHA = 8
//  ONE_MINUS_DST_ALPHA = 9
// Source and dest color are in the RGBA format, See RegisterAndDescriptorDefines.vh
// Pipelined: yes
// Depth: 3 cycles
module ColorBlender 
#(
    parameter SUB_PIXEL_WIDTH = 8,
    parameter SUB_PIXEL_CALC_PRECISION = SUB_PIXEL_WIDTH,
    localparam NUMBER_OF_SUB_PIXEL = 4,
    localparam PIXEL_WIDTH = SUB_PIXEL_WIDTH * NUMBER_OF_SUB_PIXEL
)
(
    input  wire                         aclk,
    input  wire                         resetn,
    input  wire                         ce,

    input  wire [ 3 : 0]                funcSFactor,
    input  wire [ 3 : 0]                funcDFactor,
    input  wire                         confEnable,

    input  wire [PIXEL_WIDTH - 1 : 0]   sourceColor,
    input  wire [PIXEL_WIDTH - 1 : 0]   destColor,

    output wire [PIXEL_WIDTH - 1 : 0]   color 
);
`include "RegisterAndDescriptorDefines.vh"

    localparam [SUB_PIXEL_WIDTH - 1 : 0] ONE_POINT_ZERO = { SUB_PIXEL_WIDTH { 1'h1 } };

    reg [SUB_PIXEL_WIDTH - 1 : 0] v00;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v01;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v02;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v03;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v10;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v11;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v12;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v13;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v20;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v21;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v22;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v23;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v30;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v31;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v32;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v33;
    wire [PIXEL_WIDTH - 1 : 0]    colorMixed;
    wire [PIXEL_WIDTH - 1 : 0]    step_sourceColor;

    ValueDelay #(.VALUE_SIZE(PIXEL_WIDTH), .DELAY(3)) 
        step_sourceColorDelay (.clk(aclk), .ce(ce), .in(sourceColor), .out(step_sourceColor));

    always @(posedge aclk)
    if (ce) begin : Select
        reg [SUB_PIXEL_WIDTH - 1 : 0] rs;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gs;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bs;
        reg [SUB_PIXEL_WIDTH - 1 : 0] as;
        reg [SUB_PIXEL_WIDTH - 1 : 0] rd;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gd;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bd;
        reg [SUB_PIXEL_WIDTH - 1 : 0] ad;

        rs = sourceColor[COLOR_R_POS +: SUB_PIXEL_WIDTH];
        gs = sourceColor[COLOR_G_POS +: SUB_PIXEL_WIDTH];
        bs = sourceColor[COLOR_B_POS +: SUB_PIXEL_WIDTH];
        as = sourceColor[COLOR_A_POS +: SUB_PIXEL_WIDTH];

        rd = destColor[COLOR_R_POS +: SUB_PIXEL_WIDTH];
        gd = destColor[COLOR_G_POS +: SUB_PIXEL_WIDTH];
        bd = destColor[COLOR_B_POS +: SUB_PIXEL_WIDTH];
        ad = destColor[COLOR_A_POS +: SUB_PIXEL_WIDTH];

        v10 = rs;
        v11 = gs;
        v12 = bs;
        v13 = as;

        v30 = rd;
        v31 = gd;
        v32 = bd;
        v33 = ad;

        case (funcSFactor)
            ZERO:
            begin
                v00 = 0;
                v01 = 0;
                v02 = 0;
                v03 = 0;
            end
            ONE:
            begin
                v00 = ONE_POINT_ZERO;
                v01 = ONE_POINT_ZERO;
                v02 = ONE_POINT_ZERO;
                v03 = ONE_POINT_ZERO;
            end 
            DST_COLOR:
            begin
                v00 = rd;
                v01 = gd;
                v02 = bd;
                v03 = ad;
            end 
            ONE_MINUS_DST_COLOR:
            begin
                v00 = ONE_POINT_ZERO - rd;
                v01 = ONE_POINT_ZERO - gd;
                v02 = ONE_POINT_ZERO - bd;
                v03 = ONE_POINT_ZERO - ad;
            end 
            SRC_ALPHA:
            begin
                v00 = as;
                v01 = as;
                v02 = as;
                v03 = as;
            end 
            ONE_MINUS_SRC_ALPHA:
            begin
                v00 = ONE_POINT_ZERO - as;
                v01 = ONE_POINT_ZERO - as;
                v02 = ONE_POINT_ZERO - as;
                v03 = ONE_POINT_ZERO - as;
            end 
            DST_ALPHA:
            begin
                v00 = ad;
                v01 = ad;
                v02 = ad;
                v03 = ad;
            end 
            ONE_MINUS_DST_ALPHA:
            begin
                v00 = ONE_POINT_ZERO - ad;
                v01 = ONE_POINT_ZERO - ad;
                v02 = ONE_POINT_ZERO - ad;
                v03 = ONE_POINT_ZERO - ad;
            end 
            SRC_ALPHA_SATURATE:
            begin
                v00 = (as < (ONE_POINT_ZERO - ad)) ? as : (ONE_POINT_ZERO - ad);
                v01 = (as < (ONE_POINT_ZERO - ad)) ? as : (ONE_POINT_ZERO - ad);
                v02 = (as < (ONE_POINT_ZERO - ad)) ? as : (ONE_POINT_ZERO - ad);
                v03 = ONE_POINT_ZERO;
            end 
            default:
            begin
                
            end 
        endcase

        case (funcDFactor)
            ZERO:
            begin
                v20 = 0;
                v21 = 0;
                v22 = 0;
                v23 = 0;
            end
            ONE:
            begin
                v20 = ONE_POINT_ZERO;
                v21 = ONE_POINT_ZERO;
                v22 = ONE_POINT_ZERO;
                v23 = ONE_POINT_ZERO;
            end 
            SRC_COLOR:
            begin
                v20 = rs;
                v21 = gs;
                v22 = bs;
                v23 = as;
            end 
            ONE_MINUS_SRC_COLOR:
            begin
                v20 = ONE_POINT_ZERO - rs;
                v21 = ONE_POINT_ZERO - gs;
                v22 = ONE_POINT_ZERO - bs;
                v23 = ONE_POINT_ZERO - as;
            end 
            SRC_ALPHA:
            begin
                v20 = as;
                v21 = as;
                v22 = as;
                v23 = as;
            end 
            ONE_MINUS_SRC_ALPHA:
            begin
                v20 = ONE_POINT_ZERO - as;
                v21 = ONE_POINT_ZERO - as;
                v22 = ONE_POINT_ZERO - as;
                v23 = ONE_POINT_ZERO - as;
            end 
            DST_ALPHA:
            begin
                v20 = ad;
                v21 = ad;
                v22 = ad;
                v23 = ad;
            end 
            ONE_MINUS_DST_ALPHA:
            begin
                v20 = ONE_POINT_ZERO - ad;
                v21 = ONE_POINT_ZERO - ad;
                v22 = ONE_POINT_ZERO - ad;
                v23 = ONE_POINT_ZERO - ad;
            end 
            default:
            begin
                
            end 
        endcase
    end

    ColorMixer #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
        .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION)
    ) colorMixer (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),

        .colorA({
            v00,
            v01,
            v02,
            v03
        }),
        .colorB({
            v10,
            v11,
            v12,
            v13
        }),
        .colorC({
            v20,
            v21,
            v22,
            v23
        }),
        .colorD({
            v30,
            v31,
            v32,
            v33
        }),

        .mixedColor(colorMixed)
    );

    assign color = (confEnable) ? colorMixed : step_sourceColor;
endmodule