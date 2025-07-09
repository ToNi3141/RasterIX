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
    parameter NUMBER_OF_PIXELS_PER_BEAT = 1,
    
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
    input   wire                            clk,
    input   wire                            reset,

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
    output wire [STREAM_WIDTH - 1 : 0]      m_axis_tdata
    
);
    // Stream states
    localparam COMMAND_WAIT_FOR_COMMAND = 0;
    localparam COMMAND_MEMCPY = 1;
    localparam COMMAND_MEMSET = 2;

    // Scissor function
    `include "InternalFramebufferScissorFunc.vh"
    
    // State variables for managing the memory (memset and memcpy)
    reg  [MEM_ADDR_WIDTH - 1 : 0]   cmdIndex;
    wire [MEM_ADDR_WIDTH - 1 : 0]   cmdIndexNext = cmdIndex + 1;
    reg  [5 : 0]                    cmdState;
    wire [MEM_MASK_WIDTH - 1 : 0]   cmdMask = { NUMBER_OF_PIXELS_PER_BEAT { confMask } };
    reg  [MEM_ADDR_WIDTH - 1 : 0]   cmdFbSizeInBeats;
    
    // State variables for the memset
    reg  [X_BIT_WIDTH - 1 : 0]      cmdMemsetX;
    wire [X_BIT_WIDTH - 1 : 0]      cmdMemsetXNext = cmdMemsetX + NUMBER_OF_PIXELS_PER_BEAT;
    reg  [Y_BIT_WIDTH - 1 : 0]      cmdMemsetY;
    wire [Y_BIT_WIDTH - 1 : 0]      cmdMemsetYNext = cmdMemsetY - 1;
    wire [MEM_MASK_WIDTH - 1 : 0]   cmdMemsetScissorMask;
    wire [NUMBER_OF_PIXELS_PER_BEAT - 1 : 0]   cmdMemsetScissor;
    reg                             cmdMemsetPending = 0;

    assign writeDataPort = { NUMBER_OF_PIXELS_PER_BEAT { confClearColor } };
    assign writeMaskPort = cmdMask & cmdMemsetScissorMask;
    assign writeAddrPort = cmdIndex;
    assign readAddrPort = (m_axis_tready && (cmdState == COMMAND_MEMCPY)) ? cmdIndexNext : cmdIndex;
    assign m_axis_tdata = readDataPort;

    // Scissor check for the memset command 
    genvar i, j;
    generate
        if (NUMBER_OF_PIXELS_PER_BEAT == 1)
        begin 
            assign cmdMemsetScissor = scissorFunc(confEnableScissor, confScissorStartX, confScissorStartY, confScissorEndX, confScissorEndY, cmdMemsetX, cmdMemsetY);
            assign cmdMemsetScissorMask = { NUMBER_OF_SUB_PIXELS { cmdMemsetScissor } };
        end
        else
        begin
            for (i = 0; i < NUMBER_OF_PIXELS_PER_BEAT; i = i + 1)
            begin
                assign cmdMemsetScissor[i] = scissorFunc(confEnableScissor, confScissorStartX, confScissorStartY, confScissorEndX, confScissorEndY, cmdMemsetX + i, cmdMemsetY);
                for (j = 0; j < NUMBER_OF_SUB_PIXELS; j = j + 1)
                begin
                    assign cmdMemsetScissorMask[(i * NUMBER_OF_SUB_PIXELS) + j] = cmdMemsetScissor[i];
                end
            end
        end
    endgenerate

    // Command execution
    always @(posedge clk)
    begin
        if (reset)
        begin
            cmdState <= COMMAND_WAIT_FOR_COMMAND;
            m_axis_tvalid <= 0;
            m_axis_tlast <= 0;
            writeEnablePort <= 0;
            applied <= 1;
        end
        else
        begin
            case (cmdState)
            COMMAND_WAIT_FOR_COMMAND:
            begin : waitForCommand
                cmdIndex <= 0;
                cmdMemsetX <= 0;

                // Here is a mismatch between the RAM addresses and the OpenGL coordinate system.
                // OpenGL starts at the lower left corner. But this is a fairly high address in the RAM.
                // The cmdIndex starts at zero. This is basically in OpenGL the position (0, confYResolution - 1)
                cmdMemsetY <= confYOffset + confYResolution - 1;

                cmdFbSizeInBeats <= cmdSize[PIXEL_PER_BEAT_LOG2 +: MEM_ADDR_WIDTH];

                if (apply)
                begin
                    applied <= 0;

                    cmdMemsetPending <= cmdMemset;

                    if (cmdMemset) 
                    begin
                        writeEnablePort <= 1;
                        cmdState <= COMMAND_MEMSET;
                    end

                    // Commits have priority over a clear.
                    // When both are activated, the user probably wants first to commit and then to clear it.
                    if (cmdCommit)
                    begin
                        writeEnablePort <= 0;
                        m_axis_tvalid <= 1;
                        cmdState <= COMMAND_MEMCPY;
                    end
                end
                else 
                begin
                    applied <= 1;
                end
            end
            COMMAND_MEMCPY:
            begin
                if (m_axis_tready)
                begin
                    cmdIndex <= cmdIndexNext;
                
                    if (cmdIndexNext == (cmdFbSizeInBeats - 1))
                    begin
                        m_axis_tlast <= 1;
                    end

                    // Check if we reached the end of the copy process
                    if (cmdIndexNext == cmdFbSizeInBeats)
                    begin
                        m_axis_tvalid <= 0; 
                        m_axis_tlast <= 0;

                        // Continue with memset if it is activated
                        if (cmdMemsetPending) 
                        begin
                            cmdIndex <= 0;
                            writeEnablePort <= 1;
                            cmdState <= COMMAND_MEMSET;
                        end
                        else
                        begin
                            cmdState <= COMMAND_WAIT_FOR_COMMAND;
                        end
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

                if (cmdMemsetXNext == confXResolution)
                begin
                    cmdMemsetX <= 0;
                    cmdMemsetY <= cmdMemsetYNext;
                end
                else
                begin
                    cmdMemsetX <= cmdMemsetXNext;
                end
            end
            endcase
        end
    end
endmodule
