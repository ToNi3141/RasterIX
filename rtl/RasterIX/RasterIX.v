// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

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

module RasterIX #(
    // Selects the variant. Allowed values: if, ef
    parameter VARIANT = "if",

    // The size of the internal framebuffer (in power of two)
    // Depth buffer word size: 16 bit
    // Color buffer word size: FRAMEBUFFER_SUB_PIXEL_WIDTH * (FRAMEBUFFER_ENABLE_ALPHA_CHANNEL ? 4 : 3)
    // IF only.
    parameter FRAMEBUFFER_SIZE_IN_PIXEL_LG = 16,

    // This is the color depth of the framebuffer. Note: This setting has no influence on the framebuffer stream. This steam will
    // stay at RGB565. It changes the internal representation and might be used to reduce the memory footprint.
    // Lower depth will result in color banding.
    // IF only.
    parameter FRAMEBUFFER_SUB_PIXEL_WIDTH = 5,
    // This enables the alpha channel of the framebuffer. Requires additional memory.
    // IF only.
    parameter FRAMEBUFFER_ENABLE_ALPHA_CHANNEL = 0,

    // The precision of the sub pixel calculation in the shader. Must be between 5 and 8 
    parameter SUB_PIXEL_CALC_PRECISION = 8,

    // This enables the 4 bit stencil buffer
    parameter ENABLE_STENCIL_BUFFER = 1,

    // Enables the depth buffer
    parameter ENABLE_DEPTH_BUFFER = 1,

    // Number of TMUs. Currently supported values: 1 and 2
    parameter TMU_COUNT = 2,
    parameter ENABLE_MIPMAPPING = 1,
    parameter ENABLE_TEXTURE_FILTERING = 1,
    parameter TEXTURE_PAGE_SIZE = 4096,

    // Enables the fog unit
    parameter ENABLE_FOG = 1,
    
    // The maximum size of a texture
    parameter MAX_TEXTURE_SIZE = 256,

    // Memory address width
    parameter ADDR_WIDTH = 32,
    // Memory ID width
    parameter ID_WIDTH = 8,
    // Memory data width
    parameter DATA_WIDTH = 32,
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
    parameter RASTERIZER_ENABLE_FLOAT_INTERPOLATION = 0,


    localparam CMD_STREAM_WIDTH = 32,
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

    // AXI Stream command response interface
    output wire                             m_cmd_resp_axis_tvalid,
    input  wire                             m_cmd_resp_axis_tready,
    output wire                             m_cmd_resp_axis_tlast,
    output wire [CMD_STREAM_WIDTH - 1 : 0]  m_cmd_resp_axis_tdata,

    // Framebuffer
    output wire                             swap_fb,
    output wire                             swap_fb_enable_vsync,
    output wire [ADDR_WIDTH - 1 : 0]        fb_addr,
    output wire [FB_SIZE_IN_PIXEL_LG - 1 : 0] fb_size,
    input  wire                             fb_swapped,

    // Memory Interface
    output wire [ID_WIDTH - 1 : 0]          m_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]        m_axi_awaddr,
    output wire [ 7 : 0]                    m_axi_awlen,
    output wire [ 2 : 0]                    m_axi_awsize,
    output wire [ 1 : 0]                    m_axi_awburst,
    output wire                             m_axi_awlock,
    output wire [ 3 : 0]                    m_axi_awcache,
    output wire [ 2 : 0]                    m_axi_awprot,
    output wire                             m_axi_awvalid,
    input  wire                             m_axi_awready,
    output wire [DATA_WIDTH - 1 : 0]        m_axi_wdata,
    output wire [STRB_WIDTH - 1 : 0]        m_axi_wstrb,
    output wire                             m_axi_wlast,
    output wire                             m_axi_wvalid,
    input  wire                             m_axi_wready,
    input  wire [ID_WIDTH - 1 : 0]          m_axi_bid,
    input  wire [ 1 : 0]                    m_axi_bresp,
    input  wire                             m_axi_bvalid,
    output wire                             m_axi_bready,
    output wire [ID_WIDTH - 1 : 0]          m_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_axi_araddr,
    output wire [ 7 : 0]                    m_axi_arlen,
    output wire [ 2 : 0]                    m_axi_arsize,
    output wire [ 1 : 0]                    m_axi_arburst,
    output wire                             m_axi_arlock,
    output wire [ 3 : 0]                    m_axi_arcache,
    output wire [ 2 : 0]                    m_axi_arprot,
    output wire                             m_axi_arvalid,
    input  wire                             m_axi_arready,
    input  wire [ID_WIDTH - 1 : 0]          m_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]        m_axi_rdata,
    input  wire [ 1 : 0]                    m_axi_rresp,
    input  wire                             m_axi_rlast,
    input  wire                             m_axi_rvalid,
    output wire                             m_axi_rready
);
    generate
        if (VARIANT == "if" || VARIANT == "IF")
        begin
            RasterIX_IF #(
                .FRAMEBUFFER_SIZE_IN_PIXEL_LG(FRAMEBUFFER_SIZE_IN_PIXEL_LG),
                .FRAMEBUFFER_SUB_PIXEL_WIDTH(FRAMEBUFFER_SUB_PIXEL_WIDTH),
                .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION),
                .ENABLE_STENCIL_BUFFER(ENABLE_STENCIL_BUFFER),
                .ENABLE_DEPTH_BUFFER(ENABLE_DEPTH_BUFFER),
                .TMU_COUNT(TMU_COUNT),
                .ENABLE_MIPMAPPING(ENABLE_MIPMAPPING),
                .ENABLE_TEXTURE_FILTERING(ENABLE_TEXTURE_FILTERING),
                .ENABLE_FOG(ENABLE_FOG),
                .TEXTURE_PAGE_SIZE(TEXTURE_PAGE_SIZE),
                .MAX_TEXTURE_SIZE(MAX_TEXTURE_SIZE),
                .ADDR_WIDTH(ADDR_WIDTH),
                .ID_WIDTH(ID_WIDTH),
                .DATA_WIDTH(DATA_WIDTH),
                .STRB_WIDTH(STRB_WIDTH),
                .RASTERIZER_FLOAT_PRECISION(RASTERIZER_FLOAT_PRECISION),
                .RASTERIZER_FIXPOINT_PRECISION(RASTERIZER_FIXPOINT_PRECISION),
                .RASTERIZER_ENABLE_FLOAT_INTERPOLATION(RASTERIZER_ENABLE_FLOAT_INTERPOLATION)
            ) rixif (
                .aclk(aclk),
                .resetn(resetn),

                .s_cmd_axis_tvalid(s_cmd_axis_tvalid),
                .s_cmd_axis_tready(s_cmd_axis_tready),
                .s_cmd_axis_tlast(s_cmd_axis_tlast),
                .s_cmd_axis_tdata(s_cmd_axis_tdata),

                .m_cmd_resp_axis_tvalid(m_cmd_resp_axis_tvalid),
                .m_cmd_resp_axis_tready(m_cmd_resp_axis_tready),
                .m_cmd_resp_axis_tlast(m_cmd_resp_axis_tlast),
                .m_cmd_resp_axis_tdata(m_cmd_resp_axis_tdata),

                .swap_fb(swap_fb),
                .swap_fb_enable_vsync(swap_fb_enable_vsync),
                .fb_addr(fb_addr),
                .fb_size(fb_size),
                .fb_swapped(fb_swapped),

                .m_axi_awid(m_axi_awid),
                .m_axi_awaddr(m_axi_awaddr),
                .m_axi_awlen(m_axi_awlen),
                .m_axi_awsize(m_axi_awsize),
                .m_axi_awburst(m_axi_awburst),
                .m_axi_awlock(m_axi_awlock),
                .m_axi_awcache(m_axi_awcache),
                .m_axi_awprot(m_axi_awprot),
                .m_axi_awvalid(m_axi_awvalid),
                .m_axi_awready(m_axi_awready),
                .m_axi_wdata(m_axi_wdata),
                .m_axi_wstrb(m_axi_wstrb),
                .m_axi_wlast(m_axi_wlast),
                .m_axi_wvalid(m_axi_wvalid),
                .m_axi_wready(m_axi_wready),
                .m_axi_bid(m_axi_bid),
                .m_axi_bresp(m_axi_bresp),
                .m_axi_bvalid(m_axi_bvalid),
                .m_axi_bready(m_axi_bready),
                .m_axi_arid(m_axi_arid),
                .m_axi_araddr(m_axi_araddr),
                .m_axi_arlen(m_axi_arlen),
                .m_axi_arsize(m_axi_arsize),
                .m_axi_arburst(m_axi_arburst),
                .m_axi_arlock(m_axi_arlock),
                .m_axi_arcache(m_axi_arcache),
                .m_axi_arprot(m_axi_arprot),
                .m_axi_arvalid(m_axi_arvalid),
                .m_axi_arready(m_axi_arready),
                .m_axi_rid(m_axi_rid),
                .m_axi_rdata(m_axi_rdata),
                .m_axi_rresp(m_axi_rresp),
                .m_axi_rlast(m_axi_rlast),
                .m_axi_rvalid(m_axi_rvalid),
                .m_axi_rready(m_axi_rready)
            );
        end
        if (VARIANT == "ef" || VARIANT == "EF")
        begin
            RasterIX_EF #(
                .SUB_PIXEL_CALC_PRECISION(SUB_PIXEL_CALC_PRECISION),
                .ENABLE_STENCIL_BUFFER(ENABLE_STENCIL_BUFFER),
                .ENABLE_DEPTH_BUFFER(ENABLE_DEPTH_BUFFER),
                .TMU_COUNT(TMU_COUNT),
                .ENABLE_MIPMAPPING(ENABLE_MIPMAPPING),
                .ENABLE_TEXTURE_FILTERING(ENABLE_TEXTURE_FILTERING),
                .ENABLE_FOG(ENABLE_FOG),
                .TEXTURE_PAGE_SIZE(TEXTURE_PAGE_SIZE),
                .MAX_TEXTURE_SIZE(MAX_TEXTURE_SIZE),
                .ADDR_WIDTH(ADDR_WIDTH),
                .ID_WIDTH(ID_WIDTH),
                .DATA_WIDTH(DATA_WIDTH),
                .STRB_WIDTH(STRB_WIDTH),
                .RASTERIZER_FLOAT_PRECISION(RASTERIZER_FLOAT_PRECISION),
                .RASTERIZER_FIXPOINT_PRECISION(RASTERIZER_FIXPOINT_PRECISION),
                .RASTERIZER_ENABLE_FLOAT_INTERPOLATION(RASTERIZER_ENABLE_FLOAT_INTERPOLATION)
            ) rixef (
                .aclk(aclk),
                .resetn(resetn),

                .s_cmd_axis_tvalid(s_cmd_axis_tvalid),
                .s_cmd_axis_tready(s_cmd_axis_tready),
                .s_cmd_axis_tlast(s_cmd_axis_tlast),
                .s_cmd_axis_tdata(s_cmd_axis_tdata),

                .m_cmd_resp_axis_tvalid(m_cmd_resp_axis_tvalid),
                .m_cmd_resp_axis_tready(m_cmd_resp_axis_tready),
                .m_cmd_resp_axis_tlast(m_cmd_resp_axis_tlast),
                .m_cmd_resp_axis_tdata(m_cmd_resp_axis_tdata),

                .swap_fb(swap_fb),
                .swap_fb_enable_vsync(swap_fb_enable_vsync),
                .fb_addr(fb_addr),
                .fb_size(fb_size),
                .fb_swapped(fb_swapped),
                
                .m_axi_awid(m_axi_awid),
                .m_axi_awaddr(m_axi_awaddr),
                .m_axi_awlen(m_axi_awlen),
                .m_axi_awsize(m_axi_awsize),
                .m_axi_awburst(m_axi_awburst),
                .m_axi_awlock(m_axi_awlock),
                .m_axi_awcache(m_axi_awcache),
                .m_axi_awprot(m_axi_awprot),
                .m_axi_awvalid(m_axi_awvalid),
                .m_axi_awready(m_axi_awready),
                .m_axi_wdata(m_axi_wdata),
                .m_axi_wstrb(m_axi_wstrb),
                .m_axi_wlast(m_axi_wlast),
                .m_axi_wvalid(m_axi_wvalid),
                .m_axi_wready(m_axi_wready),
                .m_axi_bid(m_axi_bid),
                .m_axi_bresp(m_axi_bresp),
                .m_axi_bvalid(m_axi_bvalid),
                .m_axi_bready(m_axi_bready),
                .m_axi_arid(m_axi_arid),
                .m_axi_araddr(m_axi_araddr),
                .m_axi_arlen(m_axi_arlen),
                .m_axi_arsize(m_axi_arsize),
                .m_axi_arburst(m_axi_arburst),
                .m_axi_arlock(m_axi_arlock),
                .m_axi_arcache(m_axi_arcache),
                .m_axi_arprot(m_axi_arprot),
                .m_axi_arvalid(m_axi_arvalid),
                .m_axi_arready(m_axi_arready),
                .m_axi_rid(m_axi_rid),
                .m_axi_rdata(m_axi_rdata),
                .m_axi_rresp(m_axi_rresp),
                .m_axi_rlast(m_axi_rlast),
                .m_axi_rvalid(m_axi_rvalid),
                .m_axi_rready(m_axi_rready)
            );
        end
    endgenerate
endmodule