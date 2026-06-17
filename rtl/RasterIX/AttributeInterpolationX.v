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

// This module increments or decrements the attributes based of the command
// from the rasterizer. It is intended that this runs in parallel with the
// attribute interpolation.
// Piplined: yes
// Depth: 1 cycle
module AttributeInterpolationX #(
    `include "RasterizerCommands.vh"
    parameter ENABLE_LOD_CALC = 1,
    parameter ENABLE_SECOND_TMU = 1,

    localparam ATTRIBUTE_SIZE = 32
)
(
    input  wire                                 aclk,
    input  wire                                 resetn,
    input  wire                                 ce,

    // Pixel Stream
    input  wire                                 valid,
    input  wire [RR_CMD_SIZE - 1 : 0]           cmd,

    // Attributes
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex0_s, // S3.28
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex0_t, // S3.28
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex0_q, // S3.28
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex0_s_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex0_t_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex0_q_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex0_s_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex0_t_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex0_q_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_s,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_t,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_q,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_s_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_t_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_q_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_s_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_t_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_q_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] depth_w, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] depth_w_inc_x, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] depth_w_inc_y, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] depth_z, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] depth_z_inc_x, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] depth_z_inc_y, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_r, // S7.24
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_g,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_b,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_a,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_r_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_g_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_b_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_a_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_r_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_g_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_b_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_a_inc_y,

    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex0_s, // S3.28
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex0_t, // S3.28
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex0_q, // S3.28
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex0_mipmap_s, // S3.28
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex0_mipmap_t, // S3.28
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex0_mipmap_q, // S3.28
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex1_s,
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex1_t,
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex1_q,
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex1_mipmap_s,
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex1_mipmap_t,
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex1_mipmap_q,
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_depth_w, // S1.30
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_depth_z, // S1.30
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_color_r, // S7.24
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_color_g,
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_color_b,
    output wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_color_a
);

    ////////////////////////////////////////////////////////////////////////////
    // Calculate the increment depending on the command of the rasterizer
    ///////////////////////////////////////////////////////////////////////////
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_s;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_t;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_q;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_s;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_t;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_q;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_depth_w;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_depth_z;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_r;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_g;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_b;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_a;

    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_s_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_t_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_q_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_s_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_t_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_q_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_depth_w_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_depth_z_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_r_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_g_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_b_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_a_queue;

    // X_INC, X_DEC and Y_INC are mutually exclusive, so a single add/sub unit
    // per attribute is sufficient. The operand mux selects +inc_x, -inc_x or
    // inc_y (the else branch is only reached for Y_INC) and feeds one adder.
    wire signed [ATTRIBUTE_SIZE - 1 : 0]    step_tex0_s  = ((cmd & RR_CMD_X_INC) != 0) ? tex0_s_inc_x  : ((cmd & RR_CMD_X_DEC) != 0) ? -tex0_s_inc_x  : tex0_s_inc_y;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]    step_tex0_t  = ((cmd & RR_CMD_X_INC) != 0) ? tex0_t_inc_x  : ((cmd & RR_CMD_X_DEC) != 0) ? -tex0_t_inc_x  : tex0_t_inc_y;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]    step_tex0_q  = ((cmd & RR_CMD_X_INC) != 0) ? tex0_q_inc_x  : ((cmd & RR_CMD_X_DEC) != 0) ? -tex0_q_inc_x  : tex0_q_inc_y;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]    step_tex1_s  = ((cmd & RR_CMD_X_INC) != 0) ? tex1_s_inc_x  : ((cmd & RR_CMD_X_DEC) != 0) ? -tex1_s_inc_x  : tex1_s_inc_y;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]    step_tex1_t  = ((cmd & RR_CMD_X_INC) != 0) ? tex1_t_inc_x  : ((cmd & RR_CMD_X_DEC) != 0) ? -tex1_t_inc_x  : tex1_t_inc_y;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]    step_tex1_q  = ((cmd & RR_CMD_X_INC) != 0) ? tex1_q_inc_x  : ((cmd & RR_CMD_X_DEC) != 0) ? -tex1_q_inc_x  : tex1_q_inc_y;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]    step_depth_w = ((cmd & RR_CMD_X_INC) != 0) ? depth_w_inc_x : ((cmd & RR_CMD_X_DEC) != 0) ? -depth_w_inc_x : depth_w_inc_y;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]    step_depth_z = ((cmd & RR_CMD_X_INC) != 0) ? depth_z_inc_x : ((cmd & RR_CMD_X_DEC) != 0) ? -depth_z_inc_x : depth_z_inc_y;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]    step_color_r = ((cmd & RR_CMD_X_INC) != 0) ? color_r_inc_x : ((cmd & RR_CMD_X_DEC) != 0) ? -color_r_inc_x : color_r_inc_y;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]    step_color_g = ((cmd & RR_CMD_X_INC) != 0) ? color_g_inc_x : ((cmd & RR_CMD_X_DEC) != 0) ? -color_g_inc_x : color_g_inc_y;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]    step_color_b = ((cmd & RR_CMD_X_INC) != 0) ? color_b_inc_x : ((cmd & RR_CMD_X_DEC) != 0) ? -color_b_inc_x : color_b_inc_y;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]    step_color_a = ((cmd & RR_CMD_X_INC) != 0) ? color_a_inc_x : ((cmd & RR_CMD_X_DEC) != 0) ? -color_a_inc_x : color_a_inc_y;

    always @(posedge aclk)
    if (ce) begin
        /* verilator lint_off WIDTHTRUNC */
        if (valid)
        begin
            if (cmd & RR_CMD_INIT)
            begin
                reg_tex0_s <= tex0_s;
                reg_tex0_t <= tex0_t;
                reg_tex0_q <= tex0_q;
                if (ENABLE_SECOND_TMU)
                begin
                    reg_tex1_s <= tex1_s;
                    reg_tex1_t <= tex1_t;
                    reg_tex1_q <= tex1_q;
                end
                else
                begin
                    reg_tex1_s <= 0;
                    reg_tex1_t <= 0;
                    reg_tex1_q <= 0;
                end
                reg_depth_w <= depth_w;
                reg_depth_z <= depth_z;
                reg_color_r <= color_r;
                reg_color_g <= color_g;
                reg_color_b <= color_b;
                reg_color_a <= color_a;
            end
            if (cmd & (RR_CMD_X_INC | RR_CMD_X_DEC | RR_CMD_Y_INC))
            begin
                reg_tex0_s <= reg_tex0_s + step_tex0_s;
                reg_tex0_t <= reg_tex0_t + step_tex0_t;
                reg_tex0_q <= reg_tex0_q + step_tex0_q;
                if (ENABLE_SECOND_TMU)
                begin
                    reg_tex1_s <= reg_tex1_s + step_tex1_s;
                    reg_tex1_t <= reg_tex1_t + step_tex1_t;
                    reg_tex1_q <= reg_tex1_q + step_tex1_q;
                end
                reg_depth_w <= reg_depth_w + step_depth_w;
                reg_depth_z <= reg_depth_z + step_depth_z;
                reg_color_r <= reg_color_r + step_color_r;
                reg_color_g <= reg_color_g + step_color_g;
                reg_color_b <= reg_color_b + step_color_b;
                reg_color_a <= reg_color_a + step_color_a;
            end
            if (cmd & RR_CMD_PUSH)
            begin
                reg_tex0_s_queue <= reg_tex0_s;
                reg_tex0_t_queue <= reg_tex0_t;
                reg_tex0_q_queue <= reg_tex0_q;
                if (ENABLE_SECOND_TMU)
                begin
                    reg_tex1_s_queue <= reg_tex1_s;
                    reg_tex1_t_queue <= reg_tex1_t;
                    reg_tex1_q_queue <= reg_tex1_q;
                end
                reg_depth_w_queue <= reg_depth_w;
                reg_depth_z_queue <= reg_depth_z;
                reg_color_r_queue <= reg_color_r;
                reg_color_g_queue <= reg_color_g;
                reg_color_b_queue <= reg_color_b;
                reg_color_a_queue <= reg_color_a;
            end
            if (cmd & RR_CMD_POP)
            begin
                reg_tex0_s <= reg_tex0_s_queue;
                reg_tex0_t <= reg_tex0_t_queue;
                reg_tex0_q <= reg_tex0_q_queue;
                if (ENABLE_SECOND_TMU)
                begin
                    reg_tex1_s <= reg_tex1_s_queue;
                    reg_tex1_t <= reg_tex1_t_queue;
                    reg_tex1_q <= reg_tex1_q_queue;
                end
                reg_depth_w <= reg_depth_w_queue;
                reg_depth_z <= reg_depth_z_queue;
                reg_color_r <= reg_color_r_queue;
                reg_color_g <= reg_color_g_queue;
                reg_color_b <= reg_color_b_queue;
                reg_color_a <= reg_color_a_queue;
            end
        end
        /* verilator lint_on WIDTHTRUNC */
    end

    // The mipmap attributes are always a constant offset (inc_x + inc_y) from
    // the base attributes, so they are derived combinationally instead of being
    // maintained in dedicated accumulators and queues.
    assign curr_tex0_s = reg_tex0_s;
    assign curr_tex0_t = reg_tex0_t;
    assign curr_tex0_q = reg_tex0_q;
    assign curr_tex0_mipmap_s = ENABLE_LOD_CALC ? (reg_tex0_s + tex0_s_inc_x + tex0_s_inc_y) : {ATTRIBUTE_SIZE{1'b0}};
    assign curr_tex0_mipmap_t = ENABLE_LOD_CALC ? (reg_tex0_t + tex0_t_inc_x + tex0_t_inc_y) : {ATTRIBUTE_SIZE{1'b0}};
    assign curr_tex0_mipmap_q = ENABLE_LOD_CALC ? (reg_tex0_q + tex0_q_inc_x + tex0_q_inc_y) : {ATTRIBUTE_SIZE{1'b0}};
    assign curr_tex1_s = reg_tex1_s;
    assign curr_tex1_t = reg_tex1_t;
    assign curr_tex1_q = reg_tex1_q;
    assign curr_tex1_mipmap_s = (ENABLE_LOD_CALC && ENABLE_SECOND_TMU) ? (reg_tex1_s + tex1_s_inc_x + tex1_s_inc_y) : {ATTRIBUTE_SIZE{1'b0}};
    assign curr_tex1_mipmap_t = (ENABLE_LOD_CALC && ENABLE_SECOND_TMU) ? (reg_tex1_t + tex1_t_inc_x + tex1_t_inc_y) : {ATTRIBUTE_SIZE{1'b0}};
    assign curr_tex1_mipmap_q = (ENABLE_LOD_CALC && ENABLE_SECOND_TMU) ? (reg_tex1_q + tex1_q_inc_x + tex1_q_inc_y) : {ATTRIBUTE_SIZE{1'b0}};
    assign curr_depth_w = reg_depth_w;
    assign curr_depth_z = reg_depth_z;
    assign curr_color_r = reg_color_r;
    assign curr_color_g = reg_color_g;
    assign curr_color_b = reg_color_b;
    assign curr_color_a = reg_color_a;
endmodule
