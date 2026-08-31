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
#include <array>
#include <cstdint>

#include "VTextureSampler.h"

struct SamplerResult
{
    std::array<uint32_t, 4> addresses;
    uint16_t subCoordS;
    uint16_t subCoordT;
    bool clampU = false;
    bool clampV = false;
};

void requestTexture(VTextureSampler* top, uint32_t texelS, uint32_t texelT, const SamplerResult& expected)
{
    top->s_texelS = texelS;
    top->s_texelT = texelT;
    top->s_valid = 1;
    rr::ut::clk(top);
    top->s_valid = 0;

    for (int cycle = 0; cycle < 3 && !top->m_valid; ++cycle)
    {
        rr::ut::clk(top);
    }

    REQUIRE(top->m_valid == 1);
    REQUIRE(top->texelAddr00 == expected.addresses[0]);
    REQUIRE(top->texelAddr01 == expected.addresses[1]);
    REQUIRE(top->texelAddr10 == expected.addresses[2]);
    REQUIRE(top->texelAddr11 == expected.addresses[3]);
    REQUIRE(top->m_texelSubCoordS == expected.subCoordS);
    REQUIRE(top->m_texelSubCoordT == expected.subCoordT);
    REQUIRE(top->m_clampU == expected.clampU);
    REQUIRE(top->m_clampV == expected.clampV);

    rr::ut::clk(top);
    REQUIRE(top->m_valid == 0);
}

VTextureSampler* makeSampler()
{
    VTextureSampler* top = rr::ut::makeTop<VTextureSampler>();
    rr::ut::reset(top);
    top->m_ready = 1;
    top->textureSizeWidth = 1;
    top->textureSizeHeight = 1;
    top->enableHalfPixelOffset = 0;
    top->s_valid = 0;
    top->s_user = 0;
    top->s_clampS = 0;
    top->s_clampT = 0;
    top->s_textureLod = 0;
    return top;
}

TEST_CASE("Calculate texture addresses and sub coordinates", "[TextureSampler]")
{
    VTextureSampler* top = makeSampler();

    requestTexture(top, 0x0000, 0x0000, { { 0, 1, 2, 3 }, 0x0000, 0x0000 });
    requestTexture(top, 0x1000, 0x1000, { { 0, 1, 2, 3 }, 0x4000, 0x4000 });
    requestTexture(top, 0x1000, 0x3000, { { 0, 1, 2, 3 }, 0x4000, 0xc000 });
    requestTexture(top, 0x3000, 0x1000, { { 0, 1, 2, 3 }, 0xc000, 0x4000 });
    requestTexture(top, 0x7fff, 0x7fff, { { 3, 2, 1, 0 }, 0xfffc, 0xfffc });
    requestTexture(top, 0x8000, 0x8000, { { 0, 1, 2, 3 }, 0x0000, 0x0000 });

    delete top;
}

TEST_CASE("Clamp texture addresses at the upper edge", "[TextureSampler]")
{
    VTextureSampler* top = makeSampler();

    top->s_clampS = 1;
    requestTexture(top, 0x4000, 0x4000, { { 3, 3, 1, 1 }, 0x0000, 0x0000, true, false });

    top->s_clampS = 0;
    top->s_clampT = 1;
    requestTexture(top, 0x4000, 0x4000, { { 3, 2, 3, 2 }, 0x0000, 0x0000, false, true });

    top->s_clampS = 1;
    requestTexture(top, 0x4000, 0x4000, { { 3, 3, 3, 3 }, 0x0000, 0x0000, true, true });

    delete top;
}

TEST_CASE("Calculate mipmap texture addresses", "[TextureSampler]")
{
    struct MipmapCase
    {
        uint8_t width;
        uint8_t height;
        uint8_t lod;
        std::array<uint32_t, 4> addresses;
    };

    constexpr std::array testCases {
        MipmapCase { 2, 3, 0, { 0, 1, 4, 5 } },
        MipmapCase { 2, 3, 1, { 32, 33, 34, 35 } },
        MipmapCase { 2, 3, 2, { 40, 40, 41, 41 } },
        MipmapCase { 2, 3, 3, { 42, 42, 42, 42 } },
        MipmapCase { 2, 2, 1, { 16, 17, 18, 19 } },
        MipmapCase { 2, 2, 2, { 20, 20, 20, 20 } },
        MipmapCase { 2, 4, 1, { 64, 65, 66, 67 } },
        MipmapCase { 2, 4, 2, { 80, 80, 81, 81 } },
        MipmapCase { 2, 4, 3, { 84, 84, 85, 85 } },
        MipmapCase { 2, 4, 4, { 86, 86, 86, 86 } },
    };

    VTextureSampler* top = makeSampler();
    for (const auto& testCase : testCases)
    {
        top->textureSizeWidth = testCase.width;
        top->textureSizeHeight = testCase.height;
        top->s_textureLod = testCase.lod;
        requestTexture(top, 0, 0, { testCase.addresses, 0, 0 });
    }

    delete top;
}

TEST_CASE("Apply half pixel offset", "[TextureSampler]")
{
    VTextureSampler* top = makeSampler();
    top->enableHalfPixelOffset = 1;

    requestTexture(top, 0, 0, { { 3, 2, 1, 0 }, 0x8000, 0x8000 });

    top->s_clampS = 1;
    top->s_clampT = 1;
    requestTexture(top, 0, 0, { { 0, 0, 0, 0 }, 0x8000, 0x8000, true, true });

    delete top;
}

TEST_CASE("Hold sampler output while stalled", "[TextureSampler]")
{
    VTextureSampler* top = makeSampler();
    top->s_user = 1;
    top->s_valid = 1;
    top->s_texelS = 0;
    top->s_texelT = 0;
    rr::ut::clk(top);
    top->s_valid = 0;
    rr::ut::clk(top);
    REQUIRE(top->m_valid == 1);

    top->m_ready = 0;
    top->s_valid = 1;
    top->s_user = 0;
    top->s_texelS = 0x7fff;
    top->s_texelT = 0x7fff;
    rr::ut::clk(top);

    REQUIRE(top->s_ready == 0);
    REQUIRE(top->m_valid == 1);
    REQUIRE(top->m_user == 1);
    REQUIRE(top->texelAddr00 == 0);
    REQUIRE(top->texelAddr01 == 1);
    REQUIRE(top->texelAddr10 == 2);
    REQUIRE(top->texelAddr11 == 3);

    delete top;
}