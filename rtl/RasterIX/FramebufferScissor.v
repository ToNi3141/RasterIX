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

// Combinational module that checks the pixel position against the scissor
// rectangle. If the pixel is outside the scissor rectangle, tstrb is zeroed.
// All other fragment stream signals pass through unchanged.
// txpos and typos are consumed by this module and not forwarded.
module FramebufferScissor #(
    // The maximum size of the screen in power of two
    parameter X_BIT_WIDTH = 11,
    parameter Y_BIT_WIDTH = 11,

    parameter PIXEL_WIDTH = 16,
    parameter ADDR_WIDTH = 32
) (
    /////////////////////////
    // Configs
    /////////////////////////
    input  wire                             confEnableScissor,
    input  wire [X_BIT_WIDTH - 1 : 0]       confScissorStartX,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confScissorStartY,
    input  wire [X_BIT_WIDTH - 1 : 0]       confScissorEndX,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confScissorEndY,

    /////////////////////////
    // Slave fragment interface
    /////////////////////////
    input  wire                             s_frag_tvalid,
    input  wire                             s_frag_tlast,
    output wire                             s_frag_tready,
    input  wire [PIXEL_WIDTH - 1 : 0]       s_frag_tdata,
    input  wire                             s_frag_tstrb,
    input  wire [ADDR_WIDTH - 1 : 0]        s_frag_taddr,
    input  wire [X_BIT_WIDTH - 1 : 0]       s_frag_txpos,
    input  wire [Y_BIT_WIDTH - 1 : 0]       s_frag_typos,

    /////////////////////////
    // Master fragment interface
    /////////////////////////
    output wire                             m_frag_tvalid,
    output wire                             m_frag_tlast,
    input  wire                             m_frag_tready,
    output wire [PIXEL_WIDTH - 1 : 0]       m_frag_tdata,
    output wire                             m_frag_tstrb,
    output wire [ADDR_WIDTH - 1 : 0]        m_frag_taddr
);

    // Pass through
    assign m_frag_tvalid = s_frag_tvalid;
    assign m_frag_tlast  = s_frag_tlast;
    assign s_frag_tready = m_frag_tready;
    assign m_frag_tdata  = s_frag_tdata;
    assign m_frag_taddr  = s_frag_taddr;

    // Scissor logic
    wire scissorPass = !confEnableScissor || ((s_frag_txpos >= confScissorStartX) && (s_frag_txpos < confScissorEndX) && (s_frag_typos >= confScissorStartY) && (s_frag_typos < confScissorEndY));
    assign m_frag_tstrb = scissorPass && s_frag_tstrb;

endmodule
