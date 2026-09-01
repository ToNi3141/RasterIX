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
// Depth: 13 cycles
module TextureMappingUnit
#(
    parameter USER_WIDTH  = 1,
    parameter SUB_PIXEL_WIDTH = 8,
    parameter SUB_PIXEL_CALC_PRECISION = SUB_PIXEL_WIDTH,

    parameter ENABLE_LOD_CALC = 1,
    parameter ENABLE_TEXTURE_FILTERING = 1,

    localparam PIXEL_WIDTH = 4 * SUB_PIXEL_WIDTH,
    parameter TEXEL_WIDTH = 16,

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

    // Texture memory read address channel
    output wire                         texelAddrValid,
    input  wire                         texelAddrReady,
    output wire [ADDR_WIDTH - 1 : 0]    texelAddr00,
    output wire [ADDR_WIDTH - 1 : 0]    texelAddr01,
    output wire [ADDR_WIDTH - 1 : 0]    texelAddr10,
    output wire [ADDR_WIDTH - 1 : 0]    texelAddr11,

    // Texture memory read texel channel
    input  wire                         texelInputValid,
    output wire                         texelInputReady,
    input  wire [TEXEL_WIDTH - 1 : 0]   texelInput00,
    input  wire [TEXEL_WIDTH - 1 : 0]   texelInput01,
    input  wire [TEXEL_WIDTH - 1 : 0]   texelInput10,
    input  wire [TEXEL_WIDTH - 1 : 0]   texelInput11,

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
    // Calculate texture addresses
    // Clocks: 2
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step1_primaryColor;
    wire [PIXEL_WIDTH - 1 : 0]  step1_previousColor;
    wire [USER_WIDTH - 1 : 0]   step1_user;
    wire [15 : 0]               step1_subCoordS;
    wire [15 : 0]               step1_subCoordT;
    wire                        step1_valid;
    wire                        step1_ready;
    wire [ADDR_WIDTH - 1 : 0]   step1_addr00;
    wire [ADDR_WIDTH - 1 : 0]   step1_addr01;
    wire [ADDR_WIDTH - 1 : 0]   step1_addr10;
    wire [ADDR_WIDTH - 1 : 0]   step1_addr11;

    TextureSampler #(
        .USER_WIDTH((2 * PIXEL_WIDTH) + USER_WIDTH)
    ) textureSampler (
        .aclk(aclk),
        .resetn(resetn),

        .textureSizeWidth(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_WIDTH_POS +: RENDER_CONFIG_TMU_TEXTURE_WIDTH_SIZE]),
        .textureSizeHeight(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_HEIGHT_POS +: RENDER_CONFIG_TMU_TEXTURE_HEIGHT_SIZE]),
        .enableHalfPixelOffset(ENABLE_TEXTURE_FILTERING & confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_MAG_FILTER_POS +: RENDER_CONFIG_TMU_TEXTURE_MAG_FILTER_SIZE]), 

        .texelAddr00(step1_addr00),
        .texelAddr01(step1_addr01),
        .texelAddr10(step1_addr10),
        .texelAddr11(step1_addr11),

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
        .m_texelSubCoordS(step1_subCoordS),
        .m_texelSubCoordT(step1_subCoordT)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Broadcast texture addresses and join texture read response with context
    // Clocks: 2
    ////////////////////////////////////////////////////////////////////////////
    localparam ADDRESS_STREAM_WIDTH = 4 * ADDR_WIDTH;
    localparam CONTEXT_STREAM_WIDTH = (2 * PIXEL_WIDTH) + USER_WIDTH + 16 + 16;
    localparam TEXEL_STREAM_WIDTH = 4 * TEXEL_WIDTH;

    wire                                    step2_ready;
    wire                                    step2_valid;
    wire [PIXEL_WIDTH - 1 : 0]              step2_primaryColor;
    wire [PIXEL_WIDTH - 1 : 0]              step2_previousColor;
    wire [USER_WIDTH - 1 : 0]               step2_user;
    wire [15 : 0]                           step2_subCoordS;
    wire [15 : 0]                           step2_subCoordT;
    wire [TEXEL_WIDTH - 1 : 0]              step2_texel00;
    wire [TEXEL_WIDTH - 1 : 0]              step2_texel01;
    wire [TEXEL_WIDTH - 1 : 0]              step2_texel10;
    wire [TEXEL_WIDTH - 1 : 0]              step2_texel11;

    wire [ 1 : 0]                           step2_broadcastValid;
    wire [ 1 : 0]                           step2_broadcastReady;
    wire [2 * ADDRESS_STREAM_WIDTH - 1 : 0] step2_broadcastData;
    wire [2 * CONTEXT_STREAM_WIDTH - 1 : 0] step2_broadcastUser;
    wire                                    step2_contextReady;
    wire                                    step2_texelReady;
    axis_broadcast #(
        .M_COUNT(2),
        .DATA_WIDTH(ADDRESS_STREAM_WIDTH),
        .KEEP_ENABLE(0),
        .LAST_ENABLE(0),
        .ID_ENABLE(0),
        .DEST_ENABLE(0),
        .USER_ENABLE(1),
        .USER_WIDTH(CONTEXT_STREAM_WIDTH)
    ) samplerBroadcast (
        .clk(aclk),
        .rst(!resetn),

        .s_axis_tdata({
            step1_addr11,
            step1_addr10,
            step1_addr01,
            step1_addr00
        }),
        .s_axis_tkeep(0),
        .s_axis_tvalid(step1_valid),
        .s_axis_tready(step1_ready),
        .s_axis_tlast(1'b0),
        .s_axis_tid(0),
        .s_axis_tdest(0),
        .s_axis_tuser({
            step1_primaryColor,
            step1_previousColor,
            step1_user,
            step1_subCoordS,
            step1_subCoordT
        }),

        .m_axis_tdata(step2_broadcastData),
        .m_axis_tkeep(),
        .m_axis_tvalid(step2_broadcastValid),
        .m_axis_tready(step2_broadcastReady),
        .m_axis_tlast(),
        .m_axis_tid(),
        .m_axis_tdest(),
        .m_axis_tuser(step2_broadcastUser)
    );

    // Texture Buffer Access
    assign texelAddrValid = step2_broadcastValid[0]; // TODO: Enable and disable texture access
    assign step2_broadcastReady[0] = texelAddrReady;
    assign texelAddr00 = step2_broadcastData[0 +: ADDR_WIDTH];
    assign texelAddr01 = step2_broadcastData[ADDR_WIDTH +: ADDR_WIDTH];
    assign texelAddr10 = step2_broadcastData[(2 * ADDR_WIDTH) +: ADDR_WIDTH];
    assign texelAddr11 = step2_broadcastData[(3 * ADDR_WIDTH) +: ADDR_WIDTH];
    assign texelInputReady = step2_texelReady;

    assign step2_broadcastReady[1] = step2_contextReady;

    StreamConcatFifo #(
        .STREAM0_WIDTH(TEXEL_STREAM_WIDTH),
        .STREAM1_WIDTH(CONTEXT_STREAM_WIDTH),
        .STREAM2_WIDTH(1), // TODO: Make this channel optional
        .STREAM3_WIDTH(1), // TODO: Make this channel optional
        .FIFO_DEPTH0_POW2(5),
        .FIFO_DEPTH1_POW2(5),
        .FIFO_DEPTH2_POW2(0),
        .FIFO_DEPTH3_POW2(0)
    ) textureReadConcat (
        .aclk(aclk),
        .resetn(resetn),

        .s_stream0_tenable(1'b1),
        .s_stream0_tvalid(texelInputValid),
        .s_stream0_tdata({
            texelInput11,
            texelInput10,
            texelInput01,
            texelInput00
        }),
        .s_stream0_tready(step2_texelReady),

        .s_stream1_tenable(1'b1),
        .s_stream1_tvalid(step2_broadcastValid[1]),
        .s_stream1_tdata({
            step2_broadcastUser[CONTEXT_STREAM_WIDTH +: CONTEXT_STREAM_WIDTH]
        }),
        .s_stream1_tready(step2_contextReady),

        .s_stream2_tenable(1'b0),
        .s_stream2_tvalid(1'b0),
        .s_stream2_tdata(1'b0),
        .s_stream2_tready(),

        .s_stream3_tenable(1'b0),
        .s_stream3_tvalid(1'b0),
        .s_stream3_tdata(1'b0),
        .s_stream3_tready(),

        .m_stream_tvalid(step2_valid),
        .m_stream_tdata({
            step2_primaryColor,
            step2_previousColor,
            step2_user,
            step2_subCoordS,
            step2_subCoordT,
            step2_texel11,
            step2_texel10,
            step2_texel01,
            step2_texel00
        }),
        .m_stream_tready(step2_ready)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 3
    // Unpack texel colors
    // Clocks: 0
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step3_texel00;
    wire [PIXEL_WIDTH - 1 : 0]  step3_texel01;
    wire [PIXEL_WIDTH - 1 : 0]  step3_texel10;
    wire [PIXEL_WIDTH - 1 : 0]  step3_texel11;


    TexelColorUnpack #(
        .TEXEL_WIDTH(TEXEL_WIDTH)
    ) texelColorUnpack00 (
        .confPixelFormat(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_POS +: RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_SIZE]),
        .texelInput(step2_texel00),
        .texelOutput(step3_texel00)
    );

    TexelColorUnpack #(
        .TEXEL_WIDTH(TEXEL_WIDTH)
    ) texelColorUnpack01 (
        .confPixelFormat(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_POS +: RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_SIZE]),
        .texelInput(step2_texel01),
        .texelOutput(step3_texel01)
    );

    TexelColorUnpack #(
        .TEXEL_WIDTH(TEXEL_WIDTH)
    ) texelColorUnpack10 (
        .confPixelFormat(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_POS +: RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_SIZE]),
        .texelInput(step2_texel10),
        .texelOutput(step3_texel10)
    );

    TexelColorUnpack #(
        .TEXEL_WIDTH(TEXEL_WIDTH)
    ) texelColorUnpack11 (
        .confPixelFormat(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_POS +: RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_SIZE]),
        .texelInput(step2_texel11),
        .texelOutput(step3_texel11)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 4
    // Filter Texture
    // Clocks: 4
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step4_primaryColor;
    wire [PIXEL_WIDTH - 1 : 0]  step4_previousColor;
    wire [PIXEL_WIDTH - 1 : 0]  step4_texel;
    wire                        step4_ready;
    wire                        step4_valid;
    wire [USER_WIDTH - 1 : 0]   step4_user;

    TextureFilter #(
        .USER_WIDTH((2 * PIXEL_WIDTH) + USER_WIDTH),
        .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION)
    ) texFilter (
        .aclk(aclk),
        .resetn(resetn),

        .enable(ENABLE_TEXTURE_FILTERING & confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_MAG_FILTER_POS +: RENDER_CONFIG_TMU_TEXTURE_MAG_FILTER_SIZE]),

        .s_valid(step2_valid),
        .s_ready(step2_ready),
        .s_user({
            step2_primaryColor,
            step2_previousColor,
            step2_user
        }),
        .s_texel00(step3_texel00),
        .s_texel01(step3_texel01),
        .s_texel10(step3_texel10),
        .s_texel11(step3_texel11),
        .s_texelSubCoordS(step2_subCoordS),
        .s_texelSubCoordT(step2_subCoordT),

        .m_valid(step4_valid),
        .m_ready(step4_ready),
        .m_user({
            step4_primaryColor,
            step4_previousColor,
            step4_user
        }),
        .m_texel(step4_texel)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 5
    // Calculate texture environment
    // Clocks: 4
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step5_texel;
    wire [PIXEL_WIDTH - 1 : 0]  step5_previousColor;
    wire                        step5_ready;
    wire                        step5_valid;
    wire [USER_WIDTH - 1 : 0]   step5_user;

    TexEnv #(
        .USER_WIDTH(PIXEL_WIDTH + USER_WIDTH),
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
        .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION)
    ) texEnv (
        .aclk(aclk),
        .resetn(resetn),

        .conf(confFunc),

        .s_valid(step4_valid),
        .s_ready(step4_ready),
        .s_user({
            step4_previousColor,
            step4_user
        }),
        .s_previousColor(step4_previousColor),
        .s_texSrcColor(step4_texel),
        .s_primaryColor(step4_primaryColor),
        .s_envColor(confTextureEnvColor),

        .m_valid(step5_valid),
        .m_ready(step5_ready),
        .m_user({
            step5_previousColor,
            step5_user
        }),
        .m_color(step5_texel)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 6
    // Output final texel color
    // Clocks: 0
    ////////////////////////////////////////////////////////////////////////////
    assign m_fragmentColor = (confEnable) ? step5_texel : step5_previousColor;
    assign m_valid = step5_valid;
    assign m_user = step5_user;
    assign step5_ready = m_ready;

endmodule


 