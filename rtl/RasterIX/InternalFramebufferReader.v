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


module InternalFramebufferReader
#(
    // Number of pixels a stream beat contains
    parameter NUMBER_OF_PIXELS_PER_BEAT = 1,
    
    // Number of sub pixels the interface of this module contains
    parameter NUMBER_OF_SUB_PIXELS = 4,
    // Number of bits of each sub pixel contains
    parameter SUB_PIXEL_WIDTH = 8,

    // The maximum size of the screen in power of two
    parameter FRAMEBUFFER_SIZE_IN_PIXEL_LG = 18, // Framebuffer size in power of two words (PIXEL_WIDTH)
    localparam ADDR_WIDTH = FRAMEBUFFER_SIZE_IN_PIXEL_LG,

    // Size of the pixels
    localparam PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS * SUB_PIXEL_WIDTH,

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
    // Fragment interface
    /////////////////////////

    // Output stream
    input  wire                             arvalid,
    input  wire                             arlast,
    // output wire                             arready,
    input  wire [ADDR_WIDTH - 1 : 0]        araddr,

    output reg                              rvalid,
    output reg                              rlast,
    // input  wire                             rready,
    output reg  [PIXEL_WIDTH - 1 : 0]       rdata,

    /////////////////////////
    // RAM interface
    /////////////////////////
    input  wire [MEM_WIDTH - 1 : 0]         readDataPort,
    output wire [MEM_ADDR_WIDTH - 1 : 0]    readAddrPort
);
    reg  [ADDR_WIDTH - 1 : 0]   memAddrReadDelay;
    wire [PIXEL_WIDTH - 1 : 0]  memDataOut;
    reg                         rvalidDelay;
    reg                         rlastDelay;
    
    generate
        if (NUMBER_OF_PIXELS_PER_BEAT == 1)
        begin
            assign readAddrPort = araddr;
            assign memDataOut = readDataPort;
        end
        else
        begin
            assign readAddrPort = araddr[PIXEL_PER_BEAT_LOG2 +: MEM_ADDR_WIDTH];
            assign memDataOut = readDataPort[memAddrReadDelay[0 +: PIXEL_PER_BEAT_LOG2] * MEM_PIXEL_WIDTH +: MEM_PIXEL_WIDTH];
        end
    endgenerate

    always @(posedge clk)
    begin
        memAddrReadDelay <= araddr;
        rlastDelay <= arlast;
        rvalidDelay <= arvalid;

        rdata <= memDataOut;
        rvalid <= rvalidDelay;
        rlast <= rlastDelay;
    end
endmodule
