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

// This module is used to interpolate the triangle attributes based 
// on the data from the rasterizer. It expects numbers in fix point format.
// Pipelined: yes
// Depth: 16 cycles
module AttributeInterpolatorX #(
    `include "RasterizerCommands.vh"
    parameter INDEX_WIDTH = 32,
    parameter SCREEN_POS_WIDTH = 11,
    parameter ENABLE_LOD_CALC = 1,
    parameter ENABLE_SECOND_TMU = 1,
    parameter SUB_PIXEL_WIDTH = 8,
    parameter CALC_PRECISION = 25, // The pricision of a signed multiplication

    localparam DEPTH_WIDTH = 16,
    localparam ATTRIBUTE_SIZE = 32,
    localparam KEEP_WIDTH = 1,
    localparam FLOAT_SIZE = 32
)
(
    input  wire                                 aclk,
    input  wire                                 resetn,

    // Pixel Stream
    output wire                                 s_attrb_tready,
    input  wire                                 s_attrb_tvalid,
    input  wire                                 s_attrb_tlast,
    input  wire [KEEP_WIDTH - 1 : 0]            s_attrb_tkeep,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]      s_attrb_tspx,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]      s_attrb_tspy,
    input  wire [INDEX_WIDTH - 1 : 0]           s_attrb_tindex,
    input  wire                                 s_attrb_tpixel,
    input  wire [RR_CMD_SIZE - 1 : 0]           s_attrb_tcmd,


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

    // Pixel Stream Interpolated
    input  wire                                 m_attrb_tready,
    output wire                                 m_attrb_tvalid,
    output wire                                 m_attrb_tpixel,
    output wire                                 m_attrb_tlast,
    output wire [KEEP_WIDTH - 1 : 0]            m_attrb_tkeep,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      m_attrb_tspx,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      m_attrb_tspy,
    output wire [INDEX_WIDTH - 1 : 0]           m_attrb_tindex,
    output wire [FLOAT_SIZE - 1 : 0]            m_attrb_tdepth_w, // Float
    output wire [ATTRIBUTE_SIZE -1 : 0]         m_attrb_tdepth_z, // Q16.16
    output wire [ATTRIBUTE_SIZE -1 : 0]         m_attrb_ttexture0_t, // S16.15
    output wire [ATTRIBUTE_SIZE -1 : 0]         m_attrb_ttexture0_s, // S16.15
    output wire [ATTRIBUTE_SIZE -1 : 0]         m_attrb_tmipmap0_t, // S16.15
    output wire [ATTRIBUTE_SIZE -1 : 0]         m_attrb_tmipmap0_s, // S16.15
    output wire [ATTRIBUTE_SIZE -1 : 0]         m_attrb_ttexture1_t, // S16.15
    output wire [ATTRIBUTE_SIZE -1 : 0]         m_attrb_ttexture1_s, // S16.15
    output wire [ATTRIBUTE_SIZE -1 : 0]         m_attrb_tmipmap1_t, // S16.15
    output wire [ATTRIBUTE_SIZE -1 : 0]         m_attrb_tmipmap1_s, // S16.15
    output wire [SUB_PIXEL_WIDTH - 1 : 0]       m_attrb_tcolor_a, // Qn.0
    output wire [SUB_PIXEL_WIDTH - 1 : 0]       m_attrb_tcolor_b, // Qn.0
    output wire [SUB_PIXEL_WIDTH - 1 : 0]       m_attrb_tcolor_g, // Qn.0
    output wire [SUB_PIXEL_WIDTH - 1 : 0]       m_attrb_tcolor_r // Qn.0
);
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex0_s;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex0_t;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex0_q;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex0_mipmap_s;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex0_mipmap_t;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex0_mipmap_q;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex1_s;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex1_t;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex1_q;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex1_mipmap_s;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex1_mipmap_t;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_tex1_mipmap_q;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_depth_w;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_depth_z;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_color_r;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_color_g;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_color_b;
    wire signed [ATTRIBUTE_SIZE - 1 : 0] curr_color_a;

    wire ce;
    assign ce = m_attrb_tready;
    assign s_attrb_tready = m_attrb_tready;

    AttributeInterpolationX #(
        .ENABLE_LOD_CALC(ENABLE_LOD_CALC),
        .ENABLE_SECOND_TMU(ENABLE_SECOND_TMU)
    ) cmd (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),
        
        .valid(s_attrb_tvalid),
        .cmd(s_attrb_tcmd),

        .tex0_s(tex0_s),
        .tex0_t(tex0_t),
        .tex0_q(tex0_q),
        .tex0_s_inc_x(tex0_s_inc_x),
        .tex0_t_inc_x(tex0_t_inc_x),
        .tex0_q_inc_x(tex0_q_inc_x),
        .tex0_s_inc_y(tex0_s_inc_y),
        .tex0_t_inc_y(tex0_t_inc_y),
        .tex0_q_inc_y(tex0_q_inc_y),
        .tex1_s(tex1_s),
        .tex1_t(tex1_t),
        .tex1_q(tex1_q),
        .tex1_s_inc_x(tex1_s_inc_x),
        .tex1_t_inc_x(tex1_t_inc_x),
        .tex1_q_inc_x(tex1_q_inc_x),
        .tex1_s_inc_y(tex1_s_inc_y),
        .tex1_t_inc_y(tex1_t_inc_y),
        .tex1_q_inc_y(tex1_q_inc_y),
        .depth_w(depth_w),
        .depth_w_inc_x(depth_w_inc_x),
        .depth_w_inc_y(depth_w_inc_y),
        .depth_z(depth_z),
        .depth_z_inc_x(depth_z_inc_x),
        .depth_z_inc_y(depth_z_inc_y),
        .color_r(color_r),
        .color_g(color_g),
        .color_b(color_b),
        .color_a(color_a),
        .color_r_inc_x(color_r_inc_x),
        .color_g_inc_x(color_g_inc_x),
        .color_b_inc_x(color_b_inc_x),
        .color_a_inc_x(color_a_inc_x),
        .color_r_inc_y(color_r_inc_y),
        .color_g_inc_y(color_g_inc_y),
        .color_b_inc_y(color_b_inc_y),
        .color_a_inc_y(color_a_inc_y),

        .curr_tex0_s(curr_tex0_s),
        .curr_tex0_t(curr_tex0_t),
        .curr_tex0_q(curr_tex0_q),
        .curr_tex0_mipmap_s(curr_tex0_mipmap_s),
        .curr_tex0_mipmap_t(curr_tex0_mipmap_t),
        .curr_tex0_mipmap_q(curr_tex0_mipmap_q),
        .curr_tex1_s(curr_tex1_s),
        .curr_tex1_t(curr_tex1_t),
        .curr_tex1_q(curr_tex1_q),
        .curr_tex1_mipmap_s(curr_tex1_mipmap_s),
        .curr_tex1_mipmap_t(curr_tex1_mipmap_t),
        .curr_tex1_mipmap_q(curr_tex1_mipmap_q),
        .curr_depth_w(curr_depth_w),
        .curr_depth_z(curr_depth_z),
        .curr_color_r(curr_color_r),
        .curr_color_g(curr_color_g),
        .curr_color_b(curr_color_b),
        .curr_color_a(curr_color_a)
    );

    AttributePerspectiveCorrectionX #(
        .INDEX_WIDTH(INDEX_WIDTH),
        .SCREEN_POS_WIDTH(SCREEN_POS_WIDTH),
        .ENABLE_LOD_CALC(ENABLE_LOD_CALC),
        .ENABLE_SECOND_TMU(ENABLE_SECOND_TMU),
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
        .CALC_PRECISION(CALC_PRECISION)
    ) interpolation (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),

        .s_attrb_tvalid(s_attrb_tvalid),
        .s_attrb_tlast(s_attrb_tlast),
        .s_attrb_tkeep(s_attrb_tkeep),
        .s_attrb_tspx(s_attrb_tspx),
        .s_attrb_tspy(s_attrb_tspy),
        .s_attrb_tindex(s_attrb_tindex),
        .s_attrb_tpixel(s_attrb_tpixel),

        .tex0_s(curr_tex0_s),
        .tex0_t(curr_tex0_t),
        .tex0_q(curr_tex0_q),
        .tex0_mipmap_s(curr_tex0_mipmap_s),
        .tex0_mipmap_t(curr_tex0_mipmap_t),
        .tex0_mipmap_q(curr_tex0_mipmap_q),
        .tex1_s(curr_tex1_s),
        .tex1_t(curr_tex1_t),
        .tex1_q(curr_tex1_q),
        .tex1_mipmap_s(curr_tex1_mipmap_s),
        .tex1_mipmap_t(curr_tex1_mipmap_t),
        .tex1_mipmap_q(curr_tex1_mipmap_q),
        .depth_w(curr_depth_w),
        .depth_z(curr_depth_z),
        .color_r(curr_color_r),
        .color_g(curr_color_g),
        .color_b(curr_color_b),
        .color_a(curr_color_a),

        .m_attrb_tvalid(m_attrb_tvalid),
        .m_attrb_tpixel(m_attrb_tpixel),
        .m_attrb_tlast(m_attrb_tlast),
        .m_attrb_tkeep(m_attrb_tkeep),
        .m_attrb_tspx(m_attrb_tspx),
        .m_attrb_tspy(m_attrb_tspy),
        .m_attrb_tindex(m_attrb_tindex),
        .m_attrb_tdepth_w(m_attrb_tdepth_w),
        .m_attrb_tdepth_z(m_attrb_tdepth_z),
        .m_attrb_ttexture0_t(m_attrb_ttexture0_t),
        .m_attrb_ttexture0_s(m_attrb_ttexture0_s),
        .m_attrb_tmipmap0_t(m_attrb_tmipmap0_t),
        .m_attrb_tmipmap0_s(m_attrb_tmipmap0_s),
        .m_attrb_ttexture1_t(m_attrb_ttexture1_t),
        .m_attrb_ttexture1_s(m_attrb_ttexture1_s),
        .m_attrb_tmipmap1_t(m_attrb_tmipmap1_t),
        .m_attrb_tmipmap1_s(m_attrb_tmipmap1_s),
        .m_attrb_tcolor_a(m_attrb_tcolor_a),
        .m_attrb_tcolor_b(m_attrb_tcolor_b),
        .m_attrb_tcolor_g(m_attrb_tcolor_g),
        .m_attrb_tcolor_r(m_attrb_tcolor_r)
    );

endmodule
