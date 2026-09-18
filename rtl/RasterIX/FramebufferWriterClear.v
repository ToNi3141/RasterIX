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
// Performance: 1 pixel per cylce
module FramebufferWriterClear #(
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,

    // The maximum size of the screen in power of two
    parameter X_BIT_WIDTH = 11,
    parameter Y_BIT_WIDTH = 11,
    parameter INDEX_WIDTH = X_BIT_WIDTH + Y_BIT_WIDTH,

    // Size of the pixels
    parameter PIXEL_WIDTH = 16,
    localparam PIXEL_MASK_WIDTH = PIXEL_WIDTH / 8,
    localparam PIXEL_WIDTH_LG = $clog2(PIXEL_WIDTH / 8)
) (
    input   wire                        aclk,
    input   wire                        resetn,

    /////////////////////////
    // Configs
    /////////////////////////
    input  wire [PIXEL_WIDTH - 1 : 0]   confClearColor,
    input  wire [X_BIT_WIDTH - 1 : 0]   confXResolution,
    input  wire [Y_BIT_WIDTH - 1 : 0]   confYResolution,
    input  wire                         confEnableScissor,
    input  wire [X_BIT_WIDTH - 1 : 0]   confScissorStartX,
    input  wire [Y_BIT_WIDTH - 1 : 0]   confScissorStartY,
    input  wire [X_BIT_WIDTH - 1 : 0]   confScissorEndX,
    input  wire [Y_BIT_WIDTH - 1 : 0]   confScissorEndY,


    /////////////////////////
    // Fragment interface
    /////////////////////////

    // Framebuffer input interface
    input  wire                         s_frag_tvalid,
    input  wire                         s_frag_tlast,
    output wire                         s_frag_tready,
    input  wire [PIXEL_WIDTH - 1 : 0]   s_frag_tdata,
    input  wire                         s_frag_tstrb,
    input  wire [ADDR_WIDTH - 1 : 0]    s_frag_taddr,
    input  wire [X_BIT_WIDTH - 1 : 0]   s_frag_txpos,
    input  wire [X_BIT_WIDTH - 1 : 0]   s_frag_typos,

    // Framebuffer output interface
    output wire                         m_frag_tvalid,
    output wire                         m_frag_tlast,
    input  wire                         m_frag_tready,
    output wire [PIXEL_WIDTH - 1 : 0]   m_frag_tdata,
    output wire                         m_frag_tstrb,
    output wire [ADDR_WIDTH - 1 : 0]    m_frag_taddr,
    output wire [X_BIT_WIDTH - 1 : 0]   m_frag_txpos,
    output wire [X_BIT_WIDTH - 1 : 0]   m_frag_typos,
    
    /////////////////////////
    // Control
    /////////////////////////

    // Cmd interface
    input  wire                         apply, // This start a command 
    output reg                          applied // This marks if the commands has been applied.

);
    // Step 0 
    // Calculation of the pixel positions
    reg  [X_BIT_WIDTH - 1 : 0]  step0_xpos;
    reg  [Y_BIT_WIDTH - 1 : 0]  step0_ypos;
    reg                         step0_valid;
    reg                         step0_last;
    reg  [X_BIT_WIDTH - 1 : 0]  step0_xend;
    reg  [Y_BIT_WIDTH - 1 : 0]  step0_yend;
    reg  [X_BIT_WIDTH - 1 : 0]  step0_xstart;
    wire [X_BIT_WIDTH - 1 : 0]  step0_xposNext = step0_xpos + 1;
    wire [X_BIT_WIDTH - 1 : 0]  step0_yposNext = step0_ypos + 1;
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
                step0_ypos <= confEnableScissor ? confScissorStartY : 0;
                step0_xstart <= confEnableScissor ? confScissorStartX : 0;
                step0_xend <= confEnableScissor ? confScissorEndX : confXResolution;
                step0_yend <= confEnableScissor ? confScissorEndY : confYResolution;
                step0_valid <= 1;
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
                step1_ypos <= step0_ypos;
            end
        end
    end

    // Step 2 
    // Muxing
    wire [ADDR_WIDTH - 1 : 0] step2_addr = { { (ADDR_WIDTH - INDEX_WIDTH) { 1'b0 } }, step1_index };
    assign m_frag_tvalid    = step1_valid ? 1              : s_frag_tvalid;
    assign m_frag_tlast     = step1_valid ? step1_last     : s_frag_tlast;
    assign s_frag_tready    = step1_valid ? 0              : m_frag_tready;
    assign m_frag_tdata     = step1_valid ? confClearColor : s_frag_tdata;
    assign m_frag_tstrb     = step1_valid ? 1              : s_frag_tstrb;
    assign m_frag_taddr     = step1_valid ? step2_addr     : s_frag_taddr;
    assign m_frag_txpos     = step1_valid ? step1_xpos     : s_frag_txpos;
    assign m_frag_typos     = step1_valid ? step1_ypos     : s_frag_typos;

endmodule