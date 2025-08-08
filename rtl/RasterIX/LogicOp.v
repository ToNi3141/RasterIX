// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2025 ToNi3141

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

// This module implements the following OPs
// OP:
//   CLEAR = 0
//   SET = 1
//   COPY = 2
//   COPY_INVERTED = 3
//   NOOP = 4
//   INVERT = 5
//   AND = 6
//   NAND = 7
//   OR = 8
//   NOR = 9
//   XOR = 10
//   EQUIV = 11
//   AND_REVERSE = 12
//   AND_INVERTED = 13
//   OR_REVERSE = 14
//   OR_INVERTED = 15
// Pipelined: n/a
// Depth: 1 cycle
module LogicOp 
#(
    parameter PIXEL_WIDTH = 4
)
(
    input  wire                         aclk,
    input  wire                         resetn,
    input  wire                         ce,
    
    input  wire [ 3 : 0]                op,
    input  wire                         enable,

    input  wire [PIXEL_WIDTH - 1 : 0]   source,
    input  wire [PIXEL_WIDTH - 1 : 0]   dest,
    output reg  [PIXEL_WIDTH - 1 : 0]   out
);
`include "RegisterAndDescriptorDefines.vh"

    wire [PIXEL_WIDTH - 1 : 0] invertedSource = ~source;
    wire [PIXEL_WIDTH - 1 : 0] invertedTest = ~dest; 

    always @(posedge aclk)
    if (ce) begin : TestFunc
        reg [PIXEL_WIDTH - 1 : 0] tmp;

        case (op)
        CLEAR:
        begin
            tmp = 0;
        end
        SET:
        begin
            tmp = ~0;
        end
        COPY:
        begin
            tmp = source;
        end
        COPY_INVERTED:
        begin
            tmp = invertedSource;
        end
        NOOP:
        begin
            tmp = dest;
        end
        INVERT:
        begin
            tmp = invertedSource;
        end
        AND:
        begin
            tmp = source & dest;
        end
        NAND:
        begin
            tmp = ~(source & dest);
        end
        OR:
        begin
            tmp = source | dest;
        end
        NOR:
        begin
            tmp = ~(source | dest);
        end
        XOR:
        begin
            tmp = source ^ dest;
        end
        EQUIV:
        begin
            tmp = ~(source ^ dest);
        end
        AND_REVERSE:
        begin
            tmp = source & invertedTest;
        end
        AND_INVERTED:
        begin
            tmp = invertedSource & dest;
        end
        OR_REVERSE:
        begin
            tmp = source | invertedTest;
        end
        OR_INVERTED:
        begin
            tmp = invertedSource | dest;
        end
        endcase

        if (enable)
        begin
            out <= tmp;
        end
        else
        begin
            out <= source;
        end
    end
endmodule