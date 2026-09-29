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

// Used to clear the framebuffer. It will trigger a write request for
// each pixel in the framebuffer including the position of the pixel.
// The FramebufferWriter can then decide to write the pixel to the
// framebuffer or omit it (for instance when the scissor test fails).
// It has a fragment in and fragment out interface. The fragment in
// interface is connected to the pixel pipeline and is deactivated
// as long as a clear is in progress.
// Performance: 1 pixel per cycle

module FramebufferClear #(
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,

    // The maximum size of the screen in power of two
    parameter X_BIT_WIDTH = 11,
    parameter Y_BIT_WIDTH = 11,
    parameter INDEX_WIDTH = X_BIT_WIDTH + Y_BIT_WIDTH,

    // Size of the framebuffer lanes
    parameter PIXEL_WIDTH = 32,
    parameter DEPTH_WIDTH = 16,
    parameter STENCIL_WIDTH = 4
) (
    input   wire                            aclk,
    input   wire                            resetn,

    /////////////////////////
    // Configs
    /////////////////////////
    input  wire [PIXEL_WIDTH - 1 : 0]       confClearColor,
    input  wire [DEPTH_WIDTH - 1 : 0]       confClearDepth,
    input  wire [STENCIL_WIDTH - 1 : 0]     confClearStencil,
    input  wire                             confColorBufferSelect,
    input  wire                             confDepthBufferSelect,
    input  wire                             confStencilBufferSelect,
    input  wire [X_BIT_WIDTH - 1 : 0]       confXResolution,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confYResolution,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confYOffset,
    input  wire                             confEnableScissor,
    input  wire [X_BIT_WIDTH - 1 : 0]       confScissorStartX,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confScissorStartY,
    input  wire [X_BIT_WIDTH - 1 : 0]       confScissorEndX,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confScissorEndY,

    /////////////////////////
    // Fragment interface
    /////////////////////////
    input  wire                             s_frag_tvalid,
    input  wire                             s_frag_tlast,
    output wire                             s_frag_tready,
    input  wire [PIXEL_WIDTH - 1 : 0]       s_frag_color_tdata,
    input  wire                             s_frag_color_tstrb,
    input  wire [DEPTH_WIDTH - 1 : 0]       s_frag_depth_tdata,
    input  wire                             s_frag_depth_tstrb,
    input  wire [STENCIL_WIDTH - 1 : 0]     s_frag_stencil_tdata,
    input  wire                             s_frag_stencil_tstrb,
    input  wire [ADDR_WIDTH - 1 : 0]        s_frag_taddr,
    input  wire [X_BIT_WIDTH - 1 : 0]       s_frag_txpos,
    input  wire [Y_BIT_WIDTH - 1 : 0]       s_frag_typos,

    output wire                             m_frag_tvalid,
    output wire                             m_frag_tlast,
    input  wire                             m_frag_tready,
    output wire [PIXEL_WIDTH - 1 : 0]       m_frag_color_tdata,
    output wire                             m_frag_color_tstrb,
    output wire [DEPTH_WIDTH - 1 : 0]       m_frag_depth_tdata,
    output wire                             m_frag_depth_tstrb,
    output wire [STENCIL_WIDTH - 1 : 0]     m_frag_stencil_tdata,
    output wire                             m_frag_stencil_tstrb,
    output wire [ADDR_WIDTH - 1 : 0]        m_frag_taddr,
    output wire [X_BIT_WIDTH - 1 : 0]       m_frag_txpos,
    output wire [Y_BIT_WIDTH - 1 : 0]       m_frag_typos,

    /////////////////////////
    // Control
    /////////////////////////
    input  wire                             apply,
    output reg                              applied
);
    function [Y_BIT_WIDTH - 1 : 0] clampToYOffset;
        input [Y_BIT_WIDTH - 1 : 0] y;
        input [Y_BIT_WIDTH - 1 : 0] yOffset;
        input [Y_BIT_WIDTH - 1 : 0] yResolution;
        reg [Y_BIT_WIDTH : 0] lineEndY;
        begin
            lineEndY = {1'b0, yOffset} + {1'b0, yResolution};
            if (y < yOffset)
                clampToYOffset = yOffset;
            else if ({1'b0, y} > lineEndY)
                clampToYOffset = lineEndY[Y_BIT_WIDTH - 1 : 0];
            else
                clampToYOffset = y;
        end
    endfunction

    wire [Y_BIT_WIDTH - 1 : 0] clampedScissorStartY = clampToYOffset(confScissorStartY, confYOffset, confYResolution);
    wire [Y_BIT_WIDTH - 1 : 0] clampedScissorEndY = clampToYOffset(confScissorEndY, confYOffset, confYResolution);

    // Step 0
    // Calculation of the pixel positions
    reg  [X_BIT_WIDTH - 1 : 0]  step0_xpos;
    reg  [Y_BIT_WIDTH - 1 : 0]  step0_ypos;
    reg                         step0_valid;
    reg                         step0_last;
    reg  [X_BIT_WIDTH - 1 : 0]  step0_xend;
    reg  [Y_BIT_WIDTH - 1 : 0]  step0_yend;
    reg  [X_BIT_WIDTH - 1 : 0]  step0_xstart;
    reg  [Y_BIT_WIDTH - 1 : 0]  step0_yoffset;
    wire [X_BIT_WIDTH - 1 : 0]  step0_xposNext = step0_xpos + 1;
    wire [Y_BIT_WIDTH - 1 : 0]  step0_yposNext = step0_ypos + 1;
    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            applied <= 1;
            step0_last <= 0;
            step0_valid <= 0;
        end
        else
        begin
            if (apply && !s_frag_tvalid)
            begin
                applied <= 0;
                step0_xpos <= confEnableScissor ? confScissorStartX : 0;
                step0_xstart <= confEnableScissor ? confScissorStartX : 0;
                step0_xend <= confEnableScissor ? confScissorEndX : confXResolution;
                if (!confEnableScissor)
                begin
                    step0_ypos <= 0;
                    step0_yend <= confYResolution;
                    step0_valid <= 1;
                end
                else if ((confYOffset != 0) && (clampedScissorEndY <= clampedScissorStartY))
                begin
                    step0_ypos <= confYResolution;
                    step0_yend <= confYResolution;
                    step0_valid <= 0;
                end
                else
                begin
                    step0_ypos <= clampedScissorStartY - confYOffset;
                    step0_yend <= clampedScissorEndY - confYOffset;
                    step0_valid <= 1;
                end
                step0_yoffset <= confYOffset;
                step0_last <= 0;
            end

            if (!applied && m_frag_tready)
            begin
                if (step0_xpos >= (step0_xend - 1))
                begin
                    step0_xpos <= step0_xstart;
                    step0_ypos <= step0_yposNext;

                    // Emergency stop if xstart equals xend.
                    // This also triggers the applied acknowledge cycle
                    step0_last <= (step0_ypos >= (step0_yend - 1));
                end
                else
                begin
                    step0_xpos <= step0_xposNext;
                    step0_last <= (step0_xposNext >= (step0_xend - 1)) && (step0_ypos >= (step0_yend - 1));
                end
                if (step0_last)
                begin
                    step0_valid <= 0;
                    applied <= 1;
                end
            end
        end
    end

    // Step 1
    // Calculation of the pixel index
    reg                         step1_valid;
    reg                         step1_last;
    reg  [X_BIT_WIDTH - 1 : 0]  step1_xpos;
    reg  [Y_BIT_WIDTH - 1 : 0]  step1_ypos;
    reg  [INDEX_WIDTH - 1 : 0]  step1_index;
    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            step1_valid <= 0;
            step1_last <= 0;
        end
        else
        begin
            if (m_frag_tready)
            begin : Step1
                reg [Y_BIT_WIDTH - 1 : 0] ypos;
                ypos = ((confYResolution - { { (Y_BIT_WIDTH - 1) { 1'b0 } }, 1'b1 }) - step0_ypos);
                step1_index <= (ypos * confXResolution) + { { (INDEX_WIDTH - X_BIT_WIDTH) { 1'b0 } }, step0_xpos };
                step1_valid <= step0_valid;
                step1_last <= step0_last;
                step1_xpos <= step0_xpos;
                step1_ypos <= step0_ypos + step0_yoffset;
            end
        end
    end

    // Step 2
    // Muxing
    wire [ADDR_WIDTH - 1 : 0] step2_addr = { { (ADDR_WIDTH - INDEX_WIDTH) { 1'b0 } }, step1_index };
    assign m_frag_tvalid = step1_valid ? 1 : s_frag_tvalid;
    assign m_frag_tlast = step1_valid ? step1_last : s_frag_tlast;
    assign s_frag_tready = step1_valid ? 0 : m_frag_tready;
    assign m_frag_color_tdata = step1_valid ? confClearColor : s_frag_color_tdata;
    assign m_frag_color_tstrb = step1_valid ? confColorBufferSelect : s_frag_color_tstrb;
    assign m_frag_depth_tdata = step1_valid ? confClearDepth : s_frag_depth_tdata;
    assign m_frag_depth_tstrb = step1_valid ? confDepthBufferSelect : s_frag_depth_tstrb;
    assign m_frag_stencil_tdata = step1_valid ? confClearStencil : s_frag_stencil_tdata;
    assign m_frag_stencil_tstrb = step1_valid ? confStencilBufferSelect : s_frag_stencil_tstrb;
    assign m_frag_taddr = step1_valid ? step2_addr : s_frag_taddr;
    assign m_frag_txpos = step1_valid ? step1_xpos : s_frag_txpos;
    assign m_frag_typos = step1_valid ? step1_ypos : s_frag_typos;

endmodule