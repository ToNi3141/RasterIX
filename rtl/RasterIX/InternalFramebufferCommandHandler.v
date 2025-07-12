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

module InternalFramebufferCommandHandler
#(
    // Number of pixels a stream beat contains
    parameter NUMBER_OF_PIXELS_PER_BEAT = 2,
    
    // Number of sub pixels the interface of this module contains
    parameter NUMBER_OF_SUB_PIXELS = 4,
    // Number of bits of each sub pixel contains
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
    input   wire                            aclk,
    input   wire                            resetn,

    /////////////////////////
    // Configs
    /////////////////////////
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
    // RAM interface
    /////////////////////////
    output wire [MEM_WIDTH - 1 : 0]         writeDataPort,
    output reg                              writeEnablePort,
    output wire [MEM_ADDR_WIDTH - 1 : 0]    writeAddrPort,
    output wire [MEM_MASK_WIDTH - 1 : 0]    writeMaskPort, 

    // Read interface port 1
    input  wire [MEM_WIDTH - 1 : 0]         readDataPort,
    output wire [MEM_ADDR_WIDTH - 1 : 0]    readAddrPort,
    
    /////////////////////////
    // Control
    /////////////////////////

    // Cmd interface
    input  wire                                 apply, // This start a command 
    output reg                                  applied, // This marks if the commands have been applied.
    input  wire                                 cmdCommit, // Starts to stream the memory content via the AXIS interface
    input  wire                                 cmdMemset, // Applies the confClearColor (with respect to the scissor) to the memory
    input  wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]   cmdSize, // Size of the stream 

    // AXI Stream master interface
    output reg                              m_axis_tvalid,
    input  wire                             m_axis_tready,
    output reg                              m_axis_tlast,
    output reg  [STREAM_WIDTH - 1 : 0]      m_axis_tdata
    
);
    // Stream states
    localparam COMMAND_WAIT_FOR_COMMAND = 0;
    localparam COMMAND_COMMIT = 1;
    localparam COMMAND_MEMSET = 2;
    
    // State variables
    reg  [MEM_ADDR_WIDTH - 1 : 0]   cmdIndex;
    wire [MEM_ADDR_WIDTH - 1 : 0]   cmdIndexNext = cmdIndex + 1;
    reg  [5 : 0]                    cmdState;
    reg  [MEM_ADDR_WIDTH - 1 : 0]   cmdFbSizeInBeats;
    
    // Scissor variables
    wire [X_BIT_WIDTH - 1 : 0]      scissorXNext = scissorX + NUMBER_OF_PIXELS_PER_BEAT;
    wire [Y_BIT_WIDTH - 1 : 0]      scissorYNext = scissorY - 1;
    reg  [X_BIT_WIDTH - 1 : 0]      scissorX;
    reg  [Y_BIT_WIDTH - 1 : 0]      scissorY;
    wire [MEM_MASK_WIDTH - 1 : 0]   scissorPixelAndColorMask;
    reg  [X_BIT_WIDTH - 1 : 0]      scissorStartX;
    reg  [Y_BIT_WIDTH - 1 : 0]      scissorStartY;
    reg  [X_BIT_WIDTH - 1 : 0]      scissorEndX;
    reg  [Y_BIT_WIDTH - 1 : 0]      scissorEndY;

    // Skid buffer
    reg  [STREAM_WIDTH - 1 : 0]     skidBufferData;
    reg                             skidBufferValid;
    reg                             skidBufferLast;

    assign writeDataPort = { NUMBER_OF_PIXELS_PER_BEAT { confClearColor } };
    assign writeMaskPort = { NUMBER_OF_PIXELS_PER_BEAT { confMask } } & scissorPixelAndColorMask;
    assign writeAddrPort = cmdIndex;

    assign readAddrPort = cmdIndex;
    wire readPortReady;
    wire readPortLast;

    ValueDelay #(
        .VALUE_SIZE(2),
        .DELAY(1)
    ) readDataPortDelay (
        .clk(aclk),
        .ce(1),
        .in({ cmdState == COMMAND_COMMIT, cmdIndexNext == cmdFbSizeInBeats }),
        .out({ readPortReady, readPortLast })
    );

    InternalFramebufferScissor #(
        .NUMBER_OF_PIXELS_PER_BEAT(NUMBER_OF_PIXELS_PER_BEAT),
        .NUMBER_OF_SUB_PIXELS(NUMBER_OF_SUB_PIXELS),
        .X_BIT_WIDTH(X_BIT_WIDTH),
        .Y_BIT_WIDTH(Y_BIT_WIDTH)
    ) scissorInst (
        .confEnableScissor(confEnableScissor),
        .confScissorStartX(confScissorStartX),
        .confScissorStartY(confScissorStartY),
        .confScissorEndX(confScissorEndX),
        .confScissorEndY(confScissorEndY),
        .x(scissorX),
        .y(scissorY),

        .pixelMask(scissorPixelAndColorMask)
    );

    // Command execution
    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            cmdState <= COMMAND_WAIT_FOR_COMMAND;
            writeEnablePort <= 0;
            applied <= 1;
            m_axis_tlast <= 0;
            m_axis_tvalid <= 0;
            skidBufferValid <= 0;
        end
        else
        begin
            case (cmdState)
            COMMAND_WAIT_FOR_COMMAND:
            begin : waitForCommand
                cmdIndex <= 0;
                scissorX <= 0;

                // Here is a mismatch between the RAM addresses and the OpenGL coordinate system.
                // OpenGL starts at the lower left corner. But this is a fairly high address in the RAM.
                // The cmdIndex starts at zero. This is basically in OpenGL the position (0, confYResolution - 1)
                scissorY <= confYOffset + confYResolution - 1;

                cmdFbSizeInBeats <= cmdSize[PIXEL_PER_BEAT_LOG2 +: MEM_ADDR_WIDTH];

                if (apply)
                begin
                    applied <= 0;

                    if (cmdMemset) 
                    begin
                        writeEnablePort <= 1;
                        scissorStartX <= confScissorStartX;
                        scissorStartY <= confScissorStartY;
                        scissorEndX <= confScissorEndX;
                        scissorEndY <= confScissorEndY;
                        cmdState <= COMMAND_MEMSET;
                    end

                    if (cmdCommit)
                    begin
                        writeEnablePort <= 0;
                        scissorStartX <= 0;
                        scissorStartY <= 0;
                        scissorEndX <= confXResolution;
                        scissorEndY <= confYResolution;
                        cmdState <= COMMAND_COMMIT;
                    end
                end
                else 
                begin
                    applied <= 1;
                end
            end
            COMMAND_COMMIT:
            begin
                if (m_axis_tready || !m_axis_tvalid)
                begin
                    m_axis_tvalid <= readPortReady;
                    if (!m_axis_tlast)
                    begin
                        if (skidBufferValid)
                        begin
                            m_axis_tdata <= skidBufferData;
                            m_axis_tlast <= skidBufferLast;
                            skidBufferValid <= 0;
                        end
                        else
                        begin
                            m_axis_tdata <= readDataPort;
                            m_axis_tlast <= readPortLast;
                        end

                        cmdIndex <= cmdIndexNext;
                        if (scissorXNext == confXResolution)
                        begin
                            scissorX <= 0;
                            scissorY <= scissorYNext;
                        end
                        else
                        begin
                            scissorX <= scissorXNext;
                        end
                    end

                    // Check if we reached the end of the copy process
                    if (m_axis_tlast)
                    begin
                        m_axis_tvalid <= 0;
                        m_axis_tlast <= 0;
 
                        cmdState <= COMMAND_WAIT_FOR_COMMAND;
                    end
                end
                else
                begin
                    if (readPortReady && !skidBufferValid)
                    begin
                        skidBufferData <= readDataPort;
                        skidBufferLast <= readPortLast;
                        skidBufferValid <= 1;
                    end
                end
            end
            COMMAND_MEMSET:
            begin
                if (cmdIndexNext == cmdFbSizeInBeats)
                begin
                    writeEnablePort <= 0;
                    cmdState <= COMMAND_WAIT_FOR_COMMAND;
                end
                cmdIndex <= cmdIndexNext;

                if (scissorXNext == confXResolution)
                begin
                    scissorX <= 0;
                    scissorY <= scissorYNext;
                end
                else
                begin
                    scissorX <= scissorXNext;
                end
            end
            endcase
        end
    end
endmodule

module InternalFramebufferScissor
#(
    // Number of pixels a stream beat contains
    parameter NUMBER_OF_PIXELS_PER_BEAT = 2,
    
    // Number of sub pixels the interface of this module contains
    parameter NUMBER_OF_SUB_PIXELS = 4,

    // The maximum size of the screen in power of two
    parameter X_BIT_WIDTH = 11,
    parameter Y_BIT_WIDTH = 11,

    localparam MASK_WIDTH = NUMBER_OF_PIXELS_PER_BEAT * NUMBER_OF_SUB_PIXELS
)
(
    input  wire                       confEnableScissor,
    input  wire [X_BIT_WIDTH - 1 : 0] confScissorStartX,
    input  wire [Y_BIT_WIDTH - 1 : 0] confScissorStartY,
    input  wire [X_BIT_WIDTH - 1 : 0] confScissorEndX,
    input  wire [Y_BIT_WIDTH - 1 : 0] confScissorEndY,
    input  wire [X_BIT_WIDTH - 1 : 0] x,
    input  wire [Y_BIT_WIDTH - 1 : 0] y,

    output wire [MASK_WIDTH - 1 : 0]  pixelMask
);
    `include "InternalFramebufferScissorFunc.vh"

    wire [NUMBER_OF_PIXELS_PER_BEAT - 1 : 0] scissorPixelMask;

    genvar i, j;
    generate
        if (NUMBER_OF_PIXELS_PER_BEAT == 1)
        begin 
            assign scissorPixelMask = scissorFunc(confEnableScissor, confScissorStartX, confScissorStartY, confScissorEndX, confScissorEndY, x, y);
            assign pixelMask = { NUMBER_OF_SUB_PIXELS { scissorPixelMask } };
        end
        else
        begin
            for (i = 0; i < NUMBER_OF_PIXELS_PER_BEAT; i = i + 1)
            begin
                assign scissorPixelMask[i] = scissorFunc(confEnableScissor, confScissorStartX, confScissorStartY, confScissorEndX, confScissorEndY, x + i, y);
                for (j = 0; j < NUMBER_OF_SUB_PIXELS; j = j + 1)
                begin
                    assign pixelMask[(i * NUMBER_OF_SUB_PIXELS) + j] = scissorPixelMask[i];
                end
            end
        end
    endgenerate

endmodule