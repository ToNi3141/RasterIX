// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

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

// Sampling SPI slave, which works with a maximum speed of sck <= clk/4.
// It samples also the csn line with two samples to avoid accidential
// de/assertions of this line to protect the transfer when using bad
// wiring.
// active is used to mark an active transfer. It is low when the csn or 
// resetn line are low. If this line goes down when csn and resetn are 
// low, then something with the physical wiring on a breadboard or 
// similar could be wrong.
module SpiSlave
(
    input  wire         clk,
    input  wire         resetn,

    input  wire         sck,
    input  wire         mosi,
    input  wire         csn,

    output reg  [7 : 0] data,
    output reg          valid,

    output reg          active
);
    reg [3 : 0] cnt;
    reg [1 : 0] clkSr;
    reg [1 : 0] csnSr;

    always @(posedge clk)
    begin
        csnSr <= { csnSr[0], csn };
        if (!resetn || &csnSr)
        begin
            valid <= 0;
            cnt <= 0;
            clkSr <= 0;
            active <= 0;
        end
        else
        begin
            clkSr <= { clkSr[0], sck };
            valid <= 0;
            if ((clkSr[1] == 0) && (clkSr[0] == 1))
            begin
                data <= { data[6 : 0], mosi };
                cnt <= cnt + 1;
                active <= 1;
                if (cnt == 4'h7)
                begin
                    valid <= 1;
                    cnt <= 4'h0;
                end
            end
        end
    end
endmodule