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

// The function interpolator will interpolate a mathematical function based on a LUT
// The used interpolation equation is: f(x) = m * x + b
// where:
// x: User input, mantissa of this value is used as multiplicand for m
// m: slope stored in the LUT and is accessed via the x's exponent
// b: offset stored in the LUT and is accessed via the x's exponent
// The LUT must have a logarithmic distribution.
// To make the interpolation more convenient and to save a reciprocal calculation, 
// the interpolator can use x directly as reciprocal. The downside of this strategy is the
// reduced precision.
// The function interpolator has 32 entries to cover input values from 1.0 to 4294967296.0
// Dataformat
// Beat 0 Lower bound in 1/x form 
// tdata as float. If x is lower, fx will be 1.0
// Beat 1 Upper bound in 1/x form
// tdata as float. If x is higher, fx will be 0.0
// Beat 2 .. 66 
// Even beat: m = tdata as S9.22
// Odd beat b = tdata as S1.30
//
// This module is pipelined. It requires 2 clock cycles until it outputs the calculated value.
module FunctionInterpolator #(
    localparam LUT_ENTRY_FIELD_WIDTH = 32,
    localparam FLOAT_WIDTH = LUT_ENTRY_FIELD_WIDTH,
    localparam INT_WIDTH = 24
)
(
    input  wire                         aclk,
    input  wire                         resetn,
    input  wire                         ce,

    input  wire [FLOAT_WIDTH - 1 : 0]   x, // IEEE 754 32bit float
    output reg  signed [INT_WIDTH - 1 : 0] fx, // 24bit signed fixpoint S1.22 number

    // LUT data
    input  wire                         s_axis_tvalid,
    output reg                          s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire [31 : 0]                s_axis_tdata
);
    localparam LUT_INTERPOLATION_STEPS = 8; // Defines the steps between two LUT entries. The range between x and x + 1 will be divided by pow(2, LUT_INTERPOLATION_STEPS) 
    localparam LUT_ENTRIES = 32;

    localparam FLOAT_EXP_SIZE = 8;
    localparam FLOAT_EXP_POS = 23;
    localparam FLOAT_MANTISSA_SIZE = 23;
    localparam FLOAT_MANTISSA_POS = 0;
    localparam FLOAT_EXP_BIAS = 126;

    localparam STATE_WRITE_LOWER_BOUND = 0;
    localparam STATE_WRITE_UPPER_BOUND = 1;
    localparam STATE_WRITE_LUT_M = 2;
    localparam STATE_WRITE_LUT_B = 3;

    // LUT bounds
    reg  [FLOAT_WIDTH - 1 : 0]  lutLowerBound = 0;
    reg  [FLOAT_WIDTH - 1 : 0]  lutUpperBound = 0;
    reg  [INT_WIDTH - 1 : 0]    lutM[0 : LUT_ENTRIES - 1];
    reg  [INT_WIDTH - 1 : 0]    lutB[0 : LUT_ENTRIES - 1];

    // LUT writer
    always @(posedge aclk)
    begin : LutWriter
        // LUT memory access
        reg  [1 : 0]                        writeState;
        reg  [$clog2(LUT_ENTRIES) - 1 : 0]  memWriteAddr;

        if (!resetn)
        begin
            memWriteAddr <= 0;
            writeState <= STATE_WRITE_LOWER_BOUND;
            s_axis_tready <= 1;
        end
        else
        begin
            if (s_axis_tvalid)
            begin
                case (writeState)
                STATE_WRITE_LOWER_BOUND:
                begin
                    lutLowerBound <= s_axis_tdata[0 +: FLOAT_WIDTH];
                    writeState <= STATE_WRITE_UPPER_BOUND;
                end
                STATE_WRITE_UPPER_BOUND:
                begin
                    lutUpperBound <= s_axis_tdata[0 +: FLOAT_WIDTH];
                    writeState <= STATE_WRITE_LUT_M;
                end
                STATE_WRITE_LUT_M:
                begin
                    lutM[memWriteAddr] <= s_axis_tdata[LUT_ENTRY_FIELD_WIDTH - INT_WIDTH +: INT_WIDTH];
                    writeState <= STATE_WRITE_LUT_B;
                end
                STATE_WRITE_LUT_B:
                begin
                    lutB[memWriteAddr] <= s_axis_tdata[LUT_ENTRY_FIELD_WIDTH - INT_WIDTH +: INT_WIDTH];
                    memWriteAddr <= memWriteAddr + 1;
                    writeState <= STATE_WRITE_LUT_M;
                end
                endcase
            end
            if (s_axis_tlast)
            begin
                memWriteAddr <= 0;
                writeState <= STATE_WRITE_LOWER_BOUND;
            end
        end
    end

    // Interpolation
    always @(posedge aclk)
    if (ce) begin : Interpolation
        // bounds
        reg lowerBoundExceeded;
        reg upperBoundExceeded;

        // Float unpacking
        reg  [FLOAT_EXP_SIZE - 1 : 0]       floatExp;
        reg  [FLOAT_MANTISSA_SIZE - 1 : 0]  floatMantissa;

        // Interpolation
        reg  signed [INT_WIDTH - 1 : 0]                             m;
        reg  signed [INT_WIDTH - 1 : 0]                             b;
        reg         [LUT_INTERPOLATION_STEPS - 1 : 0]               xs;
        reg  signed [(INT_WIDTH + LUT_INTERPOLATION_STEPS) - 1 : 0] mx;
        reg  signed [INT_WIDTH - 1 : 0]                             mxb;

        ///////////////////////////////
        // Clock 0
        ///////////////////////////////
        // Access mantissa and LUT values
        floatMantissa = -x[FLOAT_MANTISSA_POS +: FLOAT_MANTISSA_SIZE];
        xs <= floatMantissa[FLOAT_MANTISSA_SIZE - LUT_INTERPOLATION_STEPS +: LUT_INTERPOLATION_STEPS];

        lowerBoundExceeded <= x >= lutLowerBound;
        upperBoundExceeded <= x <= lutUpperBound;

        // LUT access
        floatExp = FLOAT_EXP_BIAS - x[FLOAT_EXP_POS +: FLOAT_EXP_SIZE];
        m <= lutM[floatExp[0 +: $clog2(LUT_ENTRIES)]];
        b <= lutB[floatExp[0 +: $clog2(LUT_ENTRIES)]];

        ///////////////////////////////
        // Clock 1
        ///////////////////////////////
        // Calculate (interpolation)
        mx = m * xs;
        mxb = mx[0 +: INT_WIDTH] + b;

        // Clamp to bounds
        if (lowerBoundExceeded) 
        begin
            fx <= { 1'b0, 1'b1, { (INT_WIDTH - 2) { 1'b0 } } };
        end
        else if (upperBoundExceeded)
        begin
            fx <= 0;
        end
        else
        begin
            fx <= mxb;
        end
    end
endmodule
