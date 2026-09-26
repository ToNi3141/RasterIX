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

// Deserializes a complete stream into parallel words.
// After BANK_SIZE valid beats, registers[0 +: REGISTER_WIDTH] is the first beat.
// Unlike RegisterBank, this has no address decoder or per-word write enable.
// Extra valid beats are discarded until a valid tlast starts a new stream.
module ShiftRegisterBank #(
    parameter BANK_SIZE = 8,
    parameter REGISTER_WIDTH = 32
)(
    input  wire                        aclk,
    input  wire                        resetn,
    input  wire                        s_axis_tvalid,
    input  wire                        s_axis_tlast,
    input  wire [REGISTER_WIDTH - 1 : 0] s_axis_tdata,
    output wire [(BANK_SIZE * REGISTER_WIDTH) - 1 : 0] registers,
    output reg                         registers_updated,
    input  wire                        update_acknowledged
);
    localparam COUNT_WIDTH = $clog2(BANK_SIZE + 1);

    reg [(BANK_SIZE * REGISTER_WIDTH) - 1 : 0] shiftRegisters;
    reg [COUNT_WIDTH - 1 : 0] beatCount;
    assign registers = shiftRegisters;

    always @(posedge aclk)
    begin
        if (s_axis_tvalid && resetn && (beatCount < BANK_SIZE))
        begin
            shiftRegisters <= { s_axis_tdata, shiftRegisters[(BANK_SIZE * REGISTER_WIDTH) - 1 : REGISTER_WIDTH] };
        end

        if (!resetn)
        begin
            beatCount <= 0;
            registers_updated <= 0;
        end
        else
        begin
            if (s_axis_tvalid)
            begin
                if (s_axis_tlast)
                begin
                    beatCount <= 0;
                end
                else if (beatCount < BANK_SIZE)
                begin
                    beatCount <= beatCount + 1'b1;
                end
            end

            if (update_acknowledged)
            begin
                registers_updated <= 0;
            end
            if (s_axis_tvalid && s_axis_tlast)
            begin
                registers_updated <= 1;
            end
        end
    end
endmodule