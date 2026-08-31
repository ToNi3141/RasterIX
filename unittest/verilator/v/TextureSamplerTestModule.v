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

// This is a test module for the TextureSampler, TextureBuffer, and TexelColorUnpack.
// It instantiates the texture path to ease testing the sampler with a real texture buffer.
module TextureSamplerTestModule #(
    parameter USER_WIDTH = 1,
    parameter STREAM_WIDTH = 32,
    parameter TEXEL_WIDTH = 16,

    localparam TEX_ADDR_WIDTH = 17,
    localparam PIXEL_WIDTH = 32
) 
(
    input  wire                         aclk,
    input  wire                         resetn,
    
    // Texture size
    // textureSize * 2. 0 equals 1px. 1 equals 2px. 2 equals 4px... Only power of two are allowed.
    input  wire [ 3 : 0]                textureSizeWidth, 
    input  wire [ 3 : 0]                textureSizeHeight,
    input  wire                         enableHalfPixelOffset,
    input  wire [ 3 : 0]                confPixelFormat,

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
    output wire [PIXEL_WIDTH - 1 : 0]   m_texel00, // (0, 0), as (s, t). s and t are switched since the address is constructed like {texelT, texelS}
    output wire [PIXEL_WIDTH - 1 : 0]   m_texel01, // (1, 0)
    output wire [PIXEL_WIDTH - 1 : 0]   m_texel10, // (0, 1)
    output wire [PIXEL_WIDTH - 1 : 0]   m_texel11, // (1, 1)
    output wire [15 : 0]                m_texelSubCoordS, // Q0.16
    output wire [15 : 0]                m_texelSubCoordT, // Q0.16

    // Texture Write
    input  wire                         s_axis_tvalid,
    input  wire                         s_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]  s_axis_tdata
);

    wire [TEX_ADDR_WIDTH - 1 : 0]   texelAddr00;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texelAddr01;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texelAddr10;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texelAddr11;
    wire [TEXEL_WIDTH - 1 : 0]      texelInput00;
    wire [TEXEL_WIDTH - 1 : 0]      texelInput01;
    wire [TEXEL_WIDTH - 1 : 0]      texelInput10;
    wire [TEXEL_WIDTH - 1 : 0]      texelInput11;
    wire [TEXEL_WIDTH - 1 : 0]      sampledTexel00;
    wire [TEXEL_WIDTH - 1 : 0]      sampledTexel01;
    wire [TEXEL_WIDTH - 1 : 0]      sampledTexel10;
    wire [TEXEL_WIDTH - 1 : 0]      sampledTexel11;
    wire                            sampledClampU;
    wire                            sampledClampV;
    wire                            sampledValid;
    wire [USER_WIDTH - 1 : 0]       sampledUser;
    wire [15 : 0]                   sampledSubCoordS;
    wire [15 : 0]                   sampledSubCoordT;
    wire [TEXEL_WIDTH - 1 : 0]      clampedTexel00;
    wire [TEXEL_WIDTH - 1 : 0]      clampedTexel01;
    wire [TEXEL_WIDTH - 1 : 0]      clampedTexel10;
    wire [TEXEL_WIDTH - 1 : 0]      clampedTexel11;

    TextureBuffer #(
        .TEXEL_WIDTH(TEXEL_WIDTH)
    ) texCache (
        .aclk(aclk),
        .resetn(resetn),

        .texelAddr00(texelAddr00),
        .texelAddr01(texelAddr01),
        .texelAddr10(texelAddr10),
        .texelAddr11(texelAddr11),

        .texelOutput00(texelInput00),
        .texelOutput01(texelInput01),
        .texelOutput10(texelInput10),
        .texelOutput11(texelInput11),

        .s_axis_tvalid(s_axis_tvalid),
        .s_axis_tlast(s_axis_tlast),
        .s_axis_tdata(s_axis_tdata)
    );

    TextureSampler #(
        .USER_WIDTH(USER_WIDTH),
        .TEXEL_WIDTH(TEXEL_WIDTH)
    ) textureSampler (
        .aclk(aclk),
        .resetn(resetn),

        .textureSizeWidth(textureSizeWidth),
        .textureSizeHeight(textureSizeHeight),
        .enableHalfPixelOffset(enableHalfPixelOffset),

        .texelAddr00(texelAddr00),
        .texelAddr01(texelAddr01),
        .texelAddr10(texelAddr10),
        .texelAddr11(texelAddr11),
        .texelInput00(texelInput00),
        .texelInput01(texelInput01),
        .texelInput10(texelInput10),
        .texelInput11(texelInput11),

        .s_valid(s_valid),
        .s_ready(s_ready),
        .s_user(s_user),
        .s_texelS(s_texelS),
        .s_texelT(s_texelT),
        .s_clampS(s_clampS),
        .s_clampT(s_clampT),
        .s_textureLod(s_textureLod),
        
        .m_valid(sampledValid),
        .m_ready(m_ready),
        .m_user(sampledUser),
        .m_texel00(sampledTexel00),
        .m_texel01(sampledTexel01),
        .m_texel10(sampledTexel10),
        .m_texel11(sampledTexel11),
        .m_clampU(sampledClampU),
        .m_clampV(sampledClampV),
        .m_texelSubCoordS(sampledSubCoordS),
        .m_texelSubCoordT(sampledSubCoordT)
    );

    TextureClamp #(
        .TEXEL_WIDTH(TEXEL_WIDTH),
        .USER_WIDTH(USER_WIDTH)
    ) textureClamp (
        .aclk(aclk),
        .resetn(resetn),

        .s_valid(sampledValid),
        .s_ready(),
        .s_user(sampledUser),
        .s_texel00(sampledTexel00),
        .s_texel01(sampledTexel01),
        .s_texel10(sampledTexel10),
        .s_texel11(sampledTexel11),
        .s_texelSubCoordS(sampledSubCoordS),
        .s_texelSubCoordT(sampledSubCoordT),
        .s_clampU(sampledClampU),
        .s_clampV(sampledClampV),

        .m_ready(m_ready),
        .m_valid(m_valid),
        .m_user(m_user),
        .m_texel00(clampedTexel00),
        .m_texel01(clampedTexel01),
        .m_texel10(clampedTexel10),
        .m_texel11(clampedTexel11),
        .m_texelSubCoordS(m_texelSubCoordS),
        .m_texelSubCoordT(m_texelSubCoordT)
    );

    TexelColorUnpack #(
        .TEXEL_WIDTH(TEXEL_WIDTH)
    ) texelColorUnpack00 (
        .confPixelFormat(confPixelFormat),
        .texelInput(clampedTexel00),
        .texelOutput(m_texel00)
    );

    TexelColorUnpack #(
        .TEXEL_WIDTH(TEXEL_WIDTH)
    ) texelColorUnpack01 (
        .confPixelFormat(confPixelFormat),
        .texelInput(clampedTexel01),
        .texelOutput(m_texel01)
    );

    TexelColorUnpack #(
        .TEXEL_WIDTH(TEXEL_WIDTH)
    ) texelColorUnpack10 (
        .confPixelFormat(confPixelFormat),
        .texelInput(clampedTexel10),
        .texelOutput(m_texel10)
    );

    TexelColorUnpack #(
        .TEXEL_WIDTH(TEXEL_WIDTH)
    ) texelColorUnpack11 (
        .confPixelFormat(confPixelFormat),
        .texelInput(clampedTexel11),
        .texelOutput(m_texel11)
    );

endmodule