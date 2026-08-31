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

// Gets texel coordinates and then samples a texel quad from the texture memory.
// It assumes that a memory with a fixed delay of one clock is used.
// It supports texture sizes from 1x1 to 256x256
// Delay: 5 clocks
module TextureSampler #(
    parameter TEXEL_WIDTH = 16,
    parameter USER_WIDTH = 1,
    localparam ADDR_WIDTH = 17 // Based on the maximum texture size, of 256x256 (8 bit x 8 bit) + mipmap levels in texture word addresses
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    // Texture size
    // textureSize * 2. 0 equals 1px. 1 equals 2px. 2 equals 4px... Only power of two are allowed.
    input  wire [ 3 : 0]                textureSizeWidth, 
    input  wire [ 3 : 0]                textureSizeHeight,
    input  wire                         enableHalfPixelOffset,

    // Texture memory access of a texel quad
    output reg  [ADDR_WIDTH - 1 : 0]    texelAddr00,
    output reg  [ADDR_WIDTH - 1 : 0]    texelAddr01,
    output reg  [ADDR_WIDTH - 1 : 0]    texelAddr10,
    output reg  [ADDR_WIDTH - 1 : 0]    texelAddr11,
    input  wire [TEXEL_WIDTH - 1 : 0]   texelInput00,
    input  wire [TEXEL_WIDTH - 1 : 0]   texelInput01,
    input  wire [TEXEL_WIDTH - 1 : 0]   texelInput10,
    input  wire [TEXEL_WIDTH - 1 : 0]   texelInput11,

    // Texture Read
    input  wire                         s_valid,
    output wire                         s_ready,
    input  wire [USER_WIDTH - 1 : 0]    s_user,
    input  wire [31 : 0]                s_texelS, // S16.15
    input  wire [31 : 0]                s_texelT, // S16.15
    input  wire                         s_clampS,
    input  wire                         s_clampT,
    input  wire [ 3 : 0]                s_textureLod,

    output wire                         m_valid,
    input  wire                         m_ready,
    output wire [USER_WIDTH - 1 : 0]    m_user,
    output wire [TEXEL_WIDTH - 1 : 0]   m_texel00, // (0, 0), as (s, t). s and t are switched since the address is constructed like {texelT, texelS}
    output wire [TEXEL_WIDTH - 1 : 0]   m_texel01, // (1, 0)
    output wire [TEXEL_WIDTH - 1 : 0]   m_texel10, // (0, 1)
    output wire [TEXEL_WIDTH - 1 : 0]   m_texel11, // (1, 1)
    output wire                         m_clampU,
    output wire                         m_clampV,
    // This is basically the faction of te pixel coordinate and has a range from 0.0 (0x0) to 0.999... (0xffff)
    // The integer part is not required, since the integer part only addresses the pixel and we don't care about that.
    // We just care about the coordinates within the texel quad. And if there the coordinate gets >1.0, that means, we
    // are outside of our quad which never happens.
    output wire [15 : 0]                m_texelSubCoordS, // Q0.16
    output wire [15 : 0]                m_texelSubCoordT // Q0.16
);
`include "RegisterAndDescriptorDefines.vh"

    function [15 : 0] clampTexture;
        input [31 : 0] texCoord;
        input [ 0 : 0] mode; 
        begin
            clampTexture = texCoord[0 +: 16];
            if (mode == CLAMP_TO_EDGE)
            begin
                if (texCoord[31]) // Check if it lower than 0 by only checking the sign bit
                begin
                    clampTexture = 0;
                end
                else if ((texCoord >> 15) != 0) // Check if it is greater than one by checking if the integer part is unequal to zero
                begin
                    clampTexture = 16'h7fff;
                end  
            end
        end
    endfunction

    // Convert the configured texture size into a 0.5.
    // The texture size comes as a integer value which is intepreted as a normalized 1.0.
    // Assuming a texture size of 4px (16'h100). To normalize this, the point (imaginary) must be moved to the diget bevore the 1.
    // But a 1.0 is different on a 4px texture and on a texture on another size like 16px (16'h10000). Therefore also the 
    // representation of 0.5 changes from 16'h010 to 16'h01000. This is what this function is intended for, to get a normalized
    // 0.5 depending on the size of the texture.
    function [31 : 0] convertToZeroPointFive;
        input [7 : 0] texSize;
        convertToZeroPointFive = 
        {
            1'h0, // sign
            16'h0, // integer part
            1'h0,
            texSize[0],
            texSize[1],
            texSize[2],
            texSize[3],
            texSize[4],
            texSize[5],
            texSize[6],
            texSize[7],
            6'h0
        };
    endfunction

    // See convertToZeroPointFive. It is basically the same just for 1.0
    function [31 : 0] convertToOnePointZero;
        input [7 : 0] texSize;
        convertToOnePointZero = 
        {
            1'h0, // sign
            16'h0, // integer part
            texSize[0],
            texSize[1],
            texSize[2],
            texSize[3],
            texSize[4],
            texSize[5],
            texSize[6],
            texSize[7],
            7'h0
        };
    endfunction

    wire ce;
    assign ce = m_ready;
    assign s_ready = m_ready;

    //////////////////////////////////////////////
    // STEP 0
    // Calculate the offset of the texture address based on the LOD
    // Clocks: 1
    //////////////////////////////////////////////
    localparam TEX_SIZE = 8;
    localparam TEX_SIZE_ST = TEX_SIZE * 2;
    localparam TEX_MASK_SIZE = TEX_SIZE + 1;
    localparam TEX_MASK_SIZE_ST = TEX_SIZE_ST + 2;
    localparam TEX_MASK_ONE = { { TEX_SIZE { 1'b0 } }, 1'b1 };
    localparam TEX_MASK_ST_ONE = { { TEX_SIZE_ST { 1'b0 } }, 2'b1 };
    reg [TEX_MASK_SIZE_ST - 1 : 0]  step0_offset;
    reg [ 3 : 0]                    step0_width;
    reg [ 3 : 0]                    step0_height;
    reg [ 3 : 0]                    step0_widthShift;
    reg [ 3 : 0]                    step0_heightShift;
    reg [ 7 : 0]                    step0_widthPow2minus1;
    reg [ 7 : 0]                    step0_heightPow2minus1;
    reg [31 : 0]                    step0_texelS; // S16.15
    reg [31 : 0]                    step0_texelT; // S16.15
    reg                             step0_clampS;
    reg                             step0_clampT;
    reg                             step0_valid;
    reg [USER_WIDTH - 1 : 0]        step0_user;

    always @(posedge aclk)
    if (ce) begin : LodOffsetCalc
        reg [ 3 : 0]                    width;
        reg [ 3 : 0]                    height;
        
        reg [TEX_MASK_SIZE - 1 : 0]     wMask;
        reg [TEX_MASK_SIZE - 1 : 0]     hMask;
        reg [TEX_MASK_SIZE_ST - 1 : 0]  lodMask;
        reg [TEX_MASK_SIZE_ST - 1 : 0]  mask;
        reg [TEX_MASK_SIZE_ST - 1 : 0]  maskFin;
        integer i;

        width = (s_textureLod < textureSizeWidth) ? textureSizeWidth - s_textureLod : 0;
        height = (s_textureLod < textureSizeHeight) ? textureSizeHeight - s_textureLod : 0;
        lodMask = ~((TEX_MASK_ST_ONE << ((width + height + 1))) - TEX_MASK_ST_ONE);

        wMask = ((TEX_MASK_ONE << textureSizeWidth) - TEX_MASK_ONE);
        hMask = ((TEX_MASK_ONE << textureSizeHeight) - TEX_MASK_ONE);
        for (i = 0; i < TEX_MASK_SIZE; i = i + 1)
        begin
            mask[(TEX_MASK_SIZE_ST - 1) - ((i * 2) + 1)] = wMask[i] ^ hMask[i];
            mask[(TEX_MASK_SIZE_ST - 1) - ((i * 2) + 0)] = wMask[i] | hMask[i];
        end
        maskFin = mask >> (TEX_MASK_SIZE_ST - (textureSizeWidth + textureSizeHeight + 1));
        
        step0_offset <= maskFin & lodMask;
        step0_width <= width;
        step0_height <= height;
        step0_widthShift <= 8 - width;
        step0_heightShift <= 8 - height;
        step0_widthPow2minus1 <= 1 << (width - 1);
        step0_heightPow2minus1 <= 1 << (height - 1);
        step0_texelS <= s_texelS;
        step0_texelT <= s_texelT;
        step0_clampS <= s_clampS;
        step0_clampT <= s_clampT;
        step0_valid <= s_valid;
        step0_user <= s_user;
    end

    //////////////////////////////////////////////
    // STEP 1
    // Build RAM addresses
    // Clocks: 1
    //////////////////////////////////////////////
    reg                         step1_clampS;
    reg                         step1_clampT;
    reg  [15 : 0]               step1_texelU0; // Q1.15
    reg  [15 : 0]               step1_texelU1; // Q1.15
    reg  [15 : 0]               step1_texelV0; // Q1.15
    reg  [15 : 0]               step1_texelV1; // Q1.15
    reg  [15 : 0]               step1_subCoordU; // Q0.16
    reg  [15 : 0]               step1_subCoordV; // Q0.16
    reg                         step1_valid;
    reg  [USER_WIDTH - 1 : 0]   step1_user;
    
    always @(posedge aclk)
    if (ce) begin : TexAddrCalc
        reg [31 : 0] texelS0; // S16.15
        reg [31 : 0] texelS1; // S16.15
        reg [31 : 0] texelT0; // S16.15
        reg [31 : 0] texelT1; // S16.15

        step1_clampS <= step0_clampS;
        step1_clampT <= step0_clampT;

        if (enableHalfPixelOffset)
        begin
            texelS0 = step0_texelS - convertToZeroPointFive(step0_widthPow2minus1);
            texelS1 = step0_texelS + convertToZeroPointFive(step0_widthPow2minus1);
            texelT0 = step0_texelT - convertToZeroPointFive(step0_heightPow2minus1);
            texelT1 = step0_texelT + convertToZeroPointFive(step0_heightPow2minus1);
        end
        else
        begin
            texelS0 = step0_texelS;
            texelS1 = step0_texelS + convertToOnePointZero(step0_widthPow2minus1);
            texelT0 = step0_texelT;
            texelT1 = step0_texelT + convertToOnePointZero(step0_heightPow2minus1);
        end

        step1_texelU0 = clampTexture(texelS0, step0_clampS);
        step1_texelU1 = clampTexture(texelS1, step0_clampS);
        step1_texelV0 = clampTexture(texelT0, step0_clampT);
        step1_texelV1 = clampTexture(texelT1, step0_clampT);

        texelAddr00 <= step0_offset[0 +: 17] + (({ 9'h0, step1_texelV0[7 +: 8] >> step0_heightShift } << step0_width) | { 9'h0, step1_texelU0[7 +: 8] >> step0_widthShift });
        texelAddr01 <= step0_offset[0 +: 17] + (({ 9'h0, step1_texelV0[7 +: 8] >> step0_heightShift } << step0_width) | { 9'h0, step1_texelU1[7 +: 8] >> step0_widthShift });
        texelAddr10 <= step0_offset[0 +: 17] + (({ 9'h0, step1_texelV1[7 +: 8] >> step0_heightShift } << step0_width) | { 9'h0, step1_texelU0[7 +: 8] >> step0_widthShift });
        texelAddr11 <= step0_offset[0 +: 17] + (({ 9'h0, step1_texelV1[7 +: 8] >> step0_heightShift } << step0_width) | { 9'h0, step1_texelU1[7 +: 8] >> step0_widthShift });
        step1_subCoordU <= { texelS0[0 +: 15], 1'b0 } << step0_width;
        step1_subCoordV <= { texelT0[0 +: 15], 1'b0 } << step0_height;

        step1_valid <= step0_valid;
        step1_user <= step0_user;
    end

    //////////////////////////////////////////////
    // STEP 2
    // Wait for data
    // Clocks: 2
    //////////////////////////////////////////////
    wire                        step2_clampU;
    wire                        step2_clampV;
    wire [15 : 0]               step2_subCoordU; // Q0.16
    wire [15 : 0]               step2_subCoordV; // Q0.16
    reg  [TEXEL_WIDTH - 1 : 0]  step2_texel00;
    reg  [TEXEL_WIDTH - 1 : 0]  step2_texel01;
    reg  [TEXEL_WIDTH - 1 : 0]  step2_texel10;
    reg  [TEXEL_WIDTH - 1 : 0]  step2_texel11;
    wire                        step2_valid;
    wire [USER_WIDTH - 1 : 0]   step2_user;

    // Check if we have to clamp
    // Check if the texel coordinate is smaller than texel+1. If so, we have an overflow and we have to clamp.
    // OR, since the texel coordinate is a Q1.15 number, we need a dedicated check for the integer part. Could be,
    // that just the fraction part overflows but not the whole variable. Therefore also check for it by checking the
    // most significant bit.
    wire step2_clampUTmp = step1_clampS && ((step1_texelU0 > step1_texelU1) || (!step1_texelU0[15] && step1_texelU1[15]));
    wire step2_clampVTmp = step1_clampT && ((step1_texelV0 > step1_texelV1) || (!step1_texelV0[15] && step1_texelV1[15]));

    ValueDelay #( 
        .VALUE_SIZE(16 + 16 + 1 + 1 + 1 + USER_WIDTH),
        .DELAY(2)
    ) step2_delay (
        .clk(aclk), 
        .ce(ce), 
        .in({
            step1_subCoordU,
            step1_subCoordV,
            step2_clampUTmp,
            step2_clampVTmp,
            step1_valid,
            step1_user
        }), 
        .out({
            step2_subCoordU,
            step2_subCoordV,
            step2_clampU,
            step2_clampV,
            step2_valid,
            step2_user
        })
    );

    // The texture memory has one clock delay and the texture buffer has no flow control.
    // As soon as the destination requires a stall, a pixel from the TextureBuffer must be skidded here.
    reg                         step2_skid = 0;
    reg  [TEXEL_WIDTH - 1 : 0]  step2_skid_texel00;
    reg  [TEXEL_WIDTH - 1 : 0]  step2_skid_texel01;
    reg  [TEXEL_WIDTH - 1 : 0]  step2_skid_texel10;
    reg  [TEXEL_WIDTH - 1 : 0]  step2_skid_texel11;
    always @(posedge aclk)
    begin
        if (ce)
        begin
            if (step2_skid)
            begin
                step2_skid <= 0;
                step2_texel00 <= step2_skid_texel00;
                step2_texel01 <= step2_skid_texel01;
                step2_texel10 <= step2_skid_texel10;
                step2_texel11 <= step2_skid_texel11;
            end
            else
            begin
                step2_texel00 <= texelInput00;
                step2_texel01 <= texelInput01;
                step2_texel10 <= texelInput10;
                step2_texel11 <= texelInput11;
            end
        end
        else
        begin
            if (!step2_skid)
            begin
                step2_skid <= 1;
                step2_skid_texel00 <= texelInput00;
                step2_skid_texel01 <= texelInput01;
                step2_skid_texel10 <= texelInput10;
                step2_skid_texel11 <= texelInput11;
            end
        end
    end

    //////////////////////////////////////////////
    // STEP 3
    // Output
    // Clocks: 0
    //////////////////////////////////////////////
    assign m_texel00 = step2_texel00;
    assign m_texel01 = step2_texel01;
    assign m_texel10 = step2_texel10;
    assign m_texel11 = step2_texel11;
    assign m_clampU = step2_clampU;
    assign m_clampV = step2_clampV;

    assign m_texelSubCoordS = step2_subCoordU;
    assign m_texelSubCoordT = step2_subCoordV;

    assign m_valid = step2_valid;
    assign m_user = step2_user;

endmodule 