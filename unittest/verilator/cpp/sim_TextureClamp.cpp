// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2026 ToNi3141

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

#include "VTextureClamp.h"

struct TextureClampTestCase
{
    bool clampU;
    bool clampV;
    std::array<uint16_t, 4> expectedTexels;
};

TEST_CASE("Clamp texture quads", "[TextureClamp]")
{
    constexpr std::array<uint16_t, 4> inputTexels { 0x0011, 0x0022, 0x0033, 0x0044 };
    constexpr uint16_t subCoordS = 0x1234;
    constexpr uint16_t subCoordT = 0xabcd;

    const std::array testCases {
        TextureClampTestCase { false, false, { 0x0011, 0x0022, 0x0033, 0x0044 } },
        TextureClampTestCase { true, false, { 0x0011, 0x0011, 0x0033, 0x0033 } },
        TextureClampTestCase { false, true, { 0x0011, 0x0022, 0x0011, 0x0022 } },
        TextureClampTestCase { true, true, { 0x0011, 0x0011, 0x0011, 0x0011 } },
    };

    VTextureClamp* top = rr::ut::makeTop<VTextureClamp>();
    rr::ut::reset(top);
    top->m_ready = 1;

    for (const auto& testCase : testCases)
    {
        top->s_valid = 1;
        top->s_user = 1;
        top->s_texel00 = inputTexels[0];
        top->s_texel01 = inputTexels[1];
        top->s_texel10 = inputTexels[2];
        top->s_texel11 = inputTexels[3];
        top->s_texelSubCoordS = subCoordS;
        top->s_texelSubCoordT = subCoordT;
        top->s_clampU = testCase.clampU;
        top->s_clampV = testCase.clampV;
        rr::ut::clk(top);

        REQUIRE(top->s_ready == 1);
        REQUIRE(top->m_valid == 1);
        REQUIRE(top->m_user == 1);
        REQUIRE(top->m_texel00 == testCase.expectedTexels[0]);
        REQUIRE(top->m_texel01 == testCase.expectedTexels[1]);
        REQUIRE(top->m_texel10 == testCase.expectedTexels[2]);
        REQUIRE(top->m_texel11 == testCase.expectedTexels[3]);
        REQUIRE(top->m_texelSubCoordS == subCoordS);
        REQUIRE(top->m_texelSubCoordT == subCoordT);
    }

    top->s_valid = 0;
    rr::ut::clk(top);
    REQUIRE(top->m_valid == 0);

    delete top;
}

TEST_CASE("Hold clamped texture quad while stalled", "[TextureClamp]")
{
    VTextureClamp* top = rr::ut::makeTop<VTextureClamp>();
    rr::ut::reset(top);

    top->m_ready = 1;
    top->s_valid = 1;
    top->s_user = 1;
    top->s_texel00 = 0x0011;
    top->s_texel01 = 0x0022;
    top->s_texel10 = 0x0033;
    top->s_texel11 = 0x0044;
    top->s_texelSubCoordS = 0x1234;
    top->s_texelSubCoordT = 0xabcd;
    top->s_clampU = 1;
    top->s_clampV = 1;
    rr::ut::clk(top);

    top->m_ready = 0;
    top->s_valid = 1;
    top->s_texel00 = 0x0055;
    top->s_texel01 = 0x0066;
    top->s_texel10 = 0x0077;
    top->s_texel11 = 0x0088;
    top->s_clampU = 0;
    top->s_clampV = 0;
    rr::ut::clk(top);
    REQUIRE(top->s_ready == 0);
    REQUIRE(top->m_valid == 1);
    REQUIRE(top->m_texel00 == 0x0011);
    REQUIRE(top->m_texel01 == 0x0011);
    REQUIRE(top->m_texel10 == 0x0011);
    REQUIRE(top->m_texel11 == 0x0011);
    REQUIRE(top->m_texelSubCoordS == 0x1234);
    REQUIRE(top->m_texelSubCoordT == 0xabcd);

    delete top;
}
