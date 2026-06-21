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

// Physical interface for the HyperRAM
module HyperRamPhy
#(
    parameter MEM_WIDTH = 8
)
(
    input  wire [MEM_WIDTH - 1 : 0]     s_phy_data_i,
    output wire [MEM_WIDTH - 1 : 0]     s_phy_data_o,
    input  wire [MEM_WIDTH - 1 : 0]     s_phy_data_noe,
    input  wire                         s_phy_rwds_i,
    output wire                         s_phy_rwds_o,
    input  wire                         s_phy_rwds_noe,
    input  wire                         s_phy_csn,
    input  wire                         s_phy_ck,
    input  wire                         s_phy_resetn,

    inout  wire [MEM_WIDTH - 1 : 0]     m_hr_data,
    inout  wire                         m_hr_rwds,
    output wire                         m_hr_csn,
    output wire                         m_hr_ck,
    output wire                         m_hr_resetn
);

    assign m_hr_csn = s_phy_csn;
    assign m_hr_ck = s_phy_ck;
    assign m_hr_resetn = s_phy_resetn;

    IOBUF #(
        .DRIVE(12), // Specify the output drive strength
        .IBUF_LOW_PWR("FALSE"),  // Low Power - "TRUE", High Performance = "FALSE"
        .IOSTANDARD("DEFAULT"), // Specify the I/O standard
        .SLEW("FAST") // Specify the output slew rate
    ) IOBUF_rwds (
        .O(s_phy_rwds_o),     // Buffer output
        .IO(m_hr_rwds),   // Buffer inout port (connect directly to top-level port)
        .I(s_phy_rwds_i),     // Buffer input
        .T(s_phy_rwds_noe)      // 3-state enable input, high=input, low=output
    );

    genvar i;
    generate
        for (i = 0; i < MEM_WIDTH; i = i + 1)
        begin
            IOBUF #(
                .DRIVE(12), // Specify the output drive strength
                .IBUF_LOW_PWR("FALSE"),  // Low Power - "TRUE", High Performance = "FALSE"
                .IOSTANDARD("DEFAULT"), // Specify the I/O standard
                .SLEW("FAST") // Specify the output slew rate
            ) IOBUF_data (
                .O(s_phy_data_o[i]),     // Buffer output
                .IO(m_hr_data[i]),   // Buffer inout port (connect directly to top-level port)
                .I(s_phy_data_i[i]),     // Buffer input
                .T(s_phy_data_noe[i])      // 3-state enable input, high=input, low=output
            );
        end
    endgenerate
endmodule