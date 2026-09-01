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

module TextureMMU #(
    parameter TEX_ADDR_WIDTH = 17,

    parameter PAGE_SIZE = 2048,

    parameter DATA_WIDTH = 16,
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
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    s_araddr,
    input  wire                             s_arvalid,
    output wire                             s_arready,

    output wire [DATA_WIDTH - 1 : 0]        s_rdata,
    output wire                             s_rvalid,
    input  wire                             s_rready,

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
    input  wire                             m_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]          m_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]        m_axi_rdata,
    input  wire [ 1 : 0]                    m_axi_rresp,
    input  wire                             m_axi_rlast,
    input  wire                             m_axi_rvalid,
    output wire                             m_axi_rready
);
    localparam PAGE_ENTRIES_LG = TEX_ADDR_WIDTH - $clog2(PAGE_SIZE);

    reg [ADDR_WIDTH - 1 : 0] page_table [0 : (1 << PAGE_ENTRIES_LG) - 1];

    assign s_axis_tready = 1'b1;

    assign s_rdata = m_axi_rdata;
    assign s_rvalid = m_axi_rvalid;
    assign m_axi_rready = s_rready;

    assign m_axi_arid = 0;
    wire [ADDR_WIDTH - 1 : 0] page_offset
        = { { (ADDR_WIDTH - (TEX_ADDR_WIDTH - PAGE_ENTRIES_LG)) { 1'b0 } },
           s_araddr[0 +: TEX_ADDR_WIDTH - PAGE_ENTRIES_LG] };
    assign m_axi_araddr = page_table[s_araddr[TEX_ADDR_WIDTH - PAGE_ENTRIES_LG +: PAGE_ENTRIES_LG]]
                            + page_offset;
    assign m_axi_arlen = 0; // Single beat
    assign m_axi_arsize = 3'b001; // 2 bytes per beat
    assign m_axi_arburst = 2'b01; // INCR burst
    assign m_axi_arlock = 0;
    assign m_axi_arcache = 4'b0011; // Normal cacheable
    assign m_axi_arprot = 3'b000;
    assign m_axi_arvalid = s_arvalid;
    assign s_arready = m_axi_arready;

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