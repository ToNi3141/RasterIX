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

module CommandParser #(
    parameter CMD_STREAM_WIDTH = 32,
    parameter TEXTURE_STREAM_WIDTH = 32,
    parameter FB_SIZE_IN_PIXEL_LG = 20
) (
    input  wire         aclk,
    input  wire         resetn,

    // AXI Stream command interface
    input  wire         s_cmd_axis_tvalid,
    output reg          s_cmd_axis_tready,
    input  wire         s_cmd_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  s_cmd_axis_tdata,

    output reg  [CMD_STREAM_WIDTH - 1 : 0]  m_cmd_xxx_axis_tdata,
    output reg  [ 4 : 0]    m_cmd_xxx_axis_tuser,
    output reg              m_cmd_xxx_axis_tlast,
    output wire             m_cmd_fog_axis_tvalid,
    output wire             m_cmd_rasterizer_axis_tvalid,
    output wire             m_cmd_tmu0_axis_tvalid,
    output wire             m_cmd_tmu1_axis_tvalid,
    output wire             m_cmd_config_axis_tvalid,
    input  wire             m_cmd_fog_axis_tready,
    input  wire             m_cmd_rasterizer_axis_tready,
    input  wire             m_cmd_tmu0_axis_tready,
    input  wire             m_cmd_tmu1_axis_tready,
    input  wire             m_cmd_config_axis_tready,

    // Control
    input  wire             rasterizerRunning,
    input  wire             pixelInPipeline,
    input  wire             dataInTriangleInterpolator,

    // Color/Depth buffer control
    output reg                                  colorBufferApply,
    input  wire                                 colorBufferApplied,
    output reg                                  colorBufferCmdCommit,
    output reg                                  colorBufferCmdMemset,
    output reg                                  colorBufferCmdSwap,
    output reg                                  colorBufferCmdRead,
    output reg                                  colorBufferCmdSwapEnableVsync,
    output reg  [FB_SIZE_IN_PIXEL_LG - 1 : 0]   colorBufferSize,
    output reg                                  depthBufferApply,
    input  wire                                 depthBufferApplied,
    output reg                                  depthBufferCmdCommit,
    output reg                                  depthBufferCmdMemset,
    output reg                                  depthBufferCmdRead,
    output reg  [FB_SIZE_IN_PIXEL_LG - 1 : 0]   depthBufferSize,
    output reg                                  stencilBufferApply,
    input  wire                                 stencilBufferApplied,
    output reg                                  stencilBufferCmdCommit,
    output reg                                  stencilBufferCmdMemset,
    output reg                                  stencilBufferCmdRead,
    output reg  [FB_SIZE_IN_PIXEL_LG - 1 : 0]   stencilBufferSize
);
`include "RegisterAndDescriptorDefines.vh"
    localparam DATABUS_SCALE_FACTOR = (CMD_STREAM_WIDTH / 8);
    localparam DATABUS_SCALE_FACTOR_LOG2 = $clog2(DATABUS_SCALE_FACTOR);

    // Command Interface Statemachine
    localparam WAIT_FOR_IDLE = 5'd0;
    localparam COMMAND_IN = 5'd1;
    localparam EXEC_STREAM = 5'd2;

    localparam MUX_NONE = 5'd0;
    localparam MUX_TRIANGLE_STREAM = 5'd1;
    localparam MUX_TEXTURE0_STREAM = 5'd2;
    localparam MUX_TEXTURE1_STREAM = 5'd3;
    localparam MUX_RENDER_CONFIG = 5'd4;
    localparam MUX_FOG_LUT_STREAM = 5'd5;

    // Wait For Cache Apply Statemachine
    localparam FB_CONTROL_WAITFORCOMMAND = 0;
    localparam FB_CONTROL_WAITFOREND = 1;

    initial
    begin
        if (FB_SIZE_IN_PIXEL_LG != OP_FRAMEBUFFER_SIZE_SIZE)
        begin
            $error("The command size on the OP_FRAMEBUFFER differs from the FB_SIZE_IN_PIXEL_LG. FB_SIZE_IN_PIXEL_LG must be smaller or equal.");
        end
    end

    // Command Unit Variables
    reg             framebufferCommandApply;
    wire            framebufferCommandApplied;
    reg             rasterizerIsStarted;
    reg  [ 1 : 0]   fbControlState;

    // Local Statemachine variables
    reg  [ 4 : 0]   state;
    reg  [ 4 : 0]   mux;
    reg             tvalid;
    reg  [18 : 0]   streamCounter;

    // Skid buffer
    reg  [CMD_STREAM_WIDTH - 1 : 0] tdataSkid;
    reg                             tvalidSkid;

    assign m_cmd_fog_axis_tvalid         = (mux == MUX_FOG_LUT_STREAM) ? tvalid : 0;
    assign m_cmd_rasterizer_axis_tvalid  = (mux == MUX_TRIANGLE_STREAM) ? tvalid : 0;
    assign m_cmd_tmu0_axis_tvalid        = (mux == MUX_TEXTURE0_STREAM) ? tvalid : 0;
    assign m_cmd_tmu1_axis_tvalid        = (mux == MUX_TEXTURE1_STREAM) ? tvalid : 0;
    assign m_cmd_config_axis_tvalid      = (mux == MUX_RENDER_CONFIG) ? tvalid : 0;

    assign framebufferCommandApplied = colorBufferApplied & depthBufferApplied & stencilBufferApplied;

    always @(posedge aclk)
    begin : CmdParser
        reg tready;
        case (mux)
            MUX_FOG_LUT_STREAM: tready = m_cmd_fog_axis_tready;
            MUX_RENDER_CONFIG: tready = m_cmd_config_axis_tready;
            MUX_TEXTURE0_STREAM: tready = m_cmd_tmu0_axis_tready;
            MUX_TEXTURE1_STREAM: tready = m_cmd_tmu1_axis_tready;
            MUX_TRIANGLE_STREAM: tready = m_cmd_rasterizer_axis_tready;
            MUX_NONE: tready = 1;
        endcase 

        if (!resetn)
        begin
            state <= WAIT_FOR_IDLE;
            mux <= MUX_NONE;

            fbControlState <= FB_CONTROL_WAITFORCOMMAND;
            framebufferCommandApply <= 0;
            s_cmd_axis_tready <= 0;

            tvalid <= 0;
            m_cmd_xxx_axis_tlast <= 0;

            tvalidSkid <= 0;

            rasterizerIsStarted <= 1;
        end
        else 
        begin
            case (state)
            WAIT_FOR_IDLE:
            begin : WaitForIdle
                reg rasterizationFinished;
                reg framebufferFinished;
                reg tmuFinished;
                reg triangleFastPath;

                rasterizationFinished = !rasterizerRunning && rasterizerIsStarted;
                framebufferFinished = !framebufferCommandApply && framebufferCommandApplied;
                tmuFinished = m_cmd_tmu0_axis_tready && m_cmd_tmu1_axis_tready;
                triangleFastPath = !dataInTriangleInterpolator
                    && s_cmd_axis_tvalid 
                    && (s_cmd_axis_tdata[OP_POS +: OP_SIZE] == OP_TRIANGLE_STREAM);

                if (rasterizerRunning && !rasterizerIsStarted)
                    rasterizerIsStarted <= 1;

                if (tready) // Wait till the stream has ended
                begin
                    mux <= MUX_NONE;
                    tvalid <= 0;
                    m_cmd_xxx_axis_tlast <= 0;
                    if (!m_cmd_xxx_axis_tlast 
                        && framebufferFinished
                        && rasterizationFinished 
                        && tmuFinished
                        && (triangleFastPath || !pixelInPipeline))
                    begin
                        s_cmd_axis_tready <= 1;
                        state <= COMMAND_IN;
                    end
                end
            end
            COMMAND_IN:
            begin
                if (s_cmd_axis_tvalid)
                begin
                    // Command decoding
                    case (s_cmd_axis_tdata[OP_POS +: OP_SIZE])
                    OP_TRIANGLE_STREAM:
                    begin
                        /* verilator lint_off WIDTH */
                        streamCounter <= s_cmd_axis_tdata[DATABUS_SCALE_FACTOR_LOG2 +: OP_TRIANGLE_STEEAM_SIZE_SIZE - DATABUS_SCALE_FACTOR_LOG2];
                        /* verilator lint_off WIDTH */
                        rasterizerIsStarted <= 0;
                        mux <= MUX_TRIANGLE_STREAM;
                        state <= EXEC_STREAM;
                    end
                    OP_TEXTURE_STREAM:
                    begin
                        streamCounter <= s_cmd_axis_tdata[TEXTURE_STREAM_SIZE_POS +: TEXTURE_STREAM_SIZE_SIZE];

                        if (|s_cmd_axis_tdata[TEXTURE_STREAM_SIZE_POS +: TEXTURE_STREAM_SIZE_SIZE])
                        begin
                            if (s_cmd_axis_tdata[TEXTURE_STREAM_TMU_NR_POS +: TEXTURE_STREAM_TMU_NR_SIZE] == 0)
                            begin
                                mux <= MUX_TEXTURE0_STREAM;
                            end
                            if (s_cmd_axis_tdata[TEXTURE_STREAM_TMU_NR_POS +: TEXTURE_STREAM_TMU_NR_SIZE] == 1)
                            begin
                                mux <= MUX_TEXTURE1_STREAM;
                            end
                            state <= EXEC_STREAM;
                        end
                        else
                        begin
                            s_cmd_axis_tready <= 0;
                            state <= WAIT_FOR_IDLE;    
                        end
                    end
                    OP_FOG_LUT_STREAM:
                    begin
                        streamCounter <= 66;
                        mux <= MUX_FOG_LUT_STREAM;
                        state <= EXEC_STREAM;
                    end
                    OP_RENDER_CONFIG:
                    begin
                        streamCounter <= 1;
                        m_cmd_xxx_axis_tuser <= s_cmd_axis_tdata[0 +: 5];
                        mux <= MUX_RENDER_CONFIG;
                        state <= EXEC_STREAM;
                    end
                    OP_FRAMEBUFFER:
                    begin
                        s_cmd_axis_tready <= 0;
                        colorBufferCmdCommit <= s_cmd_axis_tdata[OP_FRAMEBUFFER_COMMIT_POS];
                        colorBufferCmdMemset <= s_cmd_axis_tdata[OP_FRAMEBUFFER_MEMSET_POS];
                        colorBufferCmdSwap <= s_cmd_axis_tdata[OP_FRAMEBUFFER_SWAP_POS];
                        colorBufferCmdRead <= s_cmd_axis_tdata[OP_FRAMEBUFFER_READ_POS];
                        colorBufferCmdSwapEnableVsync <= s_cmd_axis_tdata[OP_FRAMEBUFFER_SWAP_ENABLE_VSYNC_POS];
                        colorBufferSize <= s_cmd_axis_tdata[OP_FRAMEBUFFER_SIZE_POS +: OP_FRAMEBUFFER_SIZE_SIZE];
                        depthBufferCmdCommit <= s_cmd_axis_tdata[OP_FRAMEBUFFER_COMMIT_POS];
                        depthBufferCmdMemset <= s_cmd_axis_tdata[OP_FRAMEBUFFER_MEMSET_POS];
                        depthBufferCmdRead <= s_cmd_axis_tdata[OP_FRAMEBUFFER_READ_POS];
                        depthBufferSize <= s_cmd_axis_tdata[OP_FRAMEBUFFER_SIZE_POS +: OP_FRAMEBUFFER_SIZE_SIZE];
                        stencilBufferCmdCommit <= s_cmd_axis_tdata[OP_FRAMEBUFFER_COMMIT_POS];
                        stencilBufferCmdMemset <= s_cmd_axis_tdata[OP_FRAMEBUFFER_MEMSET_POS];
                        stencilBufferCmdRead <= s_cmd_axis_tdata[OP_FRAMEBUFFER_READ_POS];
                        stencilBufferSize <= s_cmd_axis_tdata[OP_FRAMEBUFFER_SIZE_POS +: OP_FRAMEBUFFER_SIZE_SIZE];
                        colorBufferApply <= s_cmd_axis_tdata[OP_FRAMEBUFFER_COLOR_BUFFER_SELECT_POS];
                        depthBufferApply <= s_cmd_axis_tdata[OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT_POS];
                        stencilBufferApply <= s_cmd_axis_tdata[OP_FRAMEBUFFER_STENCIL_BUFFER_SELECT_POS];
                        framebufferCommandApply <= s_cmd_axis_tdata[OP_FRAMEBUFFER_COLOR_BUFFER_SELECT_POS] 
                            | (s_cmd_axis_tdata[OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT_POS] && !s_cmd_axis_tdata[OP_FRAMEBUFFER_SWAP_POS])
                            | (s_cmd_axis_tdata[OP_FRAMEBUFFER_STENCIL_BUFFER_SELECT_POS] && !s_cmd_axis_tdata[OP_FRAMEBUFFER_SWAP_POS]);
                        state <= WAIT_FOR_IDLE;
                    end
                    OP_NOP_STREAM:
                    begin
                        s_cmd_axis_tready <= 0;
                        state <= WAIT_FOR_IDLE;
                    end
                    endcase
                end
            end
            EXEC_STREAM:
            begin : Stream
                s_cmd_axis_tready <= tready;
                if (tready)
                begin
                    if (tvalidSkid)
                    begin
                        tvalid <= tvalidSkid;
                        m_cmd_xxx_axis_tdata <= tdataSkid;
                        tvalidSkid <= 0;
                    end
                    else
                    begin
                        tvalid <= s_cmd_axis_tvalid;
                        m_cmd_xxx_axis_tdata <= s_cmd_axis_tdata;
                    end

                    if (s_cmd_axis_tvalid || tvalidSkid)
                    begin
                        streamCounter <= streamCounter - 1;
                        if (streamCounter == 1)
                        begin
                            s_cmd_axis_tready <= 0;
                            m_cmd_xxx_axis_tlast <= 1;

                            state <= WAIT_FOR_IDLE;
                        end
                    end
                end
                else
                begin
                    if (s_cmd_axis_tvalid && !tvalidSkid)
                    begin
                        tdataSkid <= s_cmd_axis_tdata;
                        tvalidSkid <= s_cmd_axis_tvalid;
                    end
                end
            end
            default:
            begin
            end
            endcase

            case (fbControlState)
            FB_CONTROL_WAITFORCOMMAND:
            begin
                if (framebufferCommandApply)
                begin
                    if (framebufferCommandApplied == 0)
                    begin
                        fbControlState <= FB_CONTROL_WAITFOREND;
                    end
                end
            end
            FB_CONTROL_WAITFOREND:
            begin
                framebufferCommandApply <= 0;
                colorBufferApply <= 0;
                depthBufferApply <= 0;
                stencilBufferApply <= 0;
                if (framebufferCommandApplied)
                begin
                    fbControlState <= FB_CONTROL_WAITFORCOMMAND;
                end
            end
            endcase
        end
    end

endmodule