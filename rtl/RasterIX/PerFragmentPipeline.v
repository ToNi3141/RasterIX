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

// This pipeline is part of the pixel pipeline and handles the fragment buffer part of the pipeline
// It gets a fully calculated texel from the previous step, reads from the color and depth buffer,
// executes alpha and depth tests, and blends the texel into the current fragment
// Pipelined: yes
// Depth: 4 cycles
module PerFragmentPipeline
#(
    // The minimum bit width which is required to contain the resolution
    parameter FRAMEBUFFER_INDEX_WIDTH = 14,
    parameter SCREEN_POS_WIDTH = 16,

    parameter DEPTH_WIDTH = 16,

    parameter STENCIL_WIDTH = 4,

    parameter SUB_PIXEL_WIDTH = 8,
    localparam NUMBER_OF_SUB_PIXELS = 4,
    localparam PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS * SUB_PIXEL_WIDTH,
    parameter SUB_PIXEL_CALC_PRECISION = SUB_PIXEL_WIDTH,

    localparam FLOAT_SIZE = 32,

    localparam KEEP_WIDTH = 1
)
(
    input  wire                                     aclk,
    input  wire                                     resetn,

    // Shader configurations
    input  wire [31 : 0]                            conf,
    input  wire [31 : 0]                            confFeatureEnable,
    input  wire [31 : 0]                            confStencilBufferConfig,

    // Fragment input
    output wire                                     s_frag_tready,
    input  wire                                     s_frag_tlast,
    input  wire [KEEP_WIDTH - 1 : 0]                s_frag_tkeep,
    input  wire                                     s_frag_tvalid,
    input  wire [PIXEL_WIDTH - 1 : 0]               s_frag_tcolor, // Source color
    input  wire [31 : 0]                            s_frag_tdepth, // Source depth
    input  wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]   s_frag_tindex,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]          s_frag_tscreenPosX,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]          s_frag_tscreenPosY,
    input  wire [PIXEL_WIDTH - 1 : 0]               s_frag_tdestinationColor,
    input  wire [DEPTH_WIDTH - 1 : 0]               s_frag_tdestinationDepth,
    input  wire [STENCIL_WIDTH - 1 : 0]             s_frag_tdestinationStencil,

    // Signals when the fragment went through the pipeline
    output reg                                      fragmentProcessed,

    // Fragment output
    input  wire                                     m_frag_tready,
    output reg  [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]   m_frag_taddr,
    output reg                                      m_frag_tvalid,
    output reg                                      m_frag_tlast,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]          m_frag_tscreenPosX,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]          m_frag_tscreenPosY,
    // Color
    output reg  [PIXEL_WIDTH - 1 : 0]               m_frag_color_tdata,
    output reg                                      m_frag_color_tstrb,
    // Depth
    output reg  [DEPTH_WIDTH - 1 : 0]               m_frag_depth_tdata,
    output reg                                      m_frag_depth_tstrb,
    // Stencil
    output reg  [STENCIL_WIDTH - 1 : 0]             m_frag_stencil_tdata,
    output reg                                      m_frag_stencil_tstrb
);
`include "RegisterAndDescriptorDefines.vh"

    function [DEPTH_WIDTH - 1 : 0] clampDepth;
        input [31 : 0] depth;
        begin
            if (depth[31]) // If z is negative
            begin
                clampDepth = 0;
            end
            else if (|depth[32 - DEPTH_WIDTH +: DEPTH_WIDTH]) // If z is greater than 1.0
            begin
                clampDepth = {DEPTH_WIDTH{1'b1}};
            end
            else // If z is between 0.0 and 1.0
            begin
                clampDepth = depth[0 +: DEPTH_WIDTH];
            end
        end
    endfunction

    // Flow Control
    wire ce;
    assign ce = m_frag_tready;
    assign s_frag_tready = m_frag_tready;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 0
    // Read from framebuffer
    // Clocks: 0
    ////////////////////////////////////////////////////////////////////////////
    wire                                    step0_valid = s_frag_tvalid;
    wire                                    step0_last = s_frag_tlast;
    wire [KEEP_WIDTH  - 1 : 0]              step0_keep = s_frag_tkeep;
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]  step0_index = s_frag_tindex;
    wire [DEPTH_WIDTH - 1 : 0]              step0_depth = clampDepth(s_frag_tdepth);
    wire [SCREEN_POS_WIDTH - 1 : 0]         step0_screenPosX = s_frag_tscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step0_screenPosY = s_frag_tscreenPosY;
    wire [PIXEL_WIDTH - 1 : 0]              step0_color = s_frag_tcolor;
    wire [PIXEL_WIDTH - 1 : 0]              step0_destcolor = s_frag_tdestinationColor;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Blend color and test functions
    // Clocks: 3 
    ////////////////////////////////////////////////////////////////////////////
    wire                                    step1_valid;
    wire                                    step1_last;
    wire [KEEP_WIDTH - 1 : 0]               step1_keep;
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]  step1_index;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step1_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step1_screenPosY;
    wire [DEPTH_WIDTH - 1 : 0]              step1_depth;
    wire [PIXEL_WIDTH - 1 : 0]              step1_colorBlend;
    wire [PIXEL_WIDTH - 1 : 0]              step1_colorLogicOp;
    wire                                    step1_writeFramebuffer;
    wire [STENCIL_WIDTH - 1 : 0 ]           step1_stencil;
    wire                                    step1_writeStencilBuffer;

    ValueDelay #(
        .VALUE_SIZE(1 + 1 + KEEP_WIDTH + FRAMEBUFFER_INDEX_WIDTH + (2 * SCREEN_POS_WIDTH) + DEPTH_WIDTH), 
        .DELAY(3)
    ) step1_delay (
        .clk(aclk), 
        .ce(ce), 
        .in({
            step0_valid,
            step0_last,
            step0_keep,
            step0_index,
            step0_screenPosX,
            step0_screenPosY,
            step0_depth
        }), 
        .out({
            step1_valid,
            step1_last,
            step1_keep,
            step1_index,
            step1_screenPosX,
            step1_screenPosY,
            step1_depth
        })
    );

    ColorBlender #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
        .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION)
    ) colorBlender (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),

        .funcSFactor(conf[RENDER_CONFIG_FRAGMENT_BLEND_FUNC_SFACTOR_POS +: RENDER_CONFIG_FRAGMENT_BLEND_FUNC_SFACTOR_SIZE]),
        .funcDFactor(conf[RENDER_CONFIG_FRAGMENT_BLEND_FUNC_DFACTOR_POS +: RENDER_CONFIG_FRAGMENT_BLEND_FUNC_DFACTOR_SIZE]),
        .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_BLENDING_POS]),
        .sourceColor(step0_color),
        .destColor(step0_destcolor),

        .color(step1_colorBlend)
    );

    // Clocks: 1
    wire [PIXEL_WIDTH - 1 : 0] step1_colorLogicOpTmp;
    LogicOp #(
        .PIXEL_WIDTH(PIXEL_WIDTH)
    ) logicOp (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),

        .op(conf[RENDER_CONFIG_FRAGMENT_LOGIC_OP_POS +: RENDER_CONFIG_FRAGMENT_LOGIC_OP_SIZE]),
        .enable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_LOGIC_OP_POS]),

        .source(step0_color),
        .dest(step0_destcolor),

        .out(step1_colorLogicOpTmp)
    );

    ValueDelay #(
        .VALUE_SIZE(PIXEL_WIDTH), 
        .DELAY(2)
    ) step1_colorLogicOpDelay (
        .clk(aclk), 
        .ce(ce), 
        .in(step1_colorLogicOpTmp), 
        .out(step1_colorLogicOp)
    );

    // Clocks: 1
    wire step1_alphaTestTmp;
    TestFunc #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
    ) alphaTest (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),
        .func(conf[RENDER_CONFIG_FRAGMENT_ALPHA_TEST_FUNC_POS +: RENDER_CONFIG_FRAGMENT_ALPHA_TEST_FUNC_SIZE]),
        .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_ALPHA_TEST_POS]),
        .refVal(conf[RENDER_CONFIG_FRAGMENT_ALPHA_TEST_REF_VALUE_POS +: RENDER_CONFIG_FRAGMENT_ALPHA_TEST_REF_VALUE_SIZE]),
        .val(step0_color[COLOR_A_POS +: SUB_PIXEL_WIDTH]),
        .success(step1_alphaTestTmp)
    );

    // Clocks: 1
    wire step1_depthTestTmp;
    TestFunc #(
        .SUB_PIXEL_WIDTH(DEPTH_WIDTH)
    ) depthTest (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),
        .func(conf[RENDER_CONFIG_FRAGMENT_DEPTH_TEST_FUNC_POS +: RENDER_CONFIG_FRAGMENT_DEPTH_TEST_FUNC_SIZE]),
        .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_DEPTH_TEST_POS]),
        .refVal(s_frag_tdestinationDepth),
        .val(step0_depth),
        .success(step1_depthTestTmp)
    );

    // Clocks: 1
    wire step1_stencilTestTmp;
    wire [STENCIL_WIDTH - 1 : 0] step1_destinationStencil;
    wire [STENCIL_WIDTH - 1 : 0] step1_stencilRefValTmp = confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_REF_POS +: RENDER_CONFIG_STENCIL_BUFFER_REF_SIZE - (STENCIL_WIDTH - RENDER_CONFIG_STENCIL_BUFFER_REF_SIZE)];
    wire [STENCIL_WIDTH - 1 : 0] step1_stencilMaskTmp = confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_MASK_POS +: RENDER_CONFIG_STENCIL_BUFFER_MASK_SIZE - (STENCIL_WIDTH - RENDER_CONFIG_STENCIL_BUFFER_MASK_SIZE)];
    ValueDelay #(
        .VALUE_SIZE(STENCIL_WIDTH), 
        .DELAY(1)
    ) step1_stencilBufferDelay (
        .clk(aclk), 
        .ce(ce), 
        .in(s_frag_tdestinationStencil), 
        .out(step1_destinationStencil)
    ); 
    TestFunc #(
        .SUB_PIXEL_WIDTH(STENCIL_WIDTH)
    ) stencilTest (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),
        .func(confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_FUNC_POS +: RENDER_CONFIG_STENCIL_BUFFER_FUNC_SIZE]),
        .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_STENCIL_TEST_POS]),
        .refVal(s_frag_tdestinationStencil & step1_stencilMaskTmp),
        .val(step1_stencilRefValTmp & step1_stencilMaskTmp),
        .success(step1_stencilTestTmp)
    );

    // Clocks: 1
    wire                            step1_writeStencilBufferTmp;
    wire [STENCIL_WIDTH - 1 : 0 ]   step1_stencilTmp;
    StencilOp #(
        .STENCIL_WIDTH(STENCIL_WIDTH)
    ) stencilOp (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),
        .opZFail(confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_OP_ZFAIL_POS +: RENDER_CONFIG_STENCIL_BUFFER_OP_ZFAIL_SIZE]),
        .opZPass(confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_OP_ZPASS_POS +: RENDER_CONFIG_STENCIL_BUFFER_OP_ZPASS_SIZE]),
        .opFail(confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_OP_FAIL_POS +: RENDER_CONFIG_STENCIL_BUFFER_OP_FAIL_SIZE]),
        .refVal(confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_REF_POS +: RENDER_CONFIG_STENCIL_BUFFER_REF_SIZE - (STENCIL_WIDTH - RENDER_CONFIG_STENCIL_BUFFER_REF_SIZE)]),
        .enable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_STENCIL_TEST_POS]),
        .zTest(step1_depthTestTmp),
        .sTest(step1_stencilTestTmp),
        .sValIn(step1_destinationStencil),
        .sValOut(step1_stencilTmp),
        .write(step1_writeStencilBufferTmp)
    );

    ValueDelay #(
        .VALUE_SIZE(STENCIL_WIDTH), 
        .DELAY(1)
    ) step1_stencilDelay (
        .clk(aclk), 
        .ce(ce), 
        .in(step1_stencilTmp), 
        .out(step1_stencil)
    );

    ValueDelay #(
        .VALUE_SIZE(1), 
        .DELAY(1)
    ) step1_writeStencilDelay (
        .clk(aclk), 
        .ce(ce), 
        .in(step1_writeStencilBufferTmp), 
        .out(step1_writeStencilBuffer)
    );

    ValueDelay #(
        .VALUE_SIZE(1), 
        .DELAY(2)
    ) step1_writeFramebufferDelay (
        .clk(aclk), 
        .ce(ce), 
        .in(step1_depthTestTmp & step1_alphaTestTmp & step1_stencilTestTmp), 
        .out(step1_writeFramebuffer)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Write back
    // Clocks: 1 
    ////////////////////////////////////////////////////////////////////////////
    always @(posedge aclk)
    if (ce) begin
        if (step1_valid)
        begin : step2
            reg logicOpEnable;
            logicOpEnable = conf[RENDER_CONFIG_FRAGMENT_LOGIC_OP_POS +: RENDER_CONFIG_FRAGMENT_LOGIC_OP_SIZE];
            
            m_frag_taddr <= step1_index;
            m_frag_tscreenPosX <= step1_screenPosX;
            m_frag_tscreenPosY <= step1_screenPosY;
            m_frag_depth_tdata <= step1_depth;
            m_frag_color_tdata <= (logicOpEnable) ? step1_colorLogicOp : step1_colorBlend;
            m_frag_stencil_tdata <= step1_stencil;
        end
        fragmentProcessed <= step1_valid;
        m_frag_tvalid <= step1_valid;
        m_frag_tlast <= step1_last;
        m_frag_color_tstrb <= step1_keep & step1_writeFramebuffer;
        m_frag_depth_tstrb <= step1_keep & step1_writeFramebuffer && confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_DEPTH_TEST_POS];
        m_frag_stencil_tstrb <= step1_keep & step1_writeStencilBuffer && confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_STENCIL_TEST_POS];
    end
endmodule


 