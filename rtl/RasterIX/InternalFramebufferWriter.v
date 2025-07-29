// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2025 ToNi3141

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

// Serializes the data from the fragment interface, does a scissor test
// and writes it to the internal framebuffer memory.
module InternalFramebufferWriter
#(
    // Number of pixels a stream beat contains
    parameter NUMBER_OF_PIXELS_PER_BEAT = 1,
    
    // Number of sub pixels the interface of this module contains
    parameter NUMBER_OF_SUB_PIXELS = 4,
    // Number of bits of each sub pixel contains
    parameter SUB_PIXEL_WIDTH = 8,

    // The maximum size of the screen in power of two
    parameter X_BIT_WIDTH = 11,
    parameter Y_BIT_WIDTH = 11,
    parameter FRAMEBUFFER_SIZE_IN_PIXEL_LG = 18, // Framebuffer size in power of two words (PIXEL_WIDTH)

    // Size of the pixels
    localparam PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS * SUB_PIXEL_WIDTH,

    // Size of the internal memory
    localparam ADDR_WIDTH = FRAMEBUFFER_SIZE_IN_PIXEL_LG,

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
    input  wire                                 confEnable,
    input  wire                                 confEnableScissor,
    input  wire [X_BIT_WIDTH - 1 : 0]           confScissorStartX,
    input  wire [Y_BIT_WIDTH - 1 : 0]           confScissorStartY,
    input  wire [X_BIT_WIDTH - 1 : 0]           confScissorEndX,
    input  wire [Y_BIT_WIDTH - 1 : 0]           confScissorEndY,
    input  wire [NUMBER_OF_SUB_PIXELS - 1 : 0]  confMask,

    /////////////////////////
    // Fragment interface
    /////////////////////////
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
    // RAM interface
    /////////////////////////
    output wire [MEM_WIDTH - 1 : 0]         writeDataPort,
    output wire                             writeEnablePort,
    output wire [MEM_ADDR_WIDTH - 1 : 0]    writeAddrPort,
    output wire [MEM_MASK_WIDTH - 1 : 0]    writeMaskPort
    
);
    `include "InternalFramebufferScissorFunc.vh"

    // Interface to the memory when accessing a fragment. This interface has already the size required for the internal memory.
    wire [MEM_MASK_WIDTH - 1 : 0]   memMask = { NUMBER_OF_PIXELS_PER_BEAT { confMask } };
    wire                            memScissorTest = scissorFunc(confEnableScissor, confScissorStartX, confScissorStartY, confScissorEndX, confScissorEndY, wscreenPosX, wscreenPosY);
    
    // Fragment access including the scissor check
    genvar i, j;
    generate
        if (NUMBER_OF_PIXELS_PER_BEAT == 1)
        begin
            assign writeAddrPort = waddr;
            assign writeDataPort = wdata;
            assign writeMaskPort = memMask & { NUMBER_OF_SUB_PIXELS { memScissorTest } };
            assign writeEnablePort = wvalid && wstrb && confEnable;
        end
        else
        begin
            for (i = 0; i < NUMBER_OF_PIXELS_PER_BEAT; i = i + 1)
            begin
                for (j = 0; j < NUMBER_OF_SUB_PIXELS; j = j + 1)
                begin
                    assign writeMaskPort[(i * NUMBER_OF_SUB_PIXELS) + j] = (waddr[0 +: PIXEL_PER_BEAT_LOG2] == i) & memMask[j] & memScissorTest;
                end
            end
            assign writeAddrPort = waddr[PIXEL_PER_BEAT_LOG2 +: MEM_ADDR_WIDTH];
            assign writeDataPort = { NUMBER_OF_PIXELS_PER_BEAT { wdata } };
            assign writeEnablePort = wvalid && wstrb && confEnable;
        end
    endgenerate
endmodule
