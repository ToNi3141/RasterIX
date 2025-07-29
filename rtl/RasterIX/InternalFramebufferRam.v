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

// Wrapper around the dual port RAM to provide a
// multiplexed interface for two ports.
module InternalFramebufferRam
#(
    parameter ADDR_WIDTH = 8,
    parameter DATA_WIDTH = 16,
    parameter WRITE_STROBE_WIDTH = 8,
    localparam WRITE_MASK_WIDTH = DATA_WIDTH / WRITE_STROBE_WIDTH
)
(
    input   wire                            clk,
    input   wire                            reset,

    input   wire                            enablePort0,
    input   wire                            enablePort1,

    // Write interface port 1
    input  wire [DATA_WIDTH - 1 : 0]        writeDataPort0,
    input  wire                             writeEnablePort0,
    input  wire [ADDR_WIDTH - 1 : 0]        writeAddrPort0,
    input  wire [WRITE_MASK_WIDTH - 1 : 0]  writeMaskPort0, 

    // Read interface port 1
    output wire [DATA_WIDTH - 1 : 0]        readDataPort0,
    input  wire [ADDR_WIDTH - 1 : 0]        readAddrPort0,

    // Write interface port 2
    input  wire [DATA_WIDTH - 1 : 0]        writeDataPort1,
    input  wire                             writeEnablePort1,
    input  wire [ADDR_WIDTH - 1 : 0]        writeAddrPort1,
    input  wire [WRITE_MASK_WIDTH - 1 : 0]  writeMaskPort1, 

    // Read interface port 2
    output wire [DATA_WIDTH - 1 : 0]        readDataPort1,
    input  wire [ADDR_WIDTH - 1 : 0]        readAddrPort1
);
    // Multiplexed interface to the internal memory
    wire [ADDR_WIDTH - 1 : 0]       memBusAddrWrite     = (enablePort0) ? writeAddrPort0    : writeAddrPort1;
    wire [DATA_WIDTH - 1 : 0]       memBusDataIn        = (enablePort0) ? writeDataPort0    : writeDataPort1;
    wire                            memBusWriteEnable   = (enablePort0) ? writeEnablePort0  : writeEnablePort1;
    wire [WRITE_MASK_WIDTH - 1 : 0] memBusWriteMask     = (enablePort0) ? writeMaskPort0    : writeMaskPort1;
    wire [ADDR_WIDTH - 1 : 0]       memBusAddrRead      = (enablePort0) ? readAddrPort0     : readAddrPort1;
    wire [DATA_WIDTH - 1 : 0]       memBusDataOut;
    assign readDataPort0 = memBusDataOut;
    assign readDataPort1 = memBusDataOut;

    // Instance of the internal memory
    DualPortRam ramTile (
        .clk(clk),
        .reset(reset),

        .writeData(memBusDataIn),
        .write(memBusWriteEnable & (enablePort0 | enablePort1)),
        .writeAddr(memBusAddrWrite),
        .writeMask(memBusWriteMask),

        .readData(memBusDataOut),
        .readAddr(memBusAddrRead)
    );
    defparam ramTile.ADDR_WIDTH = ADDR_WIDTH;
    defparam ramTile.MEM_WIDTH = DATA_WIDTH;
    defparam ramTile.WRITE_STROBE_WIDTH = WRITE_STROBE_WIDTH;
endmodule
