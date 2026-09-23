// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2026 ToNi3141

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

// Texture MMU (Memory Management Unit) which contains a complete 
// page table to translate a complete texture from virtual to physical addresses.
// Pipelined: n/a
// Depth: combinational
module TextureMMU #(
    parameter TEX_ADDR_WIDTH = 18,

    parameter PAGE_SIZE = 2048,

    parameter ID_WIDTH = 4,
    parameter ADDR_WIDTH = 32
)
(
    input  wire                             aclk,
    input  wire                             resetn,

    // Pagetable interface
    input  wire                             s_axis_tvalid,
    output wire                             s_axis_tready,
    input  wire                             s_axis_tlast,
    input  wire [ADDR_WIDTH - 1 : 0]        s_axis_tdata,

    // Input interface
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    s_axi_araddr,
    input  wire [ID_WIDTH - 1 : 0]          s_axi_arid,
    input  wire [ 7 : 0]                    s_axi_arlen,
    input  wire [ 2 : 0]                    s_axi_arsize,
    input  wire [ 1 : 0]                    s_axi_arburst,
    input  wire                             s_axi_arlock,
    input  wire [ 3 : 0]                    s_axi_arcache,
    input  wire [ 2 : 0]                    s_axi_arprot,
    input  wire                             s_axi_arvalid,
    output wire                             s_axi_arready,

    // Output interface
    output wire [ID_WIDTH - 1 : 0]          m_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_axi_araddr,
    output wire [ 7 : 0]                    m_axi_arlen,
    output wire [ 2 : 0]                    m_axi_arsize,
    output wire [ 1 : 0]                    m_axi_arburst,
    output wire                             m_axi_arlock,
    output wire [ 3 : 0]                    m_axi_arcache,
    output wire [ 2 : 0]                    m_axi_arprot,
    output wire                             m_axi_arvalid,
    input  wire                             m_axi_arready
);
    localparam PAGE_ENTRIES_LG = TEX_ADDR_WIDTH - $clog2(PAGE_SIZE);

    reg [ADDR_WIDTH - 1 : 0] page_table [0 : (1 << PAGE_ENTRIES_LG) - 1];

    assign s_axis_tready = 1'b1;

    assign m_axi_arid = s_axi_arid;
    wire [ADDR_WIDTH - 1 : 0] page_offset
        = { { (ADDR_WIDTH - (TEX_ADDR_WIDTH - PAGE_ENTRIES_LG)) { 1'b0 } },
            s_axi_araddr[0 +: TEX_ADDR_WIDTH - PAGE_ENTRIES_LG] };
    assign m_axi_araddr = page_table[s_axi_araddr[TEX_ADDR_WIDTH - PAGE_ENTRIES_LG +: PAGE_ENTRIES_LG]]
                            + page_offset;
    assign m_axi_arlen = s_axi_arlen;
    assign m_axi_arsize = s_axi_arsize;
    assign m_axi_arburst = s_axi_arburst;
    assign m_axi_arlock = s_axi_arlock;
    assign m_axi_arcache = s_axi_arcache;
    assign m_axi_arprot = s_axi_arprot;
    assign m_axi_arvalid = s_axi_arvalid;
    assign s_axi_arready = m_axi_arready;

    reg [PAGE_ENTRIES_LG - 1 : 0] page_table_index;
    always @(posedge aclk) 
    begin
        if (!resetn) 
        begin
            page_table_index <= 0;
        end 
        else 
        begin
            if (s_axis_tvalid)
            begin
                page_table[page_table_index] <= s_axis_tdata[ADDR_WIDTH - 1 : 0];
                page_table_index <= page_table_index + 1;
                if (s_axis_tlast)
                begin
                    page_table_index <= 0;
                end
            end
        end
    end
endmodule 