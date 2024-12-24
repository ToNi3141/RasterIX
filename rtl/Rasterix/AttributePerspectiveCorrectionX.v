// Rasterix
// https://github.com/ToNi3141/Rasterix
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

// This module is used to calculate the perspective correction of the
// attributes given from the interpolator.
// Pipelined: yes
// Depth: 17 cycles
module AttributePerspectiveCorrectionX #(
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
    input  wire                                 ce,

    // Pixel Stream
    // TODO: Rename the signals to avoid confusion with an AXIS
    input  wire                                 s_attrb_tvalid,
    input  wire                                 s_attrb_tlast,
    input  wire [KEEP_WIDTH - 1 : 0]            s_attrb_tkeep,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]      s_attrb_tspx,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]      s_attrb_tspy,
    input  wire [INDEX_WIDTH - 1 : 0]           s_attrb_tindex,
    input  wire                                 s_attrb_tpixel,

    // Attributes
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex0_s, // S3.28
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex0_t, // S3.28
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex0_q, // S3.28
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex0_mipmap_s, // S3.28
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex0_mipmap_t, // S3.28
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex0_mipmap_q, // S3.28
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_s,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_t,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_q,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_mipmap_s,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_mipmap_t,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_mipmap_q,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] depth_w, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] depth_z, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_r, // S7.24
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_g,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_b,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_a,

    // Pixel Stream Interpolated
    output wire                                 m_attrb_tvalid,
    output wire                                 m_attrb_tlast,
    output wire [KEEP_WIDTH - 1 : 0]            m_attrb_tkeep,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      m_attrb_tspx,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      m_attrb_tspy,
    output wire [INDEX_WIDTH - 1 : 0]           m_attrb_tindex,
    output wire [FLOAT_SIZE - 1 : 0]            m_attrb_tdepth_w, // Float
    output wire [ATTRIBUTE_SIZE - 1 : 0]        m_attrb_tdepth_z, // Q16.16
    output wire [ATTRIBUTE_SIZE - 1 : 0]        m_attrb_ttexture0_t, // S16.15
    output wire [ATTRIBUTE_SIZE - 1 : 0]        m_attrb_ttexture0_s, // S16.15
    output wire [ATTRIBUTE_SIZE - 1 : 0]        m_attrb_tmipmap0_t, // S16.15
    output wire [ATTRIBUTE_SIZE - 1 : 0]        m_attrb_tmipmap0_s, // S16.15
    output wire [ATTRIBUTE_SIZE - 1 : 0]        m_attrb_ttexture1_t, // S16.15
    output wire [ATTRIBUTE_SIZE - 1 : 0]        m_attrb_ttexture1_s, // S16.15
    output wire [ATTRIBUTE_SIZE - 1 : 0]        m_attrb_tmipmap1_t, // S16.15
    output wire [ATTRIBUTE_SIZE - 1 : 0]        m_attrb_tmipmap1_s, // S16.15
    output wire [SUB_PIXEL_WIDTH - 1 : 0]       m_attrb_tcolor_a, // Qn.0
    output wire [SUB_PIXEL_WIDTH - 1 : 0]       m_attrb_tcolor_b, // Qn.0
    output wire [SUB_PIXEL_WIDTH - 1 : 0]       m_attrb_tcolor_g, // Qn.0
    output wire [SUB_PIXEL_WIDTH - 1 : 0]       m_attrb_tcolor_r // Qn.0
);
    localparam FOG_PRECISION = CALC_PRECISION - 1; // Converts the size from signed to unsigned
    localparam FOG_ITERATIONS = 2;
    localparam TEXQ_PRECISION = CALC_PRECISION - 1; // Converts the size from signed to unsigned
    localparam TEXQ_ITERATIONS = 2;
    localparam TEX_PERSP_CORR_SHIFT = TEXQ_PRECISION - 8;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Calculate the reciprocal
    // Clocks: 13
    ///////////////////////////////////////////////////////////////////////////
    localparam RECIP_DELAY = 7 + (TEXQ_ITERATIONS * 3);
    wire signed [TEXQ_PRECISION - 1 : 0]        step1_tex0_s; // S3.20
    wire signed [TEXQ_PRECISION - 1 : 0]        step1_tex0_t; // S3.20
    wire        [(TEXQ_PRECISION * 2) - 1 : 0]  step1_tex0_q; // U21.27
    wire signed [TEXQ_PRECISION - 1 : 0]        step1_tex0_mipmap_s;
    wire signed [TEXQ_PRECISION - 1 : 0]        step1_tex0_mipmap_t;
    wire        [(TEXQ_PRECISION * 2) - 1 : 0]  step1_tex0_mipmap_q;
    wire signed [TEXQ_PRECISION - 1 : 0]        step1_tex1_s;
    wire signed [TEXQ_PRECISION - 1 : 0]        step1_tex1_t;
    wire        [(TEXQ_PRECISION * 2) - 1 : 0]  step1_tex1_q;
    wire signed [TEXQ_PRECISION - 1 : 0]        step1_tex1_mipmap_s;
    wire signed [TEXQ_PRECISION - 1 : 0]        step1_tex1_mipmap_t;
    wire        [(TEXQ_PRECISION * 2) - 1 : 0]  step1_tex1_mipmap_q;
    wire        [(FOG_PRECISION * 2) - 1 : 0]   step1_depth_w; // U21.27
    wire        [DEPTH_WIDTH - 1 : 0]           step1_depth_z; // U0.16
    wire        [16 - 1 : 0]                    step1_color_r; // S7.8
    wire        [16 - 1 : 0]                    step1_color_g;
    wire        [16 - 1 : 0]                    step1_color_b;
    wire        [16 - 1 : 0]                    step1_color_a;
    wire                                        step1_tvalid;
    wire                                        step1_tlast;
    wire        [KEEP_WIDTH - 1 : 0]            step1_tkeep;
    wire        [SCREEN_POS_WIDTH - 1 : 0]      step1_tspx;
    wire        [SCREEN_POS_WIDTH - 1 : 0]      step1_tspy;
    wire        [INDEX_WIDTH - 1 : 0]           step1_tindex;

    ValueDelay #(
        .VALUE_SIZE(1), 
        .DELAY(RECIP_DELAY)
    ) step1_tvalid_delay (
        .clk(aclk), 
        .ce(ce), 
        .in(s_attrb_tvalid & s_attrb_tpixel), 
        .out(step1_tvalid)
    );
    ValueDelay #(
        .VALUE_SIZE(1), 
        .DELAY(RECIP_DELAY)
    ) step1_tlast_delay (
        .clk(aclk), 
        .ce(ce), 
        .in(s_attrb_tlast), 
        .out(step1_tlast)
    );
    ValueDelay #(
        .VALUE_SIZE(KEEP_WIDTH), 
        .DELAY(RECIP_DELAY)
    ) step1_tkeep_delay (
        .clk(aclk), 
        .ce(ce), 
        .in(s_attrb_tkeep), 
        .out(step1_tkeep)
    );

    ValueDelay #(
        .VALUE_SIZE(SCREEN_POS_WIDTH), 
        .DELAY(RECIP_DELAY)
    ) step1_tspx_delay (
        .clk(aclk), 
        .ce(ce), 
        .in(s_attrb_tspx), 
        .out(step1_tspx)
    );
    ValueDelay #(
        .VALUE_SIZE(SCREEN_POS_WIDTH), 
        .DELAY(RECIP_DELAY)
    ) step1_tspy_delay (
        .clk(aclk), 
        .ce(ce), 
        .in(s_attrb_tspy), 
        .out(step1_tspy)
    );

    ValueDelay #(
        .VALUE_SIZE(INDEX_WIDTH), 
        .DELAY(RECIP_DELAY)
    ) step1_tindex_delay (
        .clk(aclk), 
        .ce(ce), 
        .in(s_attrb_tindex), 
        .out(step1_tindex)
    );
    
    ValueDelay #(
        .VALUE_SIZE(16), 
        .DELAY(RECIP_DELAY)
    ) step1_color_r_delay (
        .clk(aclk), 
        .ce(ce), 
        .in(color_r[16 +: 16]), 
        .out(step1_color_r)
    );
    ValueDelay #(
        .VALUE_SIZE(16), 
        .DELAY(RECIP_DELAY)
    ) step1_color_g_delay (
        .clk(aclk), 
        .ce(ce), 
        .in(color_g[16 +: 16]), 
        .out(step1_color_g)
    );
    ValueDelay #(
        .VALUE_SIZE(16), 
        .DELAY(RECIP_DELAY)
    ) step1_color_b_delay (
        .clk(aclk), 
        .ce(ce), 
        .in(color_b[16 +: 16]), 
        .out(step1_color_b)
    );
    ValueDelay #(
        .VALUE_SIZE(16), 
        .DELAY(RECIP_DELAY)
    ) step1_color_a_delay (
        .clk(aclk), 
        .ce(ce), 
        .in(color_a[16 +: 16]), 
        .out(step1_color_a)
    );

    ValueDelay #(
        .VALUE_SIZE(DEPTH_WIDTH), 
        .DELAY(RECIP_DELAY)
    ) step1_depth_z_delay (
        .clk(aclk), 
        .ce(ce), 
        .in((depth_z[31]) ? { DEPTH_WIDTH { 1'b0 } } : (depth_z[30]) ? { DEPTH_WIDTH { 1'b1 } } : depth_z[14 +: DEPTH_WIDTH]), 
        .out(step1_depth_z)
    );
    XRecip #(
        .NUMBER_WIDTH(FOG_PRECISION),
        .ITERATIONS(FOG_ITERATIONS)
    ) step1_depth_w_recip (
        .clk(aclk), 
        .ce(ce), 
        .in(depth_w[ATTRIBUTE_SIZE - FOG_PRECISION - 1 +: FOG_PRECISION]), 
        .out(step1_depth_w)
    );

    ValueDelay #(
        .VALUE_SIZE(TEXQ_PRECISION), 
        .DELAY(RECIP_DELAY)
    ) step1_tex0_s_delay (
        .clk(aclk), 
        .ce(ce), 
        .in(tex0_s[ATTRIBUTE_SIZE - TEXQ_PRECISION +: TEXQ_PRECISION]), 
        .out(step1_tex0_s)
    );
    ValueDelay #(
        .VALUE_SIZE(TEXQ_PRECISION), 
        .DELAY(RECIP_DELAY)
    ) step1_tex0_t_delay (
        .clk(aclk), 
        .ce(ce), 
        .in(tex0_t[ATTRIBUTE_SIZE - TEXQ_PRECISION +: TEXQ_PRECISION]), 
        .out(step1_tex0_t)
    );
    XRecip #(
        .NUMBER_WIDTH(TEXQ_PRECISION),
        .ITERATIONS(TEXQ_ITERATIONS)
    ) step1_tex0_q_recip (
        .clk(aclk), 
        .ce(ce), 
        // S3.30 >> 7 = U3.21 Clamp to 24 bit and remove sign, because the value is normalized between 1.0 and 0.0
        .in(tex0_q[ATTRIBUTE_SIZE - TEXQ_PRECISION - 1 +: TEXQ_PRECISION]), 
        .out(step1_tex0_q)
    );
    generate
        if (ENABLE_LOD_CALC)
        begin
            ValueDelay #(
                .VALUE_SIZE(TEXQ_PRECISION), 
                .DELAY(RECIP_DELAY)
            ) step1_tex0_mipmap_s_delay (
                .clk(aclk), 
                .ce(ce), 
                .in(tex0_mipmap_s[ATTRIBUTE_SIZE - TEXQ_PRECISION +: TEXQ_PRECISION]), 
                .out(step1_tex0_mipmap_s)
            );
            ValueDelay #(
                .VALUE_SIZE(TEXQ_PRECISION), 
                .DELAY(RECIP_DELAY)
            ) step1_tex0_mipmap_t_delay (
                .clk(aclk), 
                .ce(ce), 
                .in(tex0_mipmap_t[ATTRIBUTE_SIZE - TEXQ_PRECISION +: TEXQ_PRECISION]), 
                .out(step1_tex0_mipmap_t)
            );
            XRecip #(
                .NUMBER_WIDTH(TEXQ_PRECISION),
                .ITERATIONS(TEXQ_ITERATIONS)
            ) step1_tex0_mipmap_q_recip (
                .clk(aclk),
                .ce(ce),  
                .in(tex0_mipmap_q[ATTRIBUTE_SIZE - TEXQ_PRECISION - 1 +: TEXQ_PRECISION]), 
                .out(step1_tex0_mipmap_q)
            );
        end
    endgenerate

    generate
        if (ENABLE_SECOND_TMU)
        begin
            ValueDelay #(
                .VALUE_SIZE(TEXQ_PRECISION), 
                .DELAY(RECIP_DELAY)
            ) step1_tex1_s_delay (
                .clk(aclk), 
                .ce(ce), 
                .in(tex1_s[ATTRIBUTE_SIZE - TEXQ_PRECISION +: TEXQ_PRECISION]), 
                .out(step1_tex1_s)
            );
            ValueDelay #(
                .VALUE_SIZE(TEXQ_PRECISION), 
                .DELAY(RECIP_DELAY)
            ) step1_tex1_t_delay (
                .clk(aclk), 
                .ce(ce), 
                .in(tex1_t[ATTRIBUTE_SIZE - TEXQ_PRECISION +: TEXQ_PRECISION]), 
                .out(step1_tex1_t)
            );
            XRecip #(
                .NUMBER_WIDTH(TEXQ_PRECISION),
                .ITERATIONS(TEXQ_ITERATIONS)
            ) step1_tex1_q_recip (
                .clk(aclk), 
                .ce(ce), 
                .in(tex1_q[ATTRIBUTE_SIZE - TEXQ_PRECISION - 1 +: TEXQ_PRECISION]), 
                .out(step1_tex1_q)
            );
            if (ENABLE_LOD_CALC)
            begin
                ValueDelay #(
                    .VALUE_SIZE(TEXQ_PRECISION), 
                    .DELAY(RECIP_DELAY)
                ) step1_tex1_mipmap_s_delay (
                    .clk(aclk), 
                    .ce(ce), 
                    .in(tex1_mipmap_s[ATTRIBUTE_SIZE - TEXQ_PRECISION +: TEXQ_PRECISION]), 
                    .out(step1_tex1_mipmap_s)
                );
                ValueDelay #(
                    .VALUE_SIZE(TEXQ_PRECISION), 
                    .DELAY(RECIP_DELAY)
                ) step1_tex1_mipmap_t_delay (
                    .clk(aclk), 
                    .ce(ce), 
                    .in(tex1_mipmap_t[ATTRIBUTE_SIZE - TEXQ_PRECISION +: TEXQ_PRECISION]), 
                    .out(step1_tex1_mipmap_t)
                );
                XRecip #(
                    .NUMBER_WIDTH(TEXQ_PRECISION),
                    .ITERATIONS(TEXQ_ITERATIONS)
                ) step1_tex1_mipmap_q_recip (
                    .clk(aclk), 
                    .ce(ce), 
                    .in(tex1_mipmap_q[ATTRIBUTE_SIZE - TEXQ_PRECISION - 1 +: TEXQ_PRECISION]), 
                    .out(step1_tex1_mipmap_q)
                );
            end
        end
    endgenerate

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Calculate perspective correction
    // Clocks: 4
    ///////////////////////////////////////////////////////////////////////////
    localparam I2F_DELAY = 4;
    wire [ATTRIBUTE_SIZE - 1 : 0]    step2_tex0_s; // S16.15
    wire [ATTRIBUTE_SIZE - 1 : 0]    step2_tex0_t;
    wire [ATTRIBUTE_SIZE - 1 : 0]    step2_tex0_mipmap_s;
    wire [ATTRIBUTE_SIZE - 1 : 0]    step2_tex0_mipmap_t;
    wire [ATTRIBUTE_SIZE - 1 : 0]    step2_tex1_s;
    wire [ATTRIBUTE_SIZE - 1 : 0]    step2_tex1_t;
    wire [ATTRIBUTE_SIZE - 1 : 0]    step2_tex1_mipmap_s;
    wire [ATTRIBUTE_SIZE - 1 : 0]    step2_tex1_mipmap_t;
    wire [FLOAT_SIZE - 1 : 0]        step2_depth_w;
    wire [DEPTH_WIDTH - 1 : 0]       step2_depth_z;
    wire [SUB_PIXEL_WIDTH - 1 : 0]   step2_color_r;
    wire [SUB_PIXEL_WIDTH - 1 : 0]   step2_color_g;
    wire [SUB_PIXEL_WIDTH - 1 : 0]   step2_color_b;
    wire [SUB_PIXEL_WIDTH - 1 : 0]   step2_color_a;
    wire                             step2_tvalid;
    wire                             step2_tlast;
    wire [KEEP_WIDTH - 1 : 0]        step2_tkeep;
    wire [SCREEN_POS_WIDTH - 1 : 0]  step2_tspx;
    wire [SCREEN_POS_WIDTH - 1 : 0]  step2_tspy;
    wire [INDEX_WIDTH - 1 : 0]       step2_tindex;

    ValueDelay #(.VALUE_SIZE(1), .DELAY(I2F_DELAY)) 
        step2_tvalid_delay (.clk(aclk), .ce(ce), .in(step1_tvalid), .out(step2_tvalid));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(I2F_DELAY)) 
        step2_tlast_delay (.clk(aclk), .ce(ce), .in(step1_tlast), .out(step2_tlast));
    ValueDelay #(.VALUE_SIZE(KEEP_WIDTH), .DELAY(I2F_DELAY)) 
        step2_tkeep_delay (.clk(aclk), .ce(ce), .in(step1_tkeep), .out(step2_tkeep));

    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(I2F_DELAY)) 
        step2_tspx_delay (.clk(aclk), .ce(ce), .in(step1_tspx), .out(step2_tspx));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(I2F_DELAY)) 
        step2_tspy_delay (.clk(aclk), .ce(ce), .in(step1_tspy), .out(step2_tspy));

    ValueDelay #(.VALUE_SIZE(INDEX_WIDTH), .DELAY(I2F_DELAY)) 
        step2_tindex_delay (.clk(aclk), .ce(ce), .in(step1_tindex), .out(step2_tindex));

    ValueDelay #(.VALUE_SIZE(DEPTH_WIDTH), .DELAY(I2F_DELAY)) 
        step2_tdepth_z_delay (.clk(aclk), .ce(ce), .in(step1_depth_z), .out(step2_depth_z));

    IntToFloat #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(ATTRIBUTE_SIZE))
        step2_tdepth_w_i2f (.clk(aclk), .ce(ce), .offset(-9), .in(step1_depth_w[FOG_PRECISION - 8 +: FOG_PRECISION + 8]), .out(step2_depth_w));

    reg  [TEXQ_PRECISION * 2 : 0]    step2_tex0_s_reg; // S16.15
    reg  [TEXQ_PRECISION * 2 : 0]    step2_tex0_t_reg;
    reg  [TEXQ_PRECISION * 2 : 0]    step2_tex0_mipmap_s_reg;
    reg  [TEXQ_PRECISION * 2 : 0]    step2_tex0_mipmap_t_reg;
    reg  [TEXQ_PRECISION * 2 : 0]    step2_tex1_s_reg;
    reg  [TEXQ_PRECISION * 2 : 0]    step2_tex1_t_reg;
    reg  [TEXQ_PRECISION * 2 : 0]    step2_tex1_mipmap_s_reg;
    reg  [TEXQ_PRECISION * 2 : 0]    step2_tex1_mipmap_t_reg;
    reg  [SUB_PIXEL_WIDTH - 1 : 0]   step2_color_r_reg;
    reg  [SUB_PIXEL_WIDTH - 1 : 0]   step2_color_g_reg;
    reg  [SUB_PIXEL_WIDTH - 1 : 0]   step2_color_b_reg;
    reg  [SUB_PIXEL_WIDTH - 1 : 0]   step2_color_a_reg;
    always @(posedge aclk)
    if (ce) begin : PerspCorrection
        step2_color_a_reg <= (step1_color_a[15]) ? 0 : (|step1_color_a[SUB_PIXEL_WIDTH +: 7]) ? { SUB_PIXEL_WIDTH { 1'b1 } } : step1_color_a[0 +: SUB_PIXEL_WIDTH];
        step2_color_b_reg <= (step1_color_b[15]) ? 0 : (|step1_color_b[SUB_PIXEL_WIDTH +: 7]) ? { SUB_PIXEL_WIDTH { 1'b1 } } : step1_color_b[0 +: SUB_PIXEL_WIDTH];
        step2_color_g_reg <= (step1_color_g[15]) ? 0 : (|step1_color_g[SUB_PIXEL_WIDTH +: 7]) ? { SUB_PIXEL_WIDTH { 1'b1 } } : step1_color_g[0 +: SUB_PIXEL_WIDTH];
        step2_color_r_reg <= (step1_color_r[15]) ? 0 : (|step1_color_r[SUB_PIXEL_WIDTH +: 7]) ? { SUB_PIXEL_WIDTH { 1'b1 } } : step1_color_r[0 +: SUB_PIXEL_WIDTH];

        step2_tex0_s_reg <= (step1_tex0_s * $signed({ 1'b0, step1_tex0_q[TEXQ_PRECISION - 8 +: TEXQ_PRECISION] })) >>> (TEX_PERSP_CORR_SHIFT); // U13.11 * S3.20 = 16.31 >>> 16 = S16.15
        step2_tex0_t_reg <= (step1_tex0_t * $signed({ 1'b0, step1_tex0_q[TEXQ_PRECISION - 8 +: TEXQ_PRECISION] })) >>> (TEX_PERSP_CORR_SHIFT);
        if (ENABLE_LOD_CALC)
        begin
            step2_tex0_mipmap_s_reg <= (step1_tex0_mipmap_s * $signed({ 1'b0, step1_tex0_mipmap_q[TEXQ_PRECISION - 8 +: TEXQ_PRECISION] })) >>> (TEX_PERSP_CORR_SHIFT);
            step2_tex0_mipmap_t_reg <= (step1_tex0_mipmap_t * $signed({ 1'b0, step1_tex0_mipmap_q[TEXQ_PRECISION - 8 +: TEXQ_PRECISION] })) >>> (TEX_PERSP_CORR_SHIFT);
        end
        else
        begin
            step2_tex0_mipmap_s_reg <= 0;
            step2_tex0_mipmap_t_reg <= 0;
        end

        if (ENABLE_SECOND_TMU)
        begin
            step2_tex1_s_reg <= (step1_tex1_s * $signed({ 1'b0, step1_tex1_q[TEXQ_PRECISION - 8 +: TEXQ_PRECISION] })) >>> TEX_PERSP_CORR_SHIFT;
            step2_tex1_t_reg <= (step1_tex1_t * $signed({ 1'b0, step1_tex1_q[TEXQ_PRECISION - 8 +: TEXQ_PRECISION] })) >>> TEX_PERSP_CORR_SHIFT;
            if (ENABLE_LOD_CALC)
            begin
                step2_tex1_mipmap_s_reg <= (step1_tex1_mipmap_s * $signed({ 1'b0, step1_tex1_mipmap_q[TEXQ_PRECISION - 8 +: TEXQ_PRECISION] })) >>> TEX_PERSP_CORR_SHIFT;
                step2_tex1_mipmap_t_reg <= (step1_tex1_mipmap_t * $signed({ 1'b0, step1_tex1_mipmap_q[TEXQ_PRECISION - 8 +: TEXQ_PRECISION] })) >>> TEX_PERSP_CORR_SHIFT;
            end
            else
            begin
                step2_tex1_mipmap_s_reg <= 0;
                step2_tex1_mipmap_t_reg <= 0;
            end
        end
        else
        begin
            step2_tex1_s_reg <= 0;
            step2_tex1_t_reg <= 0;
            step2_tex1_mipmap_s_reg <= 0;
            step2_tex1_mipmap_t_reg <= 0;
        end
    end

    ValueDelay #(.VALUE_SIZE(ATTRIBUTE_SIZE), .DELAY(I2F_DELAY - 1)) 
        step2_ttex0_s_delay (.clk(aclk), .ce(ce), .in(step2_tex0_s_reg[0 +: ATTRIBUTE_SIZE]), .out(step2_tex0_s));
    ValueDelay #(.VALUE_SIZE(ATTRIBUTE_SIZE), .DELAY(I2F_DELAY - 1)) 
        step2_ttex0_t_delay (.clk(aclk), .ce(ce), .in(step2_tex0_t_reg[0 +: ATTRIBUTE_SIZE]), .out(step2_tex0_t));
    ValueDelay #(.VALUE_SIZE(ATTRIBUTE_SIZE), .DELAY(I2F_DELAY - 1)) 
        step2_ttex0_mipmap_s_delay (.clk(aclk), .ce(ce), .in(step2_tex0_mipmap_s_reg[0 +: ATTRIBUTE_SIZE]), .out(step2_tex0_mipmap_s));
    ValueDelay #(.VALUE_SIZE(ATTRIBUTE_SIZE), .DELAY(I2F_DELAY - 1)) 
        step2_ttex0_mipmap_t_delay (.clk(aclk), .ce(ce), .in(step2_tex0_mipmap_t_reg[0 +: ATTRIBUTE_SIZE]), .out(step2_tex0_mipmap_t));
    ValueDelay #(.VALUE_SIZE(ATTRIBUTE_SIZE), .DELAY(I2F_DELAY - 1)) 
        step2_ttex1_s_delay (.clk(aclk), .ce(ce), .in(step2_tex1_s_reg[0 +: ATTRIBUTE_SIZE]), .out(step2_tex1_s));
    ValueDelay #(.VALUE_SIZE(ATTRIBUTE_SIZE), .DELAY(I2F_DELAY - 1)) 
        step2_ttex1_t_delay (.clk(aclk), .ce(ce), .in(step2_tex1_t_reg[0 +: ATTRIBUTE_SIZE]), .out(step2_tex1_t));
    ValueDelay #(.VALUE_SIZE(ATTRIBUTE_SIZE), .DELAY(I2F_DELAY - 1)) 
        step2_ttex1_mipmap_s_delay (.clk(aclk), .ce(ce), .in(step2_tex1_mipmap_s_reg[0 +: ATTRIBUTE_SIZE]), .out(step2_tex1_mipmap_s));
    ValueDelay #(.VALUE_SIZE(ATTRIBUTE_SIZE), .DELAY(I2F_DELAY - 1)) 
        step2_ttex1_mipmap_t_delay (.clk(aclk), .ce(ce), .in(step2_tex1_mipmap_t_reg[0 +: ATTRIBUTE_SIZE]), .out(step2_tex1_mipmap_t));
    ValueDelay #(.VALUE_SIZE(SUB_PIXEL_WIDTH), .DELAY(I2F_DELAY - 1)) 
        step2_tcolor_r_delay (.clk(aclk), .ce(ce), .in(step2_color_r_reg), .out(step2_color_r));
    ValueDelay #(.VALUE_SIZE(SUB_PIXEL_WIDTH), .DELAY(I2F_DELAY - 1)) 
        step2_tcolor_g_delay (.clk(aclk), .ce(ce), .in(step2_color_g_reg), .out(step2_color_g));
    ValueDelay #(.VALUE_SIZE(SUB_PIXEL_WIDTH), .DELAY(I2F_DELAY - 1)) 
        step2_tcolor_b_delay (.clk(aclk), .ce(ce), .in(step2_color_b_reg), .out(step2_color_b));
    ValueDelay #(.VALUE_SIZE(SUB_PIXEL_WIDTH), .DELAY(I2F_DELAY - 1)) 
        step2_tcolor_a_delay (.clk(aclk), .ce(ce), .in(step2_color_a_reg), .out(step2_color_a));

    ////////////////////////////////////////////////////////////////////////////
    // STEP 3
    // Output data
    // Clocks: 0
    ///////////////////////////////////////////////////////////////////////////
    assign m_attrb_tvalid = step2_tvalid;
    assign m_attrb_tlast = step2_tlast;
    assign m_attrb_tkeep = step2_tkeep;
    assign m_attrb_tspx = step2_tspx;
    assign m_attrb_tspy = step2_tspy;
    assign m_attrb_tindex = step2_tindex;
    assign m_attrb_tdepth_w = step2_depth_w;
    assign m_attrb_tdepth_z = { 16'h0, step2_depth_z };

    assign m_attrb_ttexture0_t = step2_tex0_t;
    assign m_attrb_ttexture0_s = step2_tex0_s;
    generate
        if (ENABLE_LOD_CALC)
        begin
            assign m_attrb_tmipmap0_t = step2_tex0_mipmap_t;
            assign m_attrb_tmipmap0_s = step2_tex0_mipmap_s;
        end
        else
        begin
            assign m_attrb_tmipmap0_t = step2_tex0_t;
            assign m_attrb_tmipmap0_s = step2_tex0_s;
        end
    endgenerate

    generate
        if (ENABLE_SECOND_TMU)
        begin
            assign m_attrb_ttexture1_t = step2_tex1_t;
            assign m_attrb_ttexture1_s = step2_tex1_s;
            if (ENABLE_LOD_CALC)
            begin
                assign m_attrb_tmipmap1_t = step2_tex1_mipmap_t;
                assign m_attrb_tmipmap1_s = step2_tex1_mipmap_s;
            end
            else
            begin
                assign m_attrb_tmipmap1_t = step2_tex1_t;
                assign m_attrb_tmipmap1_s = step2_tex1_s;
            end
        end
        else
        begin
            assign m_attrb_ttexture1_t = 0;
            assign m_attrb_ttexture1_s = 0;
            assign m_attrb_tmipmap1_t = 0;
            assign m_attrb_tmipmap1_s = 0;
        end
    endgenerate

    assign m_attrb_tcolor_a = step2_color_a;
    assign m_attrb_tcolor_b = step2_color_b;
    assign m_attrb_tcolor_g = step2_color_g;
    assign m_attrb_tcolor_r = step2_color_r;
endmodule
