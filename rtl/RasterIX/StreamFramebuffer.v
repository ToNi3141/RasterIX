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

// Framebuffer implementation which ready pixels directly from the memory.
// No cache is implemented. No RAW/WAR avoidance strategies are implemented.
// Only read a pixel from the same address again, when it is ensured, that
// a already pending pixel was written to the memory. Otherwise previous 
// results will be overwritten.
// Performance: 1 (r/w) pixel per cycle. Theoretically it is capable to 
// service the pipeline without stalling (as long as the memory connect to 
// this module is fast enough)
module StreamFramebuffer 
#(
    // Width of the axi interfaces
    parameter DATA_WIDTH = 32,
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of wstrb (width of data bus in words)
    parameter STRB_WIDTH = (DATA_WIDTH / 8),
    // Width of ID signal
    parameter ID_WIDTH = 8,

    // Configures the Coalescer instantiated on the memory master port. A value of
    // 0 or 1 bypasses coalescing entirely. A value greater than 1 coalesces up to
    // that many beats into a single AXI transaction.
    parameter MAX_BEATS_TO_COALESCE = 0,

    // The maximum size of the screen in power of two
    parameter X_BIT_WIDTH = 11,
    parameter Y_BIT_WIDTH = 11,

    // Size of the pixels
    parameter PIXEL_WIDTH = 16,
    localparam PIXEL_MASK_WIDTH = PIXEL_WIDTH / 8,
    localparam PIXEL_WIDTH_LG = $clog2(PIXEL_WIDTH / 8)
)
(
    input  wire                             aclk,
    input  wire                             resetn,

    /////////////////////////
    // Configs
    /////////////////////////
    input  wire [ADDR_WIDTH - 1 : 0]        confAddr,
    input  wire                             confEnableScissor,
    input  wire [X_BIT_WIDTH - 1 : 0]       confScissorStartX,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confScissorStartY,
    input  wire [X_BIT_WIDTH - 1 : 0]       confScissorEndX,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confScissorEndY,
    input  wire [X_BIT_WIDTH - 1 : 0]       confXResolution,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confYResolution,
    input  wire [PIXEL_MASK_WIDTH - 1 : 0]  confMask,
    input  wire [PIXEL_WIDTH - 1 : 0]       confClearColor,

    /////////////////////////
    // Control
    /////////////////////////

    // Cmd interface
    input  wire                             apply, // This start a command framebuffer clear
    output wire                             applied, // This marks if the commands has been applied.

    /////////////////////////
    // Fragment Interface
    /////////////////////////

    // Fetch interface
    input  wire                             s_fetch_arvalid,
    input  wire                             s_fetch_arlast,
    output wire                             s_fetch_arready,
    input  wire [ADDR_WIDTH - 1 : 0]        s_fetch_araddr,

    // Framebuffer read interface
    output wire                             s_frag_rvalid,
    input  wire                             s_frag_rready,
    output wire [PIXEL_WIDTH - 1 : 0]       s_frag_rdata,
    output wire                             s_frag_rlast,

    // Framebuffer write interface
    input  wire                             s_frag_wvalid,
    input  wire                             s_frag_wlast,
    output wire                             s_frag_wready,
    input  wire [PIXEL_WIDTH - 1 : 0]       s_frag_wdata,
    input  wire                             s_frag_wstrb,
    input  wire [ADDR_WIDTH - 1 : 0]        s_frag_waddr,
    input  wire [X_BIT_WIDTH - 1 : 0]       s_frag_wxpos,
    input  wire [X_BIT_WIDTH - 1 : 0]       s_frag_wypos,

    /////////////////////////
    // Memory Interface
    /////////////////////////

    output wire [ID_WIDTH - 1 : 0]          m_mem_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]        m_mem_axi_awaddr,
    output wire [ 7 : 0]                    m_mem_axi_awlen, // How many beats are in this transaction
    output wire [ 2 : 0]                    m_mem_axi_awsize, // The increment during one cycle. Means, 0 incs addr by 1, 2 by 4 and so on
    output wire [ 1 : 0]                    m_mem_axi_awburst, // 0 fixed, 1 incr, 2 wrapping
    output wire                             m_mem_axi_awlock,
    output wire [ 3 : 0]                    m_mem_axi_awcache,
    output wire [ 2 : 0]                    m_mem_axi_awprot, 
    output wire                             m_mem_axi_awvalid,
    input  wire                             m_mem_axi_awready,

    output wire [DATA_WIDTH - 1 : 0]        m_mem_axi_wdata,
    output wire [STRB_WIDTH - 1 : 0]        m_mem_axi_wstrb,
    output wire                             m_mem_axi_wlast,
    output wire                             m_mem_axi_wvalid,
    input  wire                             m_mem_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]          m_mem_axi_bid,
    input  wire [ 1 : 0]                    m_mem_axi_bresp,
    input  wire                             m_mem_axi_bvalid,
    output wire                             m_mem_axi_bready,

    output wire [ID_WIDTH - 1 : 0]          m_mem_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_mem_axi_araddr,
    output wire [ 7 : 0]                    m_mem_axi_arlen,
    output wire [ 2 : 0]                    m_mem_axi_arsize,
    output wire [ 1 : 0]                    m_mem_axi_arburst,
    output wire                             m_mem_axi_arlock,
    output wire [ 3 : 0]                    m_mem_axi_arcache,
    output wire [ 2 : 0]                    m_mem_axi_arprot,
    output wire                             m_mem_axi_arvalid,
    input  wire                             m_mem_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]          m_mem_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]        m_mem_axi_rdata,
    input  wire [ 1 : 0]                    m_mem_axi_rresp,
    input  wire                             m_mem_axi_rlast,
    input  wire                             m_mem_axi_rvalid,
    output wire                             m_mem_axi_rready
);
    wire                             frag_tvalid;
    wire                             frag_tlast;
    wire                             frag_tready;
    wire [PIXEL_WIDTH - 1 : 0]       frag_tdata;
    wire                             frag_tstrb;
    wire [ADDR_WIDTH - 1 : 0]        frag_taddr;
    wire [X_BIT_WIDTH - 1 : 0]       frag_txpos;
    wire [X_BIT_WIDTH - 1 : 0]       frag_typos;

    wire                             scissor_tvalid;
    wire                             scissor_tlast;
    wire                             scissor_tready;
    wire [PIXEL_WIDTH - 1 : 0]       scissor_tdata;
    wire                             scissor_tstrb;
    wire [ADDR_WIDTH - 1 : 0]        scissor_taddr;

    wire                             strobegen_tvalid;
    wire                             strobegen_tlast;
    wire                             strobegen_tready;
    wire [PIXEL_WIDTH - 1 : 0]       strobegen_tdata;
    wire [PIXEL_MASK_WIDTH - 1 : 0]  strobegen_tstrb;
    wire [ADDR_WIDTH - 1 : 0]        strobegen_taddr;

    wire                             write_mmu_tvalid;
    wire                             write_mmu_tlast;
    wire                             write_mmu_tready;
    wire [PIXEL_WIDTH - 1 : 0]       write_mmu_tdata;
    wire [PIXEL_MASK_WIDTH - 1 : 0]  write_mmu_tstrb;
    wire [ADDR_WIDTH - 1 : 0]        write_mmu_taddr;

    wire                             fetch_mmu_tvalid;
    wire                             fetch_mmu_tlast;
    wire                             fetch_mmu_tready;
    wire [ADDR_WIDTH - 1 : 0]        fetch_mmu_taddr;

    // Internal wires between the FramebufferAdapter and the Coalescer
    wire [ID_WIDTH - 1 : 0]          coal_awid;
    wire [ADDR_WIDTH - 1 : 0]        coal_awaddr;
    wire [ 7 : 0]                    coal_awlen;
    wire [ 2 : 0]                    coal_awsize;
    wire [ 1 : 0]                    coal_awburst;
    wire                             coal_awlock;
    wire [ 3 : 0]                    coal_awcache;
    wire [ 2 : 0]                    coal_awprot;
    wire                             coal_awvalid;
    wire                             coal_awready;

    wire [DATA_WIDTH - 1 : 0]        coal_wdata;
    wire [STRB_WIDTH - 1 : 0]        coal_wstrb;
    wire                             coal_wlast;
    wire                             coal_wvalid;
    wire                             coal_wready;

    wire [ID_WIDTH - 1 : 0]          coal_bid;
    wire [ 1 : 0]                    coal_bresp;
    wire                             coal_bvalid;
    wire                             coal_bready;

    wire [ID_WIDTH - 1 : 0]          coal_arid;
    wire [ADDR_WIDTH - 1 : 0]        coal_araddr;
    wire [ 7 : 0]                    coal_arlen;
    wire [ 2 : 0]                    coal_arsize;
    wire [ 1 : 0]                    coal_arburst;
    wire                             coal_arlock;
    wire [ 3 : 0]                    coal_arcache;
    wire [ 2 : 0]                    coal_arprot;
    wire                             coal_arvalid;
    wire                             coal_arready;

    wire [ID_WIDTH - 1 : 0]          coal_rid;
    wire [DATA_WIDTH - 1 : 0]        coal_rdata;
    wire [ 1 : 0]                    coal_rresp;
    wire                             coal_rlast;
    wire                             coal_rvalid;
    wire                             coal_rready;

    FramebufferMMU #(
        .ADDR_WIDTH(ADDR_WIDTH),
        .PIXEL_WIDTH(PIXEL_WIDTH)
    ) fbmmu_fetch (
        .confAddr(confAddr),

        .s_frag_tvalid(s_fetch_arvalid),
        .s_frag_tlast(s_fetch_arlast),
        .s_frag_tready(s_fetch_arready),
        .s_frag_tdata(0), // Unused for read address path
        .s_frag_tstrb(0), // Unused for read address path
        .s_frag_taddr(s_fetch_araddr),

        .m_frag_tvalid(fetch_mmu_tvalid),
        .m_frag_tlast(fetch_mmu_tlast),
        .m_frag_tready(fetch_mmu_tready),
        .m_frag_tdata(), // Unused for read address path
        .m_frag_tstrb(), // Unused for read address path
        .m_frag_taddr(fetch_mmu_taddr)
    );

    FramebufferAdapter #(
        .DATA_WIDTH(DATA_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .STRB_WIDTH(STRB_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .PIXEL_WIDTH(PIXEL_WIDTH)
    ) fba (
        .aclk(aclk),
        .resetn(resetn),

        .s_fetch_arvalid(fetch_mmu_tvalid),
        .s_fetch_arlast(fetch_mmu_tlast),
        .s_fetch_arready(fetch_mmu_tready),
        .s_fetch_araddr(fetch_mmu_taddr),

        .s_frag_rvalid(s_frag_rvalid),
        .s_frag_rready(s_frag_rready),
        .s_frag_rdata(s_frag_rdata),
        .s_frag_rlast(s_frag_rlast),

        .s_frag_wvalid(write_mmu_tvalid),
        .s_frag_wlast(write_mmu_tlast),
        .s_frag_wready(write_mmu_tready),
        .s_frag_wdata(write_mmu_tdata),
        .s_frag_wstrb(write_mmu_tstrb),
        .s_frag_waddr(write_mmu_taddr),

        .m_mem_axi_arid(coal_arid),
        .m_mem_axi_araddr(coal_araddr),
        .m_mem_axi_arlen(coal_arlen),
        .m_mem_axi_arsize(coal_arsize),
        .m_mem_axi_arburst(coal_arburst),
        .m_mem_axi_arlock(coal_arlock),
        .m_mem_axi_arcache(coal_arcache),
        .m_mem_axi_arprot(coal_arprot),
        .m_mem_axi_arvalid(coal_arvalid),
        .m_mem_axi_arready(coal_arready),

        .m_mem_axi_rid(coal_rid),
        .m_mem_axi_rdata(coal_rdata),
        .m_mem_axi_rresp(coal_rresp),
        .m_mem_axi_rlast(coal_rlast),
        .m_mem_axi_rvalid(coal_rvalid),
        .m_mem_axi_rready(coal_rready),

        .m_mem_axi_awid(coal_awid),
        .m_mem_axi_awaddr(coal_awaddr),
        .m_mem_axi_awlen(coal_awlen),
        .m_mem_axi_awsize(coal_awsize),
        .m_mem_axi_awburst(coal_awburst),
        .m_mem_axi_awlock(coal_awlock),
        .m_mem_axi_awcache(coal_awcache),
        .m_mem_axi_awprot(coal_awprot),
        .m_mem_axi_awvalid(coal_awvalid),
        .m_mem_axi_awready(coal_awready),

        .m_mem_axi_wdata(coal_wdata),
        .m_mem_axi_wstrb(coal_wstrb),
        .m_mem_axi_wlast(coal_wlast),
        .m_mem_axi_wvalid(coal_wvalid),
        .m_mem_axi_wready(coal_wready),

        .m_mem_axi_bid(coal_bid),
        .m_mem_axi_bresp(coal_bresp),
        .m_mem_axi_bvalid(coal_bvalid),
        .m_mem_axi_bready(coal_bready)
    );

    Coalescer #(
        .ID_WIDTH(ID_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .DATA_WIDTH(DATA_WIDTH),
        .STRB_WIDTH(STRB_WIDTH),
        .MAX_BEATS_TO_COALESCE(MAX_BEATS_TO_COALESCE)
    ) coalescer (
        .aclk(aclk),
        .resetn(resetn),

        .s_mem_axi_awid(coal_awid),
        .s_mem_axi_awaddr(coal_awaddr),
        .s_mem_axi_awlen(coal_awlen),
        .s_mem_axi_awsize(coal_awsize),
        .s_mem_axi_awburst(coal_awburst),
        .s_mem_axi_awlock(coal_awlock),
        .s_mem_axi_awcache(coal_awcache),
        .s_mem_axi_awprot(coal_awprot),
        .s_mem_axi_awvalid(coal_awvalid),
        .s_mem_axi_awready(coal_awready),

        .s_mem_axi_wdata(coal_wdata),
        .s_mem_axi_wstrb(coal_wstrb),
        .s_mem_axi_wlast(coal_wlast),
        .s_mem_axi_wvalid(coal_wvalid),
        .s_mem_axi_wready(coal_wready),

        .s_mem_axi_bid(coal_bid),
        .s_mem_axi_bresp(coal_bresp),
        .s_mem_axi_bvalid(coal_bvalid),
        .s_mem_axi_bready(coal_bready),

        .s_mem_axi_arid(coal_arid),
        .s_mem_axi_araddr(coal_araddr),
        .s_mem_axi_arlen(coal_arlen),
        .s_mem_axi_arsize(coal_arsize),
        .s_mem_axi_arburst(coal_arburst),
        .s_mem_axi_arlock(coal_arlock),
        .s_mem_axi_arcache(coal_arcache),
        .s_mem_axi_arprot(coal_arprot),
        .s_mem_axi_arvalid(coal_arvalid),
        .s_mem_axi_arready(coal_arready),

        .s_mem_axi_rid(coal_rid),
        .s_mem_axi_rdata(coal_rdata),
        .s_mem_axi_rresp(coal_rresp),
        .s_mem_axi_rlast(coal_rlast),
        .s_mem_axi_rvalid(coal_rvalid),
        .s_mem_axi_rready(coal_rready),

        .m_mem_axi_awid(m_mem_axi_awid),
        .m_mem_axi_awaddr(m_mem_axi_awaddr),
        .m_mem_axi_awlen(m_mem_axi_awlen),
        .m_mem_axi_awsize(m_mem_axi_awsize),
        .m_mem_axi_awburst(m_mem_axi_awburst),
        .m_mem_axi_awlock(m_mem_axi_awlock),
        .m_mem_axi_awcache(m_mem_axi_awcache),
        .m_mem_axi_awprot(m_mem_axi_awprot),
        .m_mem_axi_awvalid(m_mem_axi_awvalid),
        .m_mem_axi_awready(m_mem_axi_awready),

        .m_mem_axi_wdata(m_mem_axi_wdata),
        .m_mem_axi_wstrb(m_mem_axi_wstrb),
        .m_mem_axi_wlast(m_mem_axi_wlast),
        .m_mem_axi_wvalid(m_mem_axi_wvalid),
        .m_mem_axi_wready(m_mem_axi_wready),

        .m_mem_axi_bid(m_mem_axi_bid),
        .m_mem_axi_bresp(m_mem_axi_bresp),
        .m_mem_axi_bvalid(m_mem_axi_bvalid),
        .m_mem_axi_bready(m_mem_axi_bready),

        .m_mem_axi_arid(m_mem_axi_arid),
        .m_mem_axi_araddr(m_mem_axi_araddr),
        .m_mem_axi_arlen(m_mem_axi_arlen),
        .m_mem_axi_arsize(m_mem_axi_arsize),
        .m_mem_axi_arburst(m_mem_axi_arburst),
        .m_mem_axi_arlock(m_mem_axi_arlock),
        .m_mem_axi_arcache(m_mem_axi_arcache),
        .m_mem_axi_arprot(m_mem_axi_arprot),
        .m_mem_axi_arvalid(m_mem_axi_arvalid),
        .m_mem_axi_arready(m_mem_axi_arready),

        .m_mem_axi_rid(m_mem_axi_rid),
        .m_mem_axi_rdata(m_mem_axi_rdata),
        .m_mem_axi_rresp(m_mem_axi_rresp),
        .m_mem_axi_rlast(m_mem_axi_rlast),
        .m_mem_axi_rvalid(m_mem_axi_rvalid),
        .m_mem_axi_rready(m_mem_axi_rready)
    );

    FramebufferWriterClear #(
        .ADDR_WIDTH(ADDR_WIDTH),
        .X_BIT_WIDTH(X_BIT_WIDTH),
        .Y_BIT_WIDTH(Y_BIT_WIDTH),
        .PIXEL_WIDTH(PIXEL_WIDTH)
    ) fbwc (
        .aclk(aclk),
        .resetn(resetn),

        .confClearColor(confClearColor),
        .confXResolution(confXResolution),
        .confYResolution(confYResolution),
        .confEnableScissor(confEnableScissor),
        .confScissorStartX(confScissorStartX),
        .confScissorStartY(confScissorStartY),
        .confScissorEndX(confScissorEndX),
        .confScissorEndY(confScissorEndY),

        .s_frag_tvalid(s_frag_wvalid),
        .s_frag_tlast(s_frag_wlast),
        .s_frag_tready(s_frag_wready),
        .s_frag_tdata(s_frag_wdata),
        .s_frag_tstrb(s_frag_wstrb),
        .s_frag_taddr(s_frag_waddr),
        .s_frag_txpos(s_frag_wxpos),
        .s_frag_typos(s_frag_wypos),

        .m_frag_tvalid(frag_tvalid),
        .m_frag_tlast(frag_tlast),
        .m_frag_tready(frag_tready),
        .m_frag_tdata(frag_tdata),
        .m_frag_tstrb(frag_tstrb),
        .m_frag_taddr(frag_taddr),
        .m_frag_txpos(frag_txpos),
        .m_frag_typos(frag_typos),

        .apply(apply),
        .applied(applied)
    );

    FramebufferScissor #(
        .X_BIT_WIDTH(X_BIT_WIDTH),
        .Y_BIT_WIDTH(Y_BIT_WIDTH),
        .PIXEL_WIDTH(PIXEL_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH)
    ) fbs (
        .confEnableScissor(confEnableScissor),
        .confScissorStartX(confScissorStartX),
        .confScissorStartY(confScissorStartY),
        .confScissorEndX(confScissorEndX),
        .confScissorEndY(confScissorEndY),

        .s_frag_tvalid(frag_tvalid),
        .s_frag_tlast(frag_tlast),
        .s_frag_tready(frag_tready),
        .s_frag_tdata(frag_tdata),
        .s_frag_tstrb(frag_tstrb),
        .s_frag_taddr(frag_taddr),
        .s_frag_txpos(frag_txpos),
        .s_frag_typos(frag_typos),

        .m_frag_tvalid(scissor_tvalid),
        .m_frag_tlast(scissor_tlast),
        .m_frag_tready(scissor_tready),
        .m_frag_tdata(scissor_tdata),
        .m_frag_tstrb(scissor_tstrb),
        .m_frag_taddr(scissor_taddr)
    );

    FramebufferWriterStrobeGen #(
        .MASK_WIDTH(PIXEL_MASK_WIDTH),
        .PIXEL_WIDTH(PIXEL_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH)
    ) fbwsg (
        .confMask(confMask),

        .s_frag_tvalid(scissor_tvalid),
        .s_frag_tlast(scissor_tlast),
        .s_frag_tready(scissor_tready),
        .s_frag_tdata(scissor_tdata),
        .s_frag_tstrb(scissor_tstrb),
        .s_frag_taddr(scissor_taddr),

        .m_frag_tvalid(strobegen_tvalid),
        .m_frag_tlast(strobegen_tlast),
        .m_frag_tready(strobegen_tready),
        .m_frag_tdata(strobegen_tdata),
        .m_frag_tstrb(strobegen_tstrb),
        .m_frag_taddr(strobegen_taddr)
    );

    FramebufferMMU #(
        .ADDR_WIDTH(ADDR_WIDTH),
        .PIXEL_WIDTH(PIXEL_WIDTH)
    ) fbmmu_write (
        .confAddr(confAddr),

        .s_frag_tvalid(strobegen_tvalid),
        .s_frag_tlast(strobegen_tlast),
        .s_frag_tready(strobegen_tready),
        .s_frag_tdata(strobegen_tdata),
        .s_frag_tstrb(strobegen_tstrb),
        .s_frag_taddr(strobegen_taddr),

        .m_frag_tvalid(write_mmu_tvalid),
        .m_frag_tlast(write_mmu_tlast),
        .m_frag_tready(write_mmu_tready),
        .m_frag_tdata(write_mmu_tdata),
        .m_frag_tstrb(write_mmu_tstrb),
        .m_frag_taddr(write_mmu_taddr)
    );

endmodule