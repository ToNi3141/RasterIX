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

// Rasterizes a triangle by using increments and an edge walking algorithm.
// The edge walker searches for the left edge. When it is found, the position of the left edge is stored (push).
// Then the edge walker walks to the right edge.
// When the right edge is found, the increments are reset (pop) to the start of the edge.
// Then the y increment is applied.
// Then the left edge is searched again.
// When it starts from the beginning again, the edge walker searches for the right edge and so on...
// Note: Normally we can assume that after the y increment, we are inside or left of the triangle.
//      When inside: We walk to the left till the left edge is found
//      When outside: We walk to the right till the left edge is found
// But sometimes: We are after a y increment on the right side of the triangle. Therefore, when no edge is found when walking,
// the edge walker tries the other direction from the stored position, just to make sure we find
// the triangle. If still no triangle is found, a additional y increment is applied.

// The output is a stream of pixels and commands.
// m_rr_tlast: Is true when the algorithm terminates.
// m_rr_tready: Stalls the rasterizer when it is false.
// m_rr_tvalid: Is true when the algorithm runs and streams data. True does not mean, that it is inside a triangle, true 
//      only means that the rasterizer streams meaningful data.
// m_rr_tpixel: It is only true, when the edge walker is inside a triangle and false outside except for the last pixel.
//      The last pixel can be outside of the bounding box and is used, together with m_rr_tkeep to flush the pipeline.
// m_rr_tkeep: Marks a hidden pixel. When m_rr_tkeep and m_rr_tpixel are true, a pixel must be drawn. If m_rr_tkeep is false,
//      then the pixel must not be written to the framebuffer. It is used to flush the pipeline.
// m_rr_tbbx: x position in the bounding box.
// m_rr_tbby: y position in the bounding box.
// m_rr_tspx: x position on the screen.
// m_rr_tspy: y position on the screen.
// m_rr_tindex: Pixel address in the framebuffer.
// m_rr_tcmd: Command to an incremental interpolator. When calculating the attributes (color, tex coords, ...) via increments,
//      then this cmd can be used. It informs the interpolator to do a x inc/dec or a y inc. It needs to be synchronous
//      with the rasterizer.
//      Typically when m_rr_tpixel and m_rr_tvalid is true, a pixel can be drawn. It is expected that the the current attributes
//      can be used. The attributes of the current m_rr_tcmd a needed for the attributes in the next cycle. So a single cycle
//      interpolator can run in parallel to the perspective correction.
module Rasterizer
#(
    `include "RasterizerCommands.vh"

    // The maximum size of the screen in power of two
    parameter X_BIT_WIDTH = 11,
    parameter Y_BIT_WIDTH = 11,

    parameter INDEX_WIDTH = 14,

    parameter LINE_MODE = 0,

    localparam ATTRIBUTE_SIZE = 32,

    localparam KEEP_WIDTH = 1
)
(
    input wire                              clk,
    input wire                              reset,
    
    // Rasterizer Control
    output reg                              rasterizerRunning,
    input  wire                             startRendering,

    // Rasterizer config
    input  wire [Y_BIT_WIDTH - 1 : 0]       yOffset,
    input  wire [X_BIT_WIDTH - 1 : 0]       xResolution,
    input  wire [Y_BIT_WIDTH - 1 : 0]       yResolution,

    // Triangle Attributes
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    bbStart,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    bbEnd,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w0,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w1,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w2,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w0IncX,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w1IncX,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w2IncX,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w0IncY,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w1IncY,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w2IncY,

    // Fragment Stream
    output reg                              m_rr_tvalid,
    input  wire                             m_rr_tready,
    output reg                              m_rr_tlast,
    output reg                              m_rr_tpixel,
    output reg  [X_BIT_WIDTH - 1 : 0]       m_rr_tbbx,
    output reg  [Y_BIT_WIDTH - 1 : 0]       m_rr_tbby,
    output reg  [X_BIT_WIDTH - 1 : 0]       m_rr_tspx,
    output reg  [Y_BIT_WIDTH - 1 : 0]       m_rr_tspy,
    output reg  [INDEX_WIDTH - 1 : 0]       m_rr_tindex,
    output reg  [KEEP_WIDTH - 1 : 0]        m_rr_tkeep,
    output reg  [RR_CMD_SIZE - 1 : 0]       m_rr_tcmd
);
    localparam BB_X_POS = 0;
    localparam BB_Y_POS = 16;

    // Rasterizer main state machine
    localparam RASTERIZER_WAITFORCOMMAND = 0;
    localparam RASTERIZER_INIT = 1;
    localparam RASTERIZER_TEST = 2;

    // Rasterizer edge walker state machine
    localparam RASTERIZER_EDGEWALKER_SEARCH_LEFT_EDGE = 0;
    localparam RASTERIZER_EDGEWALKER_WALK_OUT = 1;
    localparam RASTERIZER_EDGEWALKER_WALK = 2;
    localparam RASTERIZER_EDGEWALKER_YINC = 3;
    localparam RASTERIZER_EDGEWALKER_INIT = 4;
    localparam RASTERIZER_EDGEWALKER_SEARCH_RIGHT_EDGE = 5;

    // Rasterizer variables
    reg  [ 5 : 0]                   rasterizerState;
    reg  [Y_BIT_WIDTH - 1 : 0]      y;
    reg  [Y_BIT_WIDTH - 1 : 0]      yScreen;
    reg  [Y_BIT_WIDTH - 1 : 0]      yScreenEnd;
    reg  [Y_BIT_WIDTH - 1 : 0]      lineBBStart;
    reg  [X_BIT_WIDTH - 1 : 0]      x;
    reg  [X_BIT_WIDTH - 1 : 0]      xStack;
    reg  [ATTRIBUTE_SIZE - 1 : 0]   regW0;
    reg  [ATTRIBUTE_SIZE - 1 : 0]   regW1;
    reg  [ATTRIBUTE_SIZE - 1 : 0]   regW2;
    reg  [ATTRIBUTE_SIZE - 1 : 0]   regW0Stack;
    reg  [ATTRIBUTE_SIZE - 1 : 0]   regW1Stack;
    reg  [ATTRIBUTE_SIZE - 1 : 0]   regW2Stack;

    wire isInTriangle = !(regW0[31] | regW1[31] | regW2[31]);
    wire isInTriangleAndInBounds = isInTriangle && (x < bbEnd[BB_X_POS +: X_BIT_WIDTH]) && (x >= bbStart[BB_X_POS +: X_BIT_WIDTH]);
    
    // Edge walker variables
    reg  [ 5 : 0] edgeWalkingState;

    always @(posedge clk)
    begin
        if (reset)
        begin
            m_rr_tlast <= 0;
            m_rr_tvalid <= 0;
            m_rr_tkeep <= ~0;
            rasterizerRunning <= 0;
            rasterizerState <= RASTERIZER_WAITFORCOMMAND;
        end
        else
        begin
            case (rasterizerState)
            RASTERIZER_WAITFORCOMMAND:
            begin
                if (m_rr_tready)
                begin
                    m_rr_tvalid <= 0;
                    m_rr_tpixel <= 0;
                    m_rr_tkeep <= ~0;
                    m_rr_tlast <= 0;
                    rasterizerRunning <= 0;
                    if (startRendering)
                    begin
                        lineBBStart <= yOffset - bbStart[BB_Y_POS +: Y_BIT_WIDTH];
                        rasterizerRunning <= 1;
                        rasterizerState <= RASTERIZER_INIT;
                        m_rr_tcmd <= RR_CMD_INIT;
                    end
                end
            end
            RASTERIZER_INIT:
            begin
                x <= bbStart[BB_X_POS +: X_BIT_WIDTH];

                regW0 <= w0;
                regW1 <= w1;
                regW2 <= w2;

                if (LINE_MODE)
                begin
                    // Shift the triangle to the current framebuffer line. Everything can be calculated in software if this implementation
                    // takes too much logic. It can be completely discarded, when the framebuffer is big enough to contain the whole screen. This is only 
                    // required in the line mode, to handle the offsets in y direction when rendering a new line.
                    // Check if the current line offset is above the bounding box. Means, the bounding box starts in this line or in lines after this line.
                    // In any case, set the current yScreen coord to the bounding box start position. If the bounding box start position is in this
                    // line, then everything is fine. If not, then yScreen will be below yScreenEnd and the rendering of the current triangle is discarded
                    // for this line.
                    if (yOffset <= bbStart[BB_Y_POS +: Y_BIT_WIDTH])
                    begin
                        yScreen <= bbStart[BB_Y_POS +: Y_BIT_WIDTH];
                        y <= bbStart[BB_Y_POS +: Y_BIT_WIDTH] - yOffset;
                    end
                    else
                    begin
                        yScreen <= yOffset;
                        y <= 0;
                    end

                    // Check if the bounding box ends in this line. If not, clamp the bounding box end to the end of the current line.
                    // If the bounding box end in this line, or in a previous line, just set yScreenEnd to the end of the bounding box.
                    // The the condition occurs that yScreenEnd is smaller than yScreen which results in discarding the triangle for this line.
                    if ((yOffset + yResolution) <= bbEnd[BB_Y_POS +: Y_BIT_WIDTH])
                    begin
                        yScreenEnd <= yOffset + yResolution;
                    end
                    else
                    begin
                        yScreenEnd <= bbEnd[BB_Y_POS +: Y_BIT_WIDTH];
                    end
                end
                else
                begin
                    yScreen <= bbStart[BB_Y_POS +: Y_BIT_WIDTH];
                    y <= bbStart[BB_Y_POS +: Y_BIT_WIDTH];
                    yScreenEnd <= bbEnd[BB_Y_POS +: Y_BIT_WIDTH];
                end

                // Start rasterization
                m_rr_tvalid <= 1;
                m_rr_tcmd <= RR_CMD_INIT;
                edgeWalkingState <= RASTERIZER_EDGEWALKER_INIT;
                rasterizerState <= RASTERIZER_TEST;
            end
            RASTERIZER_TEST:
            begin : rasterization
                reg [RR_CMD_SIZE - 1 : 0] rrCmd;
                rrCmd = RR_CMD_NOP;
                // A rasterization cycle is only executed if the shader is free. Otherwise the rasterizer will stall
                if (m_rr_tready)
                begin
                    if (yScreen < yScreenEnd)
                    begin
                        case (edgeWalkingState)
                        RASTERIZER_EDGEWALKER_INIT:
                        begin
                            // Just initialize the edge walker. No increments or so. This state is only used, when the edge walker is aborted and needs to be reinitialized for the next line.
                            rrCmd = RR_CMD_PUSH;
                            edgeWalkingState <= RASTERIZER_EDGEWALKER_SEARCH_LEFT_EDGE;
                        end
                        RASTERIZER_EDGEWALKER_SEARCH_LEFT_EDGE:
                        begin
                            if (isInTriangleAndInBounds)
                            begin
                                rrCmd = RR_CMD_PUSH | RR_CMD_X_INC;
                                m_rr_tpixel <= 1; // To prevent, that the first pixel of the triangle is skipped
                                edgeWalkingState <= RASTERIZER_EDGEWALKER_WALK;
                            end
                            else if (x == bbEnd[BB_X_POS +: X_BIT_WIDTH])
                            begin
                                // Maybe the bounding box was not fitting perfectly and we are still above the triangle.
                                rrCmd = RR_CMD_X_DEC;
                                // Reuse the walking state. This will to a y increment and the other stuff required.
                                edgeWalkingState <= RASTERIZER_EDGEWALKER_SEARCH_RIGHT_EDGE;
                            end
                            else
                            begin
                                // Keep searching
                                rrCmd = RR_CMD_X_INC;
                            end
                        end
                        RASTERIZER_EDGEWALKER_SEARCH_RIGHT_EDGE:
                        begin
                            if (isInTriangleAndInBounds)
                            begin
                                rrCmd = RR_CMD_X_DEC;
                                edgeWalkingState <= RASTERIZER_EDGEWALKER_WALK_OUT;
                            end
                            else if (x == bbStart[BB_X_POS +: X_BIT_WIDTH])
                            begin
                                // No edge found. Now skip this line.
                                rrCmd = RR_CMD_PUSH;
                                edgeWalkingState <= RASTERIZER_EDGEWALKER_YINC;
                            end
                            else
                            begin
                                // Keep searching
                                rrCmd = RR_CMD_X_DEC;
                            end
                        end
                        RASTERIZER_EDGEWALKER_WALK_OUT:
                        begin
                            if (isInTriangleAndInBounds)
                            begin
                                // Pixel Decrement
                                rrCmd = RR_CMD_X_DEC;
                            end
                            else
                            begin
                                // Pixel Increment
                                // Do it directly. Because we are already outside and only. With a increment we have the 
                                // chance to save one clock cycle. Otherwise we would lose it in the SEARCH_EDGE state.
                                rrCmd = RR_CMD_X_INC;
                                edgeWalkingState <= RASTERIZER_EDGEWALKER_SEARCH_LEFT_EDGE;
                            end
                        end
                        RASTERIZER_EDGEWALKER_YINC:
                        begin
                            // Line Increment
                            rrCmd = RR_CMD_Y_INC;
                            edgeWalkingState <= RASTERIZER_EDGEWALKER_WALK_OUT;
                        end
                        RASTERIZER_EDGEWALKER_WALK:
                        begin
                            // Render pixels
                            if (!isInTriangleAndInBounds)
                            begin
                                rrCmd = RR_CMD_POP;
                                edgeWalkingState <= RASTERIZER_EDGEWALKER_YINC;
                            end
                            else
                            begin
                                // Pixel Increment
                                rrCmd = RR_CMD_X_INC;
                            end
                            m_rr_tpixel <= isInTriangleAndInBounds;
                        end
                        endcase

                        /* verilator lint_off WIDTH */
                        // Check that the index never exceeds the borders of the view port
                        if ((y < yResolution) && (x < xResolution))
                        begin
                            m_rr_tindex <= (((yResolution - 1) - y) * xResolution) + x;
                        end
                        /* verilator lint_on WIDTH */
                        
                        // Arguments for the shader
                        m_rr_tbbx <= (x - bbStart[BB_X_POS +: X_BIT_WIDTH]);
                        m_rr_tbby <= (yScreen - bbStart[BB_Y_POS +: Y_BIT_WIDTH]);
                        m_rr_tspx <= x;
                        m_rr_tspy <= yScreen;

                        m_rr_tcmd <= RR_CMD_NOP;
                        if (rrCmd & RR_CMD_X_INC)
                        begin
                            x <= x + 1;

                            regW0 <= regW0 + $signed(w0IncX);
                            regW1 <= regW1 + $signed(w1IncX);
                            regW2 <= regW2 + $signed(w2IncX);

                            m_rr_tcmd <= rrCmd;
                            m_rr_tvalid <= 1;
                        end
                        if (rrCmd & RR_CMD_X_DEC)
                        begin
                            x <= x - 1;

                            regW0 <= regW0 - $signed(w0IncX);
                            regW1 <= regW1 - $signed(w1IncX);
                            regW2 <= regW2 - $signed(w2IncX);

                            m_rr_tcmd <= rrCmd;
                            m_rr_tvalid <= 1;
                        end
                        if (rrCmd & RR_CMD_Y_INC)
                        begin
                            y <= y + 1;
                            yScreen <= yScreen + 1;

                            regW0 <= regW0 + $signed(w0IncY);
                            regW1 <= regW1 + $signed(w1IncY);
                            regW2 <= regW2 + $signed(w2IncY);

                            m_rr_tcmd <= rrCmd;
                            m_rr_tvalid <= 1;
                        end
                        if (rrCmd & RR_CMD_PUSH)
                        begin
                            xStack <= x;

                            regW0Stack <= regW0;
                            regW1Stack <= regW1;
                            regW2Stack <= regW2;

                            m_rr_tcmd <= rrCmd;
                            m_rr_tvalid <= 1;
                        end
                        if (rrCmd & RR_CMD_POP)
                        begin
                            x <= xStack;

                            regW0 <= regW0Stack;
                            regW1 <= regW1Stack;
                            regW2 <= regW2Stack;

                            m_rr_tcmd <= rrCmd;
                            m_rr_tvalid <= 1;
                        end
                    end
                    else
                    begin
                        // Now the edge walker is below the triangle. No Triangle hit is expected anymore.
                        // That means, the edge walking is aborted.
                        m_rr_tpixel <= 1;
                        m_rr_tkeep <= 0;
                        m_rr_tlast <= 1;
                        m_rr_tvalid <= 1;
                        m_rr_tcmd <= RR_CMD_NOP;
                        rasterizerState <= RASTERIZER_WAITFORCOMMAND;
                    end
                end
            end
            endcase 
        end
    end
endmodule