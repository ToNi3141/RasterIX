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

// This module calculates the fragment color.
// It samples a texel from the texture memory, filters it and 
// executes the texture environment.
// Pipelined: yes
// Depth: 14 cycles
module TextureMappingUnit
#(
    parameter USER_WIDTH  = 1,
    parameter SUB_PIXEL_WIDTH = 8,
    parameter SUB_PIXEL_CALC_PRECISION = SUB_PIXEL_WIDTH,

    parameter ENABLE_LOD_CALC = 1,
    parameter ENABLE_TEXTURE_FILTERING = 1,

    localparam PIXEL_WIDTH = 4 * SUB_PIXEL_WIDTH,

    localparam ADDR_WIDTH = 17 // Based on the maximum texture size, of 256x256 (8 bit x 8 bit) + mipmap levels in PIXEL_WIDTH word addresses
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    // TMU configurations
    input  wire [31 : 0]                confFunc, // See TexEnv for more documentation
    input  wire [PIXEL_WIDTH - 1 : 0]   confTextureEnvColor, // CONSTANT
    input  wire [31 : 0]                confTextureConfig,
    input  wire                         confEnable,

    // Texture memory access of a texel quad
    output wire [ADDR_WIDTH - 1 : 0]    texelAddr00,
    output wire [ADDR_WIDTH - 1 : 0]    texelAddr01,
    output wire [ADDR_WIDTH - 1 : 0]    texelAddr10,
    output wire [ADDR_WIDTH - 1 : 0]    texelAddr11,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelInput00,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelInput01,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelInput10,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelInput11,

    // Fragment input
    output wire                         s_ready,
    input  wire                         s_valid,
    input  wire [USER_WIDTH - 1 : 0]    s_user,
    input  wire [PIXEL_WIDTH - 1 : 0]   s_primaryColor, // PRIMARY_COLOR
    input  wire [31 : 0]                s_textureS,
    input  wire [31 : 0]                s_textureT,
    input  wire [31 : 0]                s_mipmapS,
    input  wire [31 : 0]                s_mipmapT,
    input  wire [PIXEL_WIDTH - 1 : 0]   s_previousColor, // PREVIOUS
    
    // Fragment output
    input  wire                         m_ready,
    output wire                         m_valid,
    output wire [USER_WIDTH - 1 : 0]    m_user,
    output wire [PIXEL_WIDTH - 1 : 0]   m_fragmentColor
);
`include "RegisterAndDescriptorDefines.vh"

    ////////////////////////////////////////////////////////////////////////////
    // STEP 0
    // LOD calculation
    // Clocks: 1
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step0_primaryColor;
    wire [PIXEL_WIDTH - 1 : 0]  step0_previousColor;
    wire [PIXEL_WIDTH - 1 : 0]  step0_textureS;
    wire [PIXEL_WIDTH - 1 : 0]  step0_textureT;
    wire [ 3 : 0]               step0_lod;
    wire                        step0_ready;
    wire                        step0_valid;
    wire [USER_WIDTH - 1 : 0]   step0_user;

    generate 
        if (ENABLE_LOD_CALC)
        begin
            LodCalculator #(
                .USER_WIDTH((PIXEL_WIDTH * 4) + USER_WIDTH)
            ) lodCalculator (
                .aclk(aclk),
                .resetn(resetn),

                .confEnable(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_MIN_FILTER_POS +: RENDER_CONFIG_TMU_TEXTURE_MIN_FILTER_SIZE]),

                .s_valid(s_valid),
                .s_ready(s_ready),
                .s_user({
                    s_primaryColor,
                    s_previousColor,
                    s_textureS,
                    s_textureT,
                    s_user
                }),
                .s_textureSizeWidth(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_WIDTH_POS +: RENDER_CONFIG_TMU_TEXTURE_WIDTH_SIZE]),
                .s_textureSizeHeight(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_HEIGHT_POS +: RENDER_CONFIG_TMU_TEXTURE_HEIGHT_SIZE]),
                .s_texelS(s_textureS),
                .s_texelT(s_textureT),
                .s_texelSxy(s_mipmapS),
                .s_texelTxy(s_mipmapT),

                .m_valid(step0_valid),
                .m_ready(step0_ready),
                .m_user({
                    step0_primaryColor,
                    step0_previousColor,
                    step0_textureS,
                    step0_textureT,
                    step0_user
                }),
                .m_lod(step0_lod)
            );
        end
        else
        begin
            assign step0_lod = 0;
            assign step0_primaryColor = s_primaryColor;
            assign step0_previousColor = s_previousColor;
            assign step0_textureS = s_textureS;
            assign step0_textureT = s_textureT;
            assign step0_user = s_user; 
            assign step0_valid = s_valid;
            assign s_ready = step0_ready;
        end
    endgenerate

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Sample Texture
    // Clocks: 5
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step1_texel00;
    wire [PIXEL_WIDTH - 1 : 0]  step1_texel01;
    wire [PIXEL_WIDTH - 1 : 0]  step1_texel10;
    wire [PIXEL_WIDTH - 1 : 0]  step1_texel11;
    wire [15 : 0]               step1_texelSubCoordS;
    wire [15 : 0]               step1_texelSubCoordT;
    wire [PIXEL_WIDTH - 1 : 0]  step1_primaryColor;
    wire [PIXEL_WIDTH - 1 : 0]  step1_previousColor;
    wire                        step1_ready;
    wire                        step1_valid;
    wire [USER_WIDTH - 1 : 0]   step1_user;

    TextureSampler #(
        .USER_WIDTH((2 * PIXEL_WIDTH) + USER_WIDTH),
        .PIXEL_WIDTH(PIXEL_WIDTH)
    ) textureSampler (
        .aclk(aclk),
        .resetn(resetn),

        .textureSizeWidth(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_WIDTH_POS +: RENDER_CONFIG_TMU_TEXTURE_WIDTH_SIZE]),
        .textureSizeHeight(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_HEIGHT_POS +: RENDER_CONFIG_TMU_TEXTURE_HEIGHT_SIZE]),
        .enableHalfPixelOffset(ENABLE_TEXTURE_FILTERING & confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_MAG_FILTER_POS +: RENDER_CONFIG_TMU_TEXTURE_MAG_FILTER_SIZE]), 

        .texelAddr00(texelAddr00),
        .texelAddr01(texelAddr01),
        .texelAddr10(texelAddr10),
        .texelAddr11(texelAddr11),
        .texelInput00(texelInput00),
        .texelInput01(texelInput01),
        .texelInput10(texelInput10),
        .texelInput11(texelInput11),

        .s_valid(step0_valid),
        .s_ready(step0_ready),
        .s_user({
            step0_primaryColor,
            step0_previousColor,
            step0_user
        }),
        .s_texelS(step0_textureS),
        .s_texelT(step0_textureT),
        .s_clampS(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_CLAMP_S_POS +: RENDER_CONFIG_TMU_TEXTURE_CLAMP_S_SIZE]),
        .s_clampT(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_CLAMP_T_POS +: RENDER_CONFIG_TMU_TEXTURE_CLAMP_T_SIZE]),
        .s_textureLod(step0_lod),
        
        .m_valid(step1_valid),
        .m_ready(step1_ready),
        .m_user({
            step1_primaryColor,
            step1_previousColor,
            step1_user
        }),
        .m_texel00(step1_texel00),
        .m_texel01(step1_texel01),
        .m_texel10(step1_texel10),
        .m_texel11(step1_texel11),
        .m_texelSubCoordS(step1_texelSubCoordS),
        .m_texelSubCoordT(step1_texelSubCoordT)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Filter Texture
    // Clocks: 4
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step2_primaryColor;
    wire [PIXEL_WIDTH - 1 : 0]  step2_previousColor;
    wire [PIXEL_WIDTH - 1 : 0]  step2_texel;
    wire                        step2_ready;
    wire                        step2_valid;
    wire [USER_WIDTH - 1 : 0]   step2_user;

    TextureFilter #(
        .USER_WIDTH((2 * PIXEL_WIDTH) + USER_WIDTH),
        .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION)
    ) texFilter (
        .aclk(aclk),
        .resetn(resetn),

        .enable(ENABLE_TEXTURE_FILTERING & confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_MAG_FILTER_POS +: RENDER_CONFIG_TMU_TEXTURE_MAG_FILTER_SIZE]),

        .s_valid(step1_valid),
        .s_ready(step1_ready),
        .s_user({
            step1_primaryColor,
            step1_previousColor,
            step1_user
        }),
        .s_texel00(step1_texel00),
        .s_texel01(step1_texel01),
        .s_texel10(step1_texel10),
        .s_texel11(step1_texel11),
        .s_texelSubCoordS(step1_texelSubCoordS),
        .s_texelSubCoordT(step1_texelSubCoordT),

        .m_valid(step2_valid),
        .m_ready(step2_ready),
        .m_user({
            step2_primaryColor,
            step2_previousColor,
            step2_user
        }),
        .m_texel(step2_texel)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 3
    // Calculate texture environment
    // Clocks: 4
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step3_texel;
    wire [PIXEL_WIDTH - 1 : 0]  step3_previousColor;
    wire                        step3_ready;
    wire                        step3_valid;
    wire [USER_WIDTH - 1 : 0]   step3_user;

    TexEnv #(
        .USER_WIDTH(PIXEL_WIDTH + USER_WIDTH),
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
        .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION)
    ) texEnv (
        .aclk(aclk),
        .resetn(resetn),

        .conf(confFunc),

        .s_valid(step2_valid),
        .s_ready(step2_ready),
        .s_user({
            step2_previousColor,
            step2_user
        }),
        .s_previousColor(step2_previousColor),
        .s_texSrcColor(step2_texel),
        .s_primaryColor(step2_primaryColor),
        .s_envColor(confTextureEnvColor),

        .m_valid(step3_valid),
        .m_ready(step3_ready),
        .m_user({
            step3_previousColor,
            step3_user
        }),
        .m_color(step3_texel)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 3
    // Output final texel color
    // Clocks: 0
    ////////////////////////////////////////////////////////////////////////////
    assign m_fragmentColor = (confEnable) ? step3_texel : step3_previousColor;
    assign m_valid = step3_valid;
    assign m_user = step3_user;
    assign step3_ready = m_ready;

endmodule


 