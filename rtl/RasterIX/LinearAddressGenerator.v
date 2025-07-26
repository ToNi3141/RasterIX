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


module LinearAddressGenerator #(
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,

    // Width of ID signal
    parameter ID_WIDTH = 8,

    // Number of beats per stream (axlen)
    // Note: AxLEN means number of beats minus one. See also the AXI4 specification.
    parameter AxLEN_BEATS_PER_TRANSFER = 15, // 15 means 16 beats per transfer

    // Number of bytes per stream beat (axsize)
    // Note: AxSIZE means number of bytes per beat in log2. See also the AXI4 specification.
    parameter AxSIZE_BYTES_PER_BEAT = 3, // 2^3 = 8 bytes per beat

    // AxBURST type
    // Note: 0 fixed, 1 incr, 2 wrapping
    parameter AxBURST = 1 // 1 means incrementing burst
) (
    input  wire                         aclk,
    input  wire                         resetn,

    input  wire                         start,
    output reg                          done,

    input  wire [ADDR_WIDTH - 1 : 0]    startAddr,
    input  wire [ADDR_WIDTH - 1 : 0]    dataSizeInBeats,

    // Address channel
    output reg  [ID_WIDTH - 1 : 0]      axid,
    output reg  [ADDR_WIDTH - 1 : 0]    axaddr,
    output reg  [ 7 : 0]                axlen,
    output reg  [ 2 : 0]                axsize,
    output reg  [ 1 : 0]                axburst,
    output reg                          axlock,
    output reg  [ 3 : 0]                axcache,
    output reg  [ 2 : 0]                axprot, 
    output reg                          axvalid,
    input  wire                         axready
);
    localparam BYTES_PER_BEAT = 2 ** AxSIZE_BYTES_PER_BEAT;
    localparam BEATS_PER_TRANSFER = AxLEN_BEATS_PER_TRANSFER + 1;
    
    reg  [ADDR_WIDTH - 1 : 0] addr;
    reg  [ADDR_WIDTH - 1 : 0] addrLast;
    wire [ADDR_WIDTH - 1 : 0] addrNext = addr + (BYTES_PER_BEAT * BEATS_PER_TRANSFER);

    initial 
    begin
        axid = 0;
        axlen = AxLEN_BEATS_PER_TRANSFER[0 +: 8];
        axsize = AxSIZE_BYTES_PER_BEAT[0 +: 3];
        axburst = AxBURST[0 +: 2];
        axlock = 0;
        axcache = 0; 
        axprot = 0;
        axvalid = 0;
    end

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            axvalid <= 0;
            done <= 1;
        end
        else
        begin
            if (start && done)
            begin
                addr <= startAddr;
                addrLast <= startAddr + (dataSizeInBeats << AxSIZE_BYTES_PER_BEAT) - 1;
                axaddr <= startAddr;
                axvalid <= 1;
                done <= 0;
            end

            if (!done)
            begin
                if (addrNext < addrLast)
                begin
                    if (axready && axvalid) 
                    begin
                        addr <= addrNext;
                        axaddr <= addrNext;
                    end
                end
                else if (axready)
                begin
                    axvalid <= 0;
                    done <= 1;
                end
            end
        end
    end
endmodule
