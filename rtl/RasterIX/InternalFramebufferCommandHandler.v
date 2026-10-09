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

// Handles the internal framebuffer commands.
// It handles framebuffer commit and read commands.
module InternalFramebufferCommandHandler
#(
    // Number of pixels a stream beat contains
    parameter NUMBER_OF_PIXELS_PER_BEAT = 2,
    
    // Width of one framebuffer pixel, independent of its channel layout
    parameter PIXEL_WIDTH = 32,

    parameter FRAMEBUFFER_SIZE_IN_PIXEL_LG = 18, // Framebuffer size in power of two words (PIXEL_WIDTH)

    // The maximum size stream size
    parameter FB_SIZE_IN_PIXEL_LG = 20,

    // Address width
    parameter ADDR_WIDTH = 32,

    // Size of the internal memory
    localparam INT_MEM_SIZE = FRAMEBUFFER_SIZE_IN_PIXEL_LG,

    // Width of the AXIS interface with the frame buffer content
    localparam STREAM_WIDTH = NUMBER_OF_PIXELS_PER_BEAT * PIXEL_WIDTH,

    // Number of pixels a AXIS beat or a memory line can contain
    localparam PIXEL_PER_BEAT_LOG2 = $clog2(NUMBER_OF_PIXELS_PER_BEAT),
    // Keep aligned with AxisToAxiAdapter's fixed AWLEN of 15 (16 beats per burst).
    localparam AXI_BURST_BEATS = 16,
    localparam AXI_BURST_PIXEL_COUNT = NUMBER_OF_PIXELS_PER_BEAT * AXI_BURST_BEATS,
    localparam TRANSFER_SIZE_WIDTH = FB_SIZE_IN_PIXEL_LG + 1,
    // Size constrains of the internal memory
    // One strobe bit per logical pixel in each stream/memory beat.
    localparam PIXEL_STROBE_WIDTH = NUMBER_OF_PIXELS_PER_BEAT,
    localparam MEM_WIDTH = STREAM_WIDTH,
    localparam MEM_ADDR_WIDTH = INT_MEM_SIZE - PIXEL_PER_BEAT_LOG2
)
(
    input   wire                            aclk,
    input   wire                            resetn,

    /////////////////////////
    // RAM interface
    /////////////////////////
    output reg  [MEM_WIDTH - 1 : 0]         writeDataPort,
    output reg                              writeEnablePort,
    output wire [MEM_ADDR_WIDTH - 1 : 0]    writeAddrPort,
    output reg  [PIXEL_STROBE_WIDTH - 1 : 0] writeMaskPort,

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
    input  wire                                 cmdRead, // Starts to read the memory content via the AXIS interface
    input  wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]   cmdSize, // Exact logical framebuffer size in 16-bit pixels.
    input  wire [ADDR_WIDTH - 1 : 0]            cmdAddr,

    // AXI Stream master interface
    output reg                              m_axis_tvalid,
    input  wire                             m_axis_tready,
    output reg                              m_axis_tlast,
    output reg  [STREAM_WIDTH - 1 : 0]      m_axis_tdata,
    output reg  [PIXEL_STROBE_WIDTH - 1 : 0] m_axis_tstrb,

    input  wire                             s_axis_tvalid,
    output reg                              s_axis_tready,
    input  wire                             s_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]      s_axis_tdata,

    output reg                              m_avalid,
    output reg  [ADDR_WIDTH - 1 : 0]        m_aaddr,
    output reg  [ADDR_WIDTH - 1 : 0]        m_abeats,
    input  wire                             m_aready,
    output reg                              m_arnw // 0 = read, 1 = write
);
    // Stream states
    localparam COMMAND_WAIT_FOR_COMMAND = 0;
    localparam COMMAND_COMMIT = 1;
    localparam COMMAND_READ = 2;
    
    // Memory Address variables
    reg  [MEM_ADDR_WIDTH - 1 : 0]   cmdMemAddr;
    wire [MEM_ADDR_WIDTH - 1 : 0]   cmdMemAddrNext = cmdMemAddr + 1;
    wire [TRANSFER_SIZE_WIDTH - 1 : 0] cmdMemAddrNextExtended =
        { { (TRANSFER_SIZE_WIDTH - MEM_ADDR_WIDTH) { 1'b0 } }, cmdMemAddrNext };
    reg  [TRANSFER_SIZE_WIDTH - 1 : 0] cmdFbSizeInBeats;
    
    // State variables
    reg  [1 : 0]                    cmdState;
    
    // AXI writes use fixed 16-beat bursts; keep that padding internal to the IF transfer.
    wire [TRANSFER_SIZE_WIDTH - 1 : 0] roundedCmdSize =
        ({ 1'b0, cmdSize } + AXI_BURST_PIXEL_COUNT - 1) & ~(AXI_BURST_PIXEL_COUNT - 1);
    wire [TRANSFER_SIZE_WIDTH - 1 : 0] cmdTransferBeats = roundedCmdSize >> PIXEL_PER_BEAT_LOG2;

    // Pixel offsets preserve the exact logical size despite padded transport beats.
    reg  [FB_SIZE_IN_PIXEL_LG : 0]  pixelOffset;
    wire [FB_SIZE_IN_PIXEL_LG : 0]  pixelOffsetNext = pixelOffset + NUMBER_OF_PIXELS_PER_BEAT;
    wire [PIXEL_STROBE_WIDTH - 1 : 0] pixelStrobe;

    // Skid buffer
    reg  [STREAM_WIDTH - 1 : 0]     skidBufferData;
    reg                             skidBufferValid;
    reg                             skidBufferLast;
    reg  [PIXEL_STROBE_WIDTH - 1 : 0] skidBufferStrb;

    assign writeAddrPort = cmdMemAddr;

    assign readAddrPort = cmdMemAddr;
    wire                            readPortReady;
    wire                            readPortLast;
    wire [PIXEL_STROBE_WIDTH - 1 : 0] readPortStrb;
    wire                            commitActive = cmdState == COMMAND_COMMIT;

    genvar i;
    generate
        for (i = 0; i < NUMBER_OF_PIXELS_PER_BEAT; i = i + 1)
        begin
            assign pixelStrobe[i] = (pixelOffset + i) < cmdSize;
        end
    endgenerate

    ValueDelay #(
        .VALUE_SIZE(2 + PIXEL_STROBE_WIDTH),
        .DELAY(1)
    ) readDataPortDelay (
        .clk(aclk),
        .ce(1),
        .in({ commitActive, cmdMemAddrNextExtended == cmdFbSizeInBeats, pixelStrobe }),
        .out({ readPortReady, readPortLast, readPortStrb })
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
            m_avalid <= 0;
            s_axis_tready <= 0;
        end
        else
        begin
            case (cmdState)
            COMMAND_WAIT_FOR_COMMAND:
            begin : waitForCommand
                cmdMemAddr <= 0;
                pixelOffset <= 0;
                writeEnablePort <= 0;

                cmdFbSizeInBeats <= cmdTransferBeats;

                if (apply && !m_avalid)
                begin
                    applied <= 0;

                    m_abeats <= { { (ADDR_WIDTH - TRANSFER_SIZE_WIDTH) { 1'b0 } }, cmdTransferBeats };
                    m_aaddr <= cmdAddr;

                    if (cmdCommit)
                    begin
                        m_avalid <= 1;
                        m_arnw <= 1;
                        cmdState <= COMMAND_COMMIT;
                    end

                    if (cmdRead)
                    begin
                        s_axis_tready <= 1;
                        m_avalid <= 1;
                        m_arnw <= 0;

                        cmdState <= COMMAND_READ;
                    end
                end
                else 
                begin
                    if (!m_avalid)
                    begin
                        applied <= 1;
                    end
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
                            m_axis_tstrb <= skidBufferStrb;
                            m_axis_tlast <= skidBufferLast;
                            skidBufferValid <= 0;
                        end
                        else
                        begin
                            m_axis_tdata <= readDataPort;
                            m_axis_tstrb <= readPortStrb;
                            m_axis_tlast <= readPortLast;
                        end

                        cmdMemAddr <= cmdMemAddrNext;
                        pixelOffset <= pixelOffsetNext;
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
                        skidBufferStrb <= readPortStrb;
                        skidBufferLast <= readPortLast;
                        skidBufferValid <= 1;
                    end
                end
            end
            COMMAND_READ:
            begin
                if (s_axis_tvalid && s_axis_tready)
                begin
                    if (s_axis_tlast)
                    begin
                        s_axis_tready <= 0;
                    end
                    if (writeEnablePort)
                    begin
                        cmdMemAddr <= cmdMemAddrNext;
                    end
                    writeMaskPort <= pixelStrobe;
                    writeDataPort <= s_axis_tdata;
                    writeEnablePort <= 1;
                    pixelOffset <= pixelOffsetNext;
                end
                if (!s_axis_tready)
                begin
                    writeEnablePort <= 0;
                    cmdState <= COMMAND_WAIT_FOR_COMMAND;
                end
            end
            endcase
        end
        if (m_avalid && m_aready)
        begin
            m_avalid <= 0;
        end
    end
endmodule
