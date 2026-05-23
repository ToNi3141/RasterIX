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

// This is a true dual port ram. The main difference to the DualPortRam wrapper
// is the data out port on the write channel.
// Pipelined: n/a
// Depth: 1 cycle
module TrueDualPortRam #(
    parameter MEM_WIDTH = 16, // Memory width in bits
    parameter WRITE_STROBE_WIDTH = 4, // Write strobe in bits
    parameter MEMORY_PRIMITIVE = "block", // "distribute" or "block"
    parameter ADDR_WIDTH = 8,
    localparam WRITE_MASK_SIZE = MEM_WIDTH / WRITE_STROBE_WIDTH
)
(
    input  wire                             clk,
    input  wire                             reset,

    // Write interface. When write is 0, writeDataOut can be used to read from this channel.
    input  wire [MEM_WIDTH - 1 : 0]         writeData,
    input  wire                             write,
    input  wire [ADDR_WIDTH - 1 : 0]        writeAddr,
    input  wire [WRITE_MASK_SIZE - 1 : 0]   writeMask,
    output wire [MEM_WIDTH - 1 : 0]         writeDataOut, 

    // Read interface
    output wire [MEM_WIDTH - 1 : 0]         readData,
    input  wire [ADDR_WIDTH - 1 : 0]        readAddr
);
    
`ifndef UNITTEST
    genvar i;
    generate 
        // Note: this macro has to be added to vivado
        // set_property XPM_LIBRARIES {XPM_MEMORY} [current_project]
        for (i = 0; i < WRITE_MASK_SIZE; i = i + 1)
        begin
            xpm_memory_tdpram #(
                .ADDR_WIDTH_A(ADDR_WIDTH),               // DECIMAL
                .ADDR_WIDTH_B(ADDR_WIDTH),               // DECIMAL
                .AUTO_SLEEP_TIME(0),            // DECIMAL
                .BYTE_WRITE_WIDTH_A(WRITE_STROBE_WIDTH),        // DECIMAL
                .BYTE_WRITE_WIDTH_B(WRITE_STROBE_WIDTH),        // DECIMAL
                .CASCADE_HEIGHT(0),             // DECIMAL
                .CLOCKING_MODE("common_clock"), // String
                .ECC_MODE("no_ecc"),            // String
                .MEMORY_INIT_FILE("none"),      // String
                .MEMORY_INIT_PARAM("0"),        // String
                .MEMORY_OPTIMIZATION("true"),   // String
                .MEMORY_PRIMITIVE(MEMORY_PRIMITIVE),      // String
                .MEMORY_SIZE((2**ADDR_WIDTH) * WRITE_STROBE_WIDTH),             // DECIMAL
                .MESSAGE_CONTROL(0),            // DECIMAL
                .READ_DATA_WIDTH_A(WRITE_STROBE_WIDTH),         // DECIMAL
                .READ_DATA_WIDTH_B(WRITE_STROBE_WIDTH),         // DECIMAL
                .READ_LATENCY_A(1),             // DECIMAL
                .READ_LATENCY_B(1),             // DECIMAL
                .READ_RESET_VALUE_A("0"),       // String
                .READ_RESET_VALUE_B("0"),       // String
                .RST_MODE_A("SYNC"),            // String
                .RST_MODE_B("SYNC"),            // String
                .SIM_ASSERT_CHK(0),             // DECIMAL; 0=disable simulation messages, 1=enable simulation messages
                .USE_EMBEDDED_CONSTRAINT(0),    // DECIMAL
                .USE_MEM_INIT(0),               // DECIMAL
                .WAKEUP_TIME("disable_sleep"),  // String
                .WRITE_DATA_WIDTH_A(WRITE_STROBE_WIDTH),        // DECIMAL
                .WRITE_DATA_WIDTH_B(WRITE_STROBE_WIDTH),        // DECIMAL
                .WRITE_MODE_A("read_first"),     // String
                .WRITE_MODE_B("read_first")      // String
            )
            xpm_memory_tdpram_inst (
                .dbiterra(),             // 1-bit output: Status signal to indicate double bit error occurrence
                                                    // on the data output of port A.

                .dbiterrb(),             // 1-bit output: Status signal to indicate double bit error occurrence
                                                    // on the data output of port A.

                .douta(writeDataOut[((i + 1) * WRITE_STROBE_WIDTH) - 1 : i * WRITE_STROBE_WIDTH]),                   // READ_DATA_WIDTH_A-bit output: Data output for port A read operations.
                .doutb(readData[((i + 1) * WRITE_STROBE_WIDTH) - 1 : i * WRITE_STROBE_WIDTH]),                   // READ_DATA_WIDTH_B-bit output: Data output for port B read operations.
                .sbiterra(),             // 1-bit output: Status signal to indicate single bit error occurrence
                                                    // on the data output of port A.

                .sbiterrb(),             // 1-bit output: Status signal to indicate single bit error occurrence
                                                    // on the data output of port B.

                .addra(writeAddr),                   // ADDR_WIDTH_A-bit input: Address for port A write and read operations.
                .addrb(readAddr),                   // ADDR_WIDTH_B-bit input: Address for port B write and read operations.
                .clka(clk),                     // 1-bit input: Clock signal for port A. Also clocks port B when
                                                    // parameter CLOCKING_MODE is "common_clock".

                .clkb(clk),                     // 1-bit input: Clock signal for port B when parameter CLOCKING_MODE is
                                                    // "independent_clock". Unused when parameter CLOCKING_MODE is
                                                    // "common_clock".

                .dina(writeData[((i + 1) * WRITE_STROBE_WIDTH) - 1 : i * WRITE_STROBE_WIDTH]),                     // WRITE_DATA_WIDTH_A-bit input: Data input for port A write operations.
                .dinb(0),                     // WRITE_DATA_WIDTH_B-bit input: Data input for port B write operations.
                .ena(1),                       // 1-bit input: Memory enable signal for port A. Must be high on clock
                                                    // cycles when read or write operations are initiated. Pipelined
                                                    // internally.

                .enb(1),                       // 1-bit input: Memory enable signal for port B. Must be high on clock
                                                    // cycles when read or write operations are initiated. Pipelined
                                                    // internally.

                .injectdbiterra(0), // 1-bit input: Controls double bit error injection on input data when
                                                    // ECC enabled (Error injection capability is not available in
                                                    // "decode_only" mode).

                .injectdbiterrb(0), // 1-bit input: Controls double bit error injection on input data when
                                                    // ECC enabled (Error injection capability is not available in
                                                    // "decode_only" mode).

                .injectsbiterra(0), // 1-bit input: Controls single bit error injection on input data when
                                                    // ECC enabled (Error injection capability is not available in
                                                    // "decode_only" mode).

                .injectsbiterrb(0), // 1-bit input: Controls single bit error injection on input data when
                                                    // ECC enabled (Error injection capability is not available in
                                                    // "decode_only" mode).

                .regcea(0),                 // 1-bit input: Clock Enable for the last register stage on the output
                                                    // data path.

                .regceb(0),                 // 1-bit input: Clock Enable for the last register stage on the output
                                                    // data path.

                .rsta(reset),                     // 1-bit input: Reset signal for the final port A output register stage.
                                                    // Synchronously resets output port douta to the value specified by
                                                    // parameter READ_RESET_VALUE_A.

                .rstb(reset),                     // 1-bit input: Reset signal for the final port B output register stage.
                                                    // Synchronously resets output port doutb to the value specified by
                                                    // parameter READ_RESET_VALUE_B.

                .sleep(0),                   // 1-bit input: sleep signal to enable the dynamic power saving feature.
                .wea(writeMask[i] & write),                       // WRITE_DATA_WIDTH_A/BYTE_WRITE_WIDTH_A-bit input: Write enable vector
                                                    // for port A input data port dina. 1 bit wide when word-wide writes are
                                                    // used. In byte-wide write configurations, each bit controls the
                                                    // writing one byte of dina to address addra. For example, to
                                                    // synchronously write only bits [15-8] of dina when WRITE_DATA_WIDTH_A
                                                    // is 32, wea would be 4'b0010.

                .web(0)                        // WRITE_DATA_WIDTH_B/BYTE_WRITE_WIDTH_B-bit input: Write enable vector
                                                    // for port B input data port dinb. 1 bit wide when word-wide writes are
                                                    // used. In byte-wide write configurations, each bit controls the
                                                    // writing one byte of dinb to address addrb. For example, to
                                                    // synchronously write only bits [15-8] of dinb when WRITE_DATA_WIDTH_B
                                                    // is 32, web would be 4'b0010.
            );
        end
    endgenerate
`else
    reg [MEM_WIDTH  - 1 : 0]    mem [(1 << ADDR_WIDTH) - 1 : 0];
    reg [MEM_WIDTH - 1 : 0]     memOut;
    reg [MEM_WIDTH - 1 : 0]     writeMemOut;
    assign readData = memOut;
    assign writeDataOut = writeMemOut;
    integer i;

    always @(posedge clk)
    begin
        if (write)
        begin
            for (i = 0; i < WRITE_MASK_SIZE; i = i + 1)
            begin
                if (writeMask[i])
                    mem[writeAddr][(i * WRITE_STROBE_WIDTH) +: WRITE_STROBE_WIDTH] <= writeData[(i * WRITE_STROBE_WIDTH) +: WRITE_STROBE_WIDTH];
        
            end
        end
        writeMemOut <= mem[writeAddr];
        memOut <= mem[readAddr];
    end
`endif
endmodule
