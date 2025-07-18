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

// Renderer variant with internal memory.
module RasterIXCoreIF #(
    // The size of the internal framebuffer (in power of two)
    // Depth buffer word size: 16 bit
    // Color buffer word size: FRAMEBUFFER_SUB_PIXEL_WIDTH * (FRAMEBUFFER_ENABLE_ALPHA_CHANNEL ? 4 : 3)
    parameter FRAMEBUFFER_SIZE_IN_PIXEL_LG = 17,

    // This is the color depth of the framebuffer. Note: This setting has no influence on the framebuffer stream. This steam will
    // stay at RGB565. It changes the internal representation and might be used to reduce the memory footprint.
    // Lower depth will result in color banding.
    parameter FRAMEBUFFER_SUB_PIXEL_WIDTH = 6,
    // This enables the alpha channel of the framebuffer. Requires additional memory.
    parameter FRAMEBUFFER_ENABLE_ALPHA_CHANNEL = 0,
    // The number of sub pixels in the framebuffer
    localparam FRAMEBUFFER_NUMBER_OF_SUB_PIXELS = (FRAMEBUFFER_ENABLE_ALPHA_CHANNEL == 0) ? 3 : 4,
    // The sub pixel with in the framebuffer
    localparam PIXEL_WIDTH_FRAMEBUFFER = FRAMEBUFFER_NUMBER_OF_SUB_PIXELS * FRAMEBUFFER_SUB_PIXEL_WIDTH,

    parameter SUB_PIXEL_CALC_PRECISION = 8,

    // The width of the stencil buffer
    localparam STENCIL_WIDTH = 4,

    // The width of the depth buffer
    localparam DEPTH_WIDTH = 16,

    // This enables the 4 bit stencil buffer
    parameter ENABLE_STENCIL_BUFFER = 1,

    // Enables the stencil buffer
    parameter ENABLE_DEPTH_BUFFER = 1,

    // Number of TMUs. Currently supported values: 1 and 2
    parameter TMU_COUNT = 2,
    parameter ENABLE_MIPMAPPING = 1,
    parameter ENABLE_TEXTURE_FILTERING = 1,
    parameter TEXTURE_PAGE_SIZE = 2048,

    // Enables the fog unit
    parameter ENABLE_FOG = 1,
    
    // The bit width of the command stream interface and memory interface
    // Allowed values: 32, 64, 128, 256 bit
    localparam CMD_STREAM_WIDTH = 32,

    // The maximum size of a texture
    parameter MAX_TEXTURE_SIZE = 256,

    // Memory address width
    parameter ADDR_WIDTH = 24,
    // Memory ID width
    parameter ID_WIDTH = 8,
    // Memory data width
    parameter DATA_WIDTH = 64,
    // Memory strobe width
    parameter STRB_WIDTH = DATA_WIDTH / 8,

    // Configures the precision of the float calculations (interpolation of textures, depth, ...)
    // A lower value can significant reduce the logic consumption but can cause visible 
    // distortions in the rendered image.
    // 4 bit reducing can safe around 1k LUTs.
    // For compatibility reasons, it only cuts of the mantissa. By default it uses a 25x25 multiplier (for floatMul)
    // If you have a FPGA with only 18 bit native multipliers, reduce this value to 26.
    parameter RASTERIZER_FLOAT_PRECISION = 32,
    // When RASTERIZER_ENABLE_FLOAT_INTERPOLATION is 0, then this configures the width of the multipliers for the fix point
    // calculations. A value of 25 will instantiate signed 25 bit multipliers. The 25 already including the sign bit.
    // Lower values can lead to distortions of the fog and texels.
    parameter RASTERIZER_FIXPOINT_PRECISION = 25,
    // Enables the floating point interpolation. If this is disabled, it falls back
    // to the fix point interpolation
    parameter RASTERIZER_ENABLE_FLOAT_INTERPOLATION = 1,

    localparam FB_SIZE_IN_PIXEL_LG = 20
)
(
    input  wire                                 aclk,
    input  wire                                 resetn,

    // AXI Stream command interface
    input  wire                                 s_cmd_axis_tvalid,
    output wire                                 s_cmd_axis_tready,
    input  wire                                 s_cmd_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]      s_cmd_axis_tdata,

    // Framebuffer output
    // AXI Stream master interface (RGB565)
    output wire                                 m_framebuffer_axis_tvalid,
    input  wire                                 m_framebuffer_axis_tready,
    output wire                                 m_framebuffer_axis_tlast,
    output wire [DATA_WIDTH - 1 : 0]            m_framebuffer_axis_tdata,
    output wire [STRB_WIDTH - 1 : 0]            m_framebuffer_axis_tstrb,

    output wire                                 m_colorbuffer_tstart,
    output wire [ADDR_WIDTH - 1 : 0]            m_colorbuffer_taddr,
    output wire [ADDR_WIDTH - 1 : 0]            m_colorbuffer_tbytes,
    input  wire                                 m_colorbuffer_tdone,

    // Color
    output wire                                 swap_fb,
    output wire                                 swap_fb_enable_vsync,
    input  wire                                 fb_swapped,
    output wire [ADDR_WIDTH - 1 : 0]            fb_addr,
    output wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]   fb_size,

    // TMU 0 memory access
    output wire [ID_WIDTH - 1 : 0]              m_tmu0_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]            m_tmu0_axi_araddr,
    output wire [ 7 : 0]                        m_tmu0_axi_arlen,
    output wire [ 2 : 0]                        m_tmu0_axi_arsize,
    output wire [ 1 : 0]                        m_tmu0_axi_arburst,
    output wire                                 m_tmu0_axi_arlock,
    output wire [ 3 : 0]                        m_tmu0_axi_arcache,
    output wire [ 2 : 0]                        m_tmu0_axi_arprot,
    output wire                                 m_tmu0_axi_arvalid,
    input  wire                                 m_tmu0_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]              m_tmu0_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]            m_tmu0_axi_rdata,
    input  wire [ 1 : 0]                        m_tmu0_axi_rresp,
    input  wire                                 m_tmu0_axi_rlast,
    input  wire                                 m_tmu0_axi_rvalid,
    output wire                                 m_tmu0_axi_rready,

    // TMU 1 memory access
    output wire [ID_WIDTH - 1 : 0]              m_tmu1_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]            m_tmu1_axi_araddr,
    output wire [ 7 : 0]                        m_tmu1_axi_arlen,
    output wire [ 2 : 0]                        m_tmu1_axi_arsize,
    output wire [ 1 : 0]                        m_tmu1_axi_arburst,
    output wire                                 m_tmu1_axi_arlock,
    output wire [ 3 : 0]                        m_tmu1_axi_arcache,
    output wire [ 2 : 0]                        m_tmu1_axi_arprot,
    output wire                                 m_tmu1_axi_arvalid,
    input  wire                                 m_tmu1_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]              m_tmu1_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]            m_tmu1_axi_rdata,
    input  wire [ 1 : 0]                        m_tmu1_axi_rresp,
    input  wire                                 m_tmu1_axi_rlast,
    input  wire                                 m_tmu1_axi_rvalid,
    output wire                                 m_tmu1_axi_rready
);
`include "RegisterAndDescriptorDefines.vh"
    localparam DEFAULT_ALPHA_VAL = 0;
    localparam SCREEN_POS_WIDTH = 11;
    localparam PIXEL_WIDTH_STREAM = 16;
    localparam PIXEL_PER_BEAT = DATA_WIDTH / PIXEL_WIDTH_STREAM;
    localparam FRAMEBUFFER_SUBPIXEL_PER_BEAT = PIXEL_PER_BEAT * FRAMEBUFFER_NUMBER_OF_SUB_PIXELS;
    localparam PIPELINE_PIXEL_WIDTH = COLOR_SUB_PIXEL_WIDTH * COLOR_NUMBER_OF_SUB_PIXEL;
    // This is used to configure, if it is required to reduce / expand a vector or not. This is done by the offset:
    // When the offset is set to number of pixels, then the reduce / expand function will just copy the line
    // without removing or adding something.
    // If it is set to a lower value, then the functions will start to remove or add new pixels.
    localparam SUB_PIXEL_OFFSET = (COLOR_NUMBER_OF_SUB_PIXEL == FRAMEBUFFER_NUMBER_OF_SUB_PIXELS) ? COLOR_NUMBER_OF_SUB_PIXEL : COLOR_A_POS; 
    `ReduceVec(ColorBufferReduceVec, COLOR_SUB_PIXEL_WIDTH, COLOR_NUMBER_OF_SUB_PIXEL, SUB_PIXEL_OFFSET, COLOR_NUMBER_OF_SUB_PIXEL, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS)
    `ReduceVec(ColorBufferReduceMask, 1, COLOR_NUMBER_OF_SUB_PIXEL, SUB_PIXEL_OFFSET, COLOR_NUMBER_OF_SUB_PIXEL, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS)
    `ExpandVec(ColorBufferExpandVec, COLOR_SUB_PIXEL_WIDTH, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS, SUB_PIXEL_OFFSET, COLOR_NUMBER_OF_SUB_PIXEL, COLOR_NUMBER_OF_SUB_PIXEL)
    `Expand(ColorBufferExpand, FRAMEBUFFER_SUB_PIXEL_WIDTH, COLOR_SUB_PIXEL_WIDTH, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS)
    `Reduce(ColorBufferReduce, FRAMEBUFFER_SUB_PIXEL_WIDTH, COLOR_SUB_PIXEL_WIDTH, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS)

    wire                                             framebufferParamEnableScissor;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamScissorStartX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamScissorStartY;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamScissorEndX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamScissorEndY;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamYOffset;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamXResolution;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamYResolution;

    // Color buffer access
    wire [PIPELINE_PIXEL_WIDTH - 1 : 0]              colorBufferClearColor;
    wire [ADDR_WIDTH - 1 : 0]                        colorBufferAddr;
    wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]               colorBufferSize; 
    wire                                             colorBufferApply;
    wire                                             colorBufferApplied;
    wire                                             colorBufferCmdCommit;
    wire                                             colorBufferCmdMemset;
    wire                                             colorBufferCmdSwap;
    wire                                             colorBufferCmdSwapEnableVsync;
    wire                                             colorBufferEnable;
    wire [3 : 0]                                     colorBufferMask;
    wire                                             m_color_arvalid;
    wire                                             m_color_arlast;
    wire                                             m_color_rvalid;
    wire                                             m_color_rlast;
    wire [FRAMEBUFFER_SIZE_IN_PIXEL_LG - 1 : 0]      m_color_araddr;
    wire [FRAMEBUFFER_SIZE_IN_PIXEL_LG - 1 : 0]      m_color_waddr;
    wire                                             m_color_wvalid;
    wire [PIXEL_WIDTH_FRAMEBUFFER - 1 : 0]           m_color_rdata;
    wire [PIPELINE_PIXEL_WIDTH - 1 : 0]              m_color_wdata;
    wire                                             m_color_wstrb;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  m_color_wscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  m_color_wscreenPosY;

    // Depth buffer access
    wire [DEPTH_WIDTH - 1 : 0]                       depthBufferClearDepth;
    wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]               depthBufferSize; 
    wire                                             depthBufferApply;
    wire                                             depthBufferApplied;
    wire                                             depthBufferCmdCommit;
    wire                                             depthBufferCmdMemset;
    wire                                             depthBufferEnable;
    wire                                             depthBufferMask;
    wire                                             m_depth_arvalid;
    wire                                             m_depth_arlast;
    wire                                             m_depth_rvalid;
    wire                                             m_depth_rlast;
    wire [FRAMEBUFFER_SIZE_IN_PIXEL_LG - 1 : 0]      m_depth_araddr;
    wire [FRAMEBUFFER_SIZE_IN_PIXEL_LG - 1 : 0]      m_depth_waddr;
    wire                                             m_depth_wvalid;
    wire [DEPTH_WIDTH - 1 : 0]                       m_depth_rdata;
    wire [DEPTH_WIDTH - 1 : 0]                       m_depth_wdata;
    wire                                             m_depth_wstrb;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  m_depth_wscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  m_depth_wscreenPosY;

    // Stencil buffer access
    wire [STENCIL_WIDTH - 1 : 0]                     stencilBufferClearStencil;
    wire [FB_SIZE_IN_PIXEL_LG -1 : 0]                stencilBufferSize; 
    wire                                             stencilBufferApply;
    wire                                             stencilBufferApplied;
    wire                                             stencilBufferCmdCommit;
    wire                                             stencilBufferCmdMemset;
    wire                                             stencilBufferEnable;
    wire [STENCIL_WIDTH - 1 : 0]                     stencilBufferMask;
    wire                                             m_stencil_arvalid;
    wire                                             m_stencil_arlast;
    wire                                             m_stencil_rvalid;
    wire                                             m_stencil_rlast;
    wire [FRAMEBUFFER_SIZE_IN_PIXEL_LG - 1 : 0]      m_stencil_araddr;
    wire [FRAMEBUFFER_SIZE_IN_PIXEL_LG - 1 : 0]      m_stencil_waddr;
    wire                                             m_stencil_wvalid;
    wire [STENCIL_WIDTH - 1 : 0]                     m_stencil_rdata;
    wire [STENCIL_WIDTH - 1 : 0]                     m_stencil_wdata;
    wire                                             m_stencil_wstrb;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  m_stencil_wscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  m_stencil_wscreenPosY;

    generate
        if (ENABLE_DEPTH_BUFFER)
        begin
            InternalFramebuffer depthBuffer (  
                .clk(aclk),
                .reset(!resetn),

                .confEnable(depthBufferEnable),
                .confClearColor(depthBufferClearDepth),
                .confEnableScissor(framebufferParamEnableScissor),
                .confScissorStartX(framebufferParamScissorStartX),
                .confScissorStartY(framebufferParamScissorStartY),
                .confScissorEndX(framebufferParamScissorEndX),
                .confScissorEndY(framebufferParamScissorEndY),
                .confYOffset(framebufferParamYOffset),
                .confXResolution(framebufferParamXResolution),
                .confYResolution(framebufferParamYResolution),
                .confMask(depthBufferMask),

                .araddr(m_depth_araddr),
                .arvalid(m_depth_arvalid),
                .arlast(m_depth_arlast),
                .rvalid(m_depth_rvalid),
                .rlast(m_depth_rlast),
                .rdata(m_depth_rdata),
                .waddr(m_depth_waddr),
                .wdata(m_depth_wdata),
                .wvalid(m_depth_wvalid),
                .wstrb(m_depth_wstrb),
                .wscreenPosX(m_depth_wscreenPosX),
                .wscreenPosY(m_depth_wscreenPosY),

                .apply(depthBufferApply),
                .applied(depthBufferApplied),
                .cmdCommit(depthBufferCmdCommit),
                .cmdMemset(depthBufferCmdMemset),
                .cmdSize(depthBufferSize),

                .m_axis_tvalid(),
                .m_axis_tready(1'b1),
                .m_axis_tlast(),
                .m_axis_tdata(),

                .m_avalid(),
                .m_aaddr(),
                .m_abytes(),
                .m_aready(1'b1)
            );
            defparam depthBuffer.NUMBER_OF_PIXELS_PER_BEAT = PIXEL_PER_BEAT;
            defparam depthBuffer.NUMBER_OF_SUB_PIXELS = 1;
            defparam depthBuffer.SUB_PIXEL_WIDTH = 16;
            defparam depthBuffer.X_BIT_WIDTH = RENDER_CONFIG_X_SIZE;
            defparam depthBuffer.Y_BIT_WIDTH = RENDER_CONFIG_Y_SIZE;
            defparam depthBuffer.FRAMEBUFFER_SIZE_IN_PIXEL_LG = FRAMEBUFFER_SIZE_IN_PIXEL_LG;
            defparam depthBuffer.FB_SIZE_IN_PIXEL_LG = FB_SIZE_IN_PIXEL_LG;
        end
        else
        begin
            assign m_depth_arready = 1;
            assign m_depth_rvalid = 1;
            assign m_depth_rdata = 16'hffff;
            assign m_depth_arlast = 0;
            assign depthBufferApplied = 1;
        end
    endgenerate

    wire [(PIXEL_WIDTH_FRAMEBUFFER * PIXEL_PER_BEAT) - 1 : 0]   framebuffer_unconverted_axis_tdata;
    wire [FRAMEBUFFER_SUBPIXEL_PER_BEAT - 1 : 0]                framebuffer_unconverted_axis_tstrb;
    InternalFramebuffer colorBuffer (  
        .clk(aclk),
        .reset(!resetn),

        .confEnable(colorBufferEnable),
        .confClearColor(ColorBufferReduce(ColorBufferReduceVec(colorBufferClearColor))),
        .confEnableScissor(framebufferParamEnableScissor),
        .confScissorStartX(framebufferParamScissorStartX),
        .confScissorStartY(framebufferParamScissorStartY),
        .confScissorEndX(framebufferParamScissorEndX),
        .confScissorEndY(framebufferParamScissorEndY),
        .confYOffset(framebufferParamYOffset),
        .confXResolution(framebufferParamXResolution),
        .confYResolution(framebufferParamYResolution),
        .confMask(ColorBufferReduceMask(colorBufferMask)),

        .araddr(m_color_araddr),
        .arvalid(m_color_arvalid),
        .arlast(m_color_arlast),
        .rvalid(m_color_rvalid),
        .rlast(m_color_rlast),
        .rdata(m_color_rdata),
        .waddr(m_color_waddr),
        .wdata(ColorBufferReduce(ColorBufferReduceVec(m_color_wdata))),
        .wvalid(m_color_wvalid),
        .wstrb(m_color_wstrb),
        .wscreenPosX(m_color_wscreenPosX),
        .wscreenPosY(m_color_wscreenPosY),
        
        .apply(colorBufferApply && (colorBufferCmdCommit || colorBufferCmdMemset)),
        .applied(colorBufferApplied),
        .cmdCommit(colorBufferCmdCommit),
        .cmdMemset(colorBufferCmdMemset),
        .cmdSize(colorBufferSize),
        .cmdAddr(colorBufferAddr),

        .m_axis_tvalid(m_framebuffer_axis_tvalid),
        .m_axis_tready(m_framebuffer_axis_tready),
        .m_axis_tlast(m_framebuffer_axis_tlast),
        .m_axis_tdata(framebuffer_unconverted_axis_tdata),
        .m_axis_tstrb(framebuffer_unconverted_axis_tstrb),

        .m_avalid(m_colorbuffer_avalid),
        .m_aaddr(m_colorbuffer_aaddr),
        .m_abytes(m_colorbuffer_abytes),
        .m_aready(m_colorbuffer_aready)
    );
    defparam colorBuffer.NUMBER_OF_PIXELS_PER_BEAT = PIXEL_PER_BEAT; 
    defparam colorBuffer.NUMBER_OF_SUB_PIXELS = FRAMEBUFFER_NUMBER_OF_SUB_PIXELS;
    defparam colorBuffer.SUB_PIXEL_WIDTH = FRAMEBUFFER_SUB_PIXEL_WIDTH;
    defparam colorBuffer.X_BIT_WIDTH = RENDER_CONFIG_X_SIZE;
    defparam colorBuffer.Y_BIT_WIDTH = RENDER_CONFIG_Y_SIZE;
    defparam colorBuffer.FRAMEBUFFER_SIZE_IN_PIXEL_LG = FRAMEBUFFER_SIZE_IN_PIXEL_LG;
    defparam colorBuffer.FB_SIZE_IN_PIXEL_LG = FB_SIZE_IN_PIXEL_LG;

    // Conversion of the internal pixel representation the exnternal one required for the AXIS interface
    generate
        `XXX2RGB565(XXX2RGB565, COLOR_SUB_PIXEL_WIDTH, PIXEL_PER_BEAT);
        `Expand(ExpandFramebufferStream, FRAMEBUFFER_SUB_PIXEL_WIDTH, COLOR_SUB_PIXEL_WIDTH, PIXEL_PER_BEAT * 3);

        // Remove every other bit from the mask when RGBA is used. If only RGB is used, every third element can be removed to get two strobe bits per pixel.
        localparam MASK_BIT_TO_REMOVE = (FRAMEBUFFER_NUMBER_OF_SUB_PIXELS == 4) ? 2 : 3; 
        `ReduceVec(FramebufferMaskToByteMask, 1, FRAMEBUFFER_SUBPIXEL_PER_BEAT, 0, MASK_BIT_TO_REMOVE, STRB_WIDTH);

        if (FRAMEBUFFER_NUMBER_OF_SUB_PIXELS == 4)
        begin
            `ReduceVec(ReduceVecFramebufferStream, FRAMEBUFFER_SUB_PIXEL_WIDTH, PIXEL_PER_BEAT * COLOR_NUMBER_OF_SUB_PIXEL, COLOR_A_POS, COLOR_NUMBER_OF_SUB_PIXEL, PIXEL_PER_BEAT * 3);
            assign m_framebuffer_axis_tdata = XXX2RGB565(ExpandFramebufferStream(ReduceVecFramebufferStream(framebuffer_unconverted_axis_tdata)));
        end
        else
        begin
            assign m_framebuffer_axis_tdata = XXX2RGB565(ExpandFramebufferStream(framebuffer_unconverted_axis_tdata));
        end
        assign m_framebuffer_axis_tstrb = FramebufferMaskToByteMask(framebuffer_unconverted_axis_tstrb);
    endgenerate

    generate 
        if (ENABLE_STENCIL_BUFFER)
        begin
            InternalFramebuffer stencilBuffer (  
                .clk(aclk),
                .reset(!resetn),

                .confEnable(stencilBufferEnable),
                .confClearColor(stencilBufferClearStencil),
                .confEnableScissor(framebufferParamEnableScissor),
                .confScissorStartX(framebufferParamScissorStartX),
                .confScissorStartY(framebufferParamScissorStartY),
                .confScissorEndX(framebufferParamScissorEndX),
                .confScissorEndY(framebufferParamScissorEndY),
                .confYOffset(framebufferParamYOffset),
                .confXResolution(framebufferParamXResolution),
                .confYResolution(framebufferParamYResolution),
                .confMask(stencilBufferMask),

                .araddr(m_stencil_araddr),
                .arvalid(m_stencil_arvalid),
                .arlast(m_stencil_arlast),
                .rvalid(m_stencil_rvalid),
                .rlast(m_stencil_rlast),
                .rdata(m_stencil_rdata),
                .waddr(m_stencil_waddr),
                .wdata(m_stencil_wdata),
                .wvalid(m_stencil_wvalid),
                .wstrb(m_stencil_wstrb),
                .wscreenPosX(m_stencil_wscreenPosX),
                .wscreenPosY(m_stencil_wscreenPosY),

                .apply(stencilBufferApply),
                .applied(stencilBufferApplied),
                .cmdCommit(stencilBufferCmdCommit),
                .cmdMemset(stencilBufferCmdMemset),
                .cmdSize(stencilBufferSize),

                .m_axis_tvalid(),
                .m_axis_tready(1'b1),
                .m_axis_tlast(),
                .m_axis_tdata(),

                .m_avalid(),
                .m_aaddr(),
                .m_abytes(),
                .m_aready(1'b1)
            );
            defparam stencilBuffer.NUMBER_OF_PIXELS_PER_BEAT = PIXEL_PER_BEAT;
            defparam stencilBuffer.NUMBER_OF_SUB_PIXELS = STENCIL_WIDTH;
            defparam stencilBuffer.SUB_PIXEL_WIDTH = 1;
            defparam stencilBuffer.X_BIT_WIDTH = RENDER_CONFIG_X_SIZE;
            defparam stencilBuffer.Y_BIT_WIDTH = RENDER_CONFIG_Y_SIZE;
            defparam stencilBuffer.FRAMEBUFFER_SIZE_IN_PIXEL_LG = FRAMEBUFFER_SIZE_IN_PIXEL_LG;
            defparam stencilBuffer.FB_SIZE_IN_PIXEL_LG = FB_SIZE_IN_PIXEL_LG;
        end
        else
        begin
            assign m_stencil_arready = 1;
            assign m_stencil_rvalid = 1;
            assign m_stencil_rdata = 0;
            assign m_stencil_wready = 1;
            assign stencilBufferApplied = 1;
        end
    endgenerate

    RasterIXRenderCore #(
        .INDEX_WIDTH(FRAMEBUFFER_SIZE_IN_PIXEL_LG),
        .MAX_TEXTURE_SIZE(MAX_TEXTURE_SIZE),
        .ADDR_WIDTH(ADDR_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .TMU_COUNT(TMU_COUNT),
        .ENABLE_MIPMAPPING(ENABLE_MIPMAPPING),
        .ENABLE_TEXTURE_FILTERING(ENABLE_TEXTURE_FILTERING),
        .ENABLE_FOG(ENABLE_FOG),
        .TMU_MEMORY_WIDTH(DATA_WIDTH),
        .TEXTURE_PAGE_SIZE(TEXTURE_PAGE_SIZE),
        .ENABLE_WRITE_FIFO(0),
        .ENABLE_READ_FIFO(1), // Requires read FIFOs because the internal RAM does not have flow control
        .RASTERIZER_FLOAT_PRECISION(RASTERIZER_FLOAT_PRECISION),
        .RASTERIZER_FIXPOINT_PRECISION(RASTERIZER_FIXPOINT_PRECISION),
        .RASTERIZER_ENABLE_FLOAT_INTERPOLATION(RASTERIZER_ENABLE_FLOAT_INTERPOLATION),
        .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION)
    ) graphicCore (
        .aclk(aclk),
        .resetn(resetn),
        
        .s_cmd_axis_tvalid(s_cmd_axis_tvalid),
        .s_cmd_axis_tready(s_cmd_axis_tready),
        .s_cmd_axis_tlast(s_cmd_axis_tlast),
        .s_cmd_axis_tdata(s_cmd_axis_tdata),

        .framebufferParamEnableScissor(framebufferParamEnableScissor),
        .framebufferParamScissorStartX(framebufferParamScissorStartX),
        .framebufferParamScissorStartY(framebufferParamScissorStartY),
        .framebufferParamScissorEndX(framebufferParamScissorEndX),
        .framebufferParamScissorEndY(framebufferParamScissorEndY),
        .framebufferParamYOffset(framebufferParamYOffset),
        .framebufferParamXResolution(framebufferParamXResolution),
        .framebufferParamYResolution(framebufferParamYResolution),

        .colorBufferClearColor(colorBufferClearColor),
        .colorBufferAddr(colorBufferAddr),
        .colorBufferSize(colorBufferSize),
        .colorBufferApply(colorBufferApply),
        .colorBufferApplied(colorBufferApplied && fb_swapped),
        .colorBufferCmdCommit(colorBufferCmdCommit),
        .colorBufferCmdMemset(colorBufferCmdMemset),
        .colorBufferCmdSwap(colorBufferCmdSwap),
        .colorBufferCmdSwapEnableVsync(colorBufferCmdSwapEnableVsync),
        .colorBufferEnable(colorBufferEnable),
        .colorBufferMask(colorBufferMask),
        .m_color_arready(1),
        .m_color_arlast(m_color_arlast),
        .m_color_arvalid(m_color_arvalid),
        .m_color_araddr(m_color_araddr),
        .m_color_rready(),
        .m_color_rdata(ColorBufferExpandVec(ColorBufferExpand(m_color_rdata), DEFAULT_ALPHA_VAL)),
        .m_color_rvalid(m_color_rvalid),
        .m_color_waddr(m_color_waddr),
        .m_color_wvalid(m_color_wvalid),
        .m_color_wready(1),
        .m_color_wdata(m_color_wdata),
        .m_color_wstrb(m_color_wstrb),
        .m_color_wlast(),
        .m_color_wscreenPosX(m_color_wscreenPosX),
        .m_color_wscreenPosY(m_color_wscreenPosY),

        .depthBufferClearDepth(depthBufferClearDepth),
        .depthBufferAddr(), // Unused in the rixif config
        .depthBufferSize(depthBufferSize),
        .depthBufferApply(depthBufferApply),
        .depthBufferApplied(depthBufferApplied),
        .depthBufferCmdCommit(depthBufferCmdCommit),
        .depthBufferCmdMemset(depthBufferCmdMemset),
        .depthBufferEnable(depthBufferEnable),
        .depthBufferMask(depthBufferMask),
        .m_depth_arready(1),
        .m_depth_arlast(m_depth_arlast),
        .m_depth_arvalid(m_depth_arvalid),
        .m_depth_araddr(m_depth_araddr),
        .m_depth_rready(),
        .m_depth_rdata(m_depth_rdata),
        .m_depth_rvalid(m_depth_rvalid),
        .m_depth_waddr(m_depth_waddr),
        .m_depth_wvalid(m_depth_wvalid),
        .m_depth_wready(1),
        .m_depth_wdata(m_depth_wdata),
        .m_depth_wstrb(m_depth_wstrb),
        .m_depth_wlast(),
        .m_depth_wscreenPosX(m_depth_wscreenPosX),
        .m_depth_wscreenPosY(m_depth_wscreenPosY),

        .stencilBufferClearStencil(stencilBufferClearStencil),
        .stencilBufferAddr(), // Unused in the rixif config
        .stencilBufferSize(stencilBufferSize),
        .stencilBufferApply(stencilBufferApply),
        .stencilBufferApplied(stencilBufferApplied),
        .stencilBufferCmdCommit(stencilBufferCmdCommit),
        .stencilBufferCmdMemset(stencilBufferCmdMemset),
        .stencilBufferEnable(stencilBufferEnable),
        .stencilBufferMask(stencilBufferMask),
        .m_stencil_arready(1),
        .m_stencil_arlast(m_stencil_arlast),
        .m_stencil_arvalid(m_stencil_arvalid),
        .m_stencil_araddr(m_stencil_araddr),
        .m_stencil_rready(),
        .m_stencil_rdata(m_stencil_rdata),
        .m_stencil_rvalid(m_stencil_rvalid),
        .m_stencil_waddr(m_stencil_waddr),
        .m_stencil_wvalid(m_stencil_wvalid),
        .m_stencil_wready(1),
        .m_stencil_wdata(m_stencil_wdata),
        .m_stencil_wstrb(m_stencil_wstrb),
        .m_stencil_wlast(),
        .m_stencil_wscreenPosX(m_stencil_wscreenPosX),
        .m_stencil_wscreenPosY(m_stencil_wscreenPosY),

        .m_tmu0_axi_arid(m_tmu0_axi_arid),
        .m_tmu0_axi_araddr(m_tmu0_axi_araddr),
        .m_tmu0_axi_arlen(m_tmu0_axi_arlen),
        .m_tmu0_axi_arsize(m_tmu0_axi_arsize),
        .m_tmu0_axi_arburst(m_tmu0_axi_arburst),
        .m_tmu0_axi_arlock(m_tmu0_axi_arlock),
        .m_tmu0_axi_arcache(m_tmu0_axi_arcache),
        .m_tmu0_axi_arprot(m_tmu0_axi_arprot),
        .m_tmu0_axi_arvalid(m_tmu0_axi_arvalid),
        .m_tmu0_axi_arready(m_tmu0_axi_arready),
        .m_tmu0_axi_rid(m_tmu0_axi_rid),
        .m_tmu0_axi_rdata(m_tmu0_axi_rdata),
        .m_tmu0_axi_rresp(m_tmu0_axi_rresp),
        .m_tmu0_axi_rlast(m_tmu0_axi_rlast),
        .m_tmu0_axi_rvalid(m_tmu0_axi_rvalid),
        .m_tmu0_axi_rready(m_tmu0_axi_rready),

        .m_tmu1_axi_arid(m_tmu1_axi_arid),
        .m_tmu1_axi_araddr(m_tmu1_axi_araddr),
        .m_tmu1_axi_arlen(m_tmu1_axi_arlen),
        .m_tmu1_axi_arsize(m_tmu1_axi_arsize),
        .m_tmu1_axi_arburst(m_tmu1_axi_arburst),
        .m_tmu1_axi_arlock(m_tmu1_axi_arlock),
        .m_tmu1_axi_arcache(m_tmu1_axi_arcache),
        .m_tmu1_axi_arprot(m_tmu1_axi_arprot),
        .m_tmu1_axi_arvalid(m_tmu1_axi_arvalid),
        .m_tmu1_axi_arready(m_tmu1_axi_arready),
        .m_tmu1_axi_rid(m_tmu1_axi_rid),
        .m_tmu1_axi_rdata(m_tmu1_axi_rdata),
        .m_tmu1_axi_rresp(m_tmu1_axi_rresp),
        .m_tmu1_axi_rlast(m_tmu1_axi_rlast),
        .m_tmu1_axi_rvalid(m_tmu1_axi_rvalid),
        .m_tmu1_axi_rready(m_tmu1_axi_rready)
    );

    assign swap_fb = colorBufferApply && colorBufferCmdSwap;
    assign swap_fb_enable_vsync = colorBufferCmdSwapEnableVsync;
    assign fb_addr = colorBufferAddr;
    assign fb_size = colorBufferSize;

endmodule