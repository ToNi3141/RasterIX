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

// This module is used to calculate a whole pixel, starting from the
// meta data from the rasterizer till it is written into the frame buffer,
// including, blending, fogging, texenv, testing and so on.
// It prepares the meta data (basically converts the floats from the)
// interpolator into fixed point numbers, which can be used from the 
// fragment and framebuffer pipeline.
// Pipelined: yes
// Depth: 30 cycles
module PixelPipeline
#(
    // The minimum bit width which is required to contain the resolution
    parameter INDEX_WIDTH = 14,

    parameter SUB_PIXEL_WIDTH = 8,
    parameter SUB_PIXEL_CALC_PRECISION = SUB_PIXEL_WIDTH,
    localparam PIXEL_WIDTH = 4 * SUB_PIXEL_WIDTH,

    localparam FLOAT_SIZE = 32,

    localparam TEX_ADDR_WIDTH = 17,

    parameter ENABLE_SECOND_TMU = 1,
    parameter ENABLE_LOD_CALC = 1,
    parameter ENABLE_TEXTURE_FILTERING = 1,
    parameter ENABLE_FOG = 1,
    
    parameter SCREEN_POS_WIDTH = 11,

    localparam KEEP_WIDTH = 1,

    localparam ATTRIBUTE_SIZE = 32
)
(
    input  wire                                     aclk,
    input  wire                                     resetn,

    // Fog function LUT stream
    input  wire                                     s_fog_lut_axis_tvalid,
    output wire                                     s_fog_lut_axis_tready,
    input  wire                                     s_fog_lut_axis_tlast,
    input  wire [31 : 0]                            s_fog_lut_axis_tdata,

    // Shader configurations
    input  wire [31 : 0]                            confFeatureEnable,
    input  wire [31 : 0]                            confFragmentPipelineConfig,
    input  wire [PIXEL_WIDTH - 1 : 0]               confFragmentPipelineFogColor,
    input  wire [31 : 0]                            confStencilBufferConfig,
    input  wire [31 : 0]                            confTMU0TexEnvConfig,
    input  wire [31 : 0]                            confTMU0TextureConfig,
    input  wire [PIXEL_WIDTH - 1 : 0]               confTMU0TexEnvColor,
    input  wire [31 : 0]                            confTMU1TexEnvConfig,
    input  wire [31 : 0]                            confTMU1TextureConfig,
    input  wire [PIXEL_WIDTH - 1 : 0]               confTMU1TexEnvColor,

    // Fragment Stream
    output wire                                     s_attrb_tready,
    input  wire                                     s_attrb_tvalid,
    input  wire                                     s_attrb_tlast,
    input  wire [KEEP_WIDTH - 1 : 0]                s_attrb_tkeep,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]          s_attrb_tspx,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]          s_attrb_tspy,
    input  wire [INDEX_WIDTH - 1 : 0]               s_attrb_tindex,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]            s_attrb_tdepth_w, // Float
    input  wire [31 : 0]                            s_attrb_tdepth_z, // Q16.16
    input  wire [31 : 0]                            s_attrb_ttexture0_t, // S16.15
    input  wire [31 : 0]                            s_attrb_ttexture0_s, // S16.15
    input  wire [31 : 0]                            s_attrb_tmipmap0_t, // S16.15
    input  wire [31 : 0]                            s_attrb_tmipmap0_s, // S16.15
    input  wire [31 : 0]                            s_attrb_ttexture1_t, // S16.15
    input  wire [31 : 0]                            s_attrb_ttexture1_s, // S16.15
    input  wire [31 : 0]                            s_attrb_tmipmap1_t, // S16.15
    input  wire [31 : 0]                            s_attrb_tmipmap1_s, // S16.15
    input  wire [SUB_PIXEL_WIDTH - 1 : 0]           s_attrb_tcolor_a, // Qn
    input  wire [SUB_PIXEL_WIDTH - 1 : 0]           s_attrb_tcolor_b, // Qn
    input  wire [SUB_PIXEL_WIDTH - 1 : 0]           s_attrb_tcolor_g, // Qn
    input  wire [SUB_PIXEL_WIDTH - 1 : 0]           s_attrb_tcolor_r, // Qn

    // Texture access
    // TMU0 texel quad access
    output wire [TEX_ADDR_WIDTH - 1 : 0]            texel0Addr00,
    output wire [TEX_ADDR_WIDTH - 1 : 0]            texel0Addr01,
    output wire [TEX_ADDR_WIDTH - 1 : 0]            texel0Addr10,
    output wire [TEX_ADDR_WIDTH - 1 : 0]            texel0Addr11,
    input  wire [PIXEL_WIDTH - 1 : 0]               texel0Input00,
    input  wire [PIXEL_WIDTH - 1 : 0]               texel0Input01,
    input  wire [PIXEL_WIDTH - 1 : 0]               texel0Input10,
    input  wire [PIXEL_WIDTH - 1 : 0]               texel0Input11,
    // TMU1 texel quad access
    output wire [TEX_ADDR_WIDTH - 1 : 0]            texel1Addr00,
    output wire [TEX_ADDR_WIDTH - 1 : 0]            texel1Addr01,
    output wire [TEX_ADDR_WIDTH - 1 : 0]            texel1Addr10,
    output wire [TEX_ADDR_WIDTH - 1 : 0]            texel1Addr11,
    input  wire [PIXEL_WIDTH - 1 : 0]               texel1Input00,
    input  wire [PIXEL_WIDTH - 1 : 0]               texel1Input01,
    input  wire [PIXEL_WIDTH - 1 : 0]               texel1Input10,
    input  wire [PIXEL_WIDTH - 1 : 0]               texel1Input11,

    input  wire                                     m_frag_tready,
    output wire [PIXEL_WIDTH - 1 : 0]               m_frag_tfragmentColor,
    output wire [INDEX_WIDTH - 1 : 0]               m_frag_tindex,
    output wire [SCREEN_POS_WIDTH - 1 : 0]          m_frag_tscreenPosX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]          m_frag_tscreenPosY,
    output wire [31 : 0]                            m_frag_tdepth,
    output wire                                     m_frag_tvalid,
    output wire                                     m_frag_tlast,
    output wire                                     m_frag_tkeep
);
`include "RegisterAndDescriptorDefines.vh"

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // TMU0
    // Clocks: 14
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]              step1_fragmentColor;
    wire [INDEX_WIDTH - 1 : 0]              step1_index;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step1_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step1_screenPosY;
    wire [31 : 0]                           step1_depth;
    wire [FLOAT_SIZE - 1 : 0]               step1_depthWFloat;
    wire                                    step1_valid;
    wire                                    step1_ready;
    wire [PIXEL_WIDTH - 1 : 0]              step1_primaryColor;
    wire [31 : 0]                           step1_texture1S;
    wire [31 : 0]                           step1_texture1T;
    wire [31 : 0]                           step1_mipmap1S;
    wire [31 : 0]                           step1_mipmap1T;
    wire [KEEP_WIDTH - 1 : 0]               step1_keep;
    wire                                    step1_last;

    wire [PIXEL_WIDTH - 1 : 0] step1_primaryColorTmp = {
        s_attrb_tcolor_r,
        s_attrb_tcolor_g,
        s_attrb_tcolor_b,
        s_attrb_tcolor_a
    };

    TextureMappingUnit #(
        .USER_WIDTH(INDEX_WIDTH + (2 * SCREEN_POS_WIDTH) + 32 + FLOAT_SIZE + KEEP_WIDTH + 1 + (4 * 32) + PIXEL_WIDTH),
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
        .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION),
        .ENABLE_LOD_CALC(ENABLE_LOD_CALC),
        .ENABLE_TEXTURE_FILTERING(ENABLE_TEXTURE_FILTERING)
    ) tmu0 (
        .aclk(aclk),
        .resetn(resetn),

        .confFunc(confTMU0TexEnvConfig),
        .confTextureEnvColor(confTMU0TexEnvColor),
        .confTextureConfig(confTMU0TextureConfig),
        .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_TMU0_POS]),

        .texelAddr00(texel0Addr00),
        .texelAddr01(texel0Addr01),
        .texelAddr10(texel0Addr10),
        .texelAddr11(texel0Addr11),

        .texelInput00(texel0Input00),
        .texelInput01(texel0Input01),
        .texelInput10(texel0Input10),
        .texelInput11(texel0Input11),

        .s_ready(s_attrb_tready),
        .s_valid(s_attrb_tvalid),
        .s_user({
            s_attrb_tindex,
            s_attrb_tspx,
            s_attrb_tspy,
            s_attrb_tdepth_z,
            s_attrb_tdepth_w,
            s_attrb_tkeep,
            s_attrb_tlast,
            s_attrb_ttexture1_s,
            s_attrb_ttexture1_t,
            s_attrb_tmipmap1_s,
            s_attrb_tmipmap1_t,
            step1_primaryColorTmp
        }),
        .s_primaryColor(step1_primaryColorTmp),
        .s_textureS(s_attrb_ttexture0_s),
        .s_textureT(s_attrb_ttexture0_t),
        .s_mipmapS(s_attrb_tmipmap0_s),
        .s_mipmapT(s_attrb_tmipmap0_t),
        .s_previousColor(step1_primaryColorTmp), // For TMU0 it is the primary color, for TMUn-1 it is the output of the previous one

        .m_ready(step1_ready),
        .m_valid(step1_valid),
        .m_user({
            step1_index,
            step1_screenPosX,
            step1_screenPosY,
            step1_depth,
            step1_depthWFloat,
            step1_keep,
            step1_last,
            step1_texture1S,
            step1_texture1T,
            step1_mipmap1S,
            step1_mipmap1T,
            step1_primaryColor
        }),
        .m_fragmentColor(step1_fragmentColor)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // TMU1
    // Clocks: 14
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]              step2_fragmentColor;
    wire [INDEX_WIDTH - 1 : 0]              step2_index;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step2_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step2_screenPosY;
    wire [31 : 0]                           step2_depth;
    wire [FLOAT_SIZE - 1 : 0]               step2_depthWFloat;
    wire                                    step2_ready;
    wire                                    step2_valid;
    wire [KEEP_WIDTH - 1 : 0]               step2_keep;
    wire                                    step2_last;

    generate
        if (ENABLE_SECOND_TMU)
        begin
            TextureMappingUnit #(
                .USER_WIDTH(INDEX_WIDTH + (2 * SCREEN_POS_WIDTH) + 32 + FLOAT_SIZE + KEEP_WIDTH + 1),
                .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
                .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION),
                .ENABLE_LOD_CALC(ENABLE_LOD_CALC),
                .ENABLE_TEXTURE_FILTERING(ENABLE_TEXTURE_FILTERING)
            ) tmu1 (
                .aclk(aclk),
                .resetn(resetn),

                .confFunc(confTMU1TexEnvConfig),
                .confTextureEnvColor(confTMU1TexEnvColor),
                .confTextureConfig(confTMU1TextureConfig),
                .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_TMU1_POS]),

                .texelAddr00(texel1Addr00),
                .texelAddr01(texel1Addr01),
                .texelAddr10(texel1Addr10),
                .texelAddr11(texel1Addr11),

                .texelInput00(texel1Input00),
                .texelInput01(texel1Input01),
                .texelInput10(texel1Input10),
                .texelInput11(texel1Input11),

                .s_ready(step1_ready),
                .s_valid(step1_valid),
                .s_user({
                    step1_index,
                    step1_screenPosX,
                    step1_screenPosY,
                    step1_depth,
                    step1_depthWFloat,
                    step1_keep,
                    step1_last
                }),
                .s_primaryColor(step1_primaryColor),
                .s_textureS(step1_texture1S),
                .s_textureT(step1_texture1T),
                .s_mipmapS(step1_mipmap1S),
                .s_mipmapT(step1_mipmap1T),
                .s_previousColor(step1_fragmentColor),

                .m_ready(step2_ready),
                .m_valid(step2_valid),
                .m_user({
                    step2_index,
                    step2_screenPosX,
                    step2_screenPosY,
                    step2_depth,
                    step2_depthWFloat,
                    step2_keep,
                    step2_last
                }),
                .m_fragmentColor(step2_fragmentColor)
            );
        end
        else
        begin
            assign step2_fragmentColor = step1_fragmentColor;
            assign step2_index = step1_index;
            assign step2_screenPosX = step1_screenPosX;
            assign step2_screenPosY = step1_screenPosY;
            assign step2_depth = step1_depth;
            assign step2_depthWFloat = step1_depthWFloat;
            assign step2_valid = step1_valid;
            assign step2_keep = step1_keep;
            assign step2_last = step1_last;
            assign step1_ready = step2_ready;
            assign texel1Addr00 = 0;
            assign texel1Addr01 = 0;
            assign texel1Addr10 = 0;
            assign texel1Addr11 = 0;
        end
    endgenerate


    ////////////////////////////////////////////////////////////////////////////
    // STEP 3
    // Calculate Fog
    // Clocks: 6
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]              step3_fragmentColor;
    wire [INDEX_WIDTH - 1 : 0]              step3_index;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step3_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step3_screenPosY;
    wire [31 : 0]                           step3_depth;
    wire                                    step3_ready;
    wire                                    step3_valid;
    wire [KEEP_WIDTH - 1 : 0]               step3_keep;
    wire                                    step3_last;

    generate
        if (ENABLE_FOG)
        begin
            Fog #(
                .USER_WIDTH(INDEX_WIDTH + (2 * SCREEN_POS_WIDTH) + 32 + KEEP_WIDTH + 1),
                .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
                .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION)
            ) fog (
                .aclk(aclk),
                .resetn(resetn),

                .s_fog_lut_axis_tvalid(s_fog_lut_axis_tvalid),
                .s_fog_lut_axis_tready(s_fog_lut_axis_tready),
                .s_fog_lut_axis_tlast(s_fog_lut_axis_tlast),
                .s_fog_lut_axis_tdata(s_fog_lut_axis_tdata),

                .confFogColor(confFragmentPipelineFogColor),
                .confEnable(ENABLE_FOG && confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_FOG_POS]),

                .s_ready(step2_ready),
                .s_valid(step2_valid),
                .s_user({
                    step2_index,
                    step2_screenPosX,
                    step2_screenPosY,
                    step2_depth,
                    step2_keep,
                    step2_last
                }),
                .s_depth({ step2_depthWFloat, { (32 - FLOAT_SIZE) { 1'b0 } } }), // Fillup mantissa to cast back to 32 bit float
                .s_texelColor(step2_fragmentColor),

                .m_ready(step3_ready),
                .m_valid(step3_valid),
                .m_user({
                    step3_index,
                    step3_screenPosX,
                    step3_screenPosY,
                    step3_depth,
                    step3_keep,
                    step3_last
                }),
                .m_color(step3_fragmentColor)
            );
        end
        else
        begin
            assign step3_index = step2_index;
            assign step3_screenPosX = step2_screenPosX;
            assign step3_screenPosY = step2_screenPosY;
            assign step3_depth = step2_depth;
            assign step3_keep = step2_keep;
            assign step3_last = step2_last;
            assign step3_valid = step2_valid;
            assign step3_fragmentColor = step2_fragmentColor;

            assign step2_ready = step3_ready;

            assign s_fog_lut_axis_tready = 1;
        end
    endgenerate

    assign m_frag_tfragmentColor = step3_fragmentColor;
    assign m_frag_tindex = step3_index;
    assign m_frag_tscreenPosX = step3_screenPosX;
    assign m_frag_tscreenPosY = step3_screenPosY;
    assign m_frag_tdepth = step3_depth;
    assign m_frag_tvalid = step3_valid;
    assign m_frag_tkeep = step3_keep;
    assign m_frag_tlast = step3_last;
    assign step3_ready = m_frag_tready;

endmodule


 