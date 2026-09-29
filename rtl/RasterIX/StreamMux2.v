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

// Two-input ready/valid stream mux. select=0 selects port 0; select=1 selects port 1.
module StreamMux2 #(
    parameter DATA_WIDTH = 32
) (
    input  wire                       select,

    input  wire                       s0_valid,
    input  wire                       s0_last,
    output wire                       s0_ready,
    input  wire [DATA_WIDTH - 1 : 0]  s0_data,

    input  wire                       s1_valid,
    input  wire                       s1_last,
    output wire                       s1_ready,
    input  wire [DATA_WIDTH - 1 : 0]  s1_data,

    output wire                       m_valid,
    output wire                       m_last,
    input  wire                       m_ready,
    output wire [DATA_WIDTH - 1 : 0]  m_data
);
    assign s0_ready = !select && m_ready;
    assign s1_ready = select && m_ready;

    assign m_valid = select ? s1_valid : s0_valid;
    assign m_last = select ? s1_last : s0_last;
    assign m_data = select ? s1_data : s0_data;

endmodule