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

// This module calculates the texture environment 
// For documentation, see RegisterAndDescriptorDefines.vh:OP_RENDER_CONFIG_TMU_TEX_ENV
// Pipelined: yes
// Depth: 4 cycles
module TexEnv 
#(
    parameter USER_WIDTH = 1,
    parameter SUB_PIXEL_WIDTH = 8,
    parameter SUB_PIXEL_CALC_PRECISION = SUB_PIXEL_WIDTH,

    localparam NUMBER_OF_SUB_PIXEL = 4,
    localparam PIXEL_WIDTH = SUB_PIXEL_WIDTH * NUMBER_OF_SUB_PIXEL
)
(
    input  wire                         aclk,
    input  wire                         resetn,
    
    input  wire [31 : 0]                conf,

    output wire                         s_ready,
    input  wire                         s_valid,
    input  wire [USER_WIDTH - 1 : 0]    s_user,
    input  wire [PIXEL_WIDTH - 1 : 0]   s_previousColor, // Cf (in case of tex unit 0) or Cp (output m_color of tex unit n-1) 
    input  wire [PIXEL_WIDTH - 1 : 0]   s_texSrcColor, // Cs 
    input  wire [PIXEL_WIDTH - 1 : 0]   s_primaryColor, // Cf
    input  wire [PIXEL_WIDTH - 1 : 0]   s_envColor, // Cc

    input  wire                         m_ready,
    output wire                         m_valid,
    output wire [USER_WIDTH - 1 : 0]    m_user,
    output reg  [PIXEL_WIDTH - 1 : 0]   m_color
);
`include "RegisterAndDescriptorDefines.vh"

    localparam SIGN_WIDTH = 1;
    localparam SUB_PIXEL_WIDTH_SIGNED = SUB_PIXEL_WIDTH + SIGN_WIDTH;
    localparam PIXEL_WIDTH_SIGNED = SUB_PIXEL_WIDTH_SIGNED * NUMBER_OF_SUB_PIXEL;

    localparam [SUB_PIXEL_WIDTH - 1 : 0] ONE_DOT_ZERO_UNSIGNED = { SUB_PIXEL_WIDTH{1'b1} };
    localparam signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] ONE_DOT_ZERO = { 1'b0, { SUB_PIXEL_WIDTH{1'b1} } };
    localparam signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] ZERO_DOT_FIVE = { 1'b0, 1'b0, ONE_DOT_ZERO[0 +: SUB_PIXEL_WIDTH - 1] };

    localparam COLOR_A_SIGNED_POS = SUB_PIXEL_WIDTH_SIGNED * 0;
    localparam COLOR_B_SIGNED_POS = SUB_PIXEL_WIDTH_SIGNED * 1;
    localparam COLOR_G_SIGNED_POS = SUB_PIXEL_WIDTH_SIGNED * 2;
    localparam COLOR_R_SIGNED_POS = SUB_PIXEL_WIDTH_SIGNED * 3;

    `SaturateCastSignedToUnsigned(SaturateCastSignedToUnsigned, SUB_PIXEL_WIDTH_SIGNED)
    `ReduceAndSaturateSigned(ReduceAndSaturateSigned, SUB_PIXEL_WIDTH_SIGNED + 2, SUB_PIXEL_WIDTH_SIGNED)
    `ExpandSigned(ExpandSigned, SUB_PIXEL_WIDTH_SIGNED, SUB_PIXEL_WIDTH_SIGNED + 2)

    function [SUB_PIXEL_WIDTH - 1 : 0] SelectRgbOperand;
        input [ 1 : 0]                  conf;
        input [SUB_PIXEL_WIDTH - 1 : 0] subPixel;
        input [SUB_PIXEL_WIDTH - 1 : 0] alpha;
        
        case (conf)
            OPERAND_RGB_SRC_COLOR:
            begin
                SelectRgbOperand = subPixel;
            end
            OPERAND_RGB_ONE_MINUS_SRC_COLOR:
            begin
                SelectRgbOperand = ONE_DOT_ZERO_UNSIGNED - subPixel;
            end
            OPERAND_RGB_SRC_ALPHA:
            begin
                SelectRgbOperand = alpha;
            end
            OPERAND_RGB_ONE_MINUS_SRC_ALPHA:
            begin
                SelectRgbOperand = ONE_DOT_ZERO_UNSIGNED - alpha;
            end
        endcase
    endfunction

    function [SUB_PIXEL_WIDTH - 1 : 0] SelectAlphaOperand;
        input [ 0 : 0]                  conf;
        input [SUB_PIXEL_WIDTH - 1 : 0] alpha;
        
        case (conf)
            OPERAND_ALPHA_SRC_ALPHA:
            begin
                SelectAlphaOperand = alpha;
            end
            OPERAND_ALPHA_ONE_MINUS_SRC_ALPHA:
            begin
                SelectAlphaOperand = ONE_DOT_ZERO_UNSIGNED - alpha;
            end
        endcase
    endfunction

    function [SUB_PIXEL_WIDTH - 1 : 0] SelectSrcColor;
        input [ 1 : 0]                  conf;
        input [SUB_PIXEL_WIDTH - 1 : 0] subPixelTexture;
        input [SUB_PIXEL_WIDTH - 1 : 0] subPixelConstant;
        input [SUB_PIXEL_WIDTH - 1 : 0] subPixelPrimaryColor;
        input [SUB_PIXEL_WIDTH - 1 : 0] subPixelPrevious;

        case (conf)
            SRC_TEXTURE:
            begin
                SelectSrcColor = subPixelTexture;
            end
            SRC_CONSTANT:
            begin
                SelectSrcColor = subPixelConstant;
            end
            SRC_PRIMARY_COLOR:
            begin
                SelectSrcColor = subPixelPrimaryColor;
            end
            SRC_PREVIOUS:
            begin
                SelectSrcColor = subPixelPrevious;
            end
        endcase
    endfunction

    wire ce;
    assign ce = m_ready;
    assign s_ready = m_ready;
    
    wire [ 2 : 0] combineRgb    = conf[ RENDER_CONFIG_TMU_TEX_ENV_COMBINE_RGB_POS    +: RENDER_CONFIG_TMU_TEX_ENV_COMBINE_RGB_SIZE ];
    wire [ 2 : 0] combineAlpha  = conf[ RENDER_CONFIG_TMU_TEX_ENV_COMBINE_ALPHA_POS  +: RENDER_CONFIG_TMU_TEX_ENV_COMBINE_ALPHA_SIZE ];
    wire [ 1 : 0] srcRegRgb0    = conf[ RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB0_POS   +: RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB0_SIZE ];
    wire [ 1 : 0] srcRegRgb1    = conf[ RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB1_POS   +: RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB1_SIZE ];
    wire [ 1 : 0] srcRegRgb2    = conf[ RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB2_POS   +: RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB2_SIZE ]; 
    wire [ 1 : 0] srcRegAlpha0  = conf[ RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA0_POS +: RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA0_SIZE ];
    wire [ 1 : 0] srcRegAlpha1  = conf[ RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA1_POS +: RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA1_SIZE ];
    wire [ 1 : 0] srcRegAlpha2  = conf[ RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA2_POS +: RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA2_SIZE ];
    wire [ 1 : 0] operandRgb0   = conf[ RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB0_POS   +: RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB0_SIZE ];
    wire [ 1 : 0] operandRgb1   = conf[ RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB1_POS   +: RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB1_SIZE ];
    wire [ 1 : 0] operandRgb2   = conf[ RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB2_POS   +: RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB2_SIZE ];
    wire [ 0 : 0] operandAlpha0 = conf[ RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA0_POS +: RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA0_SIZE ];
    wire [ 0 : 0] operandAlpha1 = conf[ RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA1_POS +: RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA1_SIZE ];
    wire [ 0 : 0] operandAlpha2 = conf[ RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA2_POS +: RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA2_SIZE ];

    initial 
    begin
        if (COLOR_R_POS != (SUB_PIXEL_WIDTH * 3)) 
        begin
            $error("Expecting red to be the 3. sub pixel.");
            $finish;
        end

        if (COLOR_G_POS != (SUB_PIXEL_WIDTH * 2)) 
        begin
            $error("Expecting green to be the 2. sub pixel.");
            $finish;
        end

        if (COLOR_B_POS != (SUB_PIXEL_WIDTH * 1)) 
        begin
            $error("Expecting blue to be the 1. sub pixel.");
            $finish;
        end

        if (COLOR_A_POS != (SUB_PIXEL_WIDTH * 0)) 
        begin
            $error("Expecting alpha to be the 0. sub pixel.");
            $finish;
        end
    end

    ////////////////////////////////////////////////////////////////////////////
    // GLOBAL DELAY
    // Delays parameters globally to use them in further steps in the pipeline.
    ////////////////////////////////////////////////////////////////////////////
    wire [ 2 : 0] combineRgbDelay;
    ValueDelay #(.VALUE_SIZE(RENDER_CONFIG_TMU_TEX_ENV_COMBINE_RGB_SIZE), .DELAY(3)) 
        glob_combineRgbDelay (.clk(aclk), .ce(ce), .in(combineRgb), .out(combineRgbDelay));

    ValueDelay #(.VALUE_SIZE(1), .DELAY(4)) 
        glob_validDelay (.clk(aclk), .ce(ce), .in(s_valid), .out(m_valid));
    
    ValueDelay #(.VALUE_SIZE(USER_WIDTH), .DELAY(4)) 
        glob_userDelay (.clk(aclk), .ce(ce), .in(s_user), .out(m_user));

    ////////////////////////////////////////////////////////////////////////////
    // STEP 0
    // Select parameters and arguments
    // Clocks: 1
    ////////////////////////////////////////////////////////////////////////////
    reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] v00;
    reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] v01;
    reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] v02;
    reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] v03;
    reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] v10;
    reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] v11;
    reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] v12;
    reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] v13;
    reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] v20;
    reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] v21;
    reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] v22;
    reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] v23;
    reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] v30;
    reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] v31;
    reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] v32;
    reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] v33;
    always @(posedge aclk)
    if (ce) begin : SelectColor
        reg [SUB_PIXEL_WIDTH - 1 : 0] rt;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gt;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bt;
        reg [SUB_PIXEL_WIDTH - 1 : 0] at;
        reg [SUB_PIXEL_WIDTH - 1 : 0] rc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] ac;
        reg [SUB_PIXEL_WIDTH - 1 : 0] rpc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gpc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bpc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] apc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] rp;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gp;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bp;
        reg [SUB_PIXEL_WIDTH - 1 : 0] ap;

        reg [SUB_PIXEL_WIDTH - 1 : 0] ru0;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gu0;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bu0;
        reg [SUB_PIXEL_WIDTH - 1 : 0] au0;
        reg [SUB_PIXEL_WIDTH - 1 : 0] ru1;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gu1;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bu1;
        reg [SUB_PIXEL_WIDTH - 1 : 0] au1;
        reg [SUB_PIXEL_WIDTH - 1 : 0] ru2;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gu2;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bu2;
        reg [SUB_PIXEL_WIDTH - 1 : 0] au2;

        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] r0;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] g0;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] b0;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] a0;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] r1;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] g1;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] b1;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] a1;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] r2;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] g2;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] b2;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] a2;

        rt = s_texSrcColor[COLOR_R_POS +: SUB_PIXEL_WIDTH];
        gt = s_texSrcColor[COLOR_G_POS +: SUB_PIXEL_WIDTH];
        bt = s_texSrcColor[COLOR_B_POS +: SUB_PIXEL_WIDTH];
        at = s_texSrcColor[COLOR_A_POS +: SUB_PIXEL_WIDTH];

        rc = s_envColor[COLOR_R_POS +: SUB_PIXEL_WIDTH];
        gc = s_envColor[COLOR_G_POS +: SUB_PIXEL_WIDTH];
        bc = s_envColor[COLOR_B_POS +: SUB_PIXEL_WIDTH];
        ac = s_envColor[COLOR_A_POS +: SUB_PIXEL_WIDTH];

        rpc = s_primaryColor[COLOR_R_POS +: SUB_PIXEL_WIDTH];
        gpc = s_primaryColor[COLOR_G_POS +: SUB_PIXEL_WIDTH];
        bpc = s_primaryColor[COLOR_B_POS +: SUB_PIXEL_WIDTH];
        apc = s_primaryColor[COLOR_A_POS +: SUB_PIXEL_WIDTH];

        rp = s_previousColor[COLOR_R_POS +: SUB_PIXEL_WIDTH];
        gp = s_previousColor[COLOR_G_POS +: SUB_PIXEL_WIDTH];
        bp = s_previousColor[COLOR_B_POS +: SUB_PIXEL_WIDTH];
        ap = s_previousColor[COLOR_A_POS +: SUB_PIXEL_WIDTH];

        ru0 = SelectRgbOperand(operandRgb0, 
                              SelectSrcColor(srcRegRgb0, rt, rc, rpc, rp), 
                              SelectSrcColor(srcRegRgb0, at, ac, apc, ap));

        ru1 = SelectRgbOperand(operandRgb1, 
                              SelectSrcColor(srcRegRgb1, rt, rc, rpc, rp), 
                              SelectSrcColor(srcRegRgb1, at, ac, apc, ap));

        ru2 = SelectRgbOperand(operandRgb2, 
                              SelectSrcColor(srcRegRgb2, rt, rc, rpc, rp), 
                              SelectSrcColor(srcRegRgb2, at, ac, apc, ap));

        gu0 = SelectRgbOperand(operandRgb0, 
                              SelectSrcColor(srcRegRgb0, gt, gc, gpc, gp), 
                              SelectSrcColor(srcRegRgb0, at, ac, apc, ap));

        gu1 = SelectRgbOperand(operandRgb1, 
                              SelectSrcColor(srcRegRgb1, gt, gc, gpc, gp), 
                              SelectSrcColor(srcRegRgb1, at, ac, apc, ap));

        gu2 = SelectRgbOperand(operandRgb2, 
                              SelectSrcColor(srcRegRgb2, gt, gc, gpc, gp), 
                              SelectSrcColor(srcRegRgb2, at, ac, apc, ap));

        bu0 = SelectRgbOperand(operandRgb0, 
                              SelectSrcColor(srcRegRgb0, bt, bc, bpc, bp), 
                              SelectSrcColor(srcRegRgb0, at, ac, apc, ap));

        bu1 = SelectRgbOperand(operandRgb1, 
                              SelectSrcColor(srcRegRgb1, bt, bc, bpc, bp), 
                              SelectSrcColor(srcRegRgb1, at, ac, apc, ap));

        bu2 = SelectRgbOperand(operandRgb2, 
                              SelectSrcColor(srcRegRgb2, bt, bc, bpc, bp), 
                              SelectSrcColor(srcRegRgb2, at, ac, apc, ap));

        au0 = SelectAlphaOperand(operandAlpha0, 
                                SelectSrcColor(srcRegAlpha0, at, ac, apc, ap));

        au1 = SelectAlphaOperand(operandAlpha1, 
                                SelectSrcColor(srcRegAlpha1, at, ac, apc, ap));

        au2 = SelectAlphaOperand(operandAlpha2,
                                SelectSrcColor(srcRegAlpha2, at, ac, apc, ap));

        r0 = {1'b0, ru0};
        g0 = {1'b0, gu0};
        b0 = {1'b0, bu0};
        a0 = {1'b0, au0};
        r1 = {1'b0, ru1};
        g1 = {1'b0, gu1};
        b1 = {1'b0, bu1};
        a1 = {1'b0, au1};
        r2 = {1'b0, ru2};
        g2 = {1'b0, gu2};
        b2 = {1'b0, bu2};
        a2 = {1'b0, au2};

        case (combineRgb)
            REPLACE:
            begin
                // (x0 * 1.0) + (0.0 * 0.0)

                v00 = r0;
                v01 = g0;
                v02 = b0;

                v10 = ONE_DOT_ZERO;
                v11 = ONE_DOT_ZERO;
                v12 = ONE_DOT_ZERO;

                v20 = 0;
                v21 = 0;
                v22 = 0;

                v30 = 0;
                v31 = 0;
                v32 = 0;
            end
            MODULATE:
            begin
                // (x0 * x1) + (0 * 0)

                v00 = r0;
                v01 = g0;
                v02 = b0;

                v10 = r1;
                v11 = g1;
                v12 = b1;

                v20 = 0;
                v21 = 0;
                v22 = 0;

                v30 = 0;
                v31 = 0;
                v32 = 0;
            end
            ADD:
            begin 
                // (x0 * 1.0) + (x1 * 1.0)

                v00 = r0;
                v01 = g0;
                v02 = b0;

                v10 = ONE_DOT_ZERO;
                v11 = ONE_DOT_ZERO;
                v12 = ONE_DOT_ZERO;

                v20 = r1;
                v21 = g1;
                v22 = b1;

                v30 = ONE_DOT_ZERO;
                v31 = ONE_DOT_ZERO;
                v32 = ONE_DOT_ZERO;

            end
            ADD_SIGNED:
            begin 
                // (x0 * 1.0) + ((x1 - 0.5) * 1.0)

                v00 = r0;
                v01 = g0;
                v02 = b0;

                v10 = ONE_DOT_ZERO;
                v11 = ONE_DOT_ZERO;
                v12 = ONE_DOT_ZERO;

                v20 = r1 - ZERO_DOT_FIVE;
                v21 = g1 - ZERO_DOT_FIVE;
                v22 = b1 - ZERO_DOT_FIVE;

                v30 = ONE_DOT_ZERO;
                v31 = ONE_DOT_ZERO;
                v32 = ONE_DOT_ZERO;

            end
            INTERPOLATE:
            begin
                // (x0 * x2) + (x1 * (1.0 - x2))   
                
                v00 = r0;
                v01 = g0;
                v02 = b0;

                v10 = r2;
                v11 = g2;
                v12 = b2;

                v20 = r1;
                v21 = g1;
                v22 = b1;

                v30 = ONE_DOT_ZERO - r2;
                v31 = ONE_DOT_ZERO - g2;
                v32 = ONE_DOT_ZERO - b2;
            end 
            SUBTRACT:
            begin
                // (x0 * 1.0) + (x1 * -1.0)

                v00 = r0;
                v01 = g0;
                v02 = b0;

                v10 = ONE_DOT_ZERO;
                v11 = ONE_DOT_ZERO;
                v12 = ONE_DOT_ZERO;

                v20 = r1;
                v21 = g1;
                v22 = b1;

                v30 = -ONE_DOT_ZERO;
                v31 = -ONE_DOT_ZERO;
                v32 = -ONE_DOT_ZERO;
            end
            DOT3_RGB:
            begin
                // ((x0 - 0.5) * (x1 - 0.5)) + (0 * 0)

                v00 = r0 - ZERO_DOT_FIVE;
                v01 = g0 - ZERO_DOT_FIVE;
                v02 = b0 - ZERO_DOT_FIVE;

                v10 = r1 - ZERO_DOT_FIVE;
                v11 = g1 - ZERO_DOT_FIVE;
                v12 = b1 - ZERO_DOT_FIVE;

                v20 = 0;
                v21 = 0;
                v22 = 0;

                v30 = 0;
                v31 = 0;
                v32 = 0;
            end
            DOT3_RGBA:
            begin
                // ((x0 - 0.5) * (x1 - 0.5)) + (0 * 0)

                v00 = r0 - ZERO_DOT_FIVE;
                v01 = g0 - ZERO_DOT_FIVE;
                v02 = b0 - ZERO_DOT_FIVE;

                v10 = r1 - ZERO_DOT_FIVE;
                v11 = g1 - ZERO_DOT_FIVE;
                v12 = b1 - ZERO_DOT_FIVE;

                v20 = 0;
                v21 = 0;
                v22 = 0;

                v30 = 0;
                v31 = 0;
                v32 = 0;
            end
            default:
            begin
                
            end 
        endcase

        case (combineAlpha)
            REPLACE:
            begin
                // (x0 * 1.0) + (0.0 * 0.0)

                v03 = a0;
                v13 = ONE_DOT_ZERO;
                v23 = 0;
                v33 = 0;
            end
            MODULATE:
            begin
                // (x0 * x1) + (0 * 0)

                v03 = a0;
                v13 = a1;
                v23 = 0;
                v33 = 0;
            end
            ADD:
            begin 
                // (x0 * 1.0) + (x1 * 1.0)

                v03 = a0;
                v13 = ONE_DOT_ZERO;
                v23 = a1;
                v33 = ONE_DOT_ZERO;

            end
            ADD_SIGNED:
            begin 
                // (x0 * 1.0) + ((x1 - 0.5) * 1.0)

                v03 = a0;
                v13 = ONE_DOT_ZERO;
                v23 = a1 - ZERO_DOT_FIVE;
                v33 = ONE_DOT_ZERO;

            end
            INTERPOLATE:
            begin
                // (x0 * x2) + (x1 * (1.0 - x2))   
                
                v03 = a0;
                v13 = a2;
                v23 = a1;
                v33 = ONE_DOT_ZERO - a2;
            end 
            SUBTRACT:
            begin
                // (x0 * 1.0) + (x1 * -1.0)

                v03 = a0;
                v13 = ONE_DOT_ZERO;
                v23 = a1;
                v33 = -ONE_DOT_ZERO;
            end
            default:
            begin
                
            end 
        endcase
    end

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Mix colors
    // Clocks: 2
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH_SIGNED - 1 : 0] step1_color;
    ColorMixerSigned #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH_SIGNED),
        // Add one additional bit to the calc precision, because the config itself does not include the sign bit
        .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION + 1) 
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

        .mixedColor(step1_color)
    );
    
    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Calculate dot product sum
    // Clocks: 1
    ////////////////////////////////////////////////////////////////////////////
    always @(posedge aclk)
    if (ce) begin : DotCalculation
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] rc;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] gc;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] bc;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] ac;
        reg signed [SUB_PIXEL_WIDTH_SIGNED + 1 : 0] dotSum;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] dot;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] dotScaled;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] rcScaled;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] gcScaled;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] bcScaled;
        reg signed [SUB_PIXEL_WIDTH_SIGNED - 1 : 0] acScaled;

        rc = $signed(step1_color[COLOR_R_SIGNED_POS +: SUB_PIXEL_WIDTH_SIGNED]);
        gc = $signed(step1_color[COLOR_G_SIGNED_POS +: SUB_PIXEL_WIDTH_SIGNED]);
        bc = $signed(step1_color[COLOR_B_SIGNED_POS +: SUB_PIXEL_WIDTH_SIGNED]);
        ac = $signed(step1_color[COLOR_A_SIGNED_POS +: SUB_PIXEL_WIDTH_SIGNED]);

        dotSum = (ExpandSigned(rc) + ExpandSigned(gc) + ExpandSigned(bc)) <<< 2;

        dot = ReduceAndSaturateSigned(dotSum);

        dotScaled = ReduceAndSaturateSigned(ExpandSigned(dot) << conf[RENDER_CONFIG_TMU_TEX_ENV_SHIFT_RGB_POS +: RENDER_CONFIG_TMU_TEX_ENV_SHIFT_RGB_SIZE]);
        rcScaled = ReduceAndSaturateSigned(ExpandSigned(rc) << conf[RENDER_CONFIG_TMU_TEX_ENV_SHIFT_RGB_POS +: RENDER_CONFIG_TMU_TEX_ENV_SHIFT_RGB_SIZE]);
        gcScaled = ReduceAndSaturateSigned(ExpandSigned(gc) << conf[RENDER_CONFIG_TMU_TEX_ENV_SHIFT_RGB_POS +: RENDER_CONFIG_TMU_TEX_ENV_SHIFT_RGB_SIZE]);
        bcScaled = ReduceAndSaturateSigned(ExpandSigned(bc) << conf[RENDER_CONFIG_TMU_TEX_ENV_SHIFT_RGB_POS +: RENDER_CONFIG_TMU_TEX_ENV_SHIFT_RGB_SIZE]);
        acScaled = ReduceAndSaturateSigned(ExpandSigned(ac) << conf[RENDER_CONFIG_TMU_TEX_ENV_SHIFT_ALPHA_POS +: RENDER_CONFIG_TMU_TEX_ENV_SHIFT_ALPHA_SIZE]);
        
        case (combineRgbDelay)
            DOT3_RGBA:
                m_color <= {
                    SaturateCastSignedToUnsigned(dotScaled),
                    SaturateCastSignedToUnsigned(dotScaled),
                    SaturateCastSignedToUnsigned(dotScaled),
                    SaturateCastSignedToUnsigned(dotScaled)
                };
            DOT3_RGB:
                m_color <= {
                    SaturateCastSignedToUnsigned(dotScaled),
                    SaturateCastSignedToUnsigned(dotScaled),
                    SaturateCastSignedToUnsigned(dotScaled),
                    SaturateCastSignedToUnsigned(acScaled)
                };
            default: 
                m_color <= {
                    SaturateCastSignedToUnsigned(rcScaled),
                    SaturateCastSignedToUnsigned(gcScaled),
                    SaturateCastSignedToUnsigned(bcScaled),
                    SaturateCastSignedToUnsigned(acScaled)
                };
        endcase
    end

endmodule