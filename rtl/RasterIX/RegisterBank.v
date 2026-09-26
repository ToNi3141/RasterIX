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

// This module deserializes an AXIS parameter stream into dedicated registers.
// It uses tuser as an address offset and can set several registers in one stream.
// For instance, assume a BANK_REG_WIDTH of 32 bit and CMD_STREAM_WIDTH of 32 bit.
// Beat 0 will set register 0 + tuser, beat 1 register 1 + tuser, beat n register n + tuser.
module RegisterBank
#(
    // Bank size in multiple of BANK_REG_WIDTH bit
    parameter BANK_SIZE = 8,

    // The bit width of the command interface. Allowed values: 32, 64, 128, 256
    parameter CMD_STREAM_WIDTH = 32,

    parameter COMPRESSED = 1,

    // Bank register size
    localparam BANK_REG_WIDTH = 32,

    localparam INDEX_WIDTH = $clog2(BANK_SIZE)
)
(
    input wire                              aclk,
    input wire                              resetn,

    // Register content stream
    input  wire                             s_axis_tvalid,
    input  wire                             s_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  s_axis_tdata,
    input  wire [INDEX_WIDTH - 1 : 0]       s_axis_tuser,

    // Register bank
    output wire [(BANK_SIZE * BANK_REG_WIDTH) - 1 : 0] registers,

    // Optional signals to inform other modules, that the registers have been updated
    output reg                              registers_updated,
    input  wire                             update_acknowledged
);
    localparam REGISTERS_PER_STREAM_BEAT = CMD_STREAM_WIDTH / BANK_REG_WIDTH;

    reg [BANK_REG_WIDTH - 1 : 0] registerMem [0 : BANK_SIZE - 1];
    reg [INDEX_WIDTH - 1 : 0] registerIndex;

    always @(posedge aclk)
    begin : Copy
        integer i;
        if (!resetn)
        begin
            registerIndex <= 0;
        end

        if (update_acknowledged)
        begin
            registers_updated <= 0;
        end
        
        if (s_axis_tvalid)
        begin
            if (COMPRESSED)
            begin
                for (i = 0; i < REGISTERS_PER_STREAM_BEAT; i = i + 1)
                begin
                    registerMem[registerIndex + s_axis_tuser + i[0 +: INDEX_WIDTH]] <= s_axis_tdata[BANK_REG_WIDTH * i +: BANK_REG_WIDTH];
                end
                registerIndex <= registerIndex + REGISTERS_PER_STREAM_BEAT[0 +: INDEX_WIDTH];
            end
            else
            begin
                registerMem[registerIndex + s_axis_tuser] <= s_axis_tdata[0 +: BANK_REG_WIDTH];
                registerIndex <= registerIndex + 1;
            end

            if (s_axis_tlast)
            begin
                registers_updated <= 1;
                registerIndex <= 0;
            end
        end
    end

    generate 
        genvar i;
        for(i = 0; i < BANK_SIZE; i = i + 1)
        begin
            assign registers[i * BANK_REG_WIDTH +: BANK_REG_WIDTH] = registerMem[i]; 
        end
    endgenerate
endmodule