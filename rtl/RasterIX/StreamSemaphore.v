// Float
// https://github.com/ToNi3141/Float
// Copyright (c) 2021 ToNi3141

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

// This module observes the incoming and outgoing values of a pipeline
// and controls the flow into the pipeline.
module StreamSemaphore #(
    parameter MAX_NUMBER_OF_ELEMENTS = 128,
    parameter STREAM_WIDTH = 32,
    parameter KEEP_WIDTH = 1
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    output reg                          m_axis_tvalid,
    output wire                         m_axis_tlast,
    output wire [STREAM_WIDTH - 1 : 0]  m_axis_tdata,
    output wire [KEEP_WIDTH - 1 : 0]    m_axis_tkeep,
    input  wire                         m_axis_tready,

    input  wire                         s_axis_tvalid,
    output wire                         s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]  s_axis_tdata,
    input  wire [KEEP_WIDTH - 1 : 0]    s_axis_tkeep,

    input  wire                         sigLock,
    input  wire                         sigRelease,
    output reg                          released
);
    reg  [$clog2(MAX_NUMBER_OF_ELEMENTS) + 1 : 0]   valuesCounter = 0;
    wire                                            skidValid;

    wire sigOutgoingValue   = sigRelease;
    wire sigIncomingValue   = s_axis_tvalid && s_axis_tready && sigLock;
    wire free               = (valuesCounter < MAX_NUMBER_OF_ELEMENTS);

    skidbuffer #(
        .OPT_OUTREG(1),
        .OPT_PASSTHROUGH(0),
        .OPT_INITIAL(1),
        .DW(1 + KEEP_WIDTH + STREAM_WIDTH)
    ) skb (
        .i_clk(aclk),
        .i_reset(!resetn),

        .i_valid(s_axis_tvalid),
        .o_ready(s_axis_tready),
        .i_data({ s_axis_tlast, s_axis_tkeep, s_axis_tdata }),

        .o_valid(skidValid),
        .i_ready(free && m_axis_tready),
        .o_data({ m_axis_tlast, m_axis_tkeep, m_axis_tdata })
    );

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            valuesCounter = 0;
        end
        else
        begin
            // Value goes out and no values comes in. 
            // The pipeline loses a value: decrement
            if ((sigOutgoingValue == 1) && (sigIncomingValue == 0) && (valuesCounter != 0))
            begin
                valuesCounter = valuesCounter - 1;
            end
            // No values goes out but one comes in
            // The pipeline receives a new value: increment
            if ((sigOutgoingValue == 0) && (sigIncomingValue == 1))
            begin
                valuesCounter = valuesCounter + 1;
            end
            released <= valuesCounter == 0;   
        end
    end

    always @(*)
    begin
        m_axis_tvalid = skidValid && free;
    end
endmodule
