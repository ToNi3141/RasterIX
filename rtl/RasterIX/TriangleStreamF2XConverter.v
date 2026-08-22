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

// Parses the triangle stream, converts the parameters from floating point
// into fix point representation for the fixed point attribute interpolator
// Pipelined: yes
// Depth: 3 cycles
module TriangleStreamF2XConverter #(
    localparam CMD_STREAM_WIDTH = 32,
    localparam FLOAT_SIZE = CMD_STREAM_WIDTH
)
(
    input  wire                             aclk,
    input  wire                             resetn,
    
    input  wire                             s_axis_tvalid,
    input  wire                             s_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  s_axis_tdata,

    output reg                              m_axis_tvalid,
    output reg                              m_axis_tlast,
    output reg  [CMD_STREAM_WIDTH - 1 : 0]  m_axis_tdata
);
`include "RegisterAndDescriptorDefines.vh"
    localparam [ 0 : 0] BYPASS = 0;
    localparam [ 0 : 0] CONVERT = 1;

    localparam MUX_POS = 8;
    localparam MUX_SIZE = 1;
    localparam OFFSET_POS = 0;
    localparam OFFSET_SIZE = 8;

    localparam signed [ 7 : 0] TEXTURE_SHIFT = -8'd28;
    localparam signed [ 7 : 0] COLOR_SHIFT = -8'd24;
    localparam signed [ 7 : 0] DEPTH_SHIFT = -8'd30;
    localparam signed [ 7 : 0] NULL_SHIFT = 0;

    reg  [ 5 : 0] index;
    reg  [ 8 : 0] convertLut [0 : TRIANGLE_STREAM_SIZE - 1];

    wire                            delay_tvalid;
    wire                            delay_tlast;
    wire [CMD_STREAM_WIDTH - 1 : 0] delay_tdata;
    wire                            delay_mux;
    wire [CMD_STREAM_WIDTH - 1 : 0] convert_tdata;

    initial
    begin
        convertLut[TRIANGLE_STREAM_RESERVED]      = { BYPASS,  NULL_SHIFT    };
        convertLut[TRIANGLE_STREAM_BB_START]      = { BYPASS,  NULL_SHIFT    };
        convertLut[TRIANGLE_STREAM_BB_END]        = { BYPASS,  NULL_SHIFT    };
        convertLut[TRIANGLE_STREAM_INC_W0]        = { BYPASS,  NULL_SHIFT    };
        convertLut[TRIANGLE_STREAM_INC_W1]        = { BYPASS,  NULL_SHIFT    };
        convertLut[TRIANGLE_STREAM_INC_W2]        = { BYPASS,  NULL_SHIFT    };
        convertLut[TRIANGLE_STREAM_INC_W0_X]      = { BYPASS,  NULL_SHIFT    };
        convertLut[TRIANGLE_STREAM_INC_W1_X]      = { BYPASS,  NULL_SHIFT    };
        convertLut[TRIANGLE_STREAM_INC_W2_X]      = { BYPASS,  NULL_SHIFT    };
        convertLut[TRIANGLE_STREAM_INC_W0_Y]      = { BYPASS,  NULL_SHIFT    };
        convertLut[TRIANGLE_STREAM_INC_W1_Y]      = { BYPASS,  NULL_SHIFT    };
        convertLut[TRIANGLE_STREAM_INC_W2_Y]      = { BYPASS,  NULL_SHIFT    };
        convertLut[TRIANGLE_STREAM_INC_COLOR_R]   = { CONVERT, COLOR_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_COLOR_G]   = { CONVERT, COLOR_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_COLOR_B]   = { CONVERT, COLOR_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_COLOR_A]   = { CONVERT, COLOR_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_COLOR_R_X] = { CONVERT, COLOR_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_COLOR_G_X] = { CONVERT, COLOR_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_COLOR_B_X] = { CONVERT, COLOR_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_COLOR_A_X] = { CONVERT, COLOR_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_COLOR_R_Y] = { CONVERT, COLOR_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_COLOR_G_Y] = { CONVERT, COLOR_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_COLOR_B_Y] = { CONVERT, COLOR_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_COLOR_A_Y] = { CONVERT, COLOR_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_DEPTH_W]   = { CONVERT, DEPTH_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_DEPTH_W_X] = { CONVERT, DEPTH_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_DEPTH_W_Y] = { CONVERT, DEPTH_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_DEPTH_Z]   = { CONVERT, DEPTH_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_DEPTH_Z_X] = { CONVERT, DEPTH_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_DEPTH_Z_Y] = { CONVERT, DEPTH_SHIFT   };
        convertLut[TRIANGLE_STREAM_INC_TEX0_S]    = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX0_T]    = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX0_Q]    = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX0_S_X]  = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX0_T_X]  = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX0_Q_X]  = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX0_S_Y]  = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX0_T_Y]  = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX0_Q_Y]  = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX1_S]    = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX1_T]    = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX1_Q]    = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX1_S_X]  = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX1_T_X]  = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX1_Q_X]  = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX1_S_Y]  = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX1_T_Y]  = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_INC_TEX1_Q_Y]  = { CONVERT, TEXTURE_SHIFT };
        convertLut[TRIANGLE_STREAM_PADDING_1]     = { BYPASS,  NULL_SHIFT    };
    end

    ValueDelay #(
        .VALUE_SIZE(CMD_STREAM_WIDTH + 2 + MUX_SIZE), 
        .DELAY(2)
    ) delayStream (
        .clk(aclk), 
        .ce(1),
        .in({ convertLut[index][MUX_POS +: MUX_SIZE], s_axis_tvalid, s_axis_tlast, s_axis_tdata }), 
        .out({ delay_mux, delay_tvalid, delay_tlast, delay_tdata })
    );

    FloatToInt #(
        .MANTISSA_SIZE(FLOAT_SIZE - 9),
        .EXPONENT_SIZE(8), 
        .INT_SIZE(CMD_STREAM_WIDTH),
        .DELAY(0),
        // Values here are used for accumulation of the incremental values.
        // This is some dark fix point magic, and can result in the correct
        // result even if the converted value is out of range.
        // Instead of 0, this returns the LSb's of the converted value, 
        // which is enough for the accumulator to get the correct result.
        // Without this, narrow triangles can result in a wrong color, 
        // depth or texture value.
        .OVERFLOW_WRAP(1)
    ) convertStream (
        .clk(aclk),
        .ce(1),
        .offset(convertLut[index][OFFSET_POS +: OFFSET_SIZE]),
        .in(s_axis_tdata), 
        .out(convert_tdata)
    );

    // Input stream
    always @(posedge aclk)
    begin
        if (!resetn || (s_axis_tlast && s_axis_tvalid))
        begin
            index <= 0;
        end
        else
        begin
            if (s_axis_tvalid)
            begin
                index <= index + 1;
            end
        end
    end

    // Output stream
    always @(posedge aclk)
    begin
        m_axis_tvalid <= delay_tvalid;
        m_axis_tlast <= delay_tlast;
        if (delay_mux == CONVERT)
        begin
            m_axis_tdata <= convert_tdata;
        end
        else
        begin
            m_axis_tdata <= delay_tdata;
        end
    end

endmodule
