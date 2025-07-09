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
// cmdMemset: This command will initialize the memory with the color in confClearColor
//
// The fragment interface can be used to access single fragments from the framebuffer
//
// Improvements: The scrissor is not optimized. When memset is called, it will set the whole memory except
// the scissor area. A improved version will just set the scissor area.
// It is not implemented, because it requires additional logic for the index calculation (multiplier and so on).
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

    // The maximum size of the screen in power of two
    parameter X_BIT_WIDTH = 11,
    parameter Y_BIT_WIDTH = 11,
    parameter FRAMEBUFFER_SIZE_IN_PIXEL_LG = 18, // Framebuffer size in power of two words (PIXEL_WIDTH)

    // The maximum size stream size
    parameter FB_SIZE_IN_PIXEL_LG = 20,

    // Size of the pixels
    localparam PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS * SUB_PIXEL_WIDTH,

    // Size of the internal memory
    localparam ADDR_WIDTH = FRAMEBUFFER_SIZE_IN_PIXEL_LG,

    // Width of the AXIS interface with the frame buffer content
    localparam STREAM_WIDTH = NUMBER_OF_PIXELS_PER_BEAT * PIXEL_WIDTH,

    // Number of pixels a AXIS beat or a memory line can contain
    localparam PIXEL_PER_BEAT_LOG2 = $clog2(NUMBER_OF_PIXELS_PER_BEAT),

    // Size constrains of the internal memory
    localparam MEM_PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS * SUB_PIXEL_WIDTH,
    localparam MEM_MASK_WIDTH = NUMBER_OF_PIXELS_PER_BEAT * NUMBER_OF_SUB_PIXELS,
    localparam MEM_WIDTH = MEM_MASK_WIDTH * SUB_PIXEL_WIDTH,
    localparam MEM_ADDR_WIDTH = ADDR_WIDTH - PIXEL_PER_BEAT_LOG2
)
(
    input   wire                            clk,
    input   wire                            reset,

    /////////////////////////
    // Configs
    /////////////////////////
    input  wire                             confEnable,
    input  wire [PIXEL_WIDTH - 1 : 0]       confClearColor,
    input  wire                             confEnableScissor,
    input  wire [X_BIT_WIDTH - 1 : 0]       confScissorStartX,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confScissorStartY,
    input  wire [X_BIT_WIDTH - 1 : 0]       confScissorEndX,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confScissorEndY,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confYOffset,
    input  wire [X_BIT_WIDTH - 1 : 0]       confXResolution,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confYResolution,
    input  wire [NUMBER_OF_SUB_PIXELS - 1 : 0] confMask,

    /////////////////////////
    // Fragment interface
    /////////////////////////

    // Output stream
    input  wire                             arvalid,
    input  wire                             arlast,
    // output wire                             arready,
    input  wire [ADDR_WIDTH - 1 : 0]        araddr,

    output wire                             rvalid,
    output wire                             rlast,
    // input  wire                             rready,
    output wire [PIXEL_WIDTH - 1 : 0]       rdata,

    // Input Stream
    input  wire                             wvalid,
    // input  wire                             wlast,
    // output wire                             wready,
    input  wire [ADDR_WIDTH - 1 : 0]        waddr,
    input  wire [PIXEL_WIDTH - 1 : 0]       wdata,
    input  wire                             wstrb,
    input  wire [X_BIT_WIDTH - 1 : 0]       wscreenPosX,
    input  wire [Y_BIT_WIDTH - 1 : 0]       wscreenPosY,
    
    /////////////////////////
    // Control
    /////////////////////////

    // Cmd interface
    input  wire                                 apply, // This start a command 
    output wire                                 applied, // This marks if the commands have been applied.
    input  wire                                 cmdCommit, // Starts to stream the memory content via the AXIS interface
    input  wire                                 cmdMemset, // Applies the confClearColor (with respect to the scissor) to the memory
    input  wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]   cmdSize, // Size of the stream 

    // AXI Stream master interface
    output wire                             m_axis_tvalid,
    input  wire                             m_axis_tready,
    output wire                             m_axis_tlast,
    output wire [STREAM_WIDTH - 1 : 0]      m_axis_tdata
    
);
    wire [MEM_MASK_WIDTH - 1 : 0]   writeMaskPort1; 
    wire [MEM_ADDR_WIDTH - 1 : 0]   writeAddrPort1;
    wire [MEM_WIDTH - 1 : 0]        writeDataPort1; 
    wire                            writeEnablePort1;
    wire [MEM_ADDR_WIDTH - 1 : 0]   readAddrPort1;
    wire [MEM_WIDTH - 1 : 0]        readDataPort1;
    
    wire [MEM_MASK_WIDTH - 1 : 0]   writeMaskPort0; 
    wire [MEM_ADDR_WIDTH - 1 : 0]   writeAddrPort0;
    wire [MEM_WIDTH - 1 : 0]        writeDataPort0;
    wire                            writeEnablePort0;
    wire [MEM_ADDR_WIDTH - 1 : 0]   readAddrPort0;
    wire [MEM_WIDTH - 1 : 0]        readDataPort0;

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
        .NUMBER_OF_SUB_PIXELS(NUMBER_OF_SUB_PIXELS),
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
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
        .X_BIT_WIDTH(X_BIT_WIDTH),
        .Y_BIT_WIDTH(Y_BIT_WIDTH),
        .FRAMEBUFFER_SIZE_IN_PIXEL_LG(FRAMEBUFFER_SIZE_IN_PIXEL_LG)
    ) writer (
        .clk(clk),
        .reset(reset),

        // Configs
        .confEnable(confEnable),
        .confEnableScissor(confEnableScissor),
        .confScissorStartX(confScissorStartX),
        .confScissorStartY(confScissorStartY),
        .confScissorEndX(confScissorEndX),
        .confScissorEndY(confScissorEndY),
        .confMask(confMask),

        // Fragment interface
        .wvalid(wvalid),
        .waddr(waddr),
        .wdata(wdata),
        .wstrb(wstrb),
        .wscreenPosX(wscreenPosX),
        .wscreenPosY(wscreenPosY),

        .writeDataPort(writeDataPort1),
        .writeEnablePort(writeEnablePort1),
        .writeAddrPort(writeAddrPort1),
        .writeMaskPort(writeMaskPort1)
    );

    InternalFramebufferCommandHandler #(
        .NUMBER_OF_PIXELS_PER_BEAT(NUMBER_OF_PIXELS_PER_BEAT),
        .NUMBER_OF_SUB_PIXELS(NUMBER_OF_SUB_PIXELS),
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
        .X_BIT_WIDTH(X_BIT_WIDTH),
        .Y_BIT_WIDTH(Y_BIT_WIDTH),
        .FRAMEBUFFER_SIZE_IN_PIXEL_LG(FRAMEBUFFER_SIZE_IN_PIXEL_LG),
        .FB_SIZE_IN_PIXEL_LG(FB_SIZE_IN_PIXEL_LG)
    ) commandHandler (
        .clk(clk),
        .reset(reset),

        // Configs
        .confClearColor(confClearColor),
        .confEnableScissor(confEnableScissor),
        .confScissorStartX(confScissorStartX),
        .confScissorStartY(confScissorStartY),
        .confScissorEndX(confScissorEndX),
        .confScissorEndY(confScissorEndY),
        .confYOffset(confYOffset),
        .confXResolution(confXResolution),
        .confYResolution(confYResolution),
        .confMask(confMask),

        // RAM interface
        .writeDataPort(writeDataPort0),
        .writeEnablePort(writeEnablePort0),
        .writeAddrPort(writeAddrPort0),
        .writeMaskPort(writeMaskPort0),

        .readDataPort(readDataPort0),
        .readAddrPort(readAddrPort0),

        // Fragment interface
        .apply(apply),
        .applied(applied),
        .cmdCommit(cmdCommit),
        .cmdMemset(cmdMemset),
        .cmdSize(cmdSize),

        // AXI Stream master interface
        .m_axis_tvalid(m_axis_tvalid),
        .m_axis_tready(m_axis_tready),
        .m_axis_tlast(m_axis_tlast),
        .m_axis_tdata(m_axis_tdata)
    );
endmodule
