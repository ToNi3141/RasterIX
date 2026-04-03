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

// Combinational module that adds a base address (confAddr) to the
// fragment address.
// All other fragment stream signals pass through unchanged.
module FramebufferMMU #(
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    parameter PIXEL_WIDTH = 16,
    localparam PIXEL_MASK_WIDTH = PIXEL_WIDTH / 8,
    localparam PIXEL_WIDTH_LG = $clog2(PIXEL_WIDTH / 8)
) (
    /////////////////////////
    // Configs
    /////////////////////////
    input  wire [ADDR_WIDTH - 1 : 0]        confAddr,

    /////////////////////////
    // Slave fragment interface
    /////////////////////////
    input  wire                             s_frag_tvalid,
    input  wire                             s_frag_tlast,
    output wire                             s_frag_tready,
    input  wire [PIXEL_WIDTH - 1 : 0]       s_frag_tdata,
    input  wire [PIXEL_MASK_WIDTH - 1 : 0]  s_frag_tstrb,
    input  wire [ADDR_WIDTH - 1 : 0]        s_frag_taddr,

    /////////////////////////
    // Master fragment interface
    /////////////////////////
    output wire                             m_frag_tvalid,
    output wire                             m_frag_tlast,
    input  wire                             m_frag_tready,
    output wire [PIXEL_WIDTH - 1 : 0]       m_frag_tdata,
    output wire [PIXEL_MASK_WIDTH - 1 : 0]  m_frag_tstrb,
    output wire [ADDR_WIDTH - 1 : 0]        m_frag_taddr
);

    // Pass through
    assign m_frag_tvalid = s_frag_tvalid;
    assign m_frag_tlast  = s_frag_tlast;
    assign s_frag_tready = m_frag_tready;
    assign m_frag_tdata  = s_frag_tdata;
    assign m_frag_tstrb  = s_frag_tstrb;

    // Address translation: convert pixel index to byte address and add base
    assign m_frag_taddr = (s_frag_taddr << PIXEL_WIDTH_LG) + confAddr;

endmodule
