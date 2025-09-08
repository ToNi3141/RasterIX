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

// Simple bridge to connect a AXIS device to an serial port like SPI

module Serial2AXIS 
#(
    parameter FIFO_SIZE = 4096,
    parameter FIFO_TRESHOLD = FIFO_SIZE / 4
)
(
    input  wire         aclk,
    input  wire         resetn, 

    input  wire         serial_mosi,
    output wire         serial_miso,
    input  wire         serial_sck,
    input  wire         serial_ncs,
    output reg          serial_cts,

    output reg          m_axis_tvalid,
    input  wire         m_axis_tready,
    output reg          m_axis_tlast,
    output reg  [31:0]  m_axis_tdata,

    input  wire         s_axis_tvalid,
    output wire         s_axis_tready,
    input  wire         s_axis_tlast,
    input  wire [31:0]  s_axis_tdata
);
    localparam BW = 8;
    localparam LGFLEN = $clog2(FIFO_SIZE);

    wire [7:0]  mosi;
    wire        rxDone;
    reg  [31:0] axisData;
    wire [LGFLEN : 0]  fill;
    reg         fifoReadEnable; // Read enable controlled from the state machine below
    wire [BW - 1 : 0]  fifoReadData;
    wire        fifoEmpty;

    wire        readFromFifo = fifoReadEnable;

    // SPI_Slave #(.SPI_MODE(0)) spiSlave (resetn, aclk, rxDone, mosi, 0, 0, serial_sck, serial_miso, serial_mosi, serial_ncs);
    //uart_rx #(.CLKS_PER_BIT(16)) rxUart(aclk, rx, rxDone, mosi);
    SpiSlave spiSlave (
        .clk(aclk),
        .resetn(resetn),
        .sck(serial_sck),
        .mosi(serial_mosi),
        .active(serial_miso), // The miso line is currently used to mark if the slave works on an active transfer
        .csn(serial_ncs),
        .data(mosi),
        .valid(rxDone)
    );

    sfifo #(.BW(BW), .LGFLEN(LGFLEN), .OPT_ASYNC_READ(0))
        sfifoInst
        (
            .i_clk(aclk),
            .i_reset(!resetn),

            // Write channel
            .i_wr(rxDone),
            .i_data(mosi),
            .o_full(),
            .o_fill(fill),

            // Read channel
            .i_rd(readFromFifo), 
            .o_data(fifoReadData),
            .o_empty(fifoEmpty)
        );

    assign s_axis_tready = 1'b1;

    always @(posedge aclk)
        serial_cts <= fill <= FIFO_TRESHOLD;

    localparam WAIT_FOR_DATA = 0;
    localparam WAIT_FOR_DATA2 = 1;
    localparam WAIT_FOR_DATA3 = 2;
    localparam WAIT_FOR_DATA4 = 3;
    localparam AXIS_REQUEST = 4;
    localparam TRANSFER_END = 5;
    localparam WAIT_FOR_SYNC = 6;

    reg [5:0] state;

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            m_axis_tvalid <= 0;
            m_axis_tlast <= 0;
            fifoReadEnable <= 1;
            state <= WAIT_FOR_DATA;
        end
        else
        begin
            case (state)
            WAIT_FOR_DATA:
            begin
                // Wait till the FIFO contains data
                if (!fifoEmpty)
                begin
                    axisData[0 +: 8] <= fifoReadData; 
                    state <= WAIT_FOR_DATA2;
                end
            end
            WAIT_FOR_DATA2:
            begin
                if (!fifoEmpty)
                begin
                    axisData[8 +: 8] <= fifoReadData;
                    state <= WAIT_FOR_DATA3;
                end
            end
            WAIT_FOR_DATA3:
            begin
                if (!fifoEmpty)
                begin
                    axisData[16 +: 8] <= fifoReadData;
                    state <= WAIT_FOR_DATA4;
                end
            end
            WAIT_FOR_DATA4:
            begin
                if (!fifoEmpty)
                begin
                    axisData[24 +: 8] <= fifoReadData;
                    fifoReadEnable <= 0;
                    state <= AXIS_REQUEST;
                end
            end
            AXIS_REQUEST:
            begin
                m_axis_tvalid <= 1;
                m_axis_tdata <= axisData;
                state <= TRANSFER_END;
            end
            TRANSFER_END:
            begin
                if (m_axis_tready)
                begin
                    m_axis_tvalid <= 0;
                    fifoReadEnable <= 1;
                    state <= WAIT_FOR_DATA;
                end
            end
            endcase
        end
    end
endmodule

