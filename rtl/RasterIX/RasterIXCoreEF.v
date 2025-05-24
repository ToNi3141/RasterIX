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

// Renderer variant which utilizes a framebuffer on the memory. It currently has 
// some limitations:
// - Alpha channel on framebuffer is not supported
// - Write masks on the color components are not supported (its hard to map a RGB565 color onto 16 bits with 2 bit strobe)
//  - Write mask on the color channel itself is supported
// - Write masks on the single bits of the stencil buffer is not supported
//  - Write mask on the whole stencil channel is supported
// - Only RGB565 colors are supported
module RasterIXCoreEF #(
    // The pixel with in the framebuffer
    localparam PIXEL_WIDTH = 16, // Only RGB565 is supported

    // The width of the stencil buffer
    localparam STENCIL_WIDTH = 4,

    // The width of the depth buffer
    localparam DEPTH_WIDTH = 16,

    // The internal calculation width of a sub pixel
    parameter SUB_PIXEL_CALC_PRECISION = 8,

    // This enables the 4 bit stencil buffer
    parameter ENABLE_STENCIL_BUFFER = 1,

    // Enables the depth buffer
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

    // Memory address witdth
    parameter ADDR_WIDTH = 32,
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
    input  wire                             aclk,
    input  wire                             resetn,

    // AXI Stream command interface
    input  wire                             s_cmd_axis_tvalid,
    output wire                             s_cmd_axis_tready,
    input  wire                             s_cmd_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  s_cmd_axis_tdata,

    // Framebuffer output
    // AXI Stream master interface (RGB565)
    output wire                                 swap_fb,
    output wire                                 swap_fb_enable_vsync,
    output wire [ADDR_WIDTH - 1 : 0]            fb_addr,
    output wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]   fb_size,
    input  wire                                 fb_swapped,

    // Color Buffer
    output wire [ID_WIDTH - 1 : 0]          m_color_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]        m_color_axi_awaddr,
    output wire [ 7 : 0]                    m_color_axi_awlen,
    output wire [ 2 : 0]                    m_color_axi_awsize,
    output wire [ 1 : 0]                    m_color_axi_awburst,
    output wire                             m_color_axi_awlock,
    output wire [ 3 : 0]                    m_color_axi_awcache,
    output wire [ 2 : 0]                    m_color_axi_awprot, 
    output wire                             m_color_axi_awvalid,
    input  wire                             m_color_axi_awready,

    output wire [DATA_WIDTH - 1 : 0]        m_color_axi_wdata,
    output wire [STRB_WIDTH - 1 : 0]        m_color_axi_wstrb,
    output wire                             m_color_axi_wlast,
    output wire                             m_color_axi_wvalid,
    input  wire                             m_color_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]          m_color_axi_bid,
    input  wire [ 1 : 0]                    m_color_axi_bresp,
    input  wire                             m_color_axi_bvalid,
    output wire                             m_color_axi_bready,

    output wire [ID_WIDTH - 1 : 0]          m_color_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_color_axi_araddr,
    output wire [ 7 : 0]                    m_color_axi_arlen,
    output wire [ 2 : 0]                    m_color_axi_arsize,
    output wire [ 1 : 0]                    m_color_axi_arburst,
    output wire                             m_color_axi_arlock,
    output wire [ 3 : 0]                    m_color_axi_arcache,
    output wire [ 2 : 0]                    m_color_axi_arprot,
    output wire                             m_color_axi_arvalid,
    input  wire                             m_color_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]          m_color_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]        m_color_axi_rdata,
    input  wire [ 1 : 0]                    m_color_axi_rresp,
    input  wire                             m_color_axi_rlast,
    input  wire                             m_color_axi_rvalid,
    output wire                             m_color_axi_rready,

    // Depth Buffer
    output wire [ID_WIDTH - 1 : 0]          m_depth_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]        m_depth_axi_awaddr,
    output wire [ 7 : 0]                    m_depth_axi_awlen,
    output wire [ 2 : 0]                    m_depth_axi_awsize,
    output wire [ 1 : 0]                    m_depth_axi_awburst,
    output wire                             m_depth_axi_awlock,
    output wire [ 3 : 0]                    m_depth_axi_awcache,
    output wire [ 2 : 0]                    m_depth_axi_awprot, 
    output wire                             m_depth_axi_awvalid,
    input  wire                             m_depth_axi_awready,

    output wire [DATA_WIDTH - 1 : 0]        m_depth_axi_wdata,
    output wire [STRB_WIDTH - 1 : 0]        m_depth_axi_wstrb,
    output wire                             m_depth_axi_wlast,
    output wire                             m_depth_axi_wvalid,
    input  wire                             m_depth_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]          m_depth_axi_bid,
    input  wire [ 1 : 0]                    m_depth_axi_bresp,
    input  wire                             m_depth_axi_bvalid,
    output wire                             m_depth_axi_bready,

    output wire [ID_WIDTH - 1 : 0]          m_depth_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_depth_axi_araddr,
    output wire [ 7 : 0]                    m_depth_axi_arlen,
    output wire [ 2 : 0]                    m_depth_axi_arsize,
    output wire [ 1 : 0]                    m_depth_axi_arburst,
    output wire                             m_depth_axi_arlock,
    output wire [ 3 : 0]                    m_depth_axi_arcache,
    output wire [ 2 : 0]                    m_depth_axi_arprot,
    output wire                             m_depth_axi_arvalid,
    input  wire                             m_depth_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]          m_depth_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]        m_depth_axi_rdata,
    input  wire [ 1 : 0]                    m_depth_axi_rresp,
    input  wire                             m_depth_axi_rlast,
    input  wire                             m_depth_axi_rvalid,
    output wire                             m_depth_axi_rready,

    // Stencil Buffer
    output wire [ID_WIDTH - 1 : 0]          m_stencil_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]        m_stencil_axi_awaddr,
    output wire [ 7 : 0]                    m_stencil_axi_awlen,
    output wire [ 2 : 0]                    m_stencil_axi_awsize,
    output wire [ 1 : 0]                    m_stencil_axi_awburst,
    output wire                             m_stencil_axi_awlock,
    output wire [ 3 : 0]                    m_stencil_axi_awcache,
    output wire [ 2 : 0]                    m_stencil_axi_awprot, 
    output wire                             m_stencil_axi_awvalid,
    input  wire                             m_stencil_axi_awready,

    output wire [DATA_WIDTH - 1 : 0]        m_stencil_axi_wdata,
    output wire [STRB_WIDTH - 1 : 0]        m_stencil_axi_wstrb,
    output wire                             m_stencil_axi_wlast,
    output wire                             m_stencil_axi_wvalid,
    input  wire                             m_stencil_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]          m_stencil_axi_bid,
    input  wire [ 1 : 0]                    m_stencil_axi_bresp,
    input  wire                             m_stencil_axi_bvalid,
    output wire                             m_stencil_axi_bready,

    output wire [ID_WIDTH - 1 : 0]          m_stencil_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_stencil_axi_araddr,
    output wire [ 7 : 0]                    m_stencil_axi_arlen,
    output wire [ 2 : 0]                    m_stencil_axi_arsize,
    output wire [ 1 : 0]                    m_stencil_axi_arburst,
    output wire                             m_stencil_axi_arlock,
    output wire [ 3 : 0]                    m_stencil_axi_arcache,
    output wire [ 2 : 0]                    m_stencil_axi_arprot,
    output wire                             m_stencil_axi_arvalid,
    input  wire                             m_stencil_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]          m_stencil_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]        m_stencil_axi_rdata,
    input  wire [ 1 : 0]                    m_stencil_axi_rresp,
    input  wire                             m_stencil_axi_rlast,
    input  wire                             m_stencil_axi_rvalid,
    output wire                             m_stencil_axi_rready,

    // TMU 0 memory access
    output wire [ID_WIDTH - 1 : 0]          m_tmu0_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_tmu0_axi_araddr,
    output wire [ 7 : 0]                    m_tmu0_axi_arlen,
    output wire [ 2 : 0]                    m_tmu0_axi_arsize,
    output wire [ 1 : 0]                    m_tmu0_axi_arburst,
    output wire                             m_tmu0_axi_arlock,
    output wire [ 3 : 0]                    m_tmu0_axi_arcache,
    output wire [ 2 : 0]                    m_tmu0_axi_arprot,
    output wire                             m_tmu0_axi_arvalid,
    input  wire                             m_tmu0_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]          m_tmu0_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]        m_tmu0_axi_rdata,
    input  wire [ 1 : 0]                    m_tmu0_axi_rresp,
    input  wire                             m_tmu0_axi_rlast,
    input  wire                             m_tmu0_axi_rvalid,
    output wire                             m_tmu0_axi_rready,

    // TMU 1 memory access
    output wire [ID_WIDTH - 1 : 0]          m_tmu1_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_tmu1_axi_araddr,
    output wire [ 7 : 0]                    m_tmu1_axi_arlen,
    output wire [ 2 : 0]                    m_tmu1_axi_arsize,
    output wire [ 1 : 0]                    m_tmu1_axi_arburst,
    output wire                             m_tmu1_axi_arlock,
    output wire [ 3 : 0]                    m_tmu1_axi_arcache,
    output wire [ 2 : 0]                    m_tmu1_axi_arprot,
    output wire                             m_tmu1_axi_arvalid,
    input  wire                             m_tmu1_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]          m_tmu1_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]        m_tmu1_axi_rdata,
    input  wire [ 1 : 0]                    m_tmu1_axi_rresp,
    input  wire                             m_tmu1_axi_rlast,
    input  wire                             m_tmu1_axi_rvalid,
    output wire                             m_tmu1_axi_rready
);
`include "RegisterAndDescriptorDefines.vh"
    localparam FRAMEBUFFER_SIZE_IN_PIXEL_LG = 20; // Width of the framebuffer index. 20 bit is enough for a framebuffer with a size of 2MB (RGB565)
    localparam DEFAULT_ALPHA_VAL = 0;
    localparam SCREEN_POS_WIDTH = 11;
    localparam PIPELINE_PIXEL_WIDTH = COLOR_SUB_PIXEL_WIDTH * COLOR_NUMBER_OF_SUB_PIXEL;
    localparam FRAMEBUFFER_NUMBER_OF_SUB_PIXELS = 3; // The number of sub pixels in the framebuffer
    // This is used to configure, if it is required to reduce / expand a vector or not. This is done by the offset:
    // When the offset is set to number of pixels, then the reduce / expand function will just copy the line
    // without removing or adding something.
    // If it is set to a lower value, then the functions will start to remove or add new pixels.
    localparam SUB_PIXEL_OFFSET = (COLOR_NUMBER_OF_SUB_PIXEL == FRAMEBUFFER_NUMBER_OF_SUB_PIXELS) ? COLOR_NUMBER_OF_SUB_PIXEL : COLOR_A_POS; 
    `ReduceVec(ColorBufferReduceVec, COLOR_SUB_PIXEL_WIDTH, COLOR_NUMBER_OF_SUB_PIXEL, SUB_PIXEL_OFFSET, COLOR_NUMBER_OF_SUB_PIXEL, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS)
    `ReduceVec(ColorBufferReduceMask, 1, COLOR_NUMBER_OF_SUB_PIXEL, SUB_PIXEL_OFFSET, COLOR_NUMBER_OF_SUB_PIXEL, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS) 
    `ExpandVec(ColorBufferExpandVec, COLOR_SUB_PIXEL_WIDTH, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS, SUB_PIXEL_OFFSET, COLOR_NUMBER_OF_SUB_PIXEL, COLOR_NUMBER_OF_SUB_PIXEL)
    `XXX2RGB565(XXX2RGB565, COLOR_SUB_PIXEL_WIDTH, 1)
    `RGB5652XXX(RGB5652XXX, COLOR_SUB_PIXEL_WIDTH, 1)

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
    wire                                             colorBufferApply;
    wire                                             colorBufferApplied;
    wire                                             colorBufferCmdCommit;
    wire                                             colorBufferCmdMemset;
    wire                                             colorBufferCmdSwap;
    wire                                             colorBufferEnable;
    wire [3 : 0]                                     colorBufferMask;
    wire [COLOR_NUMBER_OF_SUB_PIXEL - 1 : 0]         colorBufferMaskReduced;
    wire                                             m_color_arvalid;
    wire                                             m_color_arlast;
    wire                                             m_color_arready;
    wire                                             m_color_rvalid;
    wire                                             m_color_rready;
    wire                                             m_color_rlast;
    wire [FRAMEBUFFER_SIZE_IN_PIXEL_LG - 1 : 0]      m_color_araddr;
    wire [FRAMEBUFFER_SIZE_IN_PIXEL_LG - 1 : 0]      m_color_waddr;
    wire                                             m_color_wvalid;
    wire                                             m_color_wready;
    wire [PIXEL_WIDTH - 1 : 0]                       m_color_rdata;
    wire [PIPELINE_PIXEL_WIDTH - 1 : 0]              m_color_wdata;
    wire                                             m_color_wstrb;
    wire                                             m_color_wlast;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  m_color_wscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  m_color_wscreenPosY;

    // Depth buffer access
    wire [DEPTH_WIDTH - 1 : 0]                       depthBufferClearDepth;
    wire [ADDR_WIDTH - 1 : 0]                        depthBufferAddr;
    wire                                             depthBufferApply;
    wire                                             depthBufferApplied;
    wire                                             depthBufferCmdCommit;
    wire                                             depthBufferCmdMemset;
    wire                                             depthBufferEnable;
    wire                                             depthBufferMask;
    wire                                             m_depth_arvalid;
    wire                                             m_depth_arlast;
    wire                                             m_depth_arready;
    wire                                             m_depth_rvalid;
    wire                                             m_depth_rready;
    wire                                             m_depth_rlast;
    wire [FRAMEBUFFER_SIZE_IN_PIXEL_LG - 1 : 0]      m_depth_araddr;
    wire [FRAMEBUFFER_SIZE_IN_PIXEL_LG - 1 : 0]      m_depth_waddr;
    wire                                             m_depth_wvalid;
    wire                                             m_depth_wready;
    wire [DEPTH_WIDTH - 1 : 0]                       m_depth_rdata;
    wire [DEPTH_WIDTH - 1 : 0]                       m_depth_wdata;
    wire                                             m_depth_wstrb;
    wire                                             m_depth_wlast;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  m_depth_wscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  m_depth_wscreenPosY;

    // Stencil buffer access
    wire [STENCIL_WIDTH - 1 : 0]                     stencilBufferClearStencil;
    wire [ADDR_WIDTH - 1 : 0]                        stencilBufferAddr;
    wire                                             stencilBufferApply;
    wire                                             stencilBufferApplied;
    wire                                             stencilBufferCmdCommit;
    wire                                             stencilBufferCmdMemset;
    wire                                             stencilBufferEnable;
    wire [STENCIL_WIDTH - 1 : 0]                     stencilBufferMask;
    wire                                             m_stencil_arvalid;
    wire                                             m_stencil_arlast;
    wire                                             m_stencil_arready;
    wire                                             m_stencil_rvalid;
    wire                                             m_stencil_rready;
    wire                                             m_stencil_rlast;
    wire [FRAMEBUFFER_SIZE_IN_PIXEL_LG - 1 : 0]      m_stencil_araddr;
    wire [FRAMEBUFFER_SIZE_IN_PIXEL_LG - 1 : 0]      m_stencil_waddr;
    wire                                             m_stencil_wvalid;
    wire                                             m_stencil_wready;
    wire [STENCIL_WIDTH - 1 : 0]                     m_stencil_rdata;
    wire [7 : 0]                                     m_stencil_rdata_byte;
    wire [STENCIL_WIDTH - 1 : 0]                     m_stencil_wdata;
    wire                                             m_stencil_wstrb;
    wire                                             m_stencil_wlast;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  m_stencil_wscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  m_stencil_wscreenPosY;

    assign colorBufferMaskReduced = ColorBufferReduceMask(colorBufferMask);
    assign m_stencil_rdata = m_stencil_rdata_byte[0 +: STENCIL_WIDTH];

    assign fb_addr = colorBufferAddr;

    generate
        if (ENABLE_DEPTH_BUFFER)
        begin
            StreamFramebuffer #(
                .DATA_WIDTH(DATA_WIDTH),
                .ADDR_WIDTH(ADDR_WIDTH),
                .ID_WIDTH(ID_WIDTH),
                .X_BIT_WIDTH(RENDER_CONFIG_X_SIZE),
                .Y_BIT_WIDTH(RENDER_CONFIG_Y_SIZE),
                .PIXEL_WIDTH(DEPTH_WIDTH),
                .STRB_WIDTH(STRB_WIDTH)
            ) depthBuffer (
                .aclk(aclk),
                .resetn(resetn),

                .confAddr(depthBufferAddr),
                .confEnableScissor(framebufferParamEnableScissor),
                .confScissorStartX(framebufferParamScissorStartX),
                .confScissorStartY(framebufferParamScissorStartY),
                .confScissorEndX(framebufferParamScissorEndX),
                .confScissorEndY(framebufferParamScissorEndY),
                .confXResolution(framebufferParamXResolution),
                .confYResolution(framebufferParamYResolution),
                .confMask({ depthBufferMask, depthBufferMask }),
                .confClearColor(depthBufferClearDepth),

                .apply(depthBufferApply & depthBufferCmdMemset),
                .applied(depthBufferApplied),

                .s_fetch_arvalid(m_depth_arvalid),
                .s_fetch_arlast(m_depth_arlast),
                .s_fetch_arready(m_depth_arready),
                .s_fetch_araddr(m_depth_araddr),

                .s_frag_rvalid(m_depth_rvalid),
                .s_frag_rready(m_depth_rready),
                .s_frag_rdata(m_depth_rdata),
                .s_frag_rlast(m_depth_rlast),

                .s_frag_wvalid(m_depth_wvalid),
                .s_frag_wlast(m_depth_wlast),
                .s_frag_wready(m_depth_wready),
                .s_frag_wdata(m_depth_wdata),
                .s_frag_wstrb(m_depth_wstrb),
                .s_frag_waddr(m_depth_waddr),
                .s_frag_wxpos(m_depth_wscreenPosX),
                .s_frag_wypos(m_depth_wscreenPosY),

                .m_mem_axi_awid(m_depth_axi_awid),
                .m_mem_axi_awaddr(m_depth_axi_awaddr),
                .m_mem_axi_awlen(m_depth_axi_awlen), 
                .m_mem_axi_awsize(m_depth_axi_awsize), 
                .m_mem_axi_awburst(m_depth_axi_awburst), 
                .m_mem_axi_awlock(m_depth_axi_awlock), 
                .m_mem_axi_awcache(m_depth_axi_awcache), 
                .m_mem_axi_awprot(m_depth_axi_awprot), 
                .m_mem_axi_awvalid(m_depth_axi_awvalid),
                .m_mem_axi_awready(m_depth_axi_awready),

                .m_mem_axi_wdata(m_depth_axi_wdata),
                .m_mem_axi_wstrb(m_depth_axi_wstrb),
                .m_mem_axi_wlast(m_depth_axi_wlast),
                .m_mem_axi_wvalid(m_depth_axi_wvalid),
                .m_mem_axi_wready(m_depth_axi_wready),

                .m_mem_axi_bid(m_depth_axi_bid),
                .m_mem_axi_bresp(m_depth_axi_bresp),
                .m_mem_axi_bvalid(m_depth_axi_bvalid),
                .m_mem_axi_bready(m_depth_axi_bready),

                .m_mem_axi_arid(m_depth_axi_arid),
                .m_mem_axi_araddr(m_depth_axi_araddr),
                .m_mem_axi_arlen(m_depth_axi_arlen),
                .m_mem_axi_arsize(m_depth_axi_arsize),
                .m_mem_axi_arburst(m_depth_axi_arburst),
                .m_mem_axi_arlock(m_depth_axi_arlock),
                .m_mem_axi_arcache(m_depth_axi_arcache),
                .m_mem_axi_arprot(m_depth_axi_arprot),
                .m_mem_axi_arvalid(m_depth_axi_arvalid),
                .m_mem_axi_arready(m_depth_axi_arready),

                .m_mem_axi_rid(m_depth_axi_rid),
                .m_mem_axi_rdata(m_depth_axi_rdata),
                .m_mem_axi_rresp(m_depth_axi_rresp),
                .m_mem_axi_rlast(m_depth_axi_rlast),
                .m_mem_axi_rvalid(m_depth_axi_rvalid),
                .m_mem_axi_rready(m_depth_axi_rready)
            );
        end
        else
        begin
            assign m_depth_arready = 1;
            assign m_depth_rvalid = 1;
            assign m_depth_rdata = 16'hffff;
            assign m_depth_wready = 1;
            assign depthBufferApplied = 1;
        end
    endgenerate

    StreamFramebuffer #(
        .DATA_WIDTH(DATA_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .X_BIT_WIDTH(RENDER_CONFIG_X_SIZE),
        .Y_BIT_WIDTH(RENDER_CONFIG_Y_SIZE),
        .PIXEL_WIDTH(PIXEL_WIDTH),
        .STRB_WIDTH(STRB_WIDTH)
    ) colorBuffer (
        .aclk(aclk),
        .resetn(resetn),

        .confAddr(colorBufferAddr),
        .confEnableScissor(framebufferParamEnableScissor),
        .confScissorStartX(framebufferParamScissorStartX),
        .confScissorStartY(framebufferParamScissorStartY),
        .confScissorEndX(framebufferParamScissorEndX),
        .confScissorEndY(framebufferParamScissorEndY),
        .confXResolution(framebufferParamXResolution),
        .confYResolution(framebufferParamYResolution),
        .confMask(colorBufferMaskReduced[0 +: 2]), // Currently 16 bit pixels are used with memorys where the strobe can only mask byte wise. To silence the warning, only select the lower two bits.
        .confClearColor(XXX2RGB565(ColorBufferReduceVec(colorBufferClearColor))),

        .apply(colorBufferApply & colorBufferCmdMemset),
        .applied(colorBufferApplied),

        .s_fetch_arvalid(m_color_arvalid),
        .s_fetch_arlast(m_color_arlast),
        .s_fetch_arready(m_color_arready),
        .s_fetch_araddr(m_color_araddr),

        .s_frag_rvalid(m_color_rvalid),
        .s_frag_rready(m_color_rready),
        .s_frag_rdata(m_color_rdata),
        .s_frag_rlast(m_color_rlast),

        .s_frag_wvalid(m_color_wvalid),
        .s_frag_wlast(m_color_wlast),
        .s_frag_wready(m_color_wready),
        .s_frag_wdata(XXX2RGB565(ColorBufferReduceVec(m_color_wdata))),
        .s_frag_wstrb(m_color_wstrb),
        .s_frag_waddr(m_color_waddr),
        .s_frag_wxpos(m_color_wscreenPosX),
        .s_frag_wypos(m_color_wscreenPosY),

        .m_mem_axi_awid(m_color_axi_awid),
        .m_mem_axi_awaddr(m_color_axi_awaddr),
        .m_mem_axi_awlen(m_color_axi_awlen), 
        .m_mem_axi_awsize(m_color_axi_awsize), 
        .m_mem_axi_awburst(m_color_axi_awburst), 
        .m_mem_axi_awlock(m_color_axi_awlock), 
        .m_mem_axi_awcache(m_color_axi_awcache), 
        .m_mem_axi_awprot(m_color_axi_awprot), 
        .m_mem_axi_awvalid(m_color_axi_awvalid),
        .m_mem_axi_awready(m_color_axi_awready),

        .m_mem_axi_wdata(m_color_axi_wdata),
        .m_mem_axi_wstrb(m_color_axi_wstrb),
        .m_mem_axi_wlast(m_color_axi_wlast),
        .m_mem_axi_wvalid(m_color_axi_wvalid),
        .m_mem_axi_wready(m_color_axi_wready),

        .m_mem_axi_bid(m_color_axi_bid),
        .m_mem_axi_bresp(m_color_axi_bresp),
        .m_mem_axi_bvalid(m_color_axi_bvalid),
        .m_mem_axi_bready(m_color_axi_bready),

        .m_mem_axi_arid(m_color_axi_arid),
        .m_mem_axi_araddr(m_color_axi_araddr),
        .m_mem_axi_arlen(m_color_axi_arlen),
        .m_mem_axi_arsize(m_color_axi_arsize),
        .m_mem_axi_arburst(m_color_axi_arburst),
        .m_mem_axi_arlock(m_color_axi_arlock),
        .m_mem_axi_arcache(m_color_axi_arcache),
        .m_mem_axi_arprot(m_color_axi_arprot),
        .m_mem_axi_arvalid(m_color_axi_arvalid),
        .m_mem_axi_arready(m_color_axi_arready),

        .m_mem_axi_rid(m_color_axi_rid),
        .m_mem_axi_rdata(m_color_axi_rdata),
        .m_mem_axi_rresp(m_color_axi_rresp),
        .m_mem_axi_rlast(m_color_axi_rlast),
        .m_mem_axi_rvalid(m_color_axi_rvalid),
        .m_mem_axi_rready(m_color_axi_rready)
    );

    generate 
        if (ENABLE_STENCIL_BUFFER)
        begin
            StreamFramebuffer #(
                .DATA_WIDTH(DATA_WIDTH),
                .ADDR_WIDTH(ADDR_WIDTH),
                .ID_WIDTH(ID_WIDTH),
                .X_BIT_WIDTH(RENDER_CONFIG_X_SIZE),
                .Y_BIT_WIDTH(RENDER_CONFIG_Y_SIZE),
                .PIXEL_WIDTH(8),
                .STRB_WIDTH(STRB_WIDTH)
            ) stencilBuffer (
                .aclk(aclk),
                .resetn(resetn),

                .confAddr(stencilBufferAddr),
                .confEnableScissor(framebufferParamEnableScissor),
                .confScissorStartX(framebufferParamScissorStartX),
                .confScissorStartY(framebufferParamScissorStartY),
                .confScissorEndX(framebufferParamScissorEndX),
                .confScissorEndY(framebufferParamScissorEndY),
                .confXResolution(framebufferParamXResolution),
                .confYResolution(framebufferParamYResolution),
                .confMask(|stencilBufferMask),
                .confClearColor(stencilBufferClearStencil),

                .apply(stencilBufferApply & stencilBufferCmdMemset),
                .applied(stencilBufferApplied),

                .s_fetch_arvalid(m_stencil_arvalid),
                .s_fetch_arlast(m_stencil_arlast),
                .s_fetch_arready(m_stencil_arready),
                .s_fetch_araddr(m_stencil_araddr),

                .s_frag_rvalid(m_stencil_rvalid),
                .s_frag_rready(m_stencil_rready),
                .s_frag_rdata(m_stencil_rdata_byte),
                .s_frag_rlast(m_stencil_rlast),

                .s_frag_wvalid(m_stencil_wvalid),
                .s_frag_wlast(m_stencil_wlast),
                .s_frag_wready(m_stencil_wready),
                .s_frag_wdata(m_stencil_wdata),
                .s_frag_wstrb(m_stencil_wstrb),
                .s_frag_waddr(m_stencil_waddr),
                .s_frag_wxpos(m_stencil_wscreenPosX),
                .s_frag_wypos(m_stencil_wscreenPosY),

                .m_mem_axi_awid(m_stencil_axi_awid),
                .m_mem_axi_awaddr(m_stencil_axi_awaddr),
                .m_mem_axi_awlen(m_stencil_axi_awlen), 
                .m_mem_axi_awsize(m_stencil_axi_awsize), 
                .m_mem_axi_awburst(m_stencil_axi_awburst), 
                .m_mem_axi_awlock(m_stencil_axi_awlock), 
                .m_mem_axi_awcache(m_stencil_axi_awcache), 
                .m_mem_axi_awprot(m_stencil_axi_awprot), 
                .m_mem_axi_awvalid(m_stencil_axi_awvalid),
                .m_mem_axi_awready(m_stencil_axi_awready),

                .m_mem_axi_wdata(m_stencil_axi_wdata),
                .m_mem_axi_wstrb(m_stencil_axi_wstrb),
                .m_mem_axi_wlast(m_stencil_axi_wlast),
                .m_mem_axi_wvalid(m_stencil_axi_wvalid),
                .m_mem_axi_wready(m_stencil_axi_wready),

                .m_mem_axi_bid(m_stencil_axi_bid),
                .m_mem_axi_bresp(m_stencil_axi_bresp),
                .m_mem_axi_bvalid(m_stencil_axi_bvalid),
                .m_mem_axi_bready(m_stencil_axi_bready),

                .m_mem_axi_arid(m_stencil_axi_arid),
                .m_mem_axi_araddr(m_stencil_axi_araddr),
                .m_mem_axi_arlen(m_stencil_axi_arlen),
                .m_mem_axi_arsize(m_stencil_axi_arsize),
                .m_mem_axi_arburst(m_stencil_axi_arburst),
                .m_mem_axi_arlock(m_stencil_axi_arlock),
                .m_mem_axi_arcache(m_stencil_axi_arcache),
                .m_mem_axi_arprot(m_stencil_axi_arprot),
                .m_mem_axi_arvalid(m_stencil_axi_arvalid),
                .m_mem_axi_arready(m_stencil_axi_arready),

                .m_mem_axi_rid(m_stencil_axi_rid),
                .m_mem_axi_rdata(m_stencil_axi_rdata),
                .m_mem_axi_rresp(m_stencil_axi_rresp),
                .m_mem_axi_rlast(m_stencil_axi_rlast),
                .m_mem_axi_rvalid(m_stencil_axi_rvalid),
                .m_mem_axi_rready(m_stencil_axi_rready)
            );
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
        .ENABLE_READ_FIFO(1),
        .READ_FIFO_SIZE(5), // Decouples the read channel from the internal pipeline
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
        .colorBufferSize(fb_size),
        .colorBufferApply(colorBufferApply),
        .colorBufferApplied(colorBufferApplied && fb_swapped && !(colorBufferCmdCommit && colorBufferApply)),
        .colorBufferCmdCommit(colorBufferCmdCommit),
        .colorBufferCmdMemset(colorBufferCmdMemset),
        .colorBufferCmdSwap(colorBufferCmdSwap),
        .colorBufferCmdSwapEnableVsync(colorBufferCmdSwapEnableVsync),
        .colorBufferEnable(colorBufferEnable),
        .colorBufferMask(colorBufferMask),
        .m_color_arready(m_color_arready),
        .m_color_arlast(m_color_arlast),
        .m_color_arvalid(m_color_arvalid),
        .m_color_araddr(m_color_araddr),
        .m_color_rready(m_color_rready),
        .m_color_rdata(ColorBufferExpandVec(RGB5652XXX(m_color_rdata), DEFAULT_ALPHA_VAL)),
        .m_color_rvalid(m_color_rvalid),
        .m_color_waddr(m_color_waddr),
        .m_color_wvalid(m_color_wvalid),
        .m_color_wready(m_color_wready),
        .m_color_wdata(m_color_wdata),
        .m_color_wstrb(m_color_wstrb),
        .m_color_wlast(m_color_wlast),
        .m_color_wscreenPosX(m_color_wscreenPosX),
        .m_color_wscreenPosY(m_color_wscreenPosY),

        .depthBufferClearDepth(depthBufferClearDepth),
        .depthBufferAddr(depthBufferAddr),
        .depthBufferSize(),
        .depthBufferApply(depthBufferApply),
        .depthBufferApplied(depthBufferApplied && !(depthBufferCmdCommit && depthBufferApply)),
        .depthBufferCmdCommit(depthBufferCmdCommit),
        .depthBufferCmdMemset(depthBufferCmdMemset),
        .depthBufferEnable(depthBufferEnable),
        .depthBufferMask(depthBufferMask),
        .m_depth_arready(m_depth_arready),
        .m_depth_arlast(m_depth_arlast),
        .m_depth_arvalid(m_depth_arvalid),
        .m_depth_araddr(m_depth_araddr),
        .m_depth_rready(m_depth_rready),
        .m_depth_rdata(m_depth_rdata),
        .m_depth_rvalid(m_depth_rvalid),
        .m_depth_waddr(m_depth_waddr),
        .m_depth_wvalid(m_depth_wvalid),
        .m_depth_wready(m_depth_wready),
        .m_depth_wdata(m_depth_wdata),
        .m_depth_wstrb(m_depth_wstrb),
        .m_depth_wlast(m_depth_wlast),
        .m_depth_wscreenPosX(m_depth_wscreenPosX),
        .m_depth_wscreenPosY(m_depth_wscreenPosY),

        .stencilBufferClearStencil(stencilBufferClearStencil),
        .stencilBufferAddr(stencilBufferAddr),
        .stencilBufferSize(),
        .stencilBufferApply(stencilBufferApply),
        .stencilBufferApplied(stencilBufferApplied && !(stencilBufferCmdCommit && stencilBufferApply)),
        .stencilBufferCmdCommit(stencilBufferCmdCommit),
        .stencilBufferCmdMemset(stencilBufferCmdMemset),
        .stencilBufferEnable(stencilBufferEnable),
        .stencilBufferMask(stencilBufferMask),
        .m_stencil_arready(m_stencil_arready),
        .m_stencil_arlast(m_stencil_arlast),
        .m_stencil_arvalid(m_stencil_arvalid),
        .m_stencil_araddr(m_stencil_araddr),
        .m_stencil_rready(m_stencil_rready),
        .m_stencil_rdata(m_stencil_rdata),
        .m_stencil_rvalid(m_stencil_rvalid),
        .m_stencil_waddr(m_stencil_waddr),
        .m_stencil_wvalid(m_stencil_wvalid),
        .m_stencil_wready(m_stencil_wready),
        .m_stencil_wdata(m_stencil_wdata),
        .m_stencil_wstrb(m_stencil_wstrb),
        .m_stencil_wlast(m_stencil_wlast),
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

endmodule