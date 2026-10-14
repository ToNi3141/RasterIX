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

// Concatenates four streams into one stream.
module StreamConcatFifo
#(
    parameter STREAM0_WIDTH = 8,
    parameter STREAM1_WIDTH = 8,
    parameter STREAM2_WIDTH = 8,
    parameter STREAM3_WIDTH = 8,

    parameter FIFO_DEPTH0_POW2 = 5,
    parameter FIFO_DEPTH1_POW2 = 5,
    parameter FIFO_DEPTH2_POW2 = 5,
    parameter FIFO_DEPTH3_POW2 = 5,

    localparam STREAMO_WIDTH = STREAM0_WIDTH + STREAM1_WIDTH + STREAM2_WIDTH + STREAM3_WIDTH,
    localparam NUMBER_OF_STREAMS = 4
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    input  wire                         s_stream0_tenable,
    input  wire                         s_stream0_tvalid,
    input  wire [STREAM0_WIDTH - 1 : 0] s_stream0_tdata,
    output wire                         s_stream0_tready,

    input  wire                         s_stream1_tenable,
    input  wire                         s_stream1_tvalid,
    input  wire [STREAM1_WIDTH - 1 : 0] s_stream1_tdata,
    output wire                         s_stream1_tready,

    input  wire                         s_stream2_tenable,
    input  wire                         s_stream2_tvalid,
    input  wire [STREAM2_WIDTH - 1 : 0] s_stream2_tdata,
    output wire                         s_stream2_tready,

    input  wire                         s_stream3_tenable,
    input  wire                         s_stream3_tvalid,
    input  wire [STREAM3_WIDTH - 1 : 0] s_stream3_tdata,
    output wire                         s_stream3_tready,

    output wire                         m_stream_tvalid,
    output wire [STREAMO_WIDTH - 1 : 0] m_stream_tdata,
    input  wire                         m_stream_tready
);
    wire [NUMBER_OF_STREAMS - 1 : 0]    stream_full;
    wire [NUMBER_OF_STREAMS - 1 : 0]    stream_empty;
    wire [STREAM0_WIDTH - 1 : 0]        s_stream0_data;
    wire [STREAM1_WIDTH - 1 : 0]        s_stream1_data;
    wire [STREAM2_WIDTH - 1 : 0]        s_stream2_data;
    wire [STREAM3_WIDTH - 1 : 0]        s_stream3_data;
    wire                                complete_transfer_available =   ((!stream_empty[0] | !s_stream0_tenable) 
                                                                        & (!stream_empty[1] | !s_stream1_tenable)
                                                                        & (!stream_empty[2] | !s_stream2_tenable)
                                                                        & (!stream_empty[3] | !s_stream3_tenable));
    wire                                stream_out_read = m_stream_tready && complete_transfer_available;

    assign s_stream0_tready = !stream_full[0];
    assign s_stream1_tready = !stream_full[1];
    assign s_stream2_tready = !stream_full[2];
    assign s_stream3_tready = !stream_full[3];

    generate
        if (FIFO_DEPTH0_POW2 == 0)
        begin
            assign s_stream0_data = s_stream0_tdata;
            assign stream_empty[0] = !s_stream0_tvalid;
            assign stream_full[0] = !stream_out_read;
        end
        else
        begin
            sfifo #(
                .BW(STREAM0_WIDTH),
                .LGFLEN(FIFO_DEPTH0_POW2),
                .OPT_ASYNC_READ(0),
                .OPT_WRITE_ON_FULL(0),
                .OPT_READ_ON_EMPTY(0),
                .OPT_READ_BYPASS(FIFO_DEPTH0_POW2 < 2)
            ) stream0_fifo (
                .i_clk(aclk),
                .i_reset(!resetn),

                .i_wr(s_stream0_tvalid),
                .i_data(s_stream0_tdata),
                .o_full(stream_full[0]),
                .o_fill(),

                .i_rd(stream_out_read),
                .o_data(s_stream0_data),
                .o_empty(stream_empty[0])
            );
        end
    endgenerate

    generate
        if (FIFO_DEPTH1_POW2 == 0)
        begin
            assign s_stream1_data = s_stream1_tdata;
            assign stream_empty[1] = !s_stream1_tvalid;
            assign stream_full[1] = !stream_out_read;
        end
        else
        begin
            sfifo #(
                .BW(STREAM1_WIDTH),
                .LGFLEN(FIFO_DEPTH1_POW2),
                .OPT_ASYNC_READ(0),
                .OPT_WRITE_ON_FULL(0),
                .OPT_READ_ON_EMPTY(0),
                .OPT_READ_BYPASS(FIFO_DEPTH1_POW2 < 2)
            ) stream1_fifo (
                .i_clk(aclk),
                .i_reset(!resetn),

                .i_wr(s_stream1_tvalid),
                .i_data(s_stream1_tdata),
                .o_full(stream_full[1]),
                .o_fill(),

                .i_rd(stream_out_read),
                .o_data(s_stream1_data),
                .o_empty(stream_empty[1])
            );
        end
    endgenerate

    generate
        if (FIFO_DEPTH2_POW2 == 0)
        begin
            assign s_stream2_data = s_stream2_tdata;
            assign stream_empty[2] = !s_stream2_tvalid;
            assign stream_full[2] = !stream_out_read;
        end
        else
        begin
            sfifo #(
                .BW(STREAM2_WIDTH),
                .LGFLEN(FIFO_DEPTH2_POW2),
                .OPT_ASYNC_READ(0),
                .OPT_WRITE_ON_FULL(0),
                .OPT_READ_ON_EMPTY(0),
                .OPT_READ_BYPASS(FIFO_DEPTH2_POW2 < 2)
            ) stream2_fifo (
                .i_clk(aclk),
                .i_reset(!resetn),

                .i_wr(s_stream2_tvalid),
                .i_data(s_stream2_tdata),
                .o_full(stream_full[2]),
                .o_fill(),

                .i_rd(stream_out_read),
                .o_data(s_stream2_data),
                .o_empty(stream_empty[2])
            );
        end
    endgenerate

    generate
        if (FIFO_DEPTH3_POW2 == 0)
        begin
            assign s_stream3_data = s_stream3_tdata;
            assign stream_empty[3] = !s_stream3_tvalid;
            assign stream_full[3] = !stream_out_read;
        end
        else
        begin
            sfifo #(
                .BW(STREAM3_WIDTH),
                .LGFLEN(FIFO_DEPTH3_POW2),
                .OPT_ASYNC_READ(0),
                .OPT_WRITE_ON_FULL(0),
                .OPT_READ_ON_EMPTY(0),
                .OPT_READ_BYPASS(FIFO_DEPTH3_POW2 < 2)
            ) stream3_fifo (
                .i_clk(aclk),
                .i_reset(!resetn),

                .i_wr(s_stream3_tvalid),
                .i_data(s_stream3_tdata),
                .o_full(stream_full[3]),
                .o_fill(),

                .i_rd(stream_out_read),
                .o_data(s_stream3_data),
                .o_empty(stream_empty[3])
            );
        end
    endgenerate

    assign m_stream_tvalid = complete_transfer_available;
    assign m_stream_tdata = { s_stream3_data, s_stream2_data, s_stream1_data, s_stream0_data };
endmodule