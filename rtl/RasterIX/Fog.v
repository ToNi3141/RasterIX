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

// Fogs a texel based on a given depth value.
// It uses a fog look up table to calculate the intensity of the fog color based on the depth.
// To see how the LUT is build, @see FunctionInterpolator
// Pipelined: yes
// Depth: 6 cycles
module Fog 
#(
    parameter USER_WIDTH = 1,
    parameter SUB_PIXEL_WIDTH = 8,
    parameter SUB_PIXEL_CALC_PRECISION = SUB_PIXEL_WIDTH,
    localparam NUMBER_OF_SUB_PIXEL = 4,
    localparam PIXEL_WIDTH = SUB_PIXEL_WIDTH * NUMBER_OF_SUB_PIXEL,

    localparam FLOAT_SIZE = 32
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    // Fog function LUT stream
    input  wire                         s_fog_lut_axis_tvalid,
    output wire                         s_fog_lut_axis_tready,
    input  wire                         s_fog_lut_axis_tlast,
    input  wire [31 : 0]                s_fog_lut_axis_tdata,

    // Fog Color
    input  wire [PIXEL_WIDTH - 1 : 0]   confFogColor,
    input  wire                         confEnable,

    // Fog calculation
    output wire                         s_ready,
    input  wire                         s_valid,
    input  wire [USER_WIDTH - 1 : 0]    s_user,
    input  wire [FLOAT_SIZE - 1 : 0]    s_depth,
    input  wire [PIXEL_WIDTH - 1 : 0]   s_texelColor,
    
    // Fogged texel
    input  wire                         m_ready,
    output wire                         m_valid,
    output wire [USER_WIDTH - 1 : 0]    m_user,
    output wire [PIXEL_WIDTH - 1 : 0]   m_color 
);
    wire ce;
    assign ce = m_ready;
    assign s_ready = m_ready;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 0
    // Calculate the fog intensity
    // Clocks: 4 
    ////////////////////////////////////////////////////////////////////////////
    wire [23 : 0]               step0_fogIntensity;
    wire [PIXEL_WIDTH - 1 : 0]  step0_texelColor;
    wire                        step0_valid;
    wire [USER_WIDTH - 1 : 0]   step0_user;

    ValueDelay #(
        .VALUE_SIZE(1 + USER_WIDTH + PIXEL_WIDTH), 
        .DELAY(4)
    ) step0_validDelay (
        .clk(aclk), 
        .ce(ce),
        .in({
            s_valid,
            s_user,
            s_texelColor
        }),
        .out({
            step0_valid,
            step0_user,
            step0_texelColor
        })
    );

    FunctionInterpolator step0_calculateFogIntensity (
        .aclk(aclk), 
        .resetn(resetn), 
        .ce(ce),
        .x(s_depth),
        .fx(step0_fogIntensity),
        .s_axis_tvalid(s_fog_lut_axis_tvalid), 
        .s_axis_tready(s_fog_lut_axis_tready), 
        .s_axis_tlast(s_fog_lut_axis_tlast), 
        .s_axis_tdata(s_fog_lut_axis_tdata)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Fog color mixing
    // Clocks: 2
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step1_texelColor;
    wire [PIXEL_WIDTH - 1 : 0]  step1_mixedColor;
    wire [USER_WIDTH - 1 : 0]   step1_user;
    wire                        step1_valid;

    ValueDelay #(
        .VALUE_SIZE(1 + USER_WIDTH + PIXEL_WIDTH), 
        .DELAY(2)
    ) step1_validDelay (
        .clk(aclk), 
        .ce(ce),
        .in({
            step0_valid,
            step0_user,
            step0_texelColor
        }),
        .out({
            step1_valid,
            step1_user,
            step1_texelColor
        })
    );

    ColorInterpolator #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
        .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION)
    ) fogInterpolator (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),

        .intensity((step0_fogIntensity[22]) ? 16'hffff : step0_fogIntensity[22 - 16 +: 16]),
        .colorA(step0_texelColor),
        .colorB(confFogColor),

        .mixedColor(step1_mixedColor)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Output fogged texel
    // Clocks: 0
    ////////////////////////////////////////////////////////////////////////////
    assign m_color = (confEnable) ? {
            step1_mixedColor[SUB_PIXEL_WIDTH * 3 +: SUB_PIXEL_WIDTH],
            step1_mixedColor[SUB_PIXEL_WIDTH * 2 +: SUB_PIXEL_WIDTH],
            step1_mixedColor[SUB_PIXEL_WIDTH * 1 +: SUB_PIXEL_WIDTH],
            step1_texelColor[SUB_PIXEL_WIDTH * 0 +: SUB_PIXEL_WIDTH] // Replace alpha, because it is specified that fog does not change the alpha value of a texel
        } : step1_texelColor;
    assign m_user = step1_user;
    assign m_valid = step1_valid;

endmodule