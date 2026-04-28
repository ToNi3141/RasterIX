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
    reg                         valid;
    reg                         last;
    reg  [ADDR_WIDTH - 1 : 0]   addr;
    reg  [X_BIT_WIDTH - 1 : 0]  xpos;
    reg  [X_BIT_WIDTH - 1 : 0]  ypos;

    wire [ADDR_WIDTH - 1 : 0]   addrNext = addr + 1;
    wire [X_BIT_WIDTH - 1 : 0]  xposNext = xpos + 1;
    wire [X_BIT_WIDTH - 1 : 0]  yposNext = ypos - 1;

    assign m_frag_tvalid    = (applied) ? s_frag_tvalid   : valid;
    assign m_frag_tlast     = (applied) ? s_frag_tlast    : last;
    assign s_frag_tready    = (applied) ? m_frag_tready   : 0;
    assign m_frag_tdata     = (applied) ? s_frag_tdata    : confClearColor;
    assign m_frag_tstrb     = (applied) ? s_frag_tstrb    : 1;
    assign m_frag_taddr     = (applied) ? s_frag_taddr    : addr;
    assign m_frag_txpos     = (applied) ? s_frag_txpos    : xpos;
    assign m_frag_typos     = (applied) ? s_frag_typos    : ypos;

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            valid <= 0;
            last <= 0;
            addr <= 0;
            xpos <= 0;
            ypos <= 0;

            applied <= 1;
        end
        else
        begin
            if (apply)
            begin
                applied <= 0;
                xpos <= 0;
                // Note: The clear process starts at a position where (0, 0) is located at the top left.
                // The OpenGL coordinate system starts at the bottom left. To switch the coordinate system
                // this module starts with the bottom first and iterates through the top.
                ypos <= confYResolution - 1;
                addr <= 0;
                last <= 0;
                valid <= 1;
            end

            if (!applied)
            begin
                if (m_frag_tready)
                begin
                    if (xposNext == confXResolution)
                    begin
                        xpos <= 0;
                        ypos <= yposNext;
                    end
                    else
                    begin
                        xpos <= xposNext;
                        last <= (ypos == 0) && ((xposNext + 1) == confXResolution);
                    end
                    addr <= addrNext;
                    if (last)
                    begin
                        applied <= 1;
                        valid <= 0;
                        last <= 0;
                    end
                end
            end
        end
    end
endmodule