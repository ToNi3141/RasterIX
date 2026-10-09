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

// The framebuffer is used to store the frame.
//
// This module uses an AXIS interface to stream out the framebuffer contents.
// To execute a command, set apply to 1. The framebuffer will then start sampling the command bits and starts executing.
// During execution, applied is set to 0. Is the execution finished, applied is set to 1.
// cmdCommit: This command will start streaming the content of the framebuffer via the AXIS interface.
// cmdRead: This command is used to stream the framebuffer content via the AXIS interface into this module (counterpart to cmdCommit).
//
// The fragment interface can be used to access single fragments from the framebuffer
//
// Pipelined: yes
// Performance: 1 pixel per cycle
// Depth: 2 cycle
module InternalFramebuffer
#(
    // Number of pixels a stream beat contains
    parameter NUMBER_OF_PIXELS_PER_BEAT = 1,
    
    // Number of sub pixels the interface of this module containts
    parameter NUMBER_OF_SUB_PIXELS = 4,
    // Number of bits of each sub pixel containts
    parameter SUB_PIXEL_WIDTH = 8,

    parameter FRAMEBUFFER_SIZE_IN_PIXEL_LG = 18, // Framebuffer size in power of two words (PIXEL_WIDTH)

    // The maximum size stream size
    parameter FB_SIZE_IN_PIXEL_LG = 20,

    // Address with
    parameter ADDR_WIDTH = 32,

    // Size of the pixels
    localparam PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS * SUB_PIXEL_WIDTH,

    // Size of the internal memory
    localparam PIXEL_ADDR_WIDTH = FRAMEBUFFER_SIZE_IN_PIXEL_LG,

    // Width of the AXIS interface with the frame buffer content
    localparam STREAM_WIDTH = NUMBER_OF_PIXELS_PER_BEAT * PIXEL_WIDTH,
    // Number of pixels a AXIS beat or a memory line can contain
    localparam PIXEL_PER_BEAT_LOG2 = $clog2(NUMBER_OF_PIXELS_PER_BEAT),

    // Size constrains of the internal memory
    localparam MEM_MASK_WIDTH = NUMBER_OF_PIXELS_PER_BEAT * NUMBER_OF_SUB_PIXELS,
    localparam MEM_WIDTH = MEM_MASK_WIDTH * SUB_PIXEL_WIDTH,
    localparam MEM_ADDR_WIDTH = PIXEL_ADDR_WIDTH - PIXEL_PER_BEAT_LOG2
)
(
    input   wire                                clk,
    input   wire                                reset,

    /////////////////////////
    // Configs
    /////////////////////////
    input  wire [NUMBER_OF_SUB_PIXELS - 1 : 0]  confMask,

    /////////////////////////
    // Fragment interface
    /////////////////////////

    // Output stream
    input  wire                                 arvalid,
    input  wire                                 arlast,
    // output wire                                 arready,
    input  wire [PIXEL_ADDR_WIDTH - 1 : 0]      araddr,

    output wire                                 rvalid,
    output wire                                 rlast,
    // input  wire                                 rready,
    output wire [PIXEL_WIDTH - 1 : 0]           rdata,

    // Input Stream 
    input  wire                                 wvalid,
    // input  wire                                 wlast,
    // output wire                                 wready,
    input  wire [PIXEL_ADDR_WIDTH - 1 : 0]      waddr,
    input  wire [PIXEL_WIDTH - 1 : 0]           wdata,
    input  wire                                 wstrb,
    /////////////////////////
    // Control
    /////////////////////////

    // Cmd interface
    input  wire                                 apply, // This start a command 
    output wire                                 applied, // This marks if the commands have been applied.
    input  wire                                 cmdCommit, // Starts to stream the memory content via the AXIS interface
    input  wire                                 cmdRead, // Reads the framebuffer content and streams it via the AXIS interface
    input  wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]   cmdSize, // Exact logical framebuffer size in 16-bit pixels.
    input  wire [ADDR_WIDTH - 1 : 0]            cmdAddr,

    // AXI Stream master interface
    output wire                                 m_axis_tvalid,
    input  wire                                 m_axis_tready,
    output wire                                 m_axis_tlast,
    output wire [STREAM_WIDTH - 1 : 0]          m_axis_tdata,
    output wire [NUMBER_OF_PIXELS_PER_BEAT - 1 : 0] m_axis_tstrb,

    input  wire                                 s_axis_tvalid,
    output wire                                 s_axis_tready,
    input  wire                                 s_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]          s_axis_tdata,

    output wire                                 m_avalid,
    output wire [ADDR_WIDTH - 1 : 0]            m_aaddr,
    output wire [ADDR_WIDTH - 1 : 0]            m_abeats,
    input  wire                                 m_aready,
    output wire                                 m_arnw
);
    wire [MEM_MASK_WIDTH - 1 : 0]   writeMaskPort1; 
    wire [MEM_ADDR_WIDTH - 1 : 0]   writeAddrPort1;
    wire [MEM_WIDTH - 1 : 0]        writeDataPort1; 
    wire                            writeEnablePort1;
    wire [MEM_ADDR_WIDTH - 1 : 0]   readAddrPort1;
    wire [MEM_WIDTH - 1 : 0]        readDataPort1;
    
    wire [NUMBER_OF_PIXELS_PER_BEAT - 1 : 0]  writePixelStrobePort0;
    wire [MEM_MASK_WIDTH - 1 : 0]             writeMaskPort0;
    wire [MEM_ADDR_WIDTH - 1 : 0]   writeAddrPort0;
    wire [MEM_WIDTH - 1 : 0]        writeDataPort0;
    wire                            writeEnablePort0;
    wire [MEM_ADDR_WIDTH - 1 : 0]   readAddrPort0;
    wire [MEM_WIDTH - 1 : 0]        readDataPort0;

    genvar pixelLane;
    generate
        for (pixelLane = 0; pixelLane < NUMBER_OF_PIXELS_PER_BEAT; pixelLane = pixelLane + 1)
        begin
            assign writeMaskPort0[(pixelLane * NUMBER_OF_SUB_PIXELS) +: NUMBER_OF_SUB_PIXELS] =
                { NUMBER_OF_SUB_PIXELS { writePixelStrobePort0[pixelLane] } };
        end
    endgenerate

    InternalFramebufferRam #(
        .ADDR_WIDTH(MEM_ADDR_WIDTH),
        .DATA_WIDTH(MEM_WIDTH),
        .WRITE_STROBE_WIDTH(SUB_PIXEL_WIDTH)
    ) internalFramebufferRam (
        .clk(clk),
        .reset(reset),

        .enablePort0(!applied),
        // Write interface port 1
        .writeDataPort0(writeDataPort0),
        .writeEnablePort0(writeEnablePort0),
        .writeAddrPort0(writeAddrPort0),
        .writeMaskPort0(writeMaskPort0),
        // Read interface port 1
        .readDataPort0(readDataPort0),
        .readAddrPort0(readAddrPort0),

        .enablePort1(applied), 
        // Write interface port 2
        .writeDataPort1(writeDataPort1), 
        .writeEnablePort1(writeEnablePort1), 
        .writeAddrPort1(writeAddrPort1), 
        .writeMaskPort1(writeMaskPort1), 
        // Read interface port 2
        .readDataPort1(readDataPort1),
        .readAddrPort1(readAddrPort1) 
    );

    InternalFramebufferReader #(
        .NUMBER_OF_PIXELS_PER_BEAT(NUMBER_OF_PIXELS_PER_BEAT),
        .PIXEL_WIDTH(PIXEL_WIDTH),
        .FRAMEBUFFER_SIZE_IN_PIXEL_LG(FRAMEBUFFER_SIZE_IN_PIXEL_LG)
    ) reader (
        .clk(clk),
        .reset(reset),

        // Fragment interface
        .arvalid(arvalid),
        .arlast(arlast),
        .araddr(araddr),

        .rvalid(rvalid),
        .rlast(rlast),
        .rdata(rdata),

        // RAM interface
        .readDataPort(readDataPort1),
        .readAddrPort(readAddrPort1)
    );

    InternalFramebufferWriter #(
        .NUMBER_OF_PIXELS_PER_BEAT(NUMBER_OF_PIXELS_PER_BEAT),
        .NUMBER_OF_SUB_PIXELS(NUMBER_OF_SUB_PIXELS),
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
        .FRAMEBUFFER_SIZE_IN_PIXEL_LG(FRAMEBUFFER_SIZE_IN_PIXEL_LG)
    ) writer (
        // Configs
        .confMask(confMask),

        // Fragment interface
        .wvalid(wvalid),
        .waddr(waddr),
        .wdata(wdata),
        .wstrb(wstrb),

        .writeDataPort(writeDataPort1),
        .writeEnablePort(writeEnablePort1),
        .writeAddrPort(writeAddrPort1),
        .writeMaskPort(writeMaskPort1)
    );

    InternalFramebufferCommandHandler #(
        .NUMBER_OF_PIXELS_PER_BEAT(NUMBER_OF_PIXELS_PER_BEAT),
        .PIXEL_WIDTH(PIXEL_WIDTH),
        .FRAMEBUFFER_SIZE_IN_PIXEL_LG(FRAMEBUFFER_SIZE_IN_PIXEL_LG),
        .FB_SIZE_IN_PIXEL_LG(FB_SIZE_IN_PIXEL_LG),
        .ADDR_WIDTH(ADDR_WIDTH)
    ) commandHandler (
        .aclk(clk),
        .resetn(!reset),

        // RAM interface
        .writeDataPort(writeDataPort0),
        .writeEnablePort(writeEnablePort0),
        .writeAddrPort(writeAddrPort0),
        .writeMaskPort(writePixelStrobePort0),

        .readDataPort(readDataPort0),
        .readAddrPort(readAddrPort0),

        // Fragment interface
        .apply(apply),
        .applied(applied),
        .cmdCommit(cmdCommit),
        .cmdRead(cmdRead),
        .cmdSize(cmdSize),
        .cmdAddr(cmdAddr),

        // AXI Stream master interface
        .m_axis_tvalid(m_axis_tvalid),
        .m_axis_tready(m_axis_tready),
        .m_axis_tlast(m_axis_tlast),
        .m_axis_tdata(m_axis_tdata),
        .m_axis_tstrb(m_axis_tstrb),

        .s_axis_tvalid(s_axis_tvalid),
        .s_axis_tready(s_axis_tready),
        .s_axis_tlast(s_axis_tlast),
        .s_axis_tdata(s_axis_tdata),

        .m_avalid(m_avalid),
        .m_aaddr(m_aaddr),
        .m_abeats(m_abeats),
        .m_aready(m_aready),
        .m_arnw(m_arnw)
    );

endmodule
