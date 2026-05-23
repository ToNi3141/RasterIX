// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2026 ToNi3141

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

// High-level system wrapper that bundles RasterIX, AxisFramebufferReader,
// and the verilog-axi axi_crossbar (2 slave x 1 master) into a single
// block-design-friendly module.
//
// AXI crossbar slave slot assignment:
//   Slot 0 (lower bits): RasterIX             (full R/W)
//   Slot 1 (upper bits): AxisFramebufferReader (read-only; write channels tied to 0)
//
// ID width convention:
//   External (crossbar master) ID width : ID_WIDTH
//   Internal (crossbar slave)  ID width : ID_WIDTH - 1  ($clog2(S_COUNT=2) = 1)

`resetall
`default_nettype none

module RasterIXSystem #(
    // -----------------------------------------------------------------------
    // RasterIX parameters
    // -----------------------------------------------------------------------

    // Selects the RasterIX variant:
    //   "if" - Internal Framebuffer: renders into a small on-chip buffer and
    //          streams it out; lower external memory bandwidth requirement.
    //   "ef" - External Framebuffer: renders directly into system memory.
    parameter RIX_VARIANT = "if",

    // IF only: log2 of the number of pixels in the internal framebuffer.
    parameter RIX_FRAMEBUFFER_SIZE_IN_PIXEL_LG = 16,

    // IF only: bits per colour channel stored in the internal framebuffer.
    // Reducing this value lowers memory footprint at the cost of colour banding.
    parameter RIX_FRAMEBUFFER_SUB_PIXEL_WIDTH = 5,

    // IF only: enable the alpha channel in the internal framebuffer.
    // Requires additional memory.
    parameter RIX_FRAMEBUFFER_ENABLE_ALPHA_CHANNEL = 0,

    // Sub-pixel calculation precision in the shader. Must be between 5 and 8.
    parameter RIX_SUB_PIXEL_CALC_PRECISION = 8,

    // Enable the 4-bit stencil buffer.
    parameter RIX_ENABLE_STENCIL_BUFFER = 1,

    // Enable the depth buffer.
    parameter RIX_ENABLE_DEPTH_BUFFER = 1,

    // Number of Texture Mapping Units. Supported values: 1 and 2.
    parameter RIX_TMU_COUNT = 2,

    // Enable mip-mapping support.
    parameter RIX_ENABLE_MIPMAPPING = 1,

    // Enable bilinear texture filtering.
    parameter RIX_ENABLE_TEXTURE_FILTERING = 1,

    // Texture page size in bytes.
    parameter RIX_TEXTURE_PAGE_SIZE = 4096,

    // Enable the fog unit.
    parameter RIX_ENABLE_FOG = 1,

    // Maximum texture dimension in pixels (e.g. 256 for 256x256 textures).
    parameter RIX_MAX_TEXTURE_SIZE = 256,

    // AXI address bus width in bits.
    parameter ADDR_WIDTH = 32,

    // AXI ID width at the external (crossbar master) port.
    // The crossbar adds $clog2(S_COUNT) = 1 bit internally, so the internal
    // slave ports use ID_WIDTH-1 bits. Minimum value: 2 (IF) or 3 (EF).
    parameter ID_WIDTH = 8,

    // AXI data bus width in bits. Applies to the memory master port and
    // the internal data path.
    parameter DATA_WIDTH = 32,

    // EF only: enable coalescing of memory write beats for higher throughput.
    parameter RIX_ENABLE_MEMORY_COALESCING = 1,

    // Floating-point interpolation mantissa width.
    // Reducing this saves LUTs at the cost of interpolation accuracy.
    // Each 4-bit reduction saves approximately 1k LUTs.
    // For FPGAs with 18-bit native multipliers, a value of 26 is recommended.
    parameter RIX_RASTERIZER_FLOAT_PRECISION = 32,

    // Fixed-point multiplier width used when RASTERIZER_ENABLE_FLOAT_INTERPOLATION=0.
    // Includes the sign bit (e.g. 25 = signed 25-bit multiplier).
    // Lower values can cause fog and texel distortions.
    parameter RIX_RASTERIZER_FIXPOINT_PRECISION = 25,

    // 1 = use floating-point attribute interpolation.
    // 0 = use fixed-point attribute interpolation (lower resource cost).
    parameter RIX_RASTERIZER_ENABLE_FLOAT_INTERPOLATION = 0,

    // -----------------------------------------------------------------------
    // AxisFramebufferReader parameters
    // -----------------------------------------------------------------------

    // Controls when fb_swapped is asserted after a framebuffer swap request:
    //   1 = blocking  : fb_swapped is asserted only after the full frame
    //                   transfer to the display stream is complete.
    //   0 = non-blocking: fb_swapped is asserted immediately when the transfer
    //                   starts (display may still be reading old data).
    parameter FBR_BLOCKING = 1,

    // log2 of the internal pixel FIFO depth used to decouple the memory read
    // path from the display stream.
    parameter FBR_FIFO_DEPTH_LG = 8,

    // Number of AXI read beats per burst issued to memory.
    // Must be <= 2^FIFO_DEPTH_LG.
    parameter FBR_BEATS_PER_BURST = 16
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    // -----------------------------------------------------------------------
    // 32-bit AXI-Stream command input
    // Carries OpenGL command stream from the host to RasterIX.
    // -----------------------------------------------------------------------
    input  wire                         s_cmd_axis_tvalid,
    output wire                         s_cmd_axis_tready,
    input  wire                         s_cmd_axis_tlast,
    input  wire [31 : 0]                s_cmd_axis_tdata,

    // -----------------------------------------------------------------------
    // 32-bit AXI-Stream command response output
    // Carries acknowledgement / result data back to the host from RasterIX.
    // -----------------------------------------------------------------------
    output wire                         m_cmd_resp_axis_tvalid,
    input  wire                         m_cmd_resp_axis_tready,
    output wire                         m_cmd_resp_axis_tlast,
    output wire [31 : 0]                m_cmd_resp_axis_tdata,

    // -----------------------------------------------------------------------
    // 16-bit AXI-Stream display output (RGB565)
    // Pixel stream produced by AxisFramebufferReader for the display controller.
    // -----------------------------------------------------------------------
    output wire                         m_disp_axis_tvalid,
    input  wire                         m_disp_axis_tready,
    output wire                         m_disp_axis_tlast,
    output wire [15 : 0]                m_disp_axis_tdata,

    // Framebuffer swap vsync enable hint from RasterIX.
    // When high, the host is expected to delay the swap acknowledgement until
    // the next vertical sync.
    output wire                         swap_fb_enable_vsync,

    // -----------------------------------------------------------------------
    // AXI4 memory master
    // Single port carrying merged traffic from RasterIX and AxisFramebufferReader
    // through the internal crossbar.
    // -----------------------------------------------------------------------

    // Write address channel
    output wire [ID_WIDTH - 1 : 0]      m_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]    m_axi_awaddr,
    output wire [ 7 : 0]                m_axi_awlen,
    output wire [ 2 : 0]                m_axi_awsize,
    output wire [ 1 : 0]                m_axi_awburst,
    output wire                         m_axi_awlock,
    output wire [ 3 : 0]                m_axi_awcache,
    output wire [ 2 : 0]                m_axi_awprot,
    output wire                         m_axi_awvalid,
    input  wire                         m_axi_awready,

    // Write data channel
    output wire [DATA_WIDTH - 1 : 0]    m_axi_wdata,
    output wire [DATA_WIDTH/8 - 1 : 0]  m_axi_wstrb,
    output wire                         m_axi_wlast,
    output wire                         m_axi_wvalid,
    input  wire                         m_axi_wready,

    // Write response channel
    input  wire [ID_WIDTH - 1 : 0]      m_axi_bid,
    input  wire [ 1 : 0]                m_axi_bresp,
    input  wire                         m_axi_bvalid,
    output wire                         m_axi_bready,

    // Read address channel
    output wire [ID_WIDTH - 1 : 0]      m_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]    m_axi_araddr,
    output wire [ 7 : 0]                m_axi_arlen,
    output wire [ 2 : 0]                m_axi_arsize,
    output wire [ 1 : 0]                m_axi_arburst,
    output wire                         m_axi_arlock,
    output wire [ 3 : 0]                m_axi_arcache,
    output wire [ 2 : 0]                m_axi_arprot,
    output wire                         m_axi_arvalid,
    input  wire                         m_axi_arready,

    // Read data channel
    input  wire [ID_WIDTH - 1 : 0]      m_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]    m_axi_rdata,
    input  wire [ 1 : 0]                m_axi_rresp,
    input  wire                         m_axi_rlast,
    input  wire                         m_axi_rvalid,
    output wire                         m_axi_rready
);

    // -----------------------------------------------------------------------
    // Local parameters
    // -----------------------------------------------------------------------

    // Number of crossbar slave ports
    localparam NRS = 2;

    // The crossbar adds $clog2(NRS) = 1 bit to slave IDs to produce the
    // master-side ID_WIDTH.  Slave ports therefore use ID_WIDTH-1 bits.
    localparam S_ID_WIDTH = ID_WIDTH - 1;

    // -----------------------------------------------------------------------
    // Internal framebuffer handshake wires
    // -----------------------------------------------------------------------

    wire                        swap_fb_int;
    wire [ADDR_WIDTH - 1 : 0]   fb_addr_int;
    wire [19 : 0]               fb_size_int;   // FB_SIZE_IN_PIXEL_LG is localparam 20
    wire                        fb_swapped_int;

    // -----------------------------------------------------------------------
    // AXI wires: RasterIX master (crossbar slave slot 0)
    // -----------------------------------------------------------------------

    wire [S_ID_WIDTH - 1 : 0]   rix_awid;
    wire [ADDR_WIDTH - 1 : 0]   rix_awaddr;
    wire [ 7 : 0]               rix_awlen;
    wire [ 2 : 0]               rix_awsize;
    wire [ 1 : 0]               rix_awburst;
    wire                        rix_awlock;
    wire [ 3 : 0]               rix_awcache;
    wire [ 2 : 0]               rix_awprot;
    wire                        rix_awvalid;
    wire                        rix_awready;

    wire [DATA_WIDTH - 1 : 0]   rix_wdata;
    wire [DATA_WIDTH/8 - 1 : 0] rix_wstrb;
    wire                        rix_wlast;
    wire                        rix_wvalid;
    wire                        rix_wready;

    wire [S_ID_WIDTH - 1 : 0]   rix_bid;
    wire [ 1 : 0]               rix_bresp;
    wire                        rix_bvalid;
    wire                        rix_bready;

    wire [S_ID_WIDTH - 1 : 0]   rix_arid;
    wire [ADDR_WIDTH - 1 : 0]   rix_araddr;
    wire [ 7 : 0]               rix_arlen;
    wire [ 2 : 0]               rix_arsize;
    wire [ 1 : 0]               rix_arburst;
    wire                        rix_arlock;
    wire [ 3 : 0]               rix_arcache;
    wire [ 2 : 0]               rix_arprot;
    wire                        rix_arvalid;
    wire                        rix_arready;

    wire [S_ID_WIDTH - 1 : 0]   rix_rid;
    wire [DATA_WIDTH - 1 : 0]   rix_rdata;
    wire [ 1 : 0]               rix_rresp;
    wire                        rix_rlast;
    wire                        rix_rvalid;
    wire                        rix_rready;

    // -----------------------------------------------------------------------
    // AXI wires: AxisFramebufferReader master (crossbar slave slot 1, read-only)
    // -----------------------------------------------------------------------

    wire [S_ID_WIDTH - 1 : 0]   fbr_arid;
    wire [ADDR_WIDTH - 1 : 0]   fbr_araddr;
    wire [ 7 : 0]               fbr_arlen;
    wire [ 2 : 0]               fbr_arsize;
    wire [ 1 : 0]               fbr_arburst;
    wire                        fbr_arlock;
    wire [ 3 : 0]               fbr_arcache;
    wire [ 2 : 0]               fbr_arprot;
    wire                        fbr_arvalid;
    wire                        fbr_arready;

    wire [S_ID_WIDTH - 1 : 0]   fbr_rid;
    wire [DATA_WIDTH - 1 : 0]   fbr_rdata;
    wire [ 1 : 0]               fbr_rresp;
    wire                        fbr_rlast;
    wire                        fbr_rvalid;
    wire                        fbr_rready;

    // -----------------------------------------------------------------------
    // Full-width crossbar slave-side output buses
    // These are declared here and sliced back to individual module wires below
    // the crossbar instantiation.
    // -----------------------------------------------------------------------

    // AW ready (one bit per slave slot)
    wire [NRS - 1 : 0]              xbar_s_awready;
    // W ready (one bit per slave slot)
    wire [NRS - 1 : 0]              xbar_s_wready;
    // B channel (write response back to slave ports)
    wire [NRS * S_ID_WIDTH - 1 : 0] xbar_s_bid;
    wire [NRS * 2 - 1 : 0]          xbar_s_bresp;
    wire [NRS - 1 : 0]              xbar_s_bvalid;
    // AR ready (one bit per slave slot)
    wire [NRS - 1 : 0]              xbar_s_arready;
    // R channel (read data back to slave ports)
    wire [NRS * S_ID_WIDTH - 1 : 0] xbar_s_rid;
    wire [NRS * DATA_WIDTH - 1 : 0] xbar_s_rdata;
    wire [NRS * 2 - 1 : 0]          xbar_s_rresp;
    wire [NRS - 1 : 0]              xbar_s_rlast;
    wire [NRS - 1 : 0]              xbar_s_rvalid;

    // -----------------------------------------------------------------------
    // Slice slot 0 (RasterIX) outputs from the crossbar slave buses
    // -----------------------------------------------------------------------

    assign rix_awready = xbar_s_awready[0];
    assign rix_wready  = xbar_s_wready [0];
    assign rix_bid     = xbar_s_bid    [S_ID_WIDTH - 1 : 0];
    assign rix_bresp   = xbar_s_bresp  [1 : 0];
    assign rix_bvalid  = xbar_s_bvalid [0];
    assign rix_arready = xbar_s_arready[0];
    assign rix_rid     = xbar_s_rid    [S_ID_WIDTH - 1 : 0];
    assign rix_rdata   = xbar_s_rdata  [DATA_WIDTH - 1 : 0];
    assign rix_rresp   = xbar_s_rresp  [1 : 0];
    assign rix_rlast   = xbar_s_rlast  [0];
    assign rix_rvalid  = xbar_s_rvalid [0];

    // -----------------------------------------------------------------------
    // Slice slot 1 (AxisFramebufferReader, read-only) outputs from the
    // crossbar slave buses.  AW/W/B outputs for slot 1 are don't-care
    // because no write transactions are ever issued on that slot.
    // -----------------------------------------------------------------------

    assign fbr_arready = xbar_s_arready[1];
    assign fbr_rid     = xbar_s_rid    [2 * S_ID_WIDTH - 1 : S_ID_WIDTH];
    assign fbr_rdata   = xbar_s_rdata  [2 * DATA_WIDTH - 1 : DATA_WIDTH];
    assign fbr_rresp   = xbar_s_rresp  [3 : 2];
    assign fbr_rlast   = xbar_s_rlast  [1];
    assign fbr_rvalid  = xbar_s_rvalid [1];

    // -----------------------------------------------------------------------
    // RasterIX instance
    // -----------------------------------------------------------------------

    RasterIX #(
        .VARIANT                              (RIX_VARIANT),
        .FRAMEBUFFER_SIZE_IN_PIXEL_LG         (RIX_FRAMEBUFFER_SIZE_IN_PIXEL_LG),
        .FRAMEBUFFER_SUB_PIXEL_WIDTH          (RIX_FRAMEBUFFER_SUB_PIXEL_WIDTH),
        .FRAMEBUFFER_ENABLE_ALPHA_CHANNEL     (RIX_FRAMEBUFFER_ENABLE_ALPHA_CHANNEL),
        .SUB_PIXEL_CALC_PRECISION             (RIX_SUB_PIXEL_CALC_PRECISION),
        .ENABLE_STENCIL_BUFFER                (RIX_ENABLE_STENCIL_BUFFER),
        .ENABLE_DEPTH_BUFFER                  (RIX_ENABLE_DEPTH_BUFFER),
        .TMU_COUNT                            (RIX_TMU_COUNT),
        .ENABLE_MIPMAPPING                    (RIX_ENABLE_MIPMAPPING),
        .ENABLE_TEXTURE_FILTERING             (RIX_ENABLE_TEXTURE_FILTERING),
        .TEXTURE_PAGE_SIZE                    (RIX_TEXTURE_PAGE_SIZE),
        .ENABLE_FOG                           (RIX_ENABLE_FOG),
        .MAX_TEXTURE_SIZE                     (RIX_MAX_TEXTURE_SIZE),
        .ADDR_WIDTH                           (ADDR_WIDTH),
        .ID_WIDTH                             (S_ID_WIDTH),
        .DATA_WIDTH                           (DATA_WIDTH),
        .STRB_WIDTH                           (DATA_WIDTH / 8),
        .ENABLE_MEMORY_COALESCING             (RIX_ENABLE_MEMORY_COALESCING),
        .RASTERIZER_FLOAT_PRECISION           (RIX_RASTERIZER_FLOAT_PRECISION),
        .RASTERIZER_FIXPOINT_PRECISION        (RIX_RASTERIZER_FIXPOINT_PRECISION),
        .RASTERIZER_ENABLE_FLOAT_INTERPOLATION(RIX_RASTERIZER_ENABLE_FLOAT_INTERPOLATION)
    ) rix_inst (
        .aclk   (aclk),
        .resetn (resetn),

        .s_cmd_axis_tvalid      (s_cmd_axis_tvalid),
        .s_cmd_axis_tready      (s_cmd_axis_tready),
        .s_cmd_axis_tlast       (s_cmd_axis_tlast),
        .s_cmd_axis_tdata       (s_cmd_axis_tdata),

        .m_cmd_resp_axis_tvalid (m_cmd_resp_axis_tvalid),
        .m_cmd_resp_axis_tready (m_cmd_resp_axis_tready),
        .m_cmd_resp_axis_tlast  (m_cmd_resp_axis_tlast),
        .m_cmd_resp_axis_tdata  (m_cmd_resp_axis_tdata),

        .swap_fb                (swap_fb_int),
        .swap_fb_enable_vsync   (swap_fb_enable_vsync),
        .fb_addr                (fb_addr_int),
        .fb_size                (fb_size_int),
        .fb_swapped             (fb_swapped_int),

        .m_axi_awid     (rix_awid),
        .m_axi_awaddr   (rix_awaddr),
        .m_axi_awlen    (rix_awlen),
        .m_axi_awsize   (rix_awsize),
        .m_axi_awburst  (rix_awburst),
        .m_axi_awlock   (rix_awlock),
        .m_axi_awcache  (rix_awcache),
        .m_axi_awprot   (rix_awprot),
        .m_axi_awvalid  (rix_awvalid),
        .m_axi_awready  (rix_awready),

        .m_axi_wdata    (rix_wdata),
        .m_axi_wstrb    (rix_wstrb),
        .m_axi_wlast    (rix_wlast),
        .m_axi_wvalid   (rix_wvalid),
        .m_axi_wready   (rix_wready),

        .m_axi_bid      (rix_bid),
        .m_axi_bresp    (rix_bresp),
        .m_axi_bvalid   (rix_bvalid),
        .m_axi_bready   (rix_bready),

        .m_axi_arid     (rix_arid),
        .m_axi_araddr   (rix_araddr),
        .m_axi_arlen    (rix_arlen),
        .m_axi_arsize   (rix_arsize),
        .m_axi_arburst  (rix_arburst),
        .m_axi_arlock   (rix_arlock),
        .m_axi_arcache  (rix_arcache),
        .m_axi_arprot   (rix_arprot),
        .m_axi_arvalid  (rix_arvalid),
        .m_axi_arready  (rix_arready),

        .m_axi_rid      (rix_rid),
        .m_axi_rdata    (rix_rdata),
        .m_axi_rresp    (rix_rresp),
        .m_axi_rlast    (rix_rlast),
        .m_axi_rvalid   (rix_rvalid),
        .m_axi_rready   (rix_rready)
    );

    // -----------------------------------------------------------------------
    // AxisFramebufferReader instance (crossbar slave slot 1, read-only)
    // -----------------------------------------------------------------------

    AxisFramebufferReader #(
        .ADDR_WIDTH     (ADDR_WIDTH),
        .DATA_WIDTH     (DATA_WIDTH),
        .ID_WIDTH       (S_ID_WIDTH),
        .BLOCKING       (FBR_BLOCKING),
        .FIFO_DEPTH_LG  (FBR_FIFO_DEPTH_LG),
        .BEATS_PER_BURST(FBR_BEATS_PER_BURST)
    ) fbr_inst (
        .aclk   (aclk),
        .resetn (resetn),

        .swap_fb    (swap_fb_int),
        .fb_addr    (fb_addr_int),
        .fb_size    (fb_size_int),
        .fb_swapped (fb_swapped_int),

        .m_disp_axis_tvalid (m_disp_axis_tvalid),
        .m_disp_axis_tready (m_disp_axis_tready),
        .m_disp_axis_tlast  (m_disp_axis_tlast),
        .m_disp_axis_tdata  (m_disp_axis_tdata),

        .m_mem_axi_arid     (fbr_arid),
        .m_mem_axi_araddr   (fbr_araddr),
        .m_mem_axi_arlen    (fbr_arlen),
        .m_mem_axi_arsize   (fbr_arsize),
        .m_mem_axi_arburst  (fbr_arburst),
        .m_mem_axi_arlock   (fbr_arlock),
        .m_mem_axi_arcache  (fbr_arcache),
        .m_mem_axi_arprot   (fbr_arprot),
        .m_mem_axi_arvalid  (fbr_arvalid),
        .m_mem_axi_arready  (fbr_arready),

        .m_mem_axi_rid      (fbr_rid),
        .m_mem_axi_rdata    (fbr_rdata),
        .m_mem_axi_rresp    (fbr_rresp),
        .m_mem_axi_rlast    (fbr_rlast),
        .m_mem_axi_rvalid   (fbr_rvalid),
        .m_mem_axi_rready   (fbr_rready)
    );

    // -----------------------------------------------------------------------
    // AXI crossbar (2 slave x 1 master)
    //   Uses the verilog-axi crossbar from rtl/3rdParty/verilog-axi/
    //
    //   Concatenation order in all NRS-wide port arrays:
    //     [upper = slot 1 (AxisFramebufferReader)] [lower = slot 0 (RasterIX)]
    //
    //   The write channels for slot 1 are permanently tied to 0 because
    //   AxisFramebufferReader never issues write transactions.
    // -----------------------------------------------------------------------

    axi_crossbar #(
        .S_COUNT        (NRS),
        .M_COUNT        (1),
        .DATA_WIDTH     (DATA_WIDTH),
        .ADDR_WIDTH     (ADDR_WIDTH),
        .STRB_WIDTH     (DATA_WIDTH / 8),
        .S_ID_WIDTH     (S_ID_WIDTH),
        .M_ID_WIDTH     (ID_WIDTH),
        .AWUSER_ENABLE  (0),
        .WUSER_ENABLE   (0),
        .BUSER_ENABLE   (0),
        .ARUSER_ENABLE  (0),
        .RUSER_ENABLE   (0),
        .S_THREADS      ({NRS{32'd2}}),
        .S_ACCEPT       ({NRS{32'd16}}),
        .M_REGIONS      (1),
        .M_BASE_ADDR    (0),
        .M_ADDR_WIDTH   (ADDR_WIDTH[0 +: 32])
    ) xbar_inst (
        .clk    (aclk),
        .rst    (!resetn),

        // ---- Slave write address channel ----
        // Slot 1 (FBReader) write channels are permanently tied to 0.
        .s_axi_awid     ({{S_ID_WIDTH{1'b0}}, rix_awid   }),
        .s_axi_awaddr   ({{ADDR_WIDTH{1'b0}}, rix_awaddr }),
        .s_axi_awlen    ({8'b0,               rix_awlen  }),
        .s_axi_awsize   ({3'b0,               rix_awsize }),
        .s_axi_awburst  ({2'b0,               rix_awburst}),
        .s_axi_awlock   ({1'b0,               rix_awlock }),
        .s_axi_awcache  ({4'b0,               rix_awcache}),
        .s_axi_awprot   ({3'b0,               rix_awprot }),
        .s_axi_awqos    (0),
        .s_axi_awuser   (0),
        .s_axi_awvalid  ({1'b0,               rix_awvalid}),
        .s_axi_awready  (xbar_s_awready),

        // ---- Slave write data channel ----
        .s_axi_wdata    ({{DATA_WIDTH{1'b0}},     rix_wdata }),
        .s_axi_wstrb    ({{(DATA_WIDTH/8){1'b0}}, rix_wstrb }),
        .s_axi_wlast    ({1'b0,                   rix_wlast }),
        .s_axi_wuser    (0),
        .s_axi_wvalid   ({1'b0,                   rix_wvalid}),
        .s_axi_wready   (xbar_s_wready),

        // ---- Slave write response channel ----
        .s_axi_bid      (xbar_s_bid),
        .s_axi_bresp    (xbar_s_bresp),
        .s_axi_buser    (),
        .s_axi_bvalid   (xbar_s_bvalid),
        .s_axi_bready   ({1'b0, rix_bready}),

        // ---- Slave read address channel ----
        .s_axi_arid     ({fbr_arid,    rix_arid   }),
        .s_axi_araddr   ({fbr_araddr,  rix_araddr }),
        .s_axi_arlen    ({fbr_arlen,   rix_arlen  }),
        .s_axi_arsize   ({fbr_arsize,  rix_arsize }),
        .s_axi_arburst  ({fbr_arburst, rix_arburst}),
        .s_axi_arlock   ({fbr_arlock,  rix_arlock }),
        .s_axi_arcache  ({fbr_arcache, rix_arcache}),
        .s_axi_arprot   ({fbr_arprot,  rix_arprot }),
        .s_axi_arqos    (0),
        .s_axi_aruser   (0),
        .s_axi_arvalid  ({fbr_arvalid, rix_arvalid}),
        .s_axi_arready  (xbar_s_arready),

        // ---- Slave read data channel ----
        .s_axi_rid      (xbar_s_rid),
        .s_axi_rdata    (xbar_s_rdata),
        .s_axi_rresp    (xbar_s_rresp),
        .s_axi_rlast    (xbar_s_rlast),
        .s_axi_ruser    (),
        .s_axi_rvalid   (xbar_s_rvalid),
        .s_axi_rready   ({fbr_rready, rix_rready}),

        // ---- Master port 0: external AXI memory ----
        .m_axi_awid     (m_axi_awid),
        .m_axi_awaddr   (m_axi_awaddr),
        .m_axi_awlen    (m_axi_awlen),
        .m_axi_awsize   (m_axi_awsize),
        .m_axi_awburst  (m_axi_awburst),
        .m_axi_awlock   (m_axi_awlock),
        .m_axi_awcache  (m_axi_awcache),
        .m_axi_awprot   (m_axi_awprot),
        .m_axi_awqos    (),
        .m_axi_awregion (),
        .m_axi_awuser   (),
        .m_axi_awvalid  (m_axi_awvalid),
        .m_axi_awready  (m_axi_awready),

        .m_axi_wdata    (m_axi_wdata),
        .m_axi_wstrb    (m_axi_wstrb),
        .m_axi_wlast    (m_axi_wlast),
        .m_axi_wuser    (),
        .m_axi_wvalid   (m_axi_wvalid),
        .m_axi_wready   (m_axi_wready),

        .m_axi_bid      (m_axi_bid),
        .m_axi_bresp    (m_axi_bresp),
        .m_axi_buser    (0),
        .m_axi_bvalid   (m_axi_bvalid),
        .m_axi_bready   (m_axi_bready),

        .m_axi_arid     (m_axi_arid),
        .m_axi_araddr   (m_axi_araddr),
        .m_axi_arlen    (m_axi_arlen),
        .m_axi_arsize   (m_axi_arsize),
        .m_axi_arburst  (m_axi_arburst),
        .m_axi_arlock   (m_axi_arlock),
        .m_axi_arcache  (m_axi_arcache),
        .m_axi_arprot   (m_axi_arprot),
        .m_axi_arqos    (),
        .m_axi_arregion (),
        .m_axi_aruser   (),
        .m_axi_arvalid  (m_axi_arvalid),
        .m_axi_arready  (m_axi_arready),

        .m_axi_rid      (m_axi_rid),
        .m_axi_rdata    (m_axi_rdata),
        .m_axi_rresp    (m_axi_rresp),
        .m_axi_rlast    (m_axi_rlast),
        .m_axi_ruser    (0),
        .m_axi_rvalid   (m_axi_rvalid),
        .m_axi_rready   (m_axi_rready)
    );

endmodule

`resetall
