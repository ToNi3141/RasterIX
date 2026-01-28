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

#include "general.hpp"
#include <algorithm>
#include <array>
#include <math.h>

// Include model header, generated from Verilating "top.v"
#include "VTextureSamplerTestModule.h"

void uploadTexture(VTextureSamplerTestModule* top)
{
    // 2x2 texture
    // | 0xf000 | 0x0f00 |
    // | 0x00f0 | 0x000f |
    top->textureSizeWidth = 0x1;
    top->textureSizeHeight = 0x1;

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 0;
    top->s_axis_tdata = 0x0f00'f000;
    rr::ut::clk(top);

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 1;
    top->s_axis_tdata = 0x000f'00f0;
    rr::ut::clk(top);

    top->s_axis_tvalid = 0;
    top->s_axis_tlast = 0;

    top->s_clampS = 0;
    top->s_clampT = 0;
}

TEST_CASE("Get various values from the texture buffer", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    rr::ut::reset(top);
    top->m_ready = 1;

    uploadTexture(top);

    // (0, 0)
    top->s_texelS = 0;
    top->s_texelT = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0xff000000);
    REQUIRE(top->m_texel01 == 0x00ff0000);
    REQUIRE(top->m_texel10 == 0x0000ff00);
    REQUIRE(top->m_texel11 == 0x000000ff);

    // (0.99.., 0.0)
    top->s_texelS = 0x7fff;
    top->s_texelT = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x00ff0000);
    REQUIRE(top->m_texel01 == 0xff000000);
    REQUIRE(top->m_texel10 == 0x000000ff);
    REQUIRE(top->m_texel11 == 0x0000ff00);

    // (0.0, 0.99..)
    top->s_texelS = 0;
    top->s_texelT = 0x7fff;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x0000ff00);
    REQUIRE(top->m_texel01 == 0x000000ff);
    REQUIRE(top->m_texel10 == 0xff000000);
    REQUIRE(top->m_texel11 == 0x00ff0000);

    // (0.99.., 0.99..)
    top->s_texelS = 0x7fff;
    top->s_texelT = 0x7fff;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x000000ff);
    REQUIRE(top->m_texel01 == 0x0000ff00);
    REQUIRE(top->m_texel10 == 0x00ff0000);
    REQUIRE(top->m_texel11 == 0xff000000);

    // (1.0, 0.0)
    top->s_texelS = 0x8000;
    top->s_texelT = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0xff000000);
    REQUIRE(top->m_texel01 == 0x00ff0000);
    REQUIRE(top->m_texel10 == 0x0000ff00);
    REQUIRE(top->m_texel11 == 0x000000ff);

    // (0.0, 1.0)
    top->s_texelS = 0;
    top->s_texelT = 0x8000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0xff000000);
    REQUIRE(top->m_texel01 == 0x00ff0000);
    REQUIRE(top->m_texel10 == 0x0000ff00);
    REQUIRE(top->m_texel11 == 0x000000ff);

    // (1.0, 1.0)
    top->s_texelS = 0x8000;
    top->s_texelT = 0x8000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0xff000000);
    REQUIRE(top->m_texel01 == 0x00ff0000);
    REQUIRE(top->m_texel10 == 0x0000ff00);
    REQUIRE(top->m_texel11 == 0x000000ff);

    // Destroy model
    delete top;
}

TEST_CASE("Get various values from the texture buffer with pipeline test", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    rr::ut::reset(top);
    top->m_ready = 1;

    uploadTexture(top);

    // (0, 0)
    top->s_texelS = 0;
    top->s_texelT = 0;
    rr::ut::clk(top);

    // (0.99.., 0.0)
    top->s_texelS = 0x7fff;
    top->s_texelT = 0;
    rr::ut::clk(top);

    // (0.0, 0.99..)
    top->s_texelS = 0;
    top->s_texelT = 0x7fff;
    rr::ut::clk(top);

    // (0.99.., 0.99..)
    top->s_texelS = 0x7fff;
    top->s_texelT = 0x7fff;
    rr::ut::clk(top);

    // Result of (0, 0)
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0xff000000);
    REQUIRE(top->m_texel01 == 0x00ff0000);
    REQUIRE(top->m_texel10 == 0x0000ff00);
    REQUIRE(top->m_texel11 == 0x000000ff);

    // (0.99.., 0.0)
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x00ff0000);
    REQUIRE(top->m_texel01 == 0xff000000);
    REQUIRE(top->m_texel10 == 0x000000ff);
    REQUIRE(top->m_texel11 == 0x0000ff00);

    // Result of (0.0, 0.99..)
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x0000ff00);
    REQUIRE(top->m_texel01 == 0x000000ff);
    REQUIRE(top->m_texel10 == 0xff000000);
    REQUIRE(top->m_texel11 == 0x00ff0000);

    // Result of (0.99.., 0.99..)
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x000000ff);
    REQUIRE(top->m_texel01 == 0x0000ff00);
    REQUIRE(top->m_texel10 == 0x00ff0000);
    REQUIRE(top->m_texel11 == 0xff000000);

    // Destroy model
    delete top;
}

TEST_CASE("Check sub coordinates", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    rr::ut::reset(top);
    top->m_ready = 1;

    uploadTexture(top);

    // texel (0.0, 0.0)
    top->s_texelS = 0x0000;
    top->s_texelT = 0x0000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0xff000000);
    REQUIRE(top->m_texel01 == 0x00ff0000);
    REQUIRE(top->m_texel10 == 0x0000ff00);
    REQUIRE(top->m_texel11 == 0x000000ff);
    // sub texel (0.0, 0.0)
    REQUIRE(top->m_texelSubCoordS == 0x0);
    REQUIRE(top->m_texelSubCoordT == 0x0);

    // To get the sub coordinate of (0.25, 0.0), we have to imagine the following things:
    // In a 2x2 texture, the coordinate (0.0, 0.0) accesses texel (0, 0). The coordinate (0.5, 0.0) accesses (1, 0).
    // Means, the distance between two texels is 0.5. If we want now the sub texel coordinate (0.25, 0.0) we have
    // now to multiply the vector with the texel distance of 0.5 which results in (0.125, 0.0). If we want to convert this
    // to access the texel (1, 0), we would have to add 0.5 to the x value which results in (0.625, 0.0).

    // texel (0.125, 0.125)
    top->s_texelS = 0x1000;
    top->s_texelT = 0x1000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0xff000000);
    REQUIRE(top->m_texel01 == 0x00ff0000);
    REQUIRE(top->m_texel10 == 0x0000ff00);
    REQUIRE(top->m_texel11 == 0x000000ff);
    // sub texel (0.25, 0.25)
    REQUIRE(top->m_texelSubCoordS == 0x4000);
    REQUIRE(top->m_texelSubCoordT == 0x4000);

    // texel (0.125, 0.375)
    top->s_texelS = 0x1000;
    top->s_texelT = 0x3000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0xff000000);
    REQUIRE(top->m_texel01 == 0x00ff0000);
    REQUIRE(top->m_texel10 == 0x0000ff00);
    REQUIRE(top->m_texel11 == 0x000000ff);
    // sub texel (0.25, 0.75)
    REQUIRE(top->m_texelSubCoordS == 0x4000);
    REQUIRE(top->m_texelSubCoordT == 0xc000);

    // texel (0.375, 0.125)
    top->s_texelS = 0x3000;
    top->s_texelT = 0x1000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0xff000000);
    REQUIRE(top->m_texel01 == 0x00ff0000);
    REQUIRE(top->m_texel10 == 0x0000ff00);
    REQUIRE(top->m_texel11 == 0x000000ff);
    // sub texel (0.75, 0.25)
    REQUIRE(top->m_texelSubCoordS == 0xc000);
    REQUIRE(top->m_texelSubCoordT == 0x4000);

    // Destroy model
    delete top;
}

TEST_CASE("clamp to border with s", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    rr::ut::reset(top);
    top->m_ready = 1;

    uploadTexture(top);

    top->s_clampS = 1;
    top->s_clampT = 0;

    // texel (0.0, 0.0)
    top->s_texelS = 0x0000;
    top->s_texelT = 0x0000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0xff000000);
    REQUIRE(top->m_texel01 == 0x00ff0000);
    REQUIRE(top->m_texel10 == 0x0000ff00);
    REQUIRE(top->m_texel11 == 0x000000ff);

    // texel (0.5, 0.0)
    top->s_texelS = 0x4000;
    top->s_texelT = 0x0000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x00ff0000);
    REQUIRE(top->m_texel01 == 0x00ff0000);
    REQUIRE(top->m_texel10 == 0x000000ff);
    REQUIRE(top->m_texel11 == 0x000000ff);

    // texel (0.5, 0.5)
    top->s_texelS = 0x4000;
    top->s_texelT = 0x4000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x000000ff);
    REQUIRE(top->m_texel01 == 0x000000ff);
    REQUIRE(top->m_texel10 == 0x00ff0000);
    REQUIRE(top->m_texel11 == 0x00ff0000);

    // Destroy model
    delete top;
}

TEST_CASE("clamp to border with t", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    rr::ut::reset(top);
    top->m_ready = 1;

    uploadTexture(top);

    top->s_clampS = 0;
    top->s_clampT = 1;

    // texel (0.0, 0.0)
    top->s_texelS = 0x0000;
    top->s_texelT = 0x0000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0xff000000);
    REQUIRE(top->m_texel01 == 0x00ff0000);
    REQUIRE(top->m_texel10 == 0x0000ff00);
    REQUIRE(top->m_texel11 == 0x000000ff);

    // texel (0.0, 0.5)
    top->s_texelS = 0x0000;
    top->s_texelT = 0x4000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x0000ff00);
    REQUIRE(top->m_texel01 == 0x000000ff);
    REQUIRE(top->m_texel10 == 0x0000ff00);
    REQUIRE(top->m_texel11 == 0x000000ff);

    // texel (0.5, 0.5)
    top->s_texelS = 0x4000;
    top->s_texelT = 0x4000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x000000ff);
    REQUIRE(top->m_texel01 == 0x0000ff00);
    REQUIRE(top->m_texel10 == 0x000000ff);
    REQUIRE(top->m_texel11 == 0x0000ff00);

    // Destroy model
    delete top;
}

TEST_CASE("clamp to border with s and t", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    rr::ut::reset(top);
    top->m_ready = 1;

    uploadTexture(top);

    top->s_clampS = 1;
    top->s_clampT = 1;

    // texel (0.0, 0.0)
    top->s_texelS = 0x0000;
    top->s_texelT = 0x0000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0xff000000);
    REQUIRE(top->m_texel01 == 0x00ff0000);
    REQUIRE(top->m_texel10 == 0x0000ff00);
    REQUIRE(top->m_texel11 == 0x000000ff);

    // texel (0.5, 0.0)
    top->s_texelS = 0x4000;
    top->s_texelT = 0x0000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x00ff0000);
    REQUIRE(top->m_texel01 == 0x00ff0000);
    REQUIRE(top->m_texel10 == 0x000000ff);
    REQUIRE(top->m_texel11 == 0x000000ff);

    // texel (0.0, 0.5)
    top->s_texelS = 0x0000;
    top->s_texelT = 0x4000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x0000ff00);
    REQUIRE(top->m_texel01 == 0x000000ff);
    REQUIRE(top->m_texel10 == 0x0000ff00);
    REQUIRE(top->m_texel11 == 0x000000ff);

    // texel (0.5, 0.5)
    top->s_texelS = 0x4000;
    top->s_texelT = 0x4000;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x000000ff);
    REQUIRE(top->m_texel01 == 0x000000ff);
    REQUIRE(top->m_texel10 == 0x000000ff);
    REQUIRE(top->m_texel11 == 0x000000ff);

    // Destroy model
    delete top;
}

void uploadMipMap4x8Texture(VTextureSamplerTestModule* top)
{
    // 4x8 texture
    // |   1 |   2 |   3 |   4 |
    // |   5 |   6 |   7 |   8 |
    // |   9 |   A |   B |   C |
    // |   D |   E |   F |  10 |
    // |  11 |  12 |  13 |  14 |
    // |  15 |  16 |  17 |  18 |
    // |  19 |  1A |  1B |  1C |
    // |  1D |  1E |  1F |  20 |
    // 2x4 texture
    // |  31 |  32 |
    // |  33 |  34 |
    // |  35 |  36 |
    // |  37 |  38 |
    // 1x2 texture
    // |  41 |
    // |  42 |
    // 1x1 texture
    // |  51 |
    top->textureSizeWidth = 0x2;
    top->textureSizeHeight = 0x3;
    top->s_textureLod = 0;

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 0;
    top->s_axis_tdata = 0x0002'0001;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0004'0003;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0006'0005;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0008'0007;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x000A'0009;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x000C'000B;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x000E'000D;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0010'000F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0012'0011;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0014'0013;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0016'0015;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0018'0017;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x001A'0019;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x001C'001B;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x001E'001D;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);

    top->s_axis_tdata = 0x0032'0031;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0034'0033;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0036'0035;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0038'0037;
    rr::ut::clk(top);

    top->s_axis_tdata = 0x0042'0041;
    rr::ut::clk(top);

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 1;
    top->s_axis_tdata = 0x0000'0051;
    rr::ut::clk(top);

    top->s_axis_tvalid = 0;
    top->s_axis_tlast = 0;

    top->s_clampS = 0;
    top->s_clampT = 0;
}

TEST_CASE("Get various values from the mipmap 4x8 texture", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    rr::ut::reset(top);
    top->m_ready = 1;

    uploadMipMap4x8Texture(top);

    // (0, 0)
    top->s_textureLod = 0;
    top->s_texelS = 0;
    top->s_texelT = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x11);
    REQUIRE(top->m_texel01 == 0x22);
    REQUIRE(top->m_texel10 == 0x55);
    REQUIRE(top->m_texel11 == 0x66);

    // (0, 0)
    top->s_textureLod = 1;
    top->s_texelS = 0;
    top->s_texelT = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x3311);
    REQUIRE(top->m_texel01 == 0x3322);
    REQUIRE(top->m_texel10 == 0x3333);
    REQUIRE(top->m_texel11 == 0x3344);

    // (0, 0)
    top->s_textureLod = 2;
    top->s_texelS = 0;
    top->s_texelT = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x4411);
    REQUIRE(top->m_texel01 == 0x4411);
    REQUIRE(top->m_texel10 == 0x4422);
    REQUIRE(top->m_texel11 == 0x4422);

    // (0, 0)
    top->s_textureLod = 3;
    top->s_texelS = 0;
    top->s_texelT = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x5511);
    REQUIRE(top->m_texel01 == 0x5511);
    REQUIRE(top->m_texel10 == 0x5511);
    REQUIRE(top->m_texel11 == 0x5511);

    // Destroy model
    delete top;
}

void uploadMipMap4x4Texture(VTextureSamplerTestModule* top)
{
    // 4x4 texture
    // |   1 |   2 |   3 |   4 |
    // |   5 |   6 |   7 |   8 |
    // |   9 |   A |   B |   C |
    // |   D |   E |   F |  10 |
    // 2x2 texture
    // |  31 |  32 |
    // |  33 |  34 |
    // 1x1 texture
    // |  51 |
    top->textureSizeWidth = 0x2;
    top->textureSizeHeight = 0x2;
    top->s_textureLod = 0;

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 0;
    top->s_axis_tdata = 0x0002'0001;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0004'0003;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0006'0005;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0008'0007;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x000A'0009;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x000C'000B;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x000E'000D;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0010'000F;
    rr::ut::clk(top);

    top->s_axis_tdata = 0x0032'0031;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0034'0033;
    rr::ut::clk(top);

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 1;
    top->s_axis_tdata = 0x0000'0051;
    rr::ut::clk(top);

    top->s_axis_tvalid = 0;
    top->s_axis_tlast = 0;

    top->s_clampS = 0;
    top->s_clampT = 0;
}

TEST_CASE("Get various values from the mipmap 4x4 texture", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    rr::ut::reset(top);
    top->m_ready = 1;

    uploadMipMap4x4Texture(top);

    // (0, 0)
    top->s_textureLod = 0;
    top->s_texelS = 0;
    top->s_texelT = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x11);
    REQUIRE(top->m_texel01 == 0x22);
    REQUIRE(top->m_texel10 == 0x55);
    REQUIRE(top->m_texel11 == 0x66);

    // (0, 0)
    top->s_textureLod = 1;
    top->s_texelS = 0;
    top->s_texelT = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x3311);
    REQUIRE(top->m_texel01 == 0x3322);
    REQUIRE(top->m_texel10 == 0x3333);
    REQUIRE(top->m_texel11 == 0x3344);

    // (0, 0)
    top->s_textureLod = 2;
    top->s_texelS = 0;
    top->s_texelT = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x5511);
    REQUIRE(top->m_texel01 == 0x5511);
    REQUIRE(top->m_texel10 == 0x5511);
    REQUIRE(top->m_texel11 == 0x5511);

    // Destroy model
    delete top;
}

void uploadMipMap4x16Texture(VTextureSamplerTestModule* top)
{
    // 4x16 texture
    // |   1 |   2 |   3 |   4 |
    // |   5 |   6 |   7 |   8 |
    // |   9 |   A |   B |   C |
    // |   D |   E |   F |  10 |
    // |  11 |  12 |  13 |  14 |
    // |  15 |  16 |  17 |  18 |
    // |  19 |  1A |  1B |  1C |
    // |  1D |  1E |  1F |  20 |
    // |  1D |  1E |  1F |  20 |
    // |  1D |  1E |  1F |  20 |
    // |  1D |  1E |  1F |  20 |
    // |  1D |  1E |  1F |  20 |
    // |  1D |  1E |  1F |  20 |
    // |  1D |  1E |  1F |  20 |
    // |  1D |  1E |  1F |  20 |
    // |  1D |  1E |  1F |  20 |
    // 2x8 texture
    // |  31 |  32 |
    // |  33 |  34 |
    // |  35 |  36 |
    // |  37 |  38 |
    // |  3A |  39 |
    // |  3A |  39 |
    // |  3A |  39 |
    // |  3A |  39 |
    // 1x4 texture
    // |  41 |
    // |  42 |
    // |  43 |
    // |  44 |
    // 1x2 texture
    // |  51 |
    // |  52 |
    // 1x1 texture
    // |  61 |
    top->textureSizeWidth = 0x2;
    top->textureSizeHeight = 0x4;
    top->s_textureLod = 0;

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 0;
    top->s_axis_tdata = 0x0002'0001;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0004'0003;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0006'0005;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0008'0007;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x000A'0009;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x000C'000B;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x000E'000D;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0010'000F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0012'0011;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0014'0013;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0016'0015;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0018'0017;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x001A'0019;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x001C'001B;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x001E'001D;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0020'001F;
    rr::ut::clk(top);

    top->s_axis_tdata = 0x0032'0031;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0034'0033;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0036'0035;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0038'0037;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x003A'0039;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x003A'0039;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x003A'0039;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x003A'0039;
    rr::ut::clk(top);

    top->s_axis_tdata = 0x0042'0041;
    rr::ut::clk(top);
    top->s_axis_tdata = 0x0044'0043;
    rr::ut::clk(top);

    top->s_axis_tdata = 0x0052'0051;
    rr::ut::clk(top);

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 1;
    top->s_axis_tdata = 0x0000'0061;
    rr::ut::clk(top);

    top->s_axis_tvalid = 0;
    top->s_axis_tlast = 0;

    top->s_clampS = 0;
    top->s_clampT = 0;
}

TEST_CASE("Get various values from the mipmap 4x16 texture", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    rr::ut::reset(top);
    top->m_ready = 1;

    uploadMipMap4x16Texture(top);

    // (0, 0)
    top->s_textureLod = 0;
    top->s_texelS = 0;
    top->s_texelT = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x11);
    REQUIRE(top->m_texel01 == 0x22);
    REQUIRE(top->m_texel10 == 0x55);
    REQUIRE(top->m_texel11 == 0x66);

    // (0, 0)
    top->s_textureLod = 1;
    top->s_texelS = 0;
    top->s_texelT = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x3311);
    REQUIRE(top->m_texel01 == 0x3322);
    REQUIRE(top->m_texel10 == 0x3333);
    REQUIRE(top->m_texel11 == 0x3344);

    // (0, 0)
    top->s_textureLod = 2;
    top->s_texelS = 0;
    top->s_texelT = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x4411);
    REQUIRE(top->m_texel01 == 0x4411);
    REQUIRE(top->m_texel10 == 0x4422);
    REQUIRE(top->m_texel11 == 0x4422);

    // (0, 0)
    top->s_textureLod = 3;
    top->s_texelS = 0;
    top->s_texelT = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x5511);
    REQUIRE(top->m_texel01 == 0x5511);
    REQUIRE(top->m_texel10 == 0x5522);
    REQUIRE(top->m_texel11 == 0x5522);

    // (0, 0)
    top->s_textureLod = 4;
    top->s_texelS = 0;
    top->s_texelT = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x6611);
    REQUIRE(top->m_texel01 == 0x6611);
    REQUIRE(top->m_texel10 == 0x6611);
    REQUIRE(top->m_texel11 == 0x6611);

    // Destroy model
    delete top;
}

TEST_CASE("Check stall", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    rr::ut::reset(top);
    top->m_ready = 1;

    uploadTexture(top);

    // (0.999, 0.999)
    top->s_texelS = 0x7fff;
    top->s_texelT = 0x7fff;
    top->s_user = 1;
    top->s_valid = 1;
    rr::ut::clk(top);
    REQUIRE(top->s_ready == 1);

    // (1.0, 1.0)
    top->s_texelS = 0x8000;
    top->s_texelT = 0x8000;
    top->s_user = 0;
    top->s_valid = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);
    top->m_ready = 0; // Do the stall here, because here is the memory request triggered
    rr::ut::clk(top);
    REQUIRE(top->s_ready == 0);
    rr::ut::clk(top);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 != 0x000000ff);
    REQUIRE(top->m_texel01 != 0x0000ff00);
    REQUIRE(top->m_texel10 != 0x00ff0000);
    REQUIRE(top->m_texel11 != 0xff000000);
    REQUIRE(top->m_valid != 1);
    REQUIRE(top->m_ready != 1);

    top->m_ready = 1;
    rr::ut::clk(top);
    REQUIRE(top->s_ready == 1);
    rr::ut::clk(top);
    REQUIRE(top->m_texel00 == 0x000000ff);
    REQUIRE(top->m_texel01 == 0x0000ff00);
    REQUIRE(top->m_texel10 == 0x00ff0000);
    REQUIRE(top->m_texel11 == 0xff000000);
    REQUIRE(top->m_valid == 1);
    REQUIRE(top->m_ready == 1);

    // Destroy model
    delete top;
}