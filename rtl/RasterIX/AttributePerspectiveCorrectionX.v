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

// This module is used to calculate the perspective correction of the
// attributes given from the interpolator.
// Pipelined: yes
// Depth: 16 cycles
module AttributePerspectiveCorrectionX #(
    parameter INDEX_WIDTH = 32,
    parameter SCREEN_POS_WIDTH = 11,
    parameter ENABLE_LOD_CALC = 1,
    parameter ENABLE_SECOND_TMU = 1,
    parameter SUB_PIXEL_WIDTH = 8,
    parameter CALC_PRECISION = 25, // The precision of a signed multiplication

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
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_s, // S3.28
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_t, // S3.28
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_q, // S3.28
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_mipmap_s, // S3.28
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_mipmap_t, // S3.28
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] tex1_mipmap_q, // S3.28
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] depth_w, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] depth_z, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_r, // S7.24
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_g, // S7.24
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_b, // S7.24
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] color_a, // S7.24

    // Pixel Stream Interpolated
    output wire                                 m_attrb_tvalid,
    output wire                                 m_attrb_tpixel,
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
    localparam FOG_PRECISION = CALC_PRECISION - 1; // Keep a spare bit for a potential sign
    localparam TEXQ_PRECISION = CALC_PRECISION - 1; // Keep a spare bit for a potential sign
    localparam TEXQ_ITERATIONS = 2;
    localparam TEXQ_SHIFT_WIDTH = 5;
    localparam TEXQ_MAGNITUDE_WIDTH = 31;
    localparam MANTISSA_SIZE = TEXQ_PRECISION + 1; // + 1 because of the hidden bit in the mantissa
    localparam RECIP_CALC_DELAY = 4 + (TEXQ_ITERATIONS * 3); // ComputeRecip
    localparam TEX_HEADROOM = 4;


    initial 
    begin
        if ((CALC_PRECISION < 18) || (CALC_PRECISION > 25))
        begin
            $error("The fixpoint calculation precision must be between 18 and 25. Currently: %d", CALC_PRECISION);
            $finish;
        end
    end

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Find the leading one of each q, needed to normalize q, s and t
    // Clocks: 1
    ///////////////////////////////////////////////////////////////////////////
    wire signed [ATTRIBUTE_SIZE - 1 : 0]        step1_tex0_s;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]        step1_tex0_t;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]        step1_tex0_q;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]        step1_tex0_mipmap_s;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]        step1_tex0_mipmap_t;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]        step1_tex0_mipmap_q;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]        step1_tex1_s;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]        step1_tex1_t;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]        step1_tex1_q;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]        step1_tex1_mipmap_s;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]        step1_tex1_mipmap_t;
    wire signed [ATTRIBUTE_SIZE - 1 : 0]        step1_tex1_mipmap_q;
    wire        [(FOG_PRECISION * 1) - 1 : 0]   step1_depth_w; // S1.22
    wire        [DEPTH_WIDTH - 1 : 0]           step1_depth_z; // U0.16
    wire        [16 - 1 : 0]                    step1_color_r; // S7.8
    wire        [16 - 1 : 0]                    step1_color_g;
    wire        [16 - 1 : 0]                    step1_color_b;
    wire        [16 - 1 : 0]                    step1_color_a;
    wire                                        step1_tvalid;
    wire                                        step1_tpixel;
    wire                                        step1_tlast;
    wire        [KEEP_WIDTH - 1 : 0]            step1_tkeep;
    wire        [SCREEN_POS_WIDTH - 1 : 0]      step1_tspx;
    wire        [SCREEN_POS_WIDTH - 1 : 0]      step1_tspy;
    wire        [INDEX_WIDTH - 1 : 0]           step1_tindex;
    wire        [TEXQ_SHIFT_WIDTH - 1 : 0]      step1_leading_one_position_tex0_q;
    wire        [TEXQ_SHIFT_WIDTH - 1 : 0]      step1_leading_one_position_tex0_mipmap_q;
    wire        [TEXQ_SHIFT_WIDTH - 1 : 0]      step1_leading_one_position_tex1_q;
    wire        [TEXQ_SHIFT_WIDTH - 1 : 0]      step1_leading_one_position_tex1_mipmap_q;

    LeadingOneFinder #(
        .OUT_SIZE(TEXQ_SHIFT_WIDTH),
        .VALUE_SIZE(TEXQ_MAGNITUDE_WIDTH)
    ) tex0QLeadingOneFinder (
        .clk(aclk),
        .ce(ce),
        .value(tex0_q[0 +: TEXQ_MAGNITUDE_WIDTH]),
        .out(step1_leading_one_position_tex0_q)
    );

    generate
        if (ENABLE_LOD_CALC)
        begin
            LeadingOneFinder #(
                .OUT_SIZE(TEXQ_SHIFT_WIDTH),
                .VALUE_SIZE(TEXQ_MAGNITUDE_WIDTH)
            ) tex0MipmapQLeadingOneFinder (
                .clk(aclk),
                .ce(ce),
                .value(tex0_mipmap_q[0 +: TEXQ_MAGNITUDE_WIDTH]),
                .out(step1_leading_one_position_tex0_mipmap_q)
            );
        end
        else
        begin
            // All-ones is LeadingOneFinder's sentinel for "value is zero"
            assign step1_leading_one_position_tex0_mipmap_q = { TEXQ_SHIFT_WIDTH { 1'b1 } };
        end
    endgenerate

    generate
        if (ENABLE_SECOND_TMU)
        begin
            LeadingOneFinder #(
                .OUT_SIZE(TEXQ_SHIFT_WIDTH),
                .VALUE_SIZE(TEXQ_MAGNITUDE_WIDTH)
            ) tex1QLeadingOneFinder (
                .clk(aclk),
                .ce(ce),
                .value(tex1_q[0 +: TEXQ_MAGNITUDE_WIDTH]),
                .out(step1_leading_one_position_tex1_q)
            );

            if (ENABLE_LOD_CALC)
            begin
                LeadingOneFinder #(
                    .OUT_SIZE(TEXQ_SHIFT_WIDTH),
                    .VALUE_SIZE(TEXQ_MAGNITUDE_WIDTH)
                ) tex1MipmapQLeadingOneFinder (
                    .clk(aclk),
                    .ce(ce),
                    .value(tex1_mipmap_q[0 +: TEXQ_MAGNITUDE_WIDTH]),
                    .out(step1_leading_one_position_tex1_mipmap_q)
                );
            end
            else
            begin
                // All-ones is LeadingOneFinder's sentinel for "value is zero"
                assign step1_leading_one_position_tex1_mipmap_q = { TEXQ_SHIFT_WIDTH { 1'b1 } };
            end
        end
        else
        begin
            // All-ones is LeadingOneFinder's sentinel for "value is zero"
            assign step1_leading_one_position_tex1_q = { TEXQ_SHIFT_WIDTH { 1'b1 } };
            assign step1_leading_one_position_tex1_mipmap_q = { TEXQ_SHIFT_WIDTH { 1'b1 } };
        end
    endgenerate

    ValueDelay #(
        .VALUE_SIZE((ATTRIBUTE_SIZE * 12) + 1 + 1 + 1 + KEEP_WIDTH + (SCREEN_POS_WIDTH * 2) + INDEX_WIDTH + 16 + 16 + 16 + 16 + DEPTH_WIDTH + FOG_PRECISION), 
        .DELAY(1)
    ) step1_delay (
        .clk(aclk), 
        .ce(ce), 
        .in({
            s_attrb_tvalid,
            s_attrb_tpixel,
            s_attrb_tlast,
            s_attrb_tkeep,
            s_attrb_tspx,
            s_attrb_tspy,
            s_attrb_tindex,
            color_r[16 +: 16],
            color_g[16 +: 16],
            color_b[16 +: 16],
            color_a[16 +: 16],
            (depth_z[31]) ? { DEPTH_WIDTH { 1'b0 } } : (depth_z[30]) ? { DEPTH_WIDTH { 1'b1 } } : depth_z[14 +: DEPTH_WIDTH],
            tex0_q, 
            tex0_s, 
            tex0_t,
            tex0_mipmap_q, 
            tex0_mipmap_s, 
            tex0_mipmap_t,
            tex1_q, 
            tex1_s, 
            tex1_t,
            tex1_mipmap_q, 
            tex1_mipmap_s, 
            tex1_mipmap_t,
            depth_w[ATTRIBUTE_SIZE - FOG_PRECISION - 0 +: FOG_PRECISION]
        }), 
        .out({
            step1_tvalid,
            step1_tpixel,
            step1_tlast,
            step1_tkeep,
            step1_tspx,
            step1_tspy,
            step1_tindex,
            step1_color_r,
            step1_color_g,
            step1_color_b,
            step1_color_a,
            step1_depth_z,
            step1_tex0_q, 
            step1_tex0_s,
            step1_tex0_t,
            step1_tex0_mipmap_q, 
            step1_tex0_mipmap_s,
            step1_tex0_mipmap_t,
            step1_tex1_q, 
            step1_tex1_s,
            step1_tex1_t,
            step1_tex1_mipmap_q, 
            step1_tex1_mipmap_s,
            step1_tex1_mipmap_t,
            step1_depth_w
        })
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Normalize q, s and t with the leading one found in STEP 1
    // Clocks: 1
    ///////////////////////////////////////////////////////////////////////////
    wire        [TEXQ_PRECISION - 1 : 0]        step2_mantissa_tex0_q; // Q1.(TEXQ_PRECISION - 1)
    wire        [TEXQ_PRECISION - 1 : 0]        step2_mantissa_tex0_mipmap_q;
    wire        [TEXQ_PRECISION - 1 : 0]        step2_mantissa_tex1_q;
    wire        [TEXQ_PRECISION - 1 : 0]        step2_mantissa_tex1_mipmap_q;
    wire signed [TEXQ_PRECISION - 1 : 0]        step2_norm_tex0_s; // S7+k.(TEXQ_PRECISION - 1 - 7 - k)
    wire signed [TEXQ_PRECISION - 1 : 0]        step2_norm_tex0_t;
    wire signed [TEXQ_PRECISION - 1 : 0]        step2_norm_tex0_mipmap_s;
    wire signed [TEXQ_PRECISION - 1 : 0]        step2_norm_tex0_mipmap_t;
    wire signed [TEXQ_PRECISION - 1 : 0]        step2_norm_tex1_s;
    wire signed [TEXQ_PRECISION - 1 : 0]        step2_norm_tex1_t;
    wire signed [TEXQ_PRECISION - 1 : 0]        step2_norm_tex1_mipmap_s;
    wire signed [TEXQ_PRECISION - 1 : 0]        step2_norm_tex1_mipmap_t;
    wire        [(FOG_PRECISION * 1) - 1 : 0]   step2_depth_w;
    wire        [DEPTH_WIDTH - 1 : 0]           step2_depth_z;
    wire        [16 - 1 : 0]                    step2_color_r;
    wire        [16 - 1 : 0]                    step2_color_g;
    wire        [16 - 1 : 0]                    step2_color_b;
    wire        [16 - 1 : 0]                    step2_color_a;
    wire                                        step2_tvalid;
    wire                                        step2_tpixel;
    wire                                        step2_tlast;
    wire        [KEEP_WIDTH - 1 : 0]            step2_tkeep;
    wire        [SCREEN_POS_WIDTH - 1 : 0]      step2_tspx;
    wire        [SCREEN_POS_WIDTH - 1 : 0]      step2_tspy;
    wire        [INDEX_WIDTH - 1 : 0]           step2_tindex;

    ExponentNormalizer #(
        .TEXQ_PRECISION(TEXQ_PRECISION),
        .TEX_HEADROOM(TEX_HEADROOM)
    ) tex0Normalizer (
        .clk(aclk), 
        .ce(ce),
        .q(step1_tex0_q), 
        .s(step1_tex0_s), 
        .t(step1_tex0_t), 
        .q_leading_one_position(step1_leading_one_position_tex0_q),
        .q_mantissa(step2_mantissa_tex0_q), 
        .s_norm(step2_norm_tex0_s), 
        .t_norm(step2_norm_tex0_t)
    );

    generate
        if (ENABLE_LOD_CALC) 
        begin
            ExponentNormalizer #(
                .TEXQ_PRECISION(TEXQ_PRECISION),
                .TEX_HEADROOM(TEX_HEADROOM)
            ) tex0MipmapNormalizer (
                .clk(aclk), 
                .ce(ce),
                .q(step1_tex0_mipmap_q), 
                .s(step1_tex0_mipmap_s), 
                .t(step1_tex0_mipmap_t), 
                .q_leading_one_position(step1_leading_one_position_tex0_mipmap_q),
                .q_mantissa(step2_mantissa_tex0_mipmap_q), 
                .s_norm(step2_norm_tex0_mipmap_s), 
                .t_norm(step2_norm_tex0_mipmap_t)
            );
        end
        else
        begin
            assign step2_mantissa_tex0_mipmap_q = { TEXQ_PRECISION { 1'b0 } };
            assign step2_norm_tex0_mipmap_s = { TEXQ_PRECISION { 1'b0 } };
            assign step2_norm_tex0_mipmap_t = { TEXQ_PRECISION { 1'b0 } };
        end
    endgenerate

    generate
        if (ENABLE_SECOND_TMU) 
        begin
            ExponentNormalizer #(
                .TEXQ_PRECISION(TEXQ_PRECISION),
                .TEX_HEADROOM(TEX_HEADROOM)
            ) tex1Normalizer (
                .clk(aclk), 
                .ce(ce),
                .q(step1_tex1_q), 
                .s(step1_tex1_s), 
                .t(step1_tex1_t), 
                .q_leading_one_position(step1_leading_one_position_tex1_q),
                .q_mantissa(step2_mantissa_tex1_q), 
                .s_norm(step2_norm_tex1_s), 
                .t_norm(step2_norm_tex1_t)
            );
        end
        else
        begin
            assign step2_mantissa_tex1_q = { TEXQ_PRECISION { 1'b0 } };
            assign step2_norm_tex1_s = { TEXQ_PRECISION { 1'b0 } };
            assign step2_norm_tex1_t = { TEXQ_PRECISION { 1'b0 } };
        end
        if (ENABLE_LOD_CALC)
        begin
            ExponentNormalizer #(
                .TEXQ_PRECISION(TEXQ_PRECISION),
                .TEX_HEADROOM(TEX_HEADROOM)
            ) tex1MipmapNormalizer (
                .clk(aclk), 
                .ce(ce),
                .q(step1_tex1_mipmap_q),
                .s(step1_tex1_mipmap_s), 
                .t(step1_tex1_mipmap_t), 
                .q_leading_one_position(step1_leading_one_position_tex1_mipmap_q),
                .q_mantissa(step2_mantissa_tex1_mipmap_q), 
                .s_norm(step2_norm_tex1_mipmap_s), 
                .t_norm(step2_norm_tex1_mipmap_t)
            );
        end
        else
        begin
            assign step2_mantissa_tex1_mipmap_q = { TEXQ_PRECISION { 1'b0 } };
            assign step2_norm_tex1_mipmap_s = { TEXQ_PRECISION { 1'b0 } };
            assign step2_norm_tex1_mipmap_t = { TEXQ_PRECISION { 1'b0 } };
        end
    endgenerate

    ValueDelay #(
        .VALUE_SIZE(1 + 1 + 1 + KEEP_WIDTH + (SCREEN_POS_WIDTH * 2) + INDEX_WIDTH + 16 + 16 + 16 + 16 + DEPTH_WIDTH + FOG_PRECISION), 
        .DELAY(1)
    ) step2_delay (
        .clk(aclk), 
        .ce(ce), 
        .in({
            step1_tvalid,
            step1_tpixel,
            step1_tlast,
            step1_tkeep,
            step1_tspx,
            step1_tspy,
            step1_tindex,
            step1_color_r,
            step1_color_g,
            step1_color_b,
            step1_color_a,
            step1_depth_z,
            step1_depth_w
        }), 
        .out({
            step2_tvalid,
            step2_tpixel,
            step2_tlast,
            step2_tkeep,
            step2_tspx,
            step2_tspy,
            step2_tindex,
            step2_color_r,
            step2_color_g,
            step2_color_b,
            step2_color_a,
            step2_depth_z,
            step2_depth_w
        })
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 3
    // Calculate the reciprocal of each normalized q
    // Clocks: 4 + (ITERATIONS * 3)
    ///////////////////////////////////////////////////////////////////////////
    wire        [TEXQ_PRECISION - 1 : 0]        step3_tex0_q; // U1.(TEXQ_PRECISION - 1)
    wire        [TEXQ_PRECISION - 1 : 0]        step3_tex0_mipmap_q;
    wire        [TEXQ_PRECISION - 1 : 0]        step3_tex1_q;
    wire        [TEXQ_PRECISION - 1 : 0]        step3_tex1_mipmap_q;
    wire signed [TEXQ_PRECISION - 1 : 0]        step3_tex0_s; // S7+k.(TEXQ_PRECISION - 1 - 7 - k)
    wire signed [TEXQ_PRECISION - 1 : 0]        step3_tex0_t;
    wire signed [TEXQ_PRECISION - 1 : 0]        step3_tex0_mipmap_s;
    wire signed [TEXQ_PRECISION - 1 : 0]        step3_tex0_mipmap_t;
    wire signed [TEXQ_PRECISION - 1 : 0]        step3_tex1_s;
    wire signed [TEXQ_PRECISION - 1 : 0]        step3_tex1_t;
    wire signed [TEXQ_PRECISION - 1 : 0]        step3_tex1_mipmap_s;
    wire signed [TEXQ_PRECISION - 1 : 0]        step3_tex1_mipmap_t;
    wire        [(FOG_PRECISION * 1) - 1 : 0]   step3_depth_w;
    wire        [DEPTH_WIDTH - 1 : 0]           step3_depth_z;
    wire        [16 - 1 : 0]                    step3_color_r;
    wire        [16 - 1 : 0]                    step3_color_g;
    wire        [16 - 1 : 0]                    step3_color_b;
    wire        [16 - 1 : 0]                    step3_color_a;
    wire                                        step3_tvalid;
    wire                                        step3_tpixel;
    wire                                        step3_tlast;
    wire        [KEEP_WIDTH - 1 : 0]            step3_tkeep;
    wire        [SCREEN_POS_WIDTH - 1 : 0]      step3_tspx;
    wire        [SCREEN_POS_WIDTH - 1 : 0]      step3_tspy;
    wire        [INDEX_WIDTH - 1 : 0]           step3_tindex;

    ValueDelay #(
        .VALUE_SIZE(1 + 1 + 1 + KEEP_WIDTH + (SCREEN_POS_WIDTH * 2) + INDEX_WIDTH + 16 + 16 + 16 + 16 + DEPTH_WIDTH + FOG_PRECISION + (TEXQ_PRECISION * 8)), 
        .DELAY(RECIP_CALC_DELAY)
    ) step3_delay (
        .clk(aclk), 
        .ce(ce), 
        .in({
            step2_tvalid,
            step2_tpixel,
            step2_tlast,
            step2_tkeep,
            step2_tspx,
            step2_tspy,
            step2_tindex,
            step2_color_r,
            step2_color_g,
            step2_color_b,
            step2_color_a,
            step2_depth_z,
            step2_depth_w,
            step2_norm_tex0_s, 
            step2_norm_tex0_t, 
            step2_norm_tex0_mipmap_s, 
            step2_norm_tex0_mipmap_t, 
            step2_norm_tex1_s, 
            step2_norm_tex1_t, 
            step2_norm_tex1_mipmap_s, 
            step2_norm_tex1_mipmap_t
        }), 
        .out({
            step3_tvalid,
            step3_tpixel,
            step3_tlast,
            step3_tkeep,
            step3_tspx,
            step3_tspy,
            step3_tindex,
            step3_color_r,
            step3_color_g,
            step3_color_b,
            step3_color_a,
            step3_depth_z,
            step3_depth_w,
            step3_tex0_s, 
            step3_tex0_t, 
            step3_tex0_mipmap_s, 
            step3_tex0_mipmap_t, 
            step3_tex1_s, 
            step3_tex1_t, 
            step3_tex1_mipmap_s, 
            step3_tex1_mipmap_t
        })
    );

    wire signed [(MANTISSA_SIZE - 1) + MANTISSA_SIZE - 1 : 0] v_tex0;
    ComputeRecip #(
        .MS(MANTISSA_SIZE),
        .ITR(TEXQ_ITERATIONS)
    ) tex0ComputeRecip (
        .clk(aclk), .ce(ce),
        .d({ 1'b0, step2_mantissa_tex0_q }),
        .v(v_tex0)
    );
    assign step3_tex0_q = v_tex0[TEXQ_PRECISION + 1 +: TEXQ_PRECISION];

    generate
        if (ENABLE_LOD_CALC)
        begin
            wire signed [(MANTISSA_SIZE - 1) + MANTISSA_SIZE - 1 : 0] v_tex0_mipmap;
            ComputeRecip #(
                .MS(MANTISSA_SIZE),
                .ITR(TEXQ_ITERATIONS)
            ) tex0MipmapComputeRecip (
                .clk(aclk), .ce(ce),
                .d({ 1'b0, step2_mantissa_tex0_mipmap_q }),
                .v(v_tex0_mipmap)
            );
            assign step3_tex0_mipmap_q = v_tex0_mipmap[TEXQ_PRECISION + 1 +: TEXQ_PRECISION];
        end
        else
        begin
            assign step3_tex0_mipmap_q = { TEXQ_PRECISION { 1'b0 } };
        end
    endgenerate

    generate
        if (ENABLE_SECOND_TMU)
        begin
            wire signed [(MANTISSA_SIZE - 1) + MANTISSA_SIZE - 1 : 0] v_tex1;
            ComputeRecip #(
                .MS(MANTISSA_SIZE),
                .ITR(TEXQ_ITERATIONS)
            ) tex1ComputeRecip (
                .clk(aclk), .ce(ce),
                .d({ 1'b0, step2_mantissa_tex1_q }),
                .v(v_tex1)
            );
            assign step3_tex1_q = v_tex1[TEXQ_PRECISION + 1 +: TEXQ_PRECISION];

            if (ENABLE_LOD_CALC)
            begin
                wire signed [(MANTISSA_SIZE - 1) + MANTISSA_SIZE - 1 : 0] v_tex1_mipmap;
                ComputeRecip #(
                    .MS(MANTISSA_SIZE),
                    .ITR(TEXQ_ITERATIONS)
                ) tex1MipmapComputeRecip (
                    .clk(aclk), .ce(ce),
                    .d({ 1'b0, step2_mantissa_tex1_mipmap_q }),
                    .v(v_tex1_mipmap)
                );
                assign step3_tex1_mipmap_q = v_tex1_mipmap[TEXQ_PRECISION + 1 +: TEXQ_PRECISION];
            end
            else
            begin
                assign step3_tex1_mipmap_q = { TEXQ_PRECISION { 1'b0 } };
            end
        end
        else
        begin
            assign step3_tex1_q = { TEXQ_PRECISION { 1'b0 } };
            assign step3_tex1_mipmap_q = { TEXQ_PRECISION { 1'b0 } };
        end
    endgenerate

    ////////////////////////////////////////////////////////////////////////////
    // STEP 4
    // Calculate perspective correction
    // Clocks: 4
    ///////////////////////////////////////////////////////////////////////////
    localparam I2F_DELAY = 4;
    wire [ATTRIBUTE_SIZE - 1 : 0]    step4_tex0_s; // S16.15
    wire [ATTRIBUTE_SIZE - 1 : 0]    step4_tex0_t;
    wire [ATTRIBUTE_SIZE - 1 : 0]    step4_tex0_mipmap_s;
    wire [ATTRIBUTE_SIZE - 1 : 0]    step4_tex0_mipmap_t;
    wire [ATTRIBUTE_SIZE - 1 : 0]    step4_tex1_s;
    wire [ATTRIBUTE_SIZE - 1 : 0]    step4_tex1_t;
    wire [ATTRIBUTE_SIZE - 1 : 0]    step4_tex1_mipmap_s;
    wire [ATTRIBUTE_SIZE - 1 : 0]    step4_tex1_mipmap_t;
    wire [FLOAT_SIZE - 1 : 0]        step4_depth_w;
    wire [DEPTH_WIDTH - 1 : 0]       step4_depth_z;
    wire [SUB_PIXEL_WIDTH - 1 : 0]   step4_color_r;
    wire [SUB_PIXEL_WIDTH - 1 : 0]   step4_color_g;
    wire [SUB_PIXEL_WIDTH - 1 : 0]   step4_color_b;
    wire [SUB_PIXEL_WIDTH - 1 : 0]   step4_color_a;
    wire                             step4_tvalid;
    wire                             step4_tpixel;
    wire                             step4_tlast;
    wire [KEEP_WIDTH - 1 : 0]        step4_tkeep;
    wire [SCREEN_POS_WIDTH - 1 : 0]  step4_tspx;
    wire [SCREEN_POS_WIDTH - 1 : 0]  step4_tspy;
    wire [INDEX_WIDTH - 1 : 0]       step4_tindex;

    ValueDelay #(
        .VALUE_SIZE(1 + 1 + 1 + KEEP_WIDTH + (2 * SCREEN_POS_WIDTH) + INDEX_WIDTH + DEPTH_WIDTH), 
        .DELAY(I2F_DELAY)
    ) step4_delay (
        .clk(aclk), 
        .ce(ce), 
        .in({
            step3_tvalid,
            step3_tpixel,
            step3_tlast,
            step3_tkeep,
            step3_tspx,
            step3_tspy,
            step3_tindex,
            step3_depth_z
        }), 
        .out({
            step4_tvalid,
            step4_tpixel,
            step4_tlast,
            step4_tkeep,
            step4_tspx,
            step4_tspy,
            step4_tindex,
            step4_depth_z
        })
    );

    IntToFloat #(
        .MANTISSA_SIZE(FLOAT_SIZE - 9), 
        .EXPONENT_SIZE(8), 
        .INT_SIZE(FOG_PRECISION)
    ) step4_tdepth_w_i2f (
        .clk(aclk), 
        .ce(ce), 
        .offset(-(FOG_PRECISION - 2)), 
        .in(step3_depth_w), 
        .out(step4_depth_w)
    );

    wire [TEXQ_PRECISION * 2 : 0]    step4_tex0_s_wire; // S16.15
    wire [TEXQ_PRECISION * 2 : 0]    step4_tex0_t_wire;
    wire [TEXQ_PRECISION * 2 : 0]    step4_tex0_mipmap_s_wire;
    wire [TEXQ_PRECISION * 2 : 0]    step4_tex0_mipmap_t_wire;
    wire [TEXQ_PRECISION * 2 : 0]    step4_tex1_s_wire;
    wire [TEXQ_PRECISION * 2 : 0]    step4_tex1_t_wire;
    wire [TEXQ_PRECISION * 2 : 0]    step4_tex1_mipmap_s_wire;
    wire [TEXQ_PRECISION * 2 : 0]    step4_tex1_mipmap_t_wire;
    wire [SUB_PIXEL_WIDTH - 1 : 0]   step4_color_r_wire;
    wire [SUB_PIXEL_WIDTH - 1 : 0]   step4_color_g_wire;
    wire [SUB_PIXEL_WIDTH - 1 : 0]   step4_color_b_wire;
    wire [SUB_PIXEL_WIDTH - 1 : 0]   step4_color_a_wire;

    PerspectiveCorrection #(
        .TEXQ_PRECISION(TEXQ_PRECISION),
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
        .ENABLE_LOD_CALC(ENABLE_LOD_CALC),
        .ENABLE_SECOND_TMU(ENABLE_SECOND_TMU),
        .TEX_HEADROOM(TEX_HEADROOM)
    ) perspCorrection (
        .clk(aclk),
        .ce(ce),
        .color_r(step3_color_r),
        .color_g(step3_color_g),
        .color_b(step3_color_b),
        .color_a(step3_color_a),
        .tex0_s(step3_tex0_s),
        .tex0_t(step3_tex0_t),
        .tex0_q(step3_tex0_q),
        .tex0_mipmap_s(step3_tex0_mipmap_s),
        .tex0_mipmap_t(step3_tex0_mipmap_t),
        .tex0_mipmap_q(step3_tex0_mipmap_q),
        .tex1_s(step3_tex1_s),
        .tex1_t(step3_tex1_t),
        .tex1_q(step3_tex1_q),
        .tex1_mipmap_s(step3_tex1_mipmap_s),
        .tex1_mipmap_t(step3_tex1_mipmap_t),
        .tex1_mipmap_q(step3_tex1_mipmap_q),
        .color_r_reg(step4_color_r_wire),
        .color_g_reg(step4_color_g_wire),
        .color_b_reg(step4_color_b_wire),
        .color_a_reg(step4_color_a_wire),
        .tex0_s_reg(step4_tex0_s_wire),
        .tex0_t_reg(step4_tex0_t_wire),
        .tex0_mipmap_s_reg(step4_tex0_mipmap_s_wire),
        .tex0_mipmap_t_reg(step4_tex0_mipmap_t_wire),
        .tex1_s_reg(step4_tex1_s_wire),
        .tex1_t_reg(step4_tex1_t_wire),
        .tex1_mipmap_s_reg(step4_tex1_mipmap_s_wire),
        .tex1_mipmap_t_reg(step4_tex1_mipmap_t_wire)
    );

    ValueDelay #(
        .VALUE_SIZE((8 * ATTRIBUTE_SIZE) + (4 * SUB_PIXEL_WIDTH)), 
        .DELAY(I2F_DELAY - 1)
    ) step4_delay2 (
        .clk(aclk), 
        .ce(ce), 
        .in({
            step4_tex0_s_wire[0 +: ATTRIBUTE_SIZE],
            step4_tex0_t_wire[0 +: ATTRIBUTE_SIZE],
            step4_tex0_mipmap_s_wire[0 +: ATTRIBUTE_SIZE],
            step4_tex0_mipmap_t_wire[0 +: ATTRIBUTE_SIZE],
            step4_tex1_s_wire[0 +: ATTRIBUTE_SIZE],
            step4_tex1_t_wire[0 +: ATTRIBUTE_SIZE],
            step4_tex1_mipmap_s_wire[0 +: ATTRIBUTE_SIZE],
            step4_tex1_mipmap_t_wire[0 +: ATTRIBUTE_SIZE],
            step4_color_r_wire,
            step4_color_g_wire,
            step4_color_b_wire,
            step4_color_a_wire
        }), 
        .out({
            step4_tex0_s,
            step4_tex0_t,
            step4_tex0_mipmap_s,
            step4_tex0_mipmap_t,
            step4_tex1_s,
            step4_tex1_t,
            step4_tex1_mipmap_s,
            step4_tex1_mipmap_t,
            step4_color_r,
            step4_color_g,
            step4_color_b,
            step4_color_a
        })
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 5
    // Output data
    // Clocks: 0
    ///////////////////////////////////////////////////////////////////////////
    assign m_attrb_tvalid = step4_tvalid;
    assign m_attrb_tpixel = step4_tpixel;
    assign m_attrb_tlast = step4_tlast;
    assign m_attrb_tkeep = step4_tkeep;
    assign m_attrb_tspx = step4_tspx;
    assign m_attrb_tspy = step4_tspy;
    assign m_attrb_tindex = step4_tindex;
    assign m_attrb_tdepth_w = step4_depth_w;
    assign m_attrb_tdepth_z = { 16'h0, step4_depth_z };

    assign m_attrb_ttexture0_t = step4_tex0_t;
    assign m_attrb_ttexture0_s = step4_tex0_s;
    generate
        if (ENABLE_LOD_CALC)
        begin
            assign m_attrb_tmipmap0_t = step4_tex0_mipmap_t;
            assign m_attrb_tmipmap0_s = step4_tex0_mipmap_s;
        end
        else
        begin
            assign m_attrb_tmipmap0_t = step4_tex0_t;
            assign m_attrb_tmipmap0_s = step4_tex0_s;
        end
    endgenerate

    generate
        if (ENABLE_SECOND_TMU)
        begin
            assign m_attrb_ttexture1_t = step4_tex1_t;
            assign m_attrb_ttexture1_s = step4_tex1_s;
            if (ENABLE_LOD_CALC)
            begin
                assign m_attrb_tmipmap1_t = step4_tex1_mipmap_t;
                assign m_attrb_tmipmap1_s = step4_tex1_mipmap_s;
            end
            else
            begin
                assign m_attrb_tmipmap1_t = step4_tex1_t;
                assign m_attrb_tmipmap1_s = step4_tex1_s;
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

    assign m_attrb_tcolor_a = step4_color_a;
    assign m_attrb_tcolor_b = step4_color_b;
    assign m_attrb_tcolor_g = step4_color_g;
    assign m_attrb_tcolor_r = step4_color_r;
endmodule

// Normalizes q by the shift derived from the LeadingOneFinder position found upstream,
// forcing the hidden one so ComputeRecip can consume it, and scales s and t with the
// same shift in the same clock so the block cancels out later when multiplied
// with recip(q).
// Pipelined: yes
// Depth: 1 cycle
module ExponentNormalizer #(
    parameter TEXQ_PRECISION = 24,
    parameter TEX_HEADROOM = 4,

    localparam SHIFT_WIDTH = 5,
    localparam MAGNITUDE_WIDTH = 31,
    localparam ATTRIBUTE_SIZE = 32,
    localparam TEX_SCALED_SIZE = ATTRIBUTE_SIZE + TEX_HEADROOM
)
(
    input  wire                                 clk,
    input  wire                                 ce,
    input  wire        [31 : 0]                 q, // S3.28, expected to be positive
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] s, // S3.28
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0] t, // S3.28
    input  wire        [SHIFT_WIDTH - 1 : 0]    q_leading_one_position,
    output reg         [TEXQ_PRECISION - 1 : 0] q_mantissa, // U1.(TEXQ_PRECISION - 1)
    output reg  signed [TEXQ_PRECISION - 1 : 0] s_norm, // S7+k.(TEXQ_PRECISION - 1 - 7 - k)
    output reg  signed [TEXQ_PRECISION - 1 : 0] t_norm
);
    function [SHIFT_WIDTH - 1 : 0] ShiftFromOnePosition;
        input [SHIFT_WIDTH - 1 : 0] position;
        begin
            // LeadingOneFinder returns all ones when q is zero
            ShiftFromOnePosition = (position == { SHIFT_WIDTH { 1'b1 } }) 
                                   ? { SHIFT_WIDTH { 1'b0 } }
                                   : ((MAGNITUDE_WIDTH - 1) - position);
        end
    endfunction

    // Treat value as a denormalized mantissa. Shift the value to the left by the amount
    // specified by the q_leading_one_position and add the hidden one to convert it to a number in the range
    // of [1.0, 2.0). This range is then sufficient for ComputeRecip to calculate the reciprocal of q.
    function [TEXQ_PRECISION - 1 : 0] Mantissa;
        input [ATTRIBUTE_SIZE - 1 : 0] value;
        reg [MAGNITUDE_WIDTH - 1 : 0] normalized;
        begin
            normalized = value[0 +: MAGNITUDE_WIDTH] << ShiftFromOnePosition(q_leading_one_position);
            Mantissa = { 1'b1, normalized[MAGNITUDE_WIDTH - TEXQ_PRECISION +: TEXQ_PRECISION - 1] };
        end
    endfunction

    // Scales an attribute with the block q_leading_one_position of q, so that the q_leading_one_position cancels
    // out when the attribute is later multiplied with the reciprocal of q.
    function signed [TEXQ_PRECISION - 1 : 0] NormalizeAttribute;
        input signed [ATTRIBUTE_SIZE - 1 : 0] value;
        reg signed [TEX_SCALED_SIZE - 1 : 0] scaled;
        begin
            // The TEX_HEADROOM is used because the texure can be in extrem cases 3 bits larger
            // than the q value (q == 1 and s == 7). The TEX_HEADROOM is used to avoid overflow
            // in these cases.
            scaled = $signed({ { TEX_HEADROOM { value[ATTRIBUTE_SIZE - 1] } }, value }) <<< ShiftFromOnePosition(q_leading_one_position);
            NormalizeAttribute = scaled[TEX_SCALED_SIZE - TEXQ_PRECISION +: TEXQ_PRECISION];
        end
    endfunction

    always @(posedge clk)
    if (ce) begin
        q_mantissa <= Mantissa(q);
        s_norm <= NormalizeAttribute(s);
        t_norm <= NormalizeAttribute(t);
    end
endmodule

// Applies the perspective correction (multiplies s and t by the reciprocal of q) and
// clamps/truncates the interpolated color channels down to the final sub-pixel width.
// Pipelined: yes
// Depth: 1 cycle
module PerspectiveCorrection #(
    parameter TEXQ_PRECISION = 24,
    parameter SUB_PIXEL_WIDTH = 8,
    parameter ENABLE_LOD_CALC = 1,
    parameter ENABLE_SECOND_TMU = 1,
    parameter TEX_HEADROOM = 4,
    localparam TEX_PERSP_CORR_SHIFT = 
        (TEXQ_PRECISION * 2) 
        - 1 // Hidden Mantissa Bit of q
        - 1 // Sign bit removed from q before the leading-one search: ATTRIBUTE_SIZE(32) → MAGNITUDE_WIDTH(31)
        - 1 // Width→index conversion: the leading one is aligned to bit (MAGNITUDE_WIDTH−1)=30, not bit MAGNITUDE_WIDTH=31
        // - k: not required because the k's of q and s/t cancel out
        - 15 // Desired number format S16.15 
        - TEX_HEADROOM
)
(
    input  wire                                 clk,
    input  wire                                 ce,

    input  wire        [16 - 1 : 0]              color_r,
    input  wire        [16 - 1 : 0]              color_g,
    input  wire        [16 - 1 : 0]              color_b,
    input  wire        [16 - 1 : 0]              color_a,

    input  wire signed [TEXQ_PRECISION - 1 : 0]  tex0_s, // S7+k.(TEXQ_PRECISION - 1 - 7 - k)
    input  wire signed [TEXQ_PRECISION - 1 : 0]  tex0_t,
    input  wire        [TEXQ_PRECISION - 1 : 0]  tex0_q, // U1.(TEXQ_PRECISION - 1)
    input  wire signed [TEXQ_PRECISION - 1 : 0]  tex0_mipmap_s,
    input  wire signed [TEXQ_PRECISION - 1 : 0]  tex0_mipmap_t,
    input  wire        [TEXQ_PRECISION - 1 : 0]  tex0_mipmap_q,
    input  wire signed [TEXQ_PRECISION - 1 : 0]  tex1_s,
    input  wire signed [TEXQ_PRECISION - 1 : 0]  tex1_t,
    input  wire        [TEXQ_PRECISION - 1 : 0]  tex1_q,
    input  wire signed [TEXQ_PRECISION - 1 : 0]  tex1_mipmap_s,
    input  wire signed [TEXQ_PRECISION - 1 : 0]  tex1_mipmap_t,
    input  wire        [TEXQ_PRECISION - 1 : 0]  tex1_mipmap_q,

    output reg         [SUB_PIXEL_WIDTH - 1 : 0] color_r_reg,
    output reg         [SUB_PIXEL_WIDTH - 1 : 0] color_g_reg,
    output reg         [SUB_PIXEL_WIDTH - 1 : 0] color_b_reg,
    output reg         [SUB_PIXEL_WIDTH - 1 : 0] color_a_reg,

    output reg         [TEXQ_PRECISION * 2 : 0]  tex0_s_reg, // S16.15
    output reg         [TEXQ_PRECISION * 2 : 0]  tex0_t_reg,
    output reg         [TEXQ_PRECISION * 2 : 0]  tex0_mipmap_s_reg,
    output reg         [TEXQ_PRECISION * 2 : 0]  tex0_mipmap_t_reg,
    output reg         [TEXQ_PRECISION * 2 : 0]  tex1_s_reg,
    output reg         [TEXQ_PRECISION * 2 : 0]  tex1_t_reg,
    output reg         [TEXQ_PRECISION * 2 : 0]  tex1_mipmap_s_reg,
    output reg         [TEXQ_PRECISION * 2 : 0]  tex1_mipmap_t_reg
);
    always @(posedge clk)
    if (ce) begin
        color_a_reg <= (color_a[15]) ? 0 : (|color_a[SUB_PIXEL_WIDTH +: 7]) ? { SUB_PIXEL_WIDTH { 1'b1 } } : color_a[0 +: SUB_PIXEL_WIDTH];
        color_b_reg <= (color_b[15]) ? 0 : (|color_b[SUB_PIXEL_WIDTH +: 7]) ? { SUB_PIXEL_WIDTH { 1'b1 } } : color_b[0 +: SUB_PIXEL_WIDTH];
        color_g_reg <= (color_g[15]) ? 0 : (|color_g[SUB_PIXEL_WIDTH +: 7]) ? { SUB_PIXEL_WIDTH { 1'b1 } } : color_g[0 +: SUB_PIXEL_WIDTH];
        color_r_reg <= (color_r[15]) ? 0 : (|color_r[SUB_PIXEL_WIDTH +: 7]) ? { SUB_PIXEL_WIDTH { 1'b1 } } : color_r[0 +: SUB_PIXEL_WIDTH];

        tex0_s_reg <= (tex0_s * $signed({ 1'b0, tex0_q })) >>> TEX_PERSP_CORR_SHIFT; // S3.(n-4) * U1.(n-1) >>> (2n - 20) = S16.15
        tex0_t_reg <= (tex0_t * $signed({ 1'b0, tex0_q })) >>> TEX_PERSP_CORR_SHIFT;
        if (ENABLE_LOD_CALC)
        begin
            tex0_mipmap_s_reg <= (tex0_mipmap_s * $signed({ 1'b0, tex0_mipmap_q })) >>> TEX_PERSP_CORR_SHIFT;
            tex0_mipmap_t_reg <= (tex0_mipmap_t * $signed({ 1'b0, tex0_mipmap_q })) >>> TEX_PERSP_CORR_SHIFT;
        end
        else
        begin
            tex0_mipmap_s_reg <= 0;
            tex0_mipmap_t_reg <= 0;
        end

        if (ENABLE_SECOND_TMU)
        begin
            tex1_s_reg <= (tex1_s * $signed({ 1'b0, tex1_q })) >>> TEX_PERSP_CORR_SHIFT;
            tex1_t_reg <= (tex1_t * $signed({ 1'b0, tex1_q })) >>> TEX_PERSP_CORR_SHIFT;
            if (ENABLE_LOD_CALC)
            begin
                tex1_mipmap_s_reg <= (tex1_mipmap_s * $signed({ 1'b0, tex1_mipmap_q })) >>> TEX_PERSP_CORR_SHIFT;
                tex1_mipmap_t_reg <= (tex1_mipmap_t * $signed({ 1'b0, tex1_mipmap_q })) >>> TEX_PERSP_CORR_SHIFT;
            end
            else
            begin
                tex1_mipmap_s_reg <= 0;
                tex1_mipmap_t_reg <= 0;
            end
        end
        else
        begin
            tex1_s_reg <= 0;
            tex1_t_reg <= 0;
            tex1_mipmap_s_reg <= 0;
            tex1_mipmap_t_reg <= 0;
        end
    end
endmodule
