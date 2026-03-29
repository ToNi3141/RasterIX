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
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_mipmap_s;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_mipmap_t;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_mipmap_q;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_s;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_t;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_q;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_mipmap_s;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_mipmap_t;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_mipmap_q;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_depth_w;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_depth_z;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_r;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_g;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_b;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_a;

    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_s_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_t_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_q_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_mipmap_s_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_mipmap_t_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_mipmap_q_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_s_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_t_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_q_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_mipmap_s_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_mipmap_t_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_mipmap_q_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_depth_w_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_depth_z_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_r_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_g_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_b_queue;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_a_queue;

    always @(posedge aclk)
    if (ce) begin
        if (valid)
        begin
            if (cmd & RR_CMD_INIT)
            begin
                reg_tex0_s <= tex0_s;
                reg_tex0_t <= tex0_t;
                reg_tex0_q <= tex0_q;
                if (ENABLE_LOD_CALC)
                begin
                    reg_tex0_mipmap_s <= tex0_s + tex0_s_inc_x + tex0_s_inc_y;
                    reg_tex0_mipmap_t <= tex0_t + tex0_t_inc_x + tex0_t_inc_y;
                    reg_tex0_mipmap_q <= tex0_q + tex0_q_inc_x + tex0_q_inc_y;
                end
                if (ENABLE_SECOND_TMU)
                begin
                    reg_tex1_s <= tex1_s;
                    reg_tex1_t <= tex1_t;
                    reg_tex1_q <= tex1_q;
                    if (ENABLE_LOD_CALC)
                    begin
                        reg_tex1_mipmap_s <= tex1_s + tex1_s_inc_x + tex1_s_inc_y;
                        reg_tex1_mipmap_t <= tex1_t + tex1_t_inc_x + tex1_t_inc_y;
                        reg_tex1_mipmap_q <= tex1_q + tex1_q_inc_x + tex1_q_inc_y;
                    end
                end
                else
                begin
                    reg_tex1_s <= 0;
                    reg_tex1_t <= 0;
                    reg_tex1_q <= 0;
                    reg_tex1_mipmap_s <= 0;
                    reg_tex1_mipmap_t <= 0;
                    reg_tex1_mipmap_q <= 0;
                end
                reg_depth_w <= depth_w;
                reg_depth_z <= depth_z;
                reg_color_r <= color_r;
                reg_color_g <= color_g;
                reg_color_b <= color_b;
                reg_color_a <= color_a;
            end
            if (cmd & RR_CMD_X_INC)
            begin
                reg_tex0_s <= reg_tex0_s + tex0_s_inc_x;
                reg_tex0_t <= reg_tex0_t + tex0_t_inc_x;
                reg_tex0_q <= reg_tex0_q + tex0_q_inc_x;
                if (ENABLE_LOD_CALC)
                begin
                    reg_tex0_mipmap_s <= reg_tex0_mipmap_s + tex0_s_inc_x;
                    reg_tex0_mipmap_t <= reg_tex0_mipmap_t + tex0_t_inc_x;
                    reg_tex0_mipmap_q <= reg_tex0_mipmap_q + tex0_q_inc_x;
                end
                if (ENABLE_SECOND_TMU)
                begin
                    reg_tex1_s <= reg_tex1_s + tex1_s_inc_x;
                    reg_tex1_t <= reg_tex1_t + tex1_t_inc_x;
                    reg_tex1_q <= reg_tex1_q + tex1_q_inc_x;
                    if (ENABLE_LOD_CALC)
                    begin
                        reg_tex1_mipmap_s <= reg_tex1_mipmap_s + tex1_s_inc_x;
                        reg_tex1_mipmap_t <= reg_tex1_mipmap_t + tex1_t_inc_x;
                        reg_tex1_mipmap_q <= reg_tex1_mipmap_q + tex1_q_inc_x;
                    end
                end
                reg_depth_w <= reg_depth_w + depth_w_inc_x;
                reg_depth_z <= reg_depth_z + depth_z_inc_x;
                reg_color_r <= reg_color_r + color_r_inc_x;
                reg_color_g <= reg_color_g + color_g_inc_x;
                reg_color_b <= reg_color_b + color_b_inc_x;
                reg_color_a <= reg_color_a + color_a_inc_x;
            end
            if (cmd & RR_CMD_X_DEC)
            begin
                reg_tex0_s <= reg_tex0_s - tex0_s_inc_x;
                reg_tex0_t <= reg_tex0_t - tex0_t_inc_x;
                reg_tex0_q <= reg_tex0_q - tex0_q_inc_x;
                if (ENABLE_LOD_CALC)
                begin
                    reg_tex0_mipmap_s <= reg_tex0_mipmap_s - tex0_s_inc_x;
                    reg_tex0_mipmap_t <= reg_tex0_mipmap_t - tex0_t_inc_x;
                    reg_tex0_mipmap_q <= reg_tex0_mipmap_q - tex0_q_inc_x;
                end
                if (ENABLE_SECOND_TMU)
                begin
                    reg_tex1_s <= reg_tex1_s - tex1_s_inc_x;
                    reg_tex1_t <= reg_tex1_t - tex1_t_inc_x;
                    reg_tex1_q <= reg_tex1_q - tex1_q_inc_x;
                    if (ENABLE_LOD_CALC)
                    begin
                        reg_tex1_mipmap_s <= reg_tex1_mipmap_s - tex1_s_inc_x;
                        reg_tex1_mipmap_t <= reg_tex1_mipmap_t - tex1_t_inc_x;
                        reg_tex1_mipmap_q <= reg_tex1_mipmap_q - tex1_q_inc_x;
                    end
                end
                reg_depth_w <= reg_depth_w - depth_w_inc_x;
                reg_depth_z <= reg_depth_z - depth_z_inc_x;
                reg_color_r <= reg_color_r - color_r_inc_x;
                reg_color_g <= reg_color_g - color_g_inc_x;
                reg_color_b <= reg_color_b - color_b_inc_x;
                reg_color_a <= reg_color_a - color_a_inc_x;
            end
            if (cmd & RR_CMD_Y_INC)
            begin
                reg_tex0_s <= reg_tex0_s + tex0_s_inc_y;
                reg_tex0_t <= reg_tex0_t + tex0_t_inc_y;
                reg_tex0_q <= reg_tex0_q + tex0_q_inc_y;
                if (ENABLE_LOD_CALC)
                begin
                    reg_tex0_mipmap_s <= reg_tex0_mipmap_s + tex0_s_inc_y;
                    reg_tex0_mipmap_t <= reg_tex0_mipmap_t + tex0_t_inc_y;
                    reg_tex0_mipmap_q <= reg_tex0_mipmap_q + tex0_q_inc_y;
                end
                if (ENABLE_SECOND_TMU)
                begin
                    reg_tex1_s <= reg_tex1_s + tex1_s_inc_y;
                    reg_tex1_t <= reg_tex1_t + tex1_t_inc_y;
                    reg_tex1_q <= reg_tex1_q + tex1_q_inc_y;
                    if (ENABLE_LOD_CALC)
                    begin
                        reg_tex1_mipmap_s <= reg_tex1_mipmap_s + tex1_s_inc_y;
                        reg_tex1_mipmap_t <= reg_tex1_mipmap_t + tex1_t_inc_y;
                        reg_tex1_mipmap_q <= reg_tex1_mipmap_q + tex1_q_inc_y;
                    end
                end
                reg_depth_w <= reg_depth_w + depth_w_inc_y;
                reg_depth_z <= reg_depth_z + depth_z_inc_y;
                reg_color_r <= reg_color_r + color_r_inc_y;
                reg_color_g <= reg_color_g + color_g_inc_y;
                reg_color_b <= reg_color_b + color_b_inc_y;
                reg_color_a <= reg_color_a + color_a_inc_y;
            end 
            if (cmd & RR_CMD_PUSH)
            begin
                reg_tex0_s_queue <= reg_tex0_s;
                reg_tex0_t_queue <= reg_tex0_t;
                reg_tex0_q_queue <= reg_tex0_q;
                if (ENABLE_LOD_CALC)
                begin
                    reg_tex0_mipmap_s_queue <= reg_tex0_mipmap_s;
                    reg_tex0_mipmap_t_queue <= reg_tex0_mipmap_t;
                    reg_tex0_mipmap_q_queue <= reg_tex0_mipmap_q;
                end
                if (ENABLE_SECOND_TMU)
                begin
                    reg_tex1_s_queue <= reg_tex1_s;
                    reg_tex1_t_queue <= reg_tex1_t;
                    reg_tex1_q_queue <= reg_tex1_q;
                    if (ENABLE_LOD_CALC)
                    begin
                        reg_tex1_mipmap_s_queue <= reg_tex1_mipmap_s;
                        reg_tex1_mipmap_t_queue <= reg_tex1_mipmap_t;
                        reg_tex1_mipmap_q_queue <= reg_tex1_mipmap_q;
                    end
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
                if (ENABLE_LOD_CALC)
                begin
                    reg_tex0_mipmap_s <= reg_tex0_mipmap_s_queue;
                    reg_tex0_mipmap_t <= reg_tex0_mipmap_t_queue;
                    reg_tex0_mipmap_q <= reg_tex0_mipmap_q_queue;
                end
                if (ENABLE_SECOND_TMU)
                begin
                    reg_tex1_s <= reg_tex1_s_queue;
                    reg_tex1_t <= reg_tex1_t_queue;
                    reg_tex1_q <= reg_tex1_q_queue;
                    if (ENABLE_LOD_CALC)
                    begin
                        reg_tex1_mipmap_s <= reg_tex1_mipmap_s_queue;
                        reg_tex1_mipmap_t <= reg_tex1_mipmap_t_queue;
                        reg_tex1_mipmap_q <= reg_tex1_mipmap_q_queue;
                    end
                end
                reg_depth_w <= reg_depth_w_queue;
                reg_depth_z <= reg_depth_z_queue;
                reg_color_r <= reg_color_r_queue;
                reg_color_g <= reg_color_g_queue;
                reg_color_b <= reg_color_b_queue;
                reg_color_a <= reg_color_a_queue;
            end
        end
    end

    assign curr_tex0_s = reg_tex0_s;
    assign curr_tex0_t = reg_tex0_t;
    assign curr_tex0_q = reg_tex0_q;
    assign curr_tex0_mipmap_s = reg_tex0_mipmap_s;
    assign curr_tex0_mipmap_t = reg_tex0_mipmap_t;
    assign curr_tex0_mipmap_q = reg_tex0_mipmap_q;
    assign curr_tex1_s = reg_tex1_s;
    assign curr_tex1_t = reg_tex1_t;
    assign curr_tex1_q = reg_tex1_q;
    assign curr_tex1_mipmap_s = reg_tex1_mipmap_s;
    assign curr_tex1_mipmap_t = reg_tex1_mipmap_t;
    assign curr_tex1_mipmap_q = reg_tex1_mipmap_q;
    assign curr_depth_w = reg_depth_w;
    assign curr_depth_z = reg_depth_z;
    assign curr_color_r = reg_color_r;
    assign curr_color_g = reg_color_g;
    assign curr_color_b = reg_color_b;
    assign curr_color_a = reg_color_a;
endmodule
