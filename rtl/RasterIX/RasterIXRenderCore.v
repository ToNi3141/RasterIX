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

module RasterIXRenderCore #(
    `include "RasterizerCommands.vh"

    // The size of the internal framebuffer (in power of two)
    // The access is in words, not bytes! A word can have the size of at least 1 bit to n bits
    parameter INDEX_WIDTH = 17,

    // The width of the address channel
    parameter ADDR_WIDTH = 32,
    parameter ID_WIDTH = 8,

    // Number of TMUs. Currently supported values: 1 and 2
    parameter TMU_COUNT = 2,
    parameter ENABLE_MIPMAPPING = 1,
    parameter ENABLE_TEXTURE_FILTERING = 1,
    parameter TMU_MEMORY_WIDTH = 64,
    parameter TEXTURE_PAGE_SIZE = 2048,

    // Enables the fogging unit
    parameter ENABLE_FOG = 1,
    
    // The bit width of the command stream interface
    // Allowed values: 32, 64, 128, 256 bit
    localparam CMD_STREAM_WIDTH = 32,

    // The maximum size of a texture
    parameter MAX_TEXTURE_SIZE = 256,

    // Enables FIFOs on the memory write channel. It can improve the performance by batching write requests.
    parameter ENABLE_WRITE_FIFO = 1,
    // Size of the write FIFO in log2(size)
    parameter WRITE_FIFO_SIZE = 4,

    // Enables FIFOs on the memory read channel. It can improve the performance by reducing stalls in the pipeline.
    parameter ENABLE_READ_FIFO = 1,
    // Size of the read FIFO in log2(size). Keep this size to make sure, it works with the rixif configuration
    parameter READ_FIFO_SIZE = 7,

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

    // The size of a sub pixel
    localparam SUB_PIXEL_WIDTH = 8,
    parameter SUB_PIXEL_CALC_PRECISION = SUB_PIXEL_WIDTH,

    // The number of sub pixel used for a pixel
    localparam NUMBER_OF_SUB_PIXELS = 4,

    // Pixel width
    localparam PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS * SUB_PIXEL_WIDTH,

    // Stencil width
    localparam STENCIL_WIDTH = 4,

    // Depth width
    localparam DEPTH_WIDTH = 16,

    // Screen pos width
    localparam SCREEN_POS_WIDTH = 11,

    // Number of pixel the pipeline can maximal contain until a stall event occurs in power of two
    localparam MAX_NUMBER_OF_PIXELS_LG = 7,

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

    output wire                                 framebufferParamEnableScissor,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      framebufferParamScissorStartX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      framebufferParamScissorStartY,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      framebufferParamScissorEndX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      framebufferParamScissorEndY,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      framebufferParamYOffset,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      framebufferParamXResolution,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      framebufferParamYResolution,

    // Color buffer access
    output wire [PIXEL_WIDTH - 1 : 0]           colorBufferClearColor,
    output wire [ADDR_WIDTH - 1 : 0]            colorBufferAddr,
    output wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]   colorBufferSize,
    output wire                                 colorBufferApply,
    input  wire                                 colorBufferApplied,
    output wire                                 colorBufferCmdCommit,
    output wire                                 colorBufferCmdMemset,
    output wire                                 colorBufferCmdSwap,
    output wire                                 colorBufferCmdSwapEnableVsync,
    output wire                                 colorBufferEnable,
    output wire [NUMBER_OF_SUB_PIXELS - 1 : 0]  colorBufferMask,
    input  wire                                 m_color_arready,
    output wire                                 m_color_arvalid,
    output wire                                 m_color_arlast,
    output wire [INDEX_WIDTH - 1 : 0]           m_color_araddr,
    output wire                                 m_color_rready,
    input  wire                                 m_color_rvalid,
    input  wire [PIXEL_WIDTH - 1 : 0]           m_color_rdata,
    output wire [INDEX_WIDTH - 1 : 0]           m_color_waddr,
    output wire                                 m_color_wvalid,
    input  wire                                 m_color_wready,
    output wire                                 m_color_wlast,
    output wire [PIXEL_WIDTH - 1 : 0]           m_color_wdata,
    output wire                                 m_color_wstrb,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      m_color_wscreenPosX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      m_color_wscreenPosY,

    // Depth buffer access
    output wire [DEPTH_WIDTH - 1 : 0]           depthBufferClearDepth,
    output wire [ADDR_WIDTH - 1 : 0]            depthBufferAddr,
    output wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]   depthBufferSize,
    output wire                                 depthBufferApply,
    input  wire                                 depthBufferApplied,
    output wire                                 depthBufferCmdCommit,
    output wire                                 depthBufferCmdMemset,
    output wire                                 depthBufferEnable,
    output wire                                 depthBufferMask,
    input  wire                                 m_depth_arready,
    output wire                                 m_depth_arvalid,
    output wire                                 m_depth_arlast,
    output wire [INDEX_WIDTH - 1 : 0]           m_depth_araddr,
    output wire                                 m_depth_rready,
    input  wire                                 m_depth_rvalid,
    input  wire [DEPTH_WIDTH - 1 : 0]           m_depth_rdata,
    output wire [INDEX_WIDTH - 1 : 0]           m_depth_waddr,
    output wire                                 m_depth_wvalid,
    input  wire                                 m_depth_wready,
    output wire                                 m_depth_wlast,
    output wire [DEPTH_WIDTH - 1 : 0]           m_depth_wdata,
    output wire                                 m_depth_wstrb,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      m_depth_wscreenPosX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      m_depth_wscreenPosY,

    // Stencil buffer access
    output wire [STENCIL_WIDTH - 1 : 0]         stencilBufferClearStencil,
    output wire [ADDR_WIDTH - 1 : 0]            stencilBufferAddr,
    output wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]   stencilBufferSize,
    output wire                                 stencilBufferApply,
    input  wire                                 stencilBufferApplied,
    output wire                                 stencilBufferCmdCommit,
    output wire                                 stencilBufferCmdMemset,
    output wire                                 stencilBufferEnable,
    output wire [STENCIL_WIDTH - 1 : 0]         stencilBufferMask,
    input  wire                                 m_stencil_arready,
    output wire                                 m_stencil_arvalid,
    output wire                                 m_stencil_arlast,
    output wire [INDEX_WIDTH - 1 : 0]           m_stencil_araddr,
    output wire                                 m_stencil_rready,
    input  wire                                 m_stencil_rvalid,
    input  wire [STENCIL_WIDTH - 1 : 0]         m_stencil_rdata,
    output wire [INDEX_WIDTH - 1 : 0]           m_stencil_waddr,
    output wire                                 m_stencil_wvalid,
    input  wire                                 m_stencil_wready,
    output wire                                 m_stencil_wlast,
    output wire [STENCIL_WIDTH - 1 : 0]         m_stencil_wdata,
    output wire                                 m_stencil_wstrb,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      m_stencil_wscreenPosX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      m_stencil_wscreenPosY,

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
    input  wire [TMU_MEMORY_WIDTH - 1 : 0]      m_tmu0_axi_rdata,
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
    input  wire [TMU_MEMORY_WIDTH - 1 : 0]      m_tmu1_axi_rdata,
    input  wire [ 1 : 0]                        m_tmu1_axi_rresp,
    input  wire                                 m_tmu1_axi_rlast,
    input  wire                                 m_tmu1_axi_rvalid,
    output wire                                 m_tmu1_axi_rready
);
`include "RegisterAndDescriptorDefines.vh"

    localparam TEX_ADDR_WIDTH = 17;
    localparam ATTRIBUTE_SIZE = 32;
    
    // The bit width of the texture stream
    localparam TEXTURE_STREAM_WIDTH = TMU_MEMORY_WIDTH;

    initial
    begin
        if (TMU_COUNT > 2)
        begin
            $error("Only a maximum of 2 TMUs are supported");
            $finish;
        end
        if (TMU_COUNT < 1)
        begin
            $error("At least one TMU must be enabled");
            $finish;
        end

        if ((SCREEN_POS_WIDTH != RENDER_CONFIG_X_SIZE) || (SCREEN_POS_WIDTH != RENDER_CONFIG_Y_SIZE))
        begin
            $error("Screen size width in the RasterIXRenderCore and RegisterAndDescriptorDefines are different");
            $finish;
        end

        if (COLOR_SUB_PIXEL_WIDTH != SUB_PIXEL_WIDTH)
        begin
            $error("The sub pixel width in the RasterIXRenderCore and RegisterAndDescriptorDefines are different");
            $finish;
        end

        if (COLOR_NUMBER_OF_SUB_PIXEL != NUMBER_OF_SUB_PIXELS)
        begin
            $error("The number of sub pixels in the RasterIXRenderCore and RegisterAndDescriptorDefines are different");
            $finish;
        end

        if ((RASTERIZER_FLOAT_PRECISION > 32) || (RASTERIZER_FLOAT_PRECISION < 20))
        begin
            $error("RASTERIZER_FLOAT_PRECISION must be between 25 and 20");
            $finish;
        end

        if ((RASTERIZER_FIXPOINT_PRECISION > 25) || (RASTERIZER_FIXPOINT_PRECISION < 16))
        begin
            $error("RASTERIZER_FIXPOINT_PRECISION must be between 16 and 25");
            $finish;
        end
    end

    localparam ENABLE_SECOND_TMU = TMU_COUNT == 2;
    
    ////////////////////////////////////////////////////////////////////////////
    // Command Parser
    // Execution unit for commands from the command interface
    // Clocks: n/a
    ////////////////////////////////////////////////////////////////////////////
    wire [CMD_STREAM_WIDTH - 1 : 0]  cmd_xxx_axis_tdata;
    wire [ 4 : 0]    cmd_xxx_axis_tuser;
    wire             cmd_xxx_axis_tlast;
    wire             cmd_fog_axis_tvalid;
    wire             cmd_rasterizer_axis_tvalid;
    wire             cmd_tmu0_axis_tvalid;
    wire             cmd_tmu1_axis_tvalid;
    wire             cmd_config_axis_tvalid;
    wire             cmd_fog_axis_tready;
    wire             cmd_rasterizer_axis_tready;
    wire             cmd_tmu0_axis_tready;
    wire             cmd_tmu1_axis_tready;
    wire             cmd_config_axis_tready;
    // Control
    wire             pixelInPipeline;
    wire             dataInTriangleInterpolator;
    wire             startRendering;
    wire             color_fifo_empty;
    wire             depth_fifo_empty;
    wire             stencil_fifo_empty;
    wire             rasterizerRunning;
    CommandParser commandParser (
        .aclk(aclk),
        .resetn(resetn),

        // AXI Stream command interface
        .s_cmd_axis_tvalid(s_cmd_axis_tvalid),
        .s_cmd_axis_tready(s_cmd_axis_tready),
        .s_cmd_axis_tlast(s_cmd_axis_tlast),
        .s_cmd_axis_tdata(s_cmd_axis_tdata),

        .m_cmd_xxx_axis_tdata(cmd_xxx_axis_tdata),
        .m_cmd_xxx_axis_tuser(cmd_xxx_axis_tuser),
        .m_cmd_xxx_axis_tlast(cmd_xxx_axis_tlast),
        .m_cmd_fog_axis_tvalid(cmd_fog_axis_tvalid),
        .m_cmd_rasterizer_axis_tvalid(cmd_rasterizer_axis_tvalid),
        .m_cmd_tmu0_axis_tvalid(cmd_tmu0_axis_tvalid),
        .m_cmd_tmu1_axis_tvalid(cmd_tmu1_axis_tvalid),
        .m_cmd_config_axis_tvalid(cmd_config_axis_tvalid),
        .m_cmd_fog_axis_tready(cmd_fog_axis_tready),
        .m_cmd_rasterizer_axis_tready(cmd_rasterizer_axis_tready),
        .m_cmd_tmu0_axis_tready(cmd_tmu0_axis_tready),
        .m_cmd_tmu1_axis_tready(cmd_tmu1_axis_tready),
        .m_cmd_config_axis_tready(cmd_config_axis_tready),

        // Rasterizer
        // Control
        .rasterizerRunning(rasterizerRunning),
        .pixelInPipeline(pixelInPipeline || !color_fifo_empty || !depth_fifo_empty || !stencil_fifo_empty),
        .dataInTriangleInterpolator(dataInInterpolator),

        // applied
        .colorBufferApply(colorBufferApply),
        .colorBufferApplied(colorBufferApplied),
        .colorBufferCmdCommit(colorBufferCmdCommit),
        .colorBufferCmdMemset(colorBufferCmdMemset),
        .colorBufferCmdSwap(colorBufferCmdSwap),
        .colorBufferCmdSwapEnableVsync(colorBufferCmdSwapEnableVsync),
        .colorBufferSize(colorBufferSize),
        .depthBufferApply(depthBufferApply),
        .depthBufferApplied(depthBufferApplied),
        .depthBufferCmdCommit(depthBufferCmdCommit),
        .depthBufferCmdMemset(depthBufferCmdMemset),
        .depthBufferSize(depthBufferSize),
        .stencilBufferApply(stencilBufferApply),
        .stencilBufferApplied(stencilBufferApplied),
        .stencilBufferCmdCommit(stencilBufferCmdCommit),
        .stencilBufferCmdMemset(stencilBufferCmdMemset),
        .stencilBufferSize(stencilBufferSize)
    );
    defparam commandParser.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;
    defparam commandParser.TEXTURE_STREAM_WIDTH = TEXTURE_STREAM_WIDTH;

    ////////////////////////////////////////////////////////////////////////////
    // Register Bank for the triangle parameters
    // Clocks: n/a
    ////////////////////////////////////////////////////////////////////////////
    wire [(TRIANGLE_STREAM_PARAM_SIZE * TRIANGLE_STREAM_SIZE) - 1 : 0] triangleParams;

    generate
        if (RASTERIZER_ENABLE_FLOAT_INTERPOLATION)
        begin
            RegisterBank triangleParameters (
                .aclk(aclk),
                .resetn(resetn),

                .s_axis_tvalid(cmd_rasterizer_axis_tvalid),
                .s_axis_tlast(cmd_xxx_axis_tlast),
                .s_axis_tdata(cmd_xxx_axis_tdata),
                .s_axis_tuser(0),

                .registers(triangleParams),

                .registers_updated(startRendering),
                .update_acknowledged(rasterizerRunning)
            );
            defparam triangleParameters.BANK_SIZE = TRIANGLE_STREAM_SIZE;
            defparam triangleParameters.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;
            assign cmd_rasterizer_axis_tready = 1;
        end
        else
        begin
            wire            triangle_axis_tvalid;
            wire            triangle_axis_tlast;
            wire [31 : 0]   triangle_axis_tdata;

            wire            trianglex_axis_tvalid;
            wire            trianglex_axis_tlast;
            wire [31 : 0]   trianglex_axis_tdata;

            axis_adapter #(
                .S_DATA_WIDTH(CMD_STREAM_WIDTH),
                .M_DATA_WIDTH(32),
                .S_KEEP_ENABLE(1),
                .M_KEEP_ENABLE(1),
                .ID_ENABLE(0),
                .DEST_ENABLE(0),
                .USER_ENABLE(0)
            ) triangleStreamAxisAdapter (
                .clk(aclk),
                .rst(!resetn),

                .s_axis_tdata(cmd_xxx_axis_tdata),
                .s_axis_tkeep(~0),
                .s_axis_tvalid(cmd_rasterizer_axis_tvalid),
                .s_axis_tready(cmd_rasterizer_axis_tready),
                .s_axis_tlast(cmd_xxx_axis_tlast),
                .s_axis_tid(0),
                .s_axis_tdest(0),
                .s_axis_tuser(0),

                .m_axis_tdata(triangle_axis_tdata),
                .m_axis_tkeep(),
                .m_axis_tvalid(triangle_axis_tvalid),
                .m_axis_tready(1),
                .m_axis_tlast(triangle_axis_tlast),
                .m_axis_tid(),
                .m_axis_tdest(),
                .m_axis_tuser()
            );

            TriangleStreamF2XConverter triangleStreamF2XConverter (
                .aclk(aclk),
                .resetn(resetn),

                .s_axis_tvalid(triangle_axis_tvalid),
                .s_axis_tlast(triangle_axis_tlast),
                .s_axis_tdata(triangle_axis_tdata),

                .m_axis_tvalid(trianglex_axis_tvalid),
                .m_axis_tlast(trianglex_axis_tlast),
                .m_axis_tdata(trianglex_axis_tdata)
            );

            RegisterBank triangleParameters (
                .aclk(aclk),
                .resetn(resetn),

                .s_axis_tvalid(trianglex_axis_tvalid),
                .s_axis_tlast(trianglex_axis_tlast),
                .s_axis_tdata(trianglex_axis_tdata),
                .s_axis_tuser(0),

                .registers(triangleParams),

                .registers_updated(startRendering),
                .update_acknowledged(rasterizerRunning)
            );
            defparam triangleParameters.BANK_SIZE = TRIANGLE_STREAM_SIZE;
            defparam triangleParameters.CMD_STREAM_WIDTH = 32;
        end
    endgenerate
    
    ////////////////////////////////////////////////////////////////////////////
    // Register Bank for the render configs
    // Clocks: n/a
    ////////////////////////////////////////////////////////////////////////////
    wire [(OP_RENDER_CONFIG_REG_WIDTH * OP_RENDER_CONFIG_NUMBER_OR_REGS) - 1 : 0] renderConfigs;
    RegisterBank renderConfigsRegBank (
        .aclk(aclk),
        .resetn(resetn),

        .s_axis_tvalid(cmd_config_axis_tvalid),
        .s_axis_tlast(cmd_xxx_axis_tlast),
        .s_axis_tdata(cmd_xxx_axis_tdata),
        .s_axis_tuser(cmd_xxx_axis_tuser),

        .registers(renderConfigs),

        .registers_updated(),
        .update_acknowledged(1'b1)
    );
    defparam renderConfigsRegBank.BANK_SIZE = OP_RENDER_CONFIG_NUMBER_OR_REGS;
    defparam renderConfigsRegBank.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;
    defparam renderConfigsRegBank.COMPRESSED = 0;
    assign cmd_config_axis_tready = 1;

    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confFeatureEnable;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confFragmentPipelineConfig;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confFragmentPipelineFogColor;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confTMU0TexEnvConfig;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confTMU0TextureConfig;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confTMU0TexEnvColor;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confScissorStartXY;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confScissorEndXY;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confYOffset;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confRenderResolution;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confTMU1TexEnvConfig;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confTMU1TextureConfig;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confTMU1TexEnvColor;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confStencilBufferConfig;
    assign confFeatureEnable = renderConfigs[OP_RENDER_CONFIG_FEATURE_ENABLE +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confFragmentPipelineConfig = renderConfigs[OP_RENDER_CONFIG_FRAGMENT_PIPELINE +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confFragmentPipelineFogColor = renderConfigs[OP_RENDER_CONFIG_FRAGMENT_FOG_COLOR +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confTMU0TexEnvConfig = renderConfigs[OP_RENDER_CONFIG_TMU0_TEX_ENV +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confTMU0TextureConfig = renderConfigs[OP_RENDER_CONFIG_TMU0_TEXTURE_CONFIG +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confTMU0TexEnvColor = renderConfigs[OP_RENDER_CONFIG_TMU0_TEX_ENV_COLOR +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confTMU1TexEnvConfig = renderConfigs[OP_RENDER_CONFIG_TMU1_TEX_ENV +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confTMU1TextureConfig = renderConfigs[OP_RENDER_CONFIG_TMU1_TEXTURE_CONFIG +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confTMU1TexEnvColor = renderConfigs[OP_RENDER_CONFIG_TMU1_TEX_ENV_COLOR +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confScissorStartXY = renderConfigs[OP_RENDER_CONFIG_SCISSOR_START_XY +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confScissorEndXY = renderConfigs[OP_RENDER_CONFIG_SCISSOR_END_XY +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confYOffset = renderConfigs[OP_RENDER_CONFIG_Y_OFFSET +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confRenderResolution = renderConfigs[OP_RENDER_CONFIG_RENDER_RESOLUTION +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confStencilBufferConfig = renderConfigs[OP_RENDER_CONFIG_STENCIL_BUFFER +: OP_RENDER_CONFIG_REG_WIDTH];
    assign colorBufferClearColor = renderConfigs[OP_RENDER_CONFIG_COLOR_BUFFER_CLEAR_COLOR +: OP_RENDER_CONFIG_REG_WIDTH];
    assign depthBufferClearDepth = renderConfigs[OP_RENDER_CONFIG_DEPTH_BUFFER_CLEAR_DEPTH +: RENDER_CONFIG_CLEAR_DEPTH_SIZE - (RENDER_CONFIG_CLEAR_DEPTH_SIZE - DEPTH_WIDTH)];
    assign stencilBufferClearStencil = confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_CLEAR_STENICL_POS +: RENDER_CONFIG_STENCIL_BUFFER_CLEAR_STENICL_SIZE - (RENDER_CONFIG_STENCIL_BUFFER_CLEAR_STENICL_SIZE - STENCIL_WIDTH)];
    assign colorBufferAddr = renderConfigs[OP_RENDER_CONFIG_COLOR_BUFFER_ADDR +: OP_RENDER_CONFIG_REG_WIDTH];
    assign depthBufferAddr = renderConfigs[OP_RENDER_CONFIG_DEPTH_BUFFER_ADDR +: OP_RENDER_CONFIG_REG_WIDTH];
    assign stencilBufferAddr = renderConfigs[OP_RENDER_CONFIG_STENCIL_BUFFER_ADDR +: OP_RENDER_CONFIG_REG_WIDTH];
    assign framebufferParamEnableScissor = confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_SCISSOR_POS];
    assign framebufferParamScissorStartX = confScissorStartXY[RENDER_CONFIG_X_POS +: RENDER_CONFIG_X_SIZE];
    assign framebufferParamScissorStartY = confScissorStartXY[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE];
    assign framebufferParamScissorEndX = confScissorEndXY[RENDER_CONFIG_X_POS +: RENDER_CONFIG_X_SIZE];
    assign framebufferParamScissorEndY = confScissorEndXY[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE];
    assign framebufferParamYOffset = confYOffset[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE];
    assign framebufferParamXResolution = confRenderResolution[RENDER_CONFIG_X_POS +: RENDER_CONFIG_X_SIZE];
    assign framebufferParamYResolution = confRenderResolution[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE];
    assign depthBufferEnable = confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_DEPTH_TEST_POS];
    assign depthBufferMask = confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_DEPTH_MASK_POS +: RENDER_CONFIG_FRAGMENT_DEPTH_MASK_SIZE];
    assign colorBufferEnable = 1'b1;
    assign colorBufferMask = { 
                    confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_COLOR_MASK_R_POS +: RENDER_CONFIG_FRAGMENT_COLOR_MASK_R_SIZE], 
                    confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_COLOR_MASK_G_POS +: RENDER_CONFIG_FRAGMENT_COLOR_MASK_G_SIZE], 
                    confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_COLOR_MASK_B_POS +: RENDER_CONFIG_FRAGMENT_COLOR_MASK_B_SIZE], 
                    confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_COLOR_MASK_A_POS +: RENDER_CONFIG_FRAGMENT_COLOR_MASK_A_SIZE] };
    assign stencilBufferEnable = confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_STENCIL_TEST_POS];
    assign stencilBufferMask = confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_STENICL_MASK_POS +: RENDER_CONFIG_STENCIL_BUFFER_STENICL_MASK_SIZE - (RENDER_CONFIG_STENCIL_BUFFER_STENICL_MASK_SIZE - STENCIL_WIDTH)];

    ////////////////////////////////////////////////////////////////////////////
    // Texture Mapping Unit Buffer 0
    // Memory area where the texture is stored
    // Clocks: n/a
    ////////////////////////////////////////////////////////////////////////////
    wire                                axis_tmu0_tvalid;
    wire                                axis_tmu0_tlast;
    wire  [TMU_MEMORY_WIDTH - 1 : 0]    axis_tmu0_tdata;
    PagedMemoryReader pagedMemoryReaderTmu0 (
        .aclk(aclk),
        .resetn(resetn),

        .m_axis_tvalid(axis_tmu0_tvalid),
        .m_axis_tlast(axis_tmu0_tlast),
        .m_axis_tdata(axis_tmu0_tdata),

        .s_axis_tvalid(cmd_tmu0_axis_tvalid),
        .s_axis_tready(cmd_tmu0_axis_tready),
        .s_axis_tlast(cmd_xxx_axis_tlast),
        .s_axis_tdata(cmd_xxx_axis_tdata[0 +: 32]),

        .m_mem_axi_arid(m_tmu0_axi_arid),
        .m_mem_axi_araddr(m_tmu0_axi_araddr),
        .m_mem_axi_arlen(m_tmu0_axi_arlen),
        .m_mem_axi_arsize(m_tmu0_axi_arsize),
        .m_mem_axi_arburst(m_tmu0_axi_arburst),
        .m_mem_axi_arlock(m_tmu0_axi_arlock),
        .m_mem_axi_arcache(m_tmu0_axi_arcache),
        .m_mem_axi_arprot(m_tmu0_axi_arprot),
        .m_mem_axi_arvalid(m_tmu0_axi_arvalid),
        .m_mem_axi_arready(m_tmu0_axi_arready),
        .m_mem_axi_rid(m_tmu0_axi_rid),
        .m_mem_axi_rdata(m_tmu0_axi_rdata),
        .m_mem_axi_rresp(m_tmu0_axi_rresp),
        .m_mem_axi_rlast(m_tmu0_axi_rlast),
        .m_mem_axi_rvalid(m_tmu0_axi_rvalid),
        .m_mem_axi_rready(m_tmu0_axi_rready)
    );
    defparam pagedMemoryReaderTmu0.DATA_WIDTH = TMU_MEMORY_WIDTH;
    defparam pagedMemoryReaderTmu0.ADDR_WIDTH = ADDR_WIDTH;
    defparam pagedMemoryReaderTmu0.ID_WIDTH = ID_WIDTH;
    defparam pagedMemoryReaderTmu0.PAGE_SIZE = TEXTURE_PAGE_SIZE;
    
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel0Addr00;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel0Addr01;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel0Addr10;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel0Addr11;
    wire [31 : 0]                   texel0Input00;
    wire [31 : 0]                   texel0Input01;
    wire [31 : 0]                   texel0Input10;
    wire [31 : 0]                   texel0Input11;
    TextureBuffer textureBufferTMU0 (
        .aclk(aclk),
        .resetn(resetn),

        .confPixelFormat(confTMU0TextureConfig[RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_POS +: RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_SIZE]),

        .texelAddr00(texel0Addr00),
        .texelAddr01(texel0Addr01),
        .texelAddr10(texel0Addr10),
        .texelAddr11(texel0Addr11),

        .texelOutput00(texel0Input00),
        .texelOutput01(texel0Input01),
        .texelOutput10(texel0Input10),
        .texelOutput11(texel0Input11),

        .s_axis_tvalid(axis_tmu0_tvalid),
        .s_axis_tlast(axis_tmu0_tlast),
        .s_axis_tdata(axis_tmu0_tdata)
    );
    defparam textureBufferTMU0.STREAM_WIDTH = TMU_MEMORY_WIDTH;
    defparam textureBufferTMU0.MAX_TEXTURE_SIZE = MAX_TEXTURE_SIZE;
    defparam textureBufferTMU0.PIXEL_WIDTH = COLOR_NUMBER_OF_SUB_PIXEL * COLOR_SUB_PIXEL_WIDTH;
    defparam textureBufferTMU0.ENABLE_LOD = ENABLE_MIPMAPPING;

    ////////////////////////////////////////////////////////////////////////////
    // Texture Mapping Unit Buffer 1
    // Memory area where the texture is stored
    // Clocks: n/a
    ////////////////////////////////////////////////////////////////////////////
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel1Addr00;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel1Addr01;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel1Addr10;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel1Addr11;
    wire [31 : 0]                   texel1Input00;
    wire [31 : 0]                   texel1Input01;
    wire [31 : 0]                   texel1Input10;
    wire [31 : 0]                   texel1Input11;
    generate
        if (ENABLE_SECOND_TMU)
        begin
            wire                                axis_tmu1_tvalid;
            wire                                axis_tmu1_tlast;
            wire  [TMU_MEMORY_WIDTH - 1 : 0]    axis_tmu1_tdata;
            PagedMemoryReader pagedMemoryReaderTmu1 (
                .aclk(aclk),
                .resetn(resetn),

                .m_axis_tvalid(axis_tmu1_tvalid),
                .m_axis_tlast(axis_tmu1_tlast),
                .m_axis_tdata(axis_tmu1_tdata),

                .s_axis_tvalid(cmd_tmu1_axis_tvalid),
                .s_axis_tready(cmd_tmu1_axis_tready),
                .s_axis_tlast(cmd_xxx_axis_tlast),
                .s_axis_tdata(cmd_xxx_axis_tdata[0 +: 32]),

                .m_mem_axi_arid(m_tmu1_axi_arid),
                .m_mem_axi_araddr(m_tmu1_axi_araddr),
                .m_mem_axi_arlen(m_tmu1_axi_arlen),
                .m_mem_axi_arsize(m_tmu1_axi_arsize),
                .m_mem_axi_arburst(m_tmu1_axi_arburst),
                .m_mem_axi_arlock(m_tmu1_axi_arlock),
                .m_mem_axi_arcache(m_tmu1_axi_arcache),
                .m_mem_axi_arprot(m_tmu1_axi_arprot),
                .m_mem_axi_arvalid(m_tmu1_axi_arvalid),
                .m_mem_axi_arready(m_tmu1_axi_arready),
                .m_mem_axi_rid(m_tmu1_axi_rid),
                .m_mem_axi_rdata(m_tmu1_axi_rdata),
                .m_mem_axi_rresp(m_tmu1_axi_rresp),
                .m_mem_axi_rlast(m_tmu1_axi_rlast),
                .m_mem_axi_rvalid(m_tmu1_axi_rvalid),
                .m_mem_axi_rready(m_tmu1_axi_rready)
            );
            defparam pagedMemoryReaderTmu1.DATA_WIDTH = TMU_MEMORY_WIDTH;
            defparam pagedMemoryReaderTmu1.ADDR_WIDTH = ADDR_WIDTH;
            defparam pagedMemoryReaderTmu1.ID_WIDTH = ID_WIDTH;
            defparam pagedMemoryReaderTmu1.PAGE_SIZE = TEXTURE_PAGE_SIZE;
            
            TextureBuffer textureBufferTMU1 (
                .aclk(aclk),
                .resetn(resetn),

                .confPixelFormat(confTMU1TextureConfig[RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_POS +: RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_SIZE]),

                .texelAddr00(texel1Addr00),
                .texelAddr01(texel1Addr01),
                .texelAddr10(texel1Addr10),
                .texelAddr11(texel1Addr11),

                .texelOutput00(texel1Input00),
                .texelOutput01(texel1Input01),
                .texelOutput10(texel1Input10),
                .texelOutput11(texel1Input11),

                .s_axis_tvalid(axis_tmu1_tvalid),
                .s_axis_tlast(axis_tmu1_tlast),
                .s_axis_tdata(axis_tmu1_tdata)
            );
            defparam textureBufferTMU1.STREAM_WIDTH = TMU_MEMORY_WIDTH;
            defparam textureBufferTMU1.MAX_TEXTURE_SIZE = MAX_TEXTURE_SIZE;
            defparam textureBufferTMU1.PIXEL_WIDTH = COLOR_NUMBER_OF_SUB_PIXEL * COLOR_SUB_PIXEL_WIDTH;
            defparam textureBufferTMU1.ENABLE_LOD = ENABLE_MIPMAPPING;
        end
        else
        begin
            assign texel1Input00 = 0;
            assign texel1Input01 = 0;
            assign texel1Input10 = 0;
            assign texel1Input11 = 0;

            assign m_tmu1_axi_rready = 1;
            assign m_tmu1_axi_arid = 0;
            assign m_tmu1_axi_araddr = 0;
            assign m_tmu1_axi_arlen = 0;
            assign m_tmu1_axi_arsize = 0;
            assign m_tmu1_axi_arburst = 0;
            assign m_tmu1_axi_arlock = 0;
            assign m_tmu1_axi_arcache = 0;
            assign m_tmu1_axi_arprot = 0;
            assign m_tmu1_axi_arvalid = 0;
            assign cmd_tmu1_axis_tready = 1;
        end
    endgenerate

    ////////////////////////////////////////////////////////////////////////////
    // STEP 0
    // Rasterizing the triangle
    // Clocks: n/a
    ////////////////////////////////////////////////////////////////////////////
    wire                            rasterizer_tvalid;
    wire                            rasterizer_tpixel;
    wire                            rasterizer_tready;
    wire                            rasterizer_tlast;
    wire                            rasterizer_tkeep;
    wire [SCREEN_POS_WIDTH - 1 : 0] rasterizer_tbbx;
    wire [SCREEN_POS_WIDTH - 1 : 0] rasterizer_tbby;
    wire [SCREEN_POS_WIDTH - 1 : 0] rasterizer_tspx;
    wire [SCREEN_POS_WIDTH - 1 : 0] rasterizer_tspy;
    wire [INDEX_WIDTH - 1 : 0]      rasterizer_tindex;
    wire [RR_CMD_SIZE - 1 : 0]      rasterizer_tcmd;

    Rasterizer rop (
        .clk(aclk), 
        .reset(!resetn), 

        .rasterizerRunning(rasterizerRunning),
        .startRendering(startRendering & !pixelInPipeline),

        .yOffset(confYOffset[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE]),
        .xResolution(confRenderResolution[RENDER_CONFIG_X_POS +: SCREEN_POS_WIDTH]),
        .yResolution(confRenderResolution[RENDER_CONFIG_Y_POS +: SCREEN_POS_WIDTH]),

        .bbStart(triangleParams[TRIANGLE_STREAM_BB_START * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .bbEnd(triangleParams[TRIANGLE_STREAM_BB_END * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w0(triangleParams[TRIANGLE_STREAM_INC_W0 * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w1(triangleParams[TRIANGLE_STREAM_INC_W1 * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w2(triangleParams[TRIANGLE_STREAM_INC_W2 * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w0IncX(triangleParams[TRIANGLE_STREAM_INC_W0_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w1IncX(triangleParams[TRIANGLE_STREAM_INC_W1_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w2IncX(triangleParams[TRIANGLE_STREAM_INC_W2_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w0IncY(triangleParams[TRIANGLE_STREAM_INC_W0_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w1IncY(triangleParams[TRIANGLE_STREAM_INC_W1_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w2IncY(triangleParams[TRIANGLE_STREAM_INC_W2_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),

        .m_rr_tvalid(rasterizer_tvalid),
        .m_rr_tpixel(rasterizer_tpixel),
        .m_rr_tready(rasterizer_tready),
        .m_rr_tlast(rasterizer_tlast),
        .m_rr_tkeep(rasterizer_tkeep),
        .m_rr_tbbx(rasterizer_tbbx),
        .m_rr_tbby(rasterizer_tbby),
        .m_rr_tspx(rasterizer_tspx),
        .m_rr_tspy(rasterizer_tspy),
        .m_rr_tindex(rasterizer_tindex),
        .m_rr_tcmd(rasterizer_tcmd)
    );
    defparam rop.X_BIT_WIDTH = SCREEN_POS_WIDTH;
    defparam rop.Y_BIT_WIDTH = SCREEN_POS_WIDTH;
    defparam rop.INDEX_WIDTH = INDEX_WIDTH;
    defparam rop.RASTERIZER_ENABLE_INITIAL_Y_INC = RASTERIZER_ENABLE_FLOAT_INTERPOLATION;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Counting of the pixels which are send from the rasterizer and which leave
    // the pipeline. Used to determine if there are pixel on the fly or not.
    ////////////////////////////////////////////////////////////////////////////
    wire fragmentProcessed;

    ValueTrack vt (
        .aclk(aclk),
        .resetn(resetn),

        .sigIncomingValue(rasterizer_tpixel & rasterizer_tvalid & rasterizer_tready),
        .sigOutgoingValue(fragmentProcessed & framebuffer_pfp_wvalid & framebuffer_pfp_wready),
        .valueInPipeline(pixelInPipeline)
    );

    ValueTrack vtTriangle (
        .aclk(aclk),
        .resetn(resetn),

        .sigIncomingValue(rasterizer_tpixel & rasterizer_tvalid & rasterizer_tready),
        .sigOutgoingValue(alrp_tpixel & alrp_tvalid & alrp_tready),
        .valueInPipeline(dataInTriangleInterpolator)
    );
    
    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Broadcaster to broadcast the stream from the rasterizer to the attribute
    // interpolator and to all framebuffers
    // Clocks: n
    ////////////////////////////////////////////////////////////////////////////
    wire [(SCREEN_POS_WIDTH * 4) - 1 : 0]   bc_bbx;
    wire [(SCREEN_POS_WIDTH * 4) - 1 : 0]   bc_bby;
    wire [(SCREEN_POS_WIDTH * 4) - 1 : 0]   bc_spx;
    wire [(SCREEN_POS_WIDTH * 4) - 1 : 0]   bc_spy;
    wire [(INDEX_WIDTH * 4) - 1 : 0]        bc_index;
    wire [ 3 : 0]                           bc_valid;
    wire [ 3 : 0]                           bc_pixel;
    wire [(RR_CMD_SIZE * 4) - 1 : 0]        bc_cmd;
    wire [ 3 : 0]                           bc_last;
    wire [ 3 : 0]                           bc_keep;
    wire [ 3 : 0]                           bc_ready;
    assign m_color_araddr     = bc_index[INDEX_WIDTH * 3 +: INDEX_WIDTH];
    assign m_depth_araddr     = bc_index[INDEX_WIDTH * 2 +: INDEX_WIDTH];
    assign m_stencil_araddr   = bc_index[INDEX_WIDTH * 1 +: INDEX_WIDTH];
    assign m_color_arvalid    = bc_valid[3] & colorBufferEnable & bc_pixel[3];
    assign m_depth_arvalid    = bc_valid[2] & depthBufferEnable & bc_pixel[2];
    assign m_stencil_arvalid  = bc_valid[1] & stencilBufferEnable & bc_pixel[1];
    assign m_color_arlast     = bc_last[3];
    assign m_depth_arlast     = bc_last[2];
    assign m_stencil_arlast   = bc_last[1];
    assign bc_ready[3] = m_color_arready | !colorBufferEnable;
    assign bc_ready[2] = m_depth_arready | !depthBufferEnable;
    assign bc_ready[1] = m_stencil_arready | !stencilBufferEnable;

    axis_broadcast rasterizerBroadcast (
        .clk(aclk),
        .rst(!resetn),

        .s_axis_tdata({
            rasterizer_tbbx,
            rasterizer_tbby,
            rasterizer_tspx,
            rasterizer_tspy,
            rasterizer_tindex,
            rasterizer_tpixel,
            rasterizer_tcmd
        }),
        .s_axis_tlast(rasterizer_tlast),
        .s_axis_tvalid(rasterizer_tvalid),
        .s_axis_tready(rasterizer_tready),
        .s_axis_tkeep(rasterizer_tkeep),
        .s_axis_tid(),
        .s_axis_tdest(),
        .s_axis_tuser(),

        .m_axis_tdata({
            bc_bbx[3 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_bby[3 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_spx[3 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_spy[3 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_index[3 * INDEX_WIDTH +: INDEX_WIDTH],
            bc_pixel[3],
            bc_cmd[3 * RR_CMD_SIZE +: RR_CMD_SIZE],
            bc_bbx[2 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_bby[2 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_spx[2 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_spy[2 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_index[2 * INDEX_WIDTH +: INDEX_WIDTH],
            bc_pixel[2],
            bc_cmd[2 * RR_CMD_SIZE +: RR_CMD_SIZE],
            bc_bbx[1 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_bby[1 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_spx[1 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_spy[1 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_index[1 * INDEX_WIDTH +: INDEX_WIDTH],
            bc_pixel[1],
            bc_cmd[1 * RR_CMD_SIZE +: RR_CMD_SIZE],
            bc_bbx[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_bby[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_spx[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_spy[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_index[0 * INDEX_WIDTH +: INDEX_WIDTH],
            bc_pixel[0],
            bc_cmd[0 * RR_CMD_SIZE +: RR_CMD_SIZE]
        }),
        .m_axis_tvalid(bc_valid),
        .m_axis_tready(bc_ready),
        .m_axis_tlast(bc_last),
        .m_axis_tkeep(bc_keep),
        .m_axis_tid(),
        .m_axis_tdest(),
        .m_axis_tuser()
    );
    defparam rasterizerBroadcast.M_COUNT = 4;
    defparam rasterizerBroadcast.DATA_WIDTH = (4 * SCREEN_POS_WIDTH) + INDEX_WIDTH + 1 + RR_CMD_SIZE;
    defparam rasterizerBroadcast.KEEP_ENABLE = 1;
    defparam rasterizerBroadcast.KEEP_WIDTH = 1;
    defparam rasterizerBroadcast.LAST_ENABLE = 1;
    defparam rasterizerBroadcast.ID_ENABLE = 0;
    defparam rasterizerBroadcast.DEST_ENABLE = 0;
    defparam rasterizerBroadcast.USER_ENABLE = 0;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 3
    // Interpolation of attributes
    // Clocks: 33
    ////////////////////////////////////////////////////////////////////////////
    wire                                    alrp_tready;
    wire                                    alrp_tvalid;
    wire                                    alrp_tpixel;
    wire                                    alrp_tlast;
    wire                                    alrp_tkeep;
    wire [SCREEN_POS_WIDTH - 1 : 0]         alrp_tspx;
    wire [SCREEN_POS_WIDTH - 1 : 0]         alrp_tspy;
    wire [INDEX_WIDTH - 1 : 0]              alrp_tindex;
    wire [ATTRIBUTE_SIZE - 1 : 0]           alrp_tdepth_w;
    wire [32 - 1 : 0]                       alrp_tdepth_z;
    wire [32 - 1 : 0]                       alrp_ttexture0_t;
    wire [32 - 1 : 0]                       alrp_ttexture0_s;
    wire [32 - 1 : 0]                       alrp_tmipmap0_t;
    wire [32 - 1 : 0]                       alrp_tmipmap0_s;
    wire [32 - 1 : 0]                       alrp_ttexture1_t;
    wire [32 - 1 : 0]                       alrp_ttexture1_s;
    wire [32 - 1 : 0]                       alrp_tmipmap1_t;
    wire [32 - 1 : 0]                       alrp_tmipmap1_s;
    wire [COLOR_SUB_PIXEL_WIDTH - 1 : 0]    alrp_tcolor_a;
    wire [COLOR_SUB_PIXEL_WIDTH - 1 : 0]    alrp_tcolor_b;
    wire [COLOR_SUB_PIXEL_WIDTH - 1 : 0]    alrp_tcolor_g;
    wire [COLOR_SUB_PIXEL_WIDTH - 1 : 0]    alrp_tcolor_r;

    generate
        if (!RASTERIZER_ENABLE_FLOAT_INTERPOLATION)
        begin
            AttributeInterpolatorX attributeInterpolator (
                .aclk(aclk),
                .resetn(resetn),

                .s_attrb_tready(bc_ready[0]),
                .s_attrb_tvalid(bc_valid[0]),
                .s_attrb_tlast(bc_last[0]),
                .s_attrb_tkeep(bc_keep[0]),
                .s_attrb_tspx(bc_spx[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH]),
                .s_attrb_tspy(bc_spy[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH]),
                .s_attrb_tindex(bc_index[0 * INDEX_WIDTH +: INDEX_WIDTH]),
                .s_attrb_tpixel(bc_pixel[0]),
                .s_attrb_tcmd(bc_cmd[0 +: RR_CMD_SIZE]),

                .tex0_s(triangleParams[TRIANGLE_STREAM_INC_TEX0_S * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex0_t(triangleParams[TRIANGLE_STREAM_INC_TEX0_T * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex0_q(triangleParams[TRIANGLE_STREAM_INC_TEX0_Q * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex0_s_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX0_S_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex0_t_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX0_T_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex0_q_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX0_Q_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex0_s_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX0_S_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex0_t_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX0_T_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex0_q_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX0_Q_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_s(triangleParams[TRIANGLE_STREAM_INC_TEX1_S * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_t(triangleParams[TRIANGLE_STREAM_INC_TEX1_T * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_q(triangleParams[TRIANGLE_STREAM_INC_TEX1_Q * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_s_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX1_S_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_t_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX1_T_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_q_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX1_Q_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_s_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX1_S_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_t_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX1_T_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_q_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX1_Q_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .depth_w(triangleParams[TRIANGLE_STREAM_INC_DEPTH_W * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .depth_w_inc_x(triangleParams[TRIANGLE_STREAM_INC_DEPTH_W_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .depth_w_inc_y(triangleParams[TRIANGLE_STREAM_INC_DEPTH_W_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .depth_z(triangleParams[TRIANGLE_STREAM_INC_DEPTH_Z * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .depth_z_inc_x(triangleParams[TRIANGLE_STREAM_INC_DEPTH_Z_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .depth_z_inc_y(triangleParams[TRIANGLE_STREAM_INC_DEPTH_Z_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_r(triangleParams[TRIANGLE_STREAM_INC_COLOR_R * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_r_inc_x(triangleParams[TRIANGLE_STREAM_INC_COLOR_R_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_r_inc_y(triangleParams[TRIANGLE_STREAM_INC_COLOR_R_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_g(triangleParams[TRIANGLE_STREAM_INC_COLOR_G * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_g_inc_x(triangleParams[TRIANGLE_STREAM_INC_COLOR_G_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_g_inc_y(triangleParams[TRIANGLE_STREAM_INC_COLOR_G_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_b(triangleParams[TRIANGLE_STREAM_INC_COLOR_B * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_b_inc_x(triangleParams[TRIANGLE_STREAM_INC_COLOR_B_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_b_inc_y(triangleParams[TRIANGLE_STREAM_INC_COLOR_B_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_a(triangleParams[TRIANGLE_STREAM_INC_COLOR_A * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_a_inc_x(triangleParams[TRIANGLE_STREAM_INC_COLOR_A_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_a_inc_y(triangleParams[TRIANGLE_STREAM_INC_COLOR_A_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),

                .m_attrb_tready(alrp_tready),
                .m_attrb_tvalid(alrp_tvalid),
                .m_attrb_tpixel(alrp_tpixel),
                .m_attrb_tlast(alrp_tlast),
                .m_attrb_tkeep(alrp_tkeep),
                .m_attrb_tspx(alrp_tspx),
                .m_attrb_tspy(alrp_tspy),
                .m_attrb_tindex(alrp_tindex),
                .m_attrb_tdepth_w(alrp_tdepth_w),
                .m_attrb_tdepth_z(alrp_tdepth_z),
                .m_attrb_ttexture0_t(alrp_ttexture0_t),
                .m_attrb_ttexture0_s(alrp_ttexture0_s),
                .m_attrb_tmipmap0_t(alrp_tmipmap0_t),
                .m_attrb_tmipmap0_s(alrp_tmipmap0_s),
                .m_attrb_ttexture1_t(alrp_ttexture1_t),
                .m_attrb_ttexture1_s(alrp_ttexture1_s),
                .m_attrb_tmipmap1_t(alrp_tmipmap1_t),
                .m_attrb_tmipmap1_s(alrp_tmipmap1_s),
                .m_attrb_tcolor_a(alrp_tcolor_a),
                .m_attrb_tcolor_b(alrp_tcolor_b),
                .m_attrb_tcolor_g(alrp_tcolor_g),
                .m_attrb_tcolor_r(alrp_tcolor_r)
            );
            defparam attributeInterpolator.INDEX_WIDTH = INDEX_WIDTH;
            defparam attributeInterpolator.SCREEN_POS_WIDTH = SCREEN_POS_WIDTH;
            defparam attributeInterpolator.ENABLE_LOD_CALC = ENABLE_MIPMAPPING;
            defparam attributeInterpolator.ENABLE_SECOND_TMU = ENABLE_SECOND_TMU;
            defparam attributeInterpolator.CALC_PRECISION = RASTERIZER_FIXPOINT_PRECISION;
        end
        else
        begin
            wire                            ftx_tready;
            wire                            ftx_tvalid;
            wire                            ftx_tpixel;
            wire                            ftx_tlast;
            wire                            ftx_tkeep;
            wire [SCREEN_POS_WIDTH - 1 : 0] ftx_tspx;
            wire [SCREEN_POS_WIDTH - 1 : 0] ftx_tspy;
            wire [INDEX_WIDTH - 1 : 0]      ftx_tindex;
            wire [ATTRIBUTE_SIZE - 1 : 0]   ftx_tdepth_w;
            wire [ATTRIBUTE_SIZE - 1 : 0]   ftx_tdepth_z;
            wire [ATTRIBUTE_SIZE - 1 : 0]   ftx_ttexture0_t;
            wire [ATTRIBUTE_SIZE - 1 : 0]   ftx_ttexture0_s;
            wire [ATTRIBUTE_SIZE - 1 : 0]   ftx_tmipmap0_t;
            wire [ATTRIBUTE_SIZE - 1 : 0]   ftx_tmipmap0_s;
            wire [ATTRIBUTE_SIZE - 1 : 0]   ftx_ttexture1_t;
            wire [ATTRIBUTE_SIZE - 1 : 0]   ftx_ttexture1_s;
            wire [ATTRIBUTE_SIZE - 1 : 0]   ftx_tmipmap1_t;
            wire [ATTRIBUTE_SIZE - 1 : 0]   ftx_tmipmap1_s;
            wire [ATTRIBUTE_SIZE - 1 : 0]   ftx_tcolor_a;
            wire [ATTRIBUTE_SIZE - 1 : 0]   ftx_tcolor_b;
            wire [ATTRIBUTE_SIZE - 1 : 0]   ftx_tcolor_g;
            wire [ATTRIBUTE_SIZE - 1 : 0]   ftx_tcolor_r;

            AttributeInterpolator attributeInterpolator (
                .aclk(aclk),
                .resetn(resetn),

                .s_attrb_tready(bc_ready[0]),
                .s_attrb_tvalid(bc_valid[0]),
                .s_attrb_tpixel(bc_pixel[0]),
                .s_attrb_tlast(bc_last[0]),
                .s_attrb_tkeep(bc_keep[0]),
                .s_attrb_tbbx(bc_bbx[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH]),
                .s_attrb_tbby(bc_bby[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH]),
                .s_attrb_tspx(bc_spx[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH]),
                .s_attrb_tspy(bc_spy[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH]),
                .s_attrb_tindex(bc_index[0 * INDEX_WIDTH +: INDEX_WIDTH]),

                .tex0_s(triangleParams[TRIANGLE_STREAM_INC_TEX0_S * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex0_t(triangleParams[TRIANGLE_STREAM_INC_TEX0_T * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex0_q(triangleParams[TRIANGLE_STREAM_INC_TEX0_Q * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex0_s_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX0_S_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex0_t_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX0_T_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex0_q_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX0_Q_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex0_s_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX0_S_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex0_t_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX0_T_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex0_q_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX0_Q_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_s(triangleParams[TRIANGLE_STREAM_INC_TEX1_S * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_t(triangleParams[TRIANGLE_STREAM_INC_TEX1_T * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_q(triangleParams[TRIANGLE_STREAM_INC_TEX1_Q * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_s_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX1_S_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_t_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX1_T_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_q_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX1_Q_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_s_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX1_S_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_t_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX1_T_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .tex1_q_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX1_Q_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .depth_w(triangleParams[TRIANGLE_STREAM_INC_DEPTH_W * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .depth_w_inc_x(triangleParams[TRIANGLE_STREAM_INC_DEPTH_W_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .depth_w_inc_y(triangleParams[TRIANGLE_STREAM_INC_DEPTH_W_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .depth_z(triangleParams[TRIANGLE_STREAM_INC_DEPTH_Z * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .depth_z_inc_x(triangleParams[TRIANGLE_STREAM_INC_DEPTH_Z_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .depth_z_inc_y(triangleParams[TRIANGLE_STREAM_INC_DEPTH_Z_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_r(triangleParams[TRIANGLE_STREAM_INC_COLOR_R * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_r_inc_x(triangleParams[TRIANGLE_STREAM_INC_COLOR_R_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_r_inc_y(triangleParams[TRIANGLE_STREAM_INC_COLOR_R_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_g(triangleParams[TRIANGLE_STREAM_INC_COLOR_G * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_g_inc_x(triangleParams[TRIANGLE_STREAM_INC_COLOR_G_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_g_inc_y(triangleParams[TRIANGLE_STREAM_INC_COLOR_G_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_b(triangleParams[TRIANGLE_STREAM_INC_COLOR_B * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_b_inc_x(triangleParams[TRIANGLE_STREAM_INC_COLOR_B_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_b_inc_y(triangleParams[TRIANGLE_STREAM_INC_COLOR_B_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_a(triangleParams[TRIANGLE_STREAM_INC_COLOR_A * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_a_inc_x(triangleParams[TRIANGLE_STREAM_INC_COLOR_A_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
                .color_a_inc_y(triangleParams[TRIANGLE_STREAM_INC_COLOR_A_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),

                .m_attrb_tready(ftx_tready),
                .m_attrb_tvalid(ftx_tvalid),
                .m_attrb_tpixel(ftx_tpixel),
                .m_attrb_tlast(ftx_tlast),
                .m_attrb_tkeep(ftx_tkeep),
                .m_attrb_tspx(ftx_tspx),
                .m_attrb_tspy(ftx_tspy),
                .m_attrb_tindex(ftx_tindex),
                .m_attrb_tdepth_w(ftx_tdepth_w),
                .m_attrb_tdepth_z(ftx_tdepth_z),
                .m_attrb_ttexture0_t(ftx_ttexture0_t),
                .m_attrb_ttexture0_s(ftx_ttexture0_s),
                .m_attrb_tmipmap0_t(ftx_tmipmap0_t),
                .m_attrb_tmipmap0_s(ftx_tmipmap0_s),
                .m_attrb_ttexture1_t(ftx_ttexture1_t),
                .m_attrb_ttexture1_s(ftx_ttexture1_s),
                .m_attrb_tmipmap1_t(ftx_tmipmap1_t),
                .m_attrb_tmipmap1_s(ftx_tmipmap1_s),
                .m_attrb_tcolor_a(ftx_tcolor_a),
                .m_attrb_tcolor_b(ftx_tcolor_b),
                .m_attrb_tcolor_g(ftx_tcolor_g),
                .m_attrb_tcolor_r(ftx_tcolor_r)
            );
            defparam attributeInterpolator.RASTERIZER_FLOAT_PRECISION = RASTERIZER_FLOAT_PRECISION;
            defparam attributeInterpolator.INDEX_WIDTH = INDEX_WIDTH;
            defparam attributeInterpolator.SCREEN_POS_WIDTH = SCREEN_POS_WIDTH;
            defparam attributeInterpolator.ENABLE_LOD_CALC = ENABLE_MIPMAPPING;
            defparam attributeInterpolator.ENABLE_SECOND_TMU = ENABLE_SECOND_TMU;

            AttributeF2XConverter af2x (
                .aclk(aclk),
                .resetn(resetn),

                .s_ftx_tready(ftx_tready),
                .s_ftx_tvalid(ftx_tvalid),
                .s_ftx_tpixel(ftx_tpixel),
                .s_ftx_tlast(ftx_tlast),
                .s_ftx_tkeep(ftx_tkeep),
                .s_ftx_tspx(ftx_tspx),
                .s_ftx_tspy(ftx_tspy),
                .s_ftx_tindex(ftx_tindex),
                .s_ftx_tdepth_w(ftx_tdepth_w),
                .s_ftx_tdepth_z(ftx_tdepth_z),
                .s_ftx_ttexture0_t(ftx_ttexture0_t),
                .s_ftx_ttexture0_s(ftx_ttexture0_s),
                .s_ftx_tmipmap0_t(ftx_tmipmap0_t),
                .s_ftx_tmipmap0_s(ftx_tmipmap0_s),
                .s_ftx_ttexture1_t(ftx_ttexture1_t),
                .s_ftx_ttexture1_s(ftx_ttexture1_s),
                .s_ftx_tmipmap1_t(ftx_tmipmap1_t),
                .s_ftx_tmipmap1_s(ftx_tmipmap1_s),
                .s_ftx_tcolor_a(ftx_tcolor_a),
                .s_ftx_tcolor_b(ftx_tcolor_b),
                .s_ftx_tcolor_g(ftx_tcolor_g),
                .s_ftx_tcolor_r(ftx_tcolor_r),

                .m_ftx_tready(alrp_tready),
                .m_ftx_tvalid(alrp_tvalid),
                .m_ftx_tpixel(alrp_tpixel),
                .m_ftx_tlast(alrp_tlast),
                .m_ftx_tkeep(alrp_tkeep),
                .m_ftx_tspx(alrp_tspx),
                .m_ftx_tspy(alrp_tspy),
                .m_ftx_tindex(alrp_tindex),
                .m_ftx_tdepth_w(alrp_tdepth_w),
                .m_ftx_tdepth_z(alrp_tdepth_z),
                .m_ftx_ttexture0_t(alrp_ttexture0_t),
                .m_ftx_ttexture0_s(alrp_ttexture0_s),
                .m_ftx_tmipmap0_t(alrp_tmipmap0_t),
                .m_ftx_tmipmap0_s(alrp_tmipmap0_s),
                .m_ftx_ttexture1_t(alrp_ttexture1_t),
                .m_ftx_ttexture1_s(alrp_ttexture1_s),
                .m_ftx_tmipmap1_t(alrp_tmipmap1_t),
                .m_ftx_tmipmap1_s(alrp_tmipmap1_s),
                .m_ftx_tcolor_a(alrp_tcolor_a),
                .m_ftx_tcolor_b(alrp_tcolor_b),
                .m_ftx_tcolor_g(alrp_tcolor_g),
                .m_ftx_tcolor_r(alrp_tcolor_r)
            );
            defparam af2x.INDEX_WIDTH = INDEX_WIDTH;
            defparam af2x.SCREEN_POS_WIDTH = SCREEN_POS_WIDTH;
            defparam af2x.ENABLE_LOD_CALC = ENABLE_MIPMAPPING;
            defparam af2x.ENABLE_SECOND_TMU = ENABLE_SECOND_TMU;
            defparam af2x.SUB_PIXEL_WIDTH = COLOR_SUB_PIXEL_WIDTH;
            defparam af2x.DEPTH_WIDTH = DEPTH_WIDTH;
        end
    endgenerate

    ////////////////////////////////////////////////////////////////////////////
    // STEP 4
    // Texturing triangle, fogging
    // Clocks: 30
    ////////////////////////////////////////////////////////////////////////////
    wire                                        framebuffer_tready;
    wire [(COLOR_SUB_PIXEL_WIDTH * 4) - 1 : 0]  framebuffer_tfragmentColor;
    wire [INDEX_WIDTH - 1 : 0]                  framebuffer_tindex;
    wire [SCREEN_POS_WIDTH - 1 : 0]             framebuffer_tscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]             framebuffer_tscreenPosY;
    wire [31 : 0]                               framebuffer_tdepth;
    wire                                        framebuffer_tvalid;
    wire                                        framebuffer_tlast;
    wire                                        framebuffer_tkeep;

    PixelPipeline pixelPipeline (    
        .aclk(aclk),
        .resetn(resetn),

        .s_fog_lut_axis_tvalid(cmd_fog_axis_tvalid),
        .s_fog_lut_axis_tready(cmd_fog_axis_tready),
        .s_fog_lut_axis_tlast(cmd_xxx_axis_tlast),
        .s_fog_lut_axis_tdata(cmd_xxx_axis_tdata[0 +: 32]),

        .confFeatureEnable(confFeatureEnable),
        .confFragmentPipelineConfig(confFragmentPipelineConfig),
        .confFragmentPipelineFogColor(confFragmentPipelineFogColor),
        .confStencilBufferConfig(confStencilBufferConfig),
        .confTMU0TexEnvConfig(confTMU0TexEnvConfig),
        .confTMU0TextureConfig(confTMU0TextureConfig),
        .confTMU0TexEnvColor(confTMU0TexEnvColor),
        .confTMU1TexEnvConfig(confTMU1TexEnvConfig),
        .confTMU1TextureConfig(confTMU1TextureConfig),
        .confTMU1TexEnvColor(confTMU1TexEnvColor),

        .s_attrb_tready(alrp_tready),
        // Memory requests are only executed for valid pixels.
        // This pipeline does not need to be executed, when no pixels are available.
        // Reduces the number of texture lookups.
        .s_attrb_tvalid(alrp_tvalid & alrp_tpixel),
        .s_attrb_tlast(alrp_tlast),
        .s_attrb_tkeep(alrp_tkeep),
        .s_attrb_tspx(alrp_tspx),
        .s_attrb_tspy(alrp_tspy),
        .s_attrb_tindex(alrp_tindex),
        .s_attrb_tdepth_w(alrp_tdepth_w),
        .s_attrb_tdepth_z(alrp_tdepth_z),
        .s_attrb_ttexture0_t(alrp_ttexture0_t),
        .s_attrb_ttexture0_s(alrp_ttexture0_s),
        .s_attrb_tmipmap0_t(alrp_tmipmap0_t),
        .s_attrb_tmipmap0_s(alrp_tmipmap0_s),
        .s_attrb_ttexture1_t(alrp_ttexture1_t),
        .s_attrb_ttexture1_s(alrp_ttexture1_s),
        .s_attrb_tmipmap1_t(alrp_tmipmap1_t),
        .s_attrb_tmipmap1_s(alrp_tmipmap1_s),
        .s_attrb_tcolor_a(alrp_tcolor_a),
        .s_attrb_tcolor_b(alrp_tcolor_b),
        .s_attrb_tcolor_g(alrp_tcolor_g),
        .s_attrb_tcolor_r(alrp_tcolor_r),

        .texel0Addr00(texel0Addr00),
        .texel0Addr01(texel0Addr01),
        .texel0Addr10(texel0Addr10),
        .texel0Addr11(texel0Addr11),

        .texel0Input00(texel0Input00),
        .texel0Input01(texel0Input01),
        .texel0Input10(texel0Input10),
        .texel0Input11(texel0Input11),

        .texel1Addr00(texel1Addr00),
        .texel1Addr01(texel1Addr01),
        .texel1Addr10(texel1Addr10),
        .texel1Addr11(texel1Addr11),

        .texel1Input00(texel1Input00),
        .texel1Input01(texel1Input01),
        .texel1Input10(texel1Input10),
        .texel1Input11(texel1Input11),

        .m_frag_tready(framebuffer_tready),
        .m_frag_tvalid(framebuffer_tvalid),
        .m_frag_tlast(framebuffer_tlast),
        .m_frag_tkeep(framebuffer_tkeep),
        .m_frag_tfragmentColor(framebuffer_tfragmentColor),
        .m_frag_tdepth(framebuffer_tdepth),
        .m_frag_tindex(framebuffer_tindex),
        .m_frag_tscreenPosX(framebuffer_tscreenPosX),
        .m_frag_tscreenPosY(framebuffer_tscreenPosY)
    );
    defparam pixelPipeline.INDEX_WIDTH = INDEX_WIDTH;
    defparam pixelPipeline.SUB_PIXEL_WIDTH = COLOR_SUB_PIXEL_WIDTH;
    defparam pixelPipeline.SUB_PIXEL_CALC_PRECISION = SUB_PIXEL_CALC_PRECISION;
    defparam pixelPipeline.ENABLE_SECOND_TMU = ENABLE_SECOND_TMU;
    defparam pixelPipeline.SCREEN_POS_WIDTH = SCREEN_POS_WIDTH;
    defparam pixelPipeline.ENABLE_LOD_CALC = ENABLE_MIPMAPPING;
    defparam pixelPipeline.ENABLE_TEXTURE_FILTERING = ENABLE_TEXTURE_FILTERING;
    defparam pixelPipeline.ENABLE_FOG = ENABLE_FOG;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 5
    // Concatinates the stream from the pixel pipeline and from the framebuffers
    // Clocks: n/a
    ////////////////////////////////////////////////////////////////////////////
    localparam FRAGMENT_STREAM_WIDTH = (COLOR_SUB_PIXEL_WIDTH * 4) + 32 + INDEX_WIDTH + (SCREEN_POS_WIDTH * 2) + 1 + 1;

    wire fragment_stream_out_tvalid;
    wire fragment_stream_out_tready;
    wire [(FRAGMENT_STREAM_WIDTH + PIXEL_WIDTH + DEPTH_WIDTH + STENCIL_WIDTH) - 1 : 0] fragment_stream_out_tdata;

    StreamConcatFifo streamConcatFifo (
        .aclk(aclk),
        .resetn(resetn),

        .s_stream0_tenable(1'b1),
        .s_stream0_tvalid(framebuffer_tvalid),
        .s_stream0_tdata({ 
            framebuffer_tkeep,
            framebuffer_tlast,
            framebuffer_tscreenPosY,
            framebuffer_tscreenPosX,
            framebuffer_tindex,
            framebuffer_tdepth, 
            framebuffer_tfragmentColor 
        }),
        .s_stream0_tready(framebuffer_tready),

        .s_stream1_tenable(colorBufferEnable),
        .s_stream1_tvalid(m_color_rvalid),
        .s_stream1_tdata(m_color_rdata),
        .s_stream1_tready(m_color_rready),

        .s_stream2_tenable(depthBufferEnable),
        .s_stream2_tvalid(m_depth_rvalid),
        .s_stream2_tdata(m_depth_rdata),
        .s_stream2_tready(m_depth_rready),

        .s_stream3_tenable(stencilBufferEnable),
        .s_stream3_tvalid(m_stencil_rvalid),
        .s_stream3_tdata(m_stencil_rdata),
        .s_stream3_tready(m_stencil_rready),

        .m_stream_tvalid(fragment_stream_out_tvalid),
        .m_stream_tdata(fragment_stream_out_tdata),
        .m_stream_tready(fragment_stream_out_tready)
    );
    defparam streamConcatFifo.FIFO_DEPTH0_POW2 = (ENABLE_READ_FIFO == 1) ? READ_FIFO_SIZE : 1;
    defparam streamConcatFifo.FIFO_DEPTH1_POW2 = (ENABLE_READ_FIFO == 1) ? READ_FIFO_SIZE : 1;
    defparam streamConcatFifo.FIFO_DEPTH2_POW2 = (ENABLE_READ_FIFO == 1) ? READ_FIFO_SIZE : 1;
    defparam streamConcatFifo.FIFO_DEPTH3_POW2 = (ENABLE_READ_FIFO == 1) ? READ_FIFO_SIZE : 1;
    defparam streamConcatFifo.STREAM0_WIDTH = FRAGMENT_STREAM_WIDTH;
    defparam streamConcatFifo.STREAM1_WIDTH = PIXEL_WIDTH;
    defparam streamConcatFifo.STREAM2_WIDTH = DEPTH_WIDTH;
    defparam streamConcatFifo.STREAM3_WIDTH = STENCIL_WIDTH;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 6
    // Access framebuffer, blend, test and save pixel in framebuffer
    // Clocks: 4
    ////////////////////////////////////////////////////////////////////////////
    wire [INDEX_WIDTH - 1 : 0]      framebuffer_pfp_waddr;
    wire                            framebuffer_pfp_wvalid;
    wire                            framebuffer_pfp_wlast;
    wire                            framebuffer_pfp_wready;
    wire [SCREEN_POS_WIDTH - 1 : 0] framebuffer_pfp_wscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0] framebuffer_pfp_wscreenPosY;

    wire [PIXEL_WIDTH - 1 : 0]      framebuffer_color_pfp_wdata;
    wire                            framebuffer_color_pfp_wstrb;

    wire [DEPTH_WIDTH - 1 : 0]      framebuffer_depth_pfp_wdata;
    wire                            framebuffer_depth_pfp_wstrb;

    wire [STENCIL_WIDTH - 1 : 0]    framebuffer_stencil_pfp_wdata;
    wire                            framebuffer_stencil_pfp_wstrb;

    PerFragmentPipeline perFragmentPipeline (
        .aclk(aclk),
        .resetn(resetn),

        .conf(confFragmentPipelineConfig),
        .confFeatureEnable(confFeatureEnable),
        .confStencilBufferConfig(confStencilBufferConfig),

        .s_frag_tready(fragment_stream_out_tready),
        .s_frag_tlast(fragment_stream_out_tdata[(COLOR_SUB_PIXEL_WIDTH * 4) + 32 + INDEX_WIDTH + (SCREEN_POS_WIDTH * 2) +: 1]),
        .s_frag_tkeep(fragment_stream_out_tdata[(COLOR_SUB_PIXEL_WIDTH * 4) + 32 + INDEX_WIDTH + (SCREEN_POS_WIDTH * 2) + 1 +: 1]),
        .s_frag_tvalid(fragment_stream_out_tvalid),
        .s_frag_tcolor(fragment_stream_out_tdata[0 +: (COLOR_SUB_PIXEL_WIDTH * 4)]),
        .s_frag_tdepth(fragment_stream_out_tdata[(COLOR_SUB_PIXEL_WIDTH * 4) +: 32]),
        .s_frag_tindex(fragment_stream_out_tdata[(COLOR_SUB_PIXEL_WIDTH * 4) + 32 +: INDEX_WIDTH]),
        .s_frag_tscreenPosX(fragment_stream_out_tdata[(COLOR_SUB_PIXEL_WIDTH * 4) + 32 + INDEX_WIDTH +: SCREEN_POS_WIDTH]),
        .s_frag_tscreenPosY(fragment_stream_out_tdata[(COLOR_SUB_PIXEL_WIDTH * 4) + 32 + INDEX_WIDTH + SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH]),
        .s_frag_color_tdata(fragment_stream_out_tdata[FRAGMENT_STREAM_WIDTH +: PIXEL_WIDTH]),
        .s_frag_depth_tdata(fragment_stream_out_tdata[FRAGMENT_STREAM_WIDTH + PIXEL_WIDTH +: DEPTH_WIDTH]),
        .s_frag_stencil_tdata(fragment_stream_out_tdata[FRAGMENT_STREAM_WIDTH + PIXEL_WIDTH + DEPTH_WIDTH +: STENCIL_WIDTH]),

        .fragmentProcessed(fragmentProcessed),

        .m_frag_tready(framebuffer_pfp_wready),
        .m_frag_taddr(framebuffer_pfp_waddr),
        .m_frag_tvalid(framebuffer_pfp_wvalid),
        .m_frag_tlast(framebuffer_pfp_wlast),
        .m_frag_tscreenPosX(framebuffer_pfp_wscreenPosX),
        .m_frag_tscreenPosY(framebuffer_pfp_wscreenPosY),
        
        .m_frag_color_tdata(framebuffer_color_pfp_wdata),
        .m_frag_color_tstrb(framebuffer_color_pfp_wstrb),

        .m_frag_depth_tdata(framebuffer_depth_pfp_wdata),
        .m_frag_depth_tstrb(framebuffer_depth_pfp_wstrb),

        .m_frag_stencil_tdata(framebuffer_stencil_pfp_wdata),
        .m_frag_stencil_tstrb(framebuffer_stencil_pfp_wstrb)
    );
    defparam perFragmentPipeline.FRAMEBUFFER_INDEX_WIDTH = INDEX_WIDTH;
    defparam perFragmentPipeline.SCREEN_POS_WIDTH = SCREEN_POS_WIDTH;
    defparam perFragmentPipeline.DEPTH_WIDTH = DEPTH_WIDTH;
    defparam perFragmentPipeline.STENCIL_WIDTH = STENCIL_WIDTH;
    defparam perFragmentPipeline.SUB_PIXEL_WIDTH = COLOR_SUB_PIXEL_WIDTH;
    defparam perFragmentPipeline.SUB_PIXEL_CALC_PRECISION = SUB_PIXEL_CALC_PRECISION;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 7
    // Broadcast fragment stream to output fifos
    // Clocks: 1
    ////////////////////////////////////////////////////////////////////////////
    wire [(INDEX_WIDTH * 3) - 1 : 0]        framebuffer_bc_waddr;
    wire [ 2 : 0]                           framebuffer_bc_wvalid;
    wire [ 2 : 0]                           framebuffer_bc_wlast;
    wire [ 2 : 0]                           framebuffer_bc_wfull; // it's a !wready signal
    wire [(SCREEN_POS_WIDTH * 3) - 1 : 0]   framebuffer_bc_wscreenPosX;
    wire [(SCREEN_POS_WIDTH * 3) - 1 : 0]   framebuffer_bc_wscreenPosY;

    wire [(PIXEL_WIDTH * 3) - 1 : 0]        framebuffer_color_bc_wdata;
    wire  [ 2 : 0]                          framebuffer_color_bc_wstrb;

    wire [(DEPTH_WIDTH * 3) - 1 : 0]        framebuffer_depth_bc_wdata;
    wire [ 2 : 0]                           framebuffer_depth_bc_wstrb;

    wire [(STENCIL_WIDTH * 3) - 1 : 0]      framebuffer_stencil_bc_wdata;
    wire [ 2 : 0]                           framebuffer_stencil_bc_wstrb;

    axis_broadcast fragmentBroadcast (
        .clk(aclk),
        .rst(!resetn),

        .s_axis_tdata({
            framebuffer_pfp_waddr,
            framebuffer_pfp_wscreenPosX,
            framebuffer_pfp_wscreenPosY,
            framebuffer_color_pfp_wdata,
            framebuffer_color_pfp_wstrb,
            framebuffer_depth_pfp_wdata,
            framebuffer_depth_pfp_wstrb,
            framebuffer_stencil_pfp_wdata,
            framebuffer_stencil_pfp_wstrb
        }),
        .s_axis_tlast(framebuffer_pfp_wlast),
        .s_axis_tvalid(framebuffer_pfp_wvalid),
        .s_axis_tready(framebuffer_pfp_wready), 
        .s_axis_tkeep(~0),
        .s_axis_tid(0),
        .s_axis_tdest(0),
        .s_axis_tuser(0),

        .m_axis_tdata({
            framebuffer_bc_waddr[2 * INDEX_WIDTH +: INDEX_WIDTH],
            framebuffer_bc_wscreenPosX[2 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            framebuffer_bc_wscreenPosY[2 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            framebuffer_color_bc_wdata[2 * PIXEL_WIDTH +: PIXEL_WIDTH],
            framebuffer_color_bc_wstrb[2],
            framebuffer_depth_bc_wdata[2 * DEPTH_WIDTH +: DEPTH_WIDTH],
            framebuffer_depth_bc_wstrb[2],
            framebuffer_stencil_bc_wdata[2 * STENCIL_WIDTH +: STENCIL_WIDTH],
            framebuffer_stencil_bc_wstrb[2],
            framebuffer_bc_waddr[1 * INDEX_WIDTH +: INDEX_WIDTH],
            framebuffer_bc_wscreenPosX[1 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            framebuffer_bc_wscreenPosY[1 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            framebuffer_color_bc_wdata[1 * PIXEL_WIDTH +: PIXEL_WIDTH],
            framebuffer_color_bc_wstrb[1],
            framebuffer_depth_bc_wdata[1 * DEPTH_WIDTH +: DEPTH_WIDTH],
            framebuffer_depth_bc_wstrb[1],
            framebuffer_stencil_bc_wdata[1 * STENCIL_WIDTH +: STENCIL_WIDTH],
            framebuffer_stencil_bc_wstrb[1],
            framebuffer_bc_waddr[0 * INDEX_WIDTH +: INDEX_WIDTH],
            framebuffer_bc_wscreenPosX[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            framebuffer_bc_wscreenPosY[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            framebuffer_color_bc_wdata[0 * PIXEL_WIDTH +: PIXEL_WIDTH],
            framebuffer_color_bc_wstrb[0],
            framebuffer_depth_bc_wdata[0 * DEPTH_WIDTH +: DEPTH_WIDTH],
            framebuffer_depth_bc_wstrb[0],
            framebuffer_stencil_bc_wdata[0 * STENCIL_WIDTH +: STENCIL_WIDTH],
            framebuffer_stencil_bc_wstrb[0]
        }),
        .m_axis_tvalid(framebuffer_bc_wvalid),
        .m_axis_tready({
            !framebuffer_bc_wfull[2],
            !framebuffer_bc_wfull[1],
            !framebuffer_bc_wfull[0]
        }),
        .m_axis_tlast(framebuffer_bc_wlast),
        .m_axis_tkeep(),
        .m_axis_tid(),
        .m_axis_tdest(),
        .m_axis_tuser()
    );
    defparam fragmentBroadcast.M_COUNT = 3;
    defparam fragmentBroadcast.DATA_WIDTH = INDEX_WIDTH + (2 * SCREEN_POS_WIDTH) + PIXEL_WIDTH + 1 + DEPTH_WIDTH + 1 + STENCIL_WIDTH + 1;
    defparam fragmentBroadcast.KEEP_ENABLE = 0;
    defparam fragmentBroadcast.KEEP_WIDTH = 1;
    defparam fragmentBroadcast.LAST_ENABLE = 1;
    defparam fragmentBroadcast.ID_ENABLE = 0;
    defparam fragmentBroadcast.DEST_ENABLE = 0;
    defparam fragmentBroadcast.USER_ENABLE = 0;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 8
    // FIFOs for the flow control on the write channel
    // Clocks: 1
    ////////////////////////////////////////////////////////////////////////////
    generate
        if (ENABLE_WRITE_FIFO)
        begin
            localparam COLOR_FIFO_WIDTH = 1 + 1 + SCREEN_POS_WIDTH + SCREEN_POS_WIDTH + INDEX_WIDTH + PIXEL_WIDTH;
            sfifo colorWriteFifo (
                .i_clk(aclk),
                .i_reset(!resetn),

                .i_wr(framebuffer_bc_wvalid[2]),
                .i_data({ 
                    framebuffer_bc_waddr[2 * INDEX_WIDTH +: INDEX_WIDTH],
                    framebuffer_bc_wlast[2], 
                    framebuffer_bc_wscreenPosX[2 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH], 
                    framebuffer_bc_wscreenPosY[2 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH], 
                    framebuffer_color_bc_wdata[2 * PIXEL_WIDTH +: PIXEL_WIDTH], 
                    framebuffer_color_bc_wstrb[2]
                }),
                .o_full(framebuffer_bc_wfull[2]),
                .o_fill(),

                .i_rd(m_color_wready),
                .o_data({
                    m_color_waddr,
                    m_color_wlast,
                    m_color_wscreenPosX,
                    m_color_wscreenPosY,
                    m_color_wdata,
                    m_color_wstrb
                }),
                .o_empty(color_fifo_empty)    
            );
            defparam colorWriteFifo.BW = COLOR_FIFO_WIDTH;
            defparam colorWriteFifo.LGFLEN = WRITE_FIFO_SIZE;
            defparam colorWriteFifo.OPT_ASYNC_READ = 0;

            assign m_color_wvalid = !color_fifo_empty;
        end
        else
        begin
            assign m_color_wvalid = framebuffer_bc_wvalid[2];
            assign m_color_waddr = framebuffer_bc_waddr[2 * INDEX_WIDTH +: INDEX_WIDTH];
            assign m_color_wlast = framebuffer_bc_wlast[2];
            assign m_color_wscreenPosX = framebuffer_bc_wscreenPosX[2 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH];
            assign m_color_wscreenPosY = framebuffer_bc_wscreenPosY[2 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH];
            assign m_color_wdata = framebuffer_color_bc_wdata[2 * PIXEL_WIDTH +: PIXEL_WIDTH];
            assign m_color_wstrb = framebuffer_color_bc_wstrb[2];
            assign framebuffer_bc_wfull[2] = !m_color_wready;

            assign color_fifo_empty = 1;
        end
    endgenerate

    generate
        if (ENABLE_WRITE_FIFO)
        begin
            localparam DEPTH_FIFO_WIDTH = 1 + 1 + SCREEN_POS_WIDTH + SCREEN_POS_WIDTH + INDEX_WIDTH + DEPTH_WIDTH;
            sfifo depthWriteFifo (
                .i_clk(aclk),
                .i_reset(!resetn),

                .i_wr(framebuffer_bc_wvalid[1]),
                .i_data({ 
                    framebuffer_bc_waddr[1 * INDEX_WIDTH +: INDEX_WIDTH],
                    framebuffer_bc_wlast[1], 
                    framebuffer_bc_wscreenPosX[1 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH], 
                    framebuffer_bc_wscreenPosY[1 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH], 
                    framebuffer_depth_bc_wdata[1 * DEPTH_WIDTH +: DEPTH_WIDTH], 
                    framebuffer_depth_bc_wstrb[1]
                }),
                .o_full(framebuffer_bc_wfull[1]),
                .o_fill(),

                .i_rd(m_depth_wready),
                .o_data({
                    m_depth_waddr,
                    m_depth_wlast,
                    m_depth_wscreenPosX,
                    m_depth_wscreenPosY,
                    m_depth_wdata,
                    m_depth_wstrb
                }),
                .o_empty(depth_fifo_empty)    
            );
            defparam depthWriteFifo.BW = DEPTH_FIFO_WIDTH;
            defparam depthWriteFifo.LGFLEN = WRITE_FIFO_SIZE;
            defparam depthWriteFifo.OPT_ASYNC_READ = 0;

            assign m_depth_wvalid = !depth_fifo_empty;
        end
        else
        begin
            assign m_depth_wvalid = framebuffer_bc_wvalid[1];
            assign m_depth_waddr = framebuffer_bc_waddr[1 * INDEX_WIDTH +: INDEX_WIDTH];
            assign m_depth_wlast = framebuffer_bc_wlast[1];
            assign m_depth_wscreenPosX = framebuffer_bc_wscreenPosX[1 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH];
            assign m_depth_wscreenPosY = framebuffer_bc_wscreenPosY[1 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH];
            assign m_depth_wdata = framebuffer_depth_bc_wdata[1 * DEPTH_WIDTH +: DEPTH_WIDTH];
            assign m_depth_wstrb = framebuffer_depth_bc_wstrb[1];
            assign framebuffer_bc_wfull[1] = !m_depth_wready;

            assign depth_fifo_empty = 1;
        end
    endgenerate
    
    generate
        if (ENABLE_WRITE_FIFO)
        begin
            localparam STENCIL_FIFO_WIDTH = 1 + 1 + SCREEN_POS_WIDTH + SCREEN_POS_WIDTH + INDEX_WIDTH + STENCIL_WIDTH;
            sfifo stencilWriteFifo (
                .i_clk(aclk),
                .i_reset(!resetn),

                .i_wr(framebuffer_bc_wvalid[0]),
                .i_data({ 
                    framebuffer_bc_waddr[0 * INDEX_WIDTH +: INDEX_WIDTH],
                    framebuffer_bc_wlast[0], 
                    framebuffer_bc_wscreenPosX[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH], 
                    framebuffer_bc_wscreenPosY[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH], 
                    framebuffer_stencil_bc_wdata[0 * STENCIL_WIDTH +: STENCIL_WIDTH], 
                    framebuffer_stencil_bc_wstrb[0]
                }),
                .o_full(framebuffer_bc_wfull[0]),
                .o_fill(),

                .i_rd(m_stencil_wready),
                .o_data({
                    m_stencil_waddr,
                    m_stencil_wlast,
                    m_stencil_wscreenPosX,
                    m_stencil_wscreenPosY,
                    m_stencil_wdata,
                    m_stencil_wstrb
                }),
                .o_empty(stencil_fifo_empty)    
            );
            defparam stencilWriteFifo.BW = STENCIL_FIFO_WIDTH;
            defparam stencilWriteFifo.LGFLEN = WRITE_FIFO_SIZE;
            defparam stencilWriteFifo.OPT_ASYNC_READ = 0;

            assign m_stencil_wvalid = !stencil_fifo_empty;
        end
        else
        begin
            assign m_stencil_wvalid = framebuffer_bc_wvalid[0];
            assign m_stencil_waddr = framebuffer_bc_waddr[0 * INDEX_WIDTH +: INDEX_WIDTH];
            assign m_stencil_wlast = framebuffer_bc_wlast[0];
            assign m_stencil_wscreenPosX = framebuffer_bc_wscreenPosX[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH];
            assign m_stencil_wscreenPosY = framebuffer_bc_wscreenPosY[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH];
            assign m_stencil_wdata = framebuffer_stencil_bc_wdata[0 * STENCIL_WIDTH +: STENCIL_WIDTH];
            assign m_stencil_wstrb = framebuffer_stencil_bc_wstrb[0];
            assign framebuffer_bc_wfull[0] = !m_stencil_wready;

            assign stencil_fifo_empty = 1;
        end
    endgenerate
endmodule