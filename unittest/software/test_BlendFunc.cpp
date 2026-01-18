// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2025 ToNi3141

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

#define CATCH_CONFIG_MAIN
#include "../3rdParty/catch.hpp"
#include "Helper.hpp"
#include "renderer/softwarerasterizer/BlendFunc.hpp"

using rr::Vec4;
using rr::ut::vec4Approx;
// Note: We use softwarerasterizer::BlendFunc explicitly to avoid
// ambiguity with rr::BlendFunc (the enum)

TEST_CASE("BlendFunc disabled returns source unchanged", "[BlendFunc]")
{
    rr::softwarerasterizer::BlendFunc blend;
    blend.setEnable(false);

    Vec4 src { 0.5f, 0.6f, 0.7f, 0.8f };
    Vec4 dst { 0.1f, 0.2f, 0.3f, 0.4f };

    Vec4 result = blend.blend(src, dst);

    REQUIRE(vec4Approx(result, src));
}

TEST_CASE("BlendFunc ZERO factor", "[BlendFunc]")
{
    rr::softwarerasterizer::BlendFunc blend;
    blend.setEnable(true);
    blend.setSFactor(rr::BlendFunc::ZERO);
    blend.setDFactor(rr::BlendFunc::ZERO);

    Vec4 src { 0.5f, 0.6f, 0.7f, 0.8f };
    Vec4 dst { 0.1f, 0.2f, 0.3f, 0.4f };

    Vec4 result = blend.blend(src, dst);
    Vec4 expected { 0.0f, 0.0f, 0.0f, 0.0f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("BlendFunc ONE factor", "[BlendFunc]")
{
    rr::softwarerasterizer::BlendFunc blend;
    blend.setEnable(true);
    blend.setSFactor(rr::BlendFunc::ONE);
    blend.setDFactor(rr::BlendFunc::ONE);

    Vec4 src { 0.3f, 0.3f, 0.3f, 0.3f };
    Vec4 dst { 0.2f, 0.2f, 0.2f, 0.2f };

    Vec4 result = blend.blend(src, dst);
    Vec4 expected { 0.5f, 0.5f, 0.5f, 0.5f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("BlendFunc DST_COLOR factor", "[BlendFunc]")
{
    rr::softwarerasterizer::BlendFunc blend;
    blend.setEnable(true);
    blend.setSFactor(rr::BlendFunc::DST_COLOR);
    blend.setDFactor(rr::BlendFunc::ZERO);

    Vec4 src { 1.0f, 1.0f, 1.0f, 1.0f };
    Vec4 dst { 0.5f, 0.4f, 0.3f, 0.2f };

    // result = src * dst + dst * 0 = dst
    Vec4 result = blend.blend(src, dst);
    Vec4 expected { 0.5f, 0.4f, 0.3f, 0.2f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("BlendFunc ONE_MINUS_SRC_COLOR factor", "[BlendFunc]")
{
    rr::softwarerasterizer::BlendFunc blend;
    blend.setEnable(true);
    blend.setSFactor(rr::BlendFunc::ZERO);
    blend.setDFactor(rr::BlendFunc::ONE_MINUS_SRC_COLOR);

    Vec4 src { 0.2f, 0.3f, 0.4f, 0.5f };
    Vec4 dst { 1.0f, 1.0f, 1.0f, 1.0f };

    // result = src * 0 + dst * (1 - src) = (0.8, 0.7, 0.6, 0.5)
    Vec4 result = blend.blend(src, dst);
    Vec4 expected { 0.8f, 0.7f, 0.6f, 0.5f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("BlendFunc ONE_MINUS_DST_COLOR factor", "[BlendFunc]")
{
    rr::softwarerasterizer::BlendFunc blend;
    blend.setEnable(true);
    blend.setSFactor(rr::BlendFunc::ONE_MINUS_DST_COLOR);
    blend.setDFactor(rr::BlendFunc::ZERO);

    Vec4 src { 1.0f, 1.0f, 1.0f, 1.0f };
    Vec4 dst { 0.2f, 0.3f, 0.4f, 0.5f };

    // result = src * (1 - dst) = (0.8, 0.7, 0.6, 0.5)
    Vec4 result = blend.blend(src, dst);
    Vec4 expected { 0.8f, 0.7f, 0.6f, 0.5f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("BlendFunc SRC_ALPHA factor", "[BlendFunc]")
{
    rr::softwarerasterizer::BlendFunc blend;
    blend.setEnable(true);
    blend.setSFactor(rr::BlendFunc::SRC_ALPHA);
    blend.setDFactor(rr::BlendFunc::ZERO);

    Vec4 src { 1.0f, 1.0f, 1.0f, 0.5f };
    Vec4 dst { 0.0f, 0.0f, 0.0f, 0.0f };

    // result = src * src.a = (0.5, 0.5, 0.5, 0.25)
    Vec4 result = blend.blend(src, dst);
    Vec4 expected { 0.5f, 0.5f, 0.5f, 0.25f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("BlendFunc DST_ALPHA factor", "[BlendFunc]")
{
    rr::softwarerasterizer::BlendFunc blend;
    blend.setEnable(true);
    blend.setSFactor(rr::BlendFunc::DST_ALPHA);
    blend.setDFactor(rr::BlendFunc::ZERO);

    Vec4 src { 1.0f, 1.0f, 1.0f, 1.0f };
    Vec4 dst { 0.0f, 0.0f, 0.0f, 0.4f };

    // result = src * dst.a = (0.4, 0.4, 0.4, 0.4)
    Vec4 result = blend.blend(src, dst);
    Vec4 expected { 0.4f, 0.4f, 0.4f, 0.4f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("BlendFunc ONE_MINUS_SRC_ALPHA factor", "[BlendFunc]")
{
    rr::softwarerasterizer::BlendFunc blend;
    blend.setEnable(true);
    blend.setSFactor(rr::BlendFunc::ZERO);
    blend.setDFactor(rr::BlendFunc::ONE_MINUS_SRC_ALPHA);

    Vec4 src { 0.0f, 0.0f, 0.0f, 0.3f };
    Vec4 dst { 1.0f, 1.0f, 1.0f, 1.0f };

    // result = dst * (1 - src.a) = (0.7, 0.7, 0.7, 0.7)
    Vec4 result = blend.blend(src, dst);
    Vec4 expected { 0.7f, 0.7f, 0.7f, 0.7f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("BlendFunc ONE_MINUS_DST_ALPHA factor", "[BlendFunc]")
{
    rr::softwarerasterizer::BlendFunc blend;
    blend.setEnable(true);
    blend.setSFactor(rr::BlendFunc::ONE_MINUS_DST_ALPHA);
    blend.setDFactor(rr::BlendFunc::ZERO);

    Vec4 src { 1.0f, 1.0f, 1.0f, 1.0f };
    Vec4 dst { 0.0f, 0.0f, 0.0f, 0.6f };

    // result = src * (1 - dst.a) = (0.4, 0.4, 0.4, 0.4)
    Vec4 result = blend.blend(src, dst);
    Vec4 expected { 0.4f, 0.4f, 0.4f, 0.4f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("BlendFunc SRC_ALPHA_SATURATE factor", "[BlendFunc]")
{
    rr::softwarerasterizer::BlendFunc blend;
    blend.setEnable(true);
    blend.setSFactor(rr::BlendFunc::SRC_ALPHA_SATURATE);
    blend.setDFactor(rr::BlendFunc::ZERO);

    SECTION("src.alpha < (1 - dst.alpha)")
    {
        Vec4 src { 1.0f, 1.0f, 1.0f, 0.3f };
        Vec4 dst { 0.0f, 0.0f, 0.0f, 0.5f };

        // f = min(0.3, 0.5) = 0.3
        // result = src * (0.3, 0.3, 0.3, 1.0) = (0.3, 0.3, 0.3, 0.3)
        Vec4 result = blend.blend(src, dst);
        Vec4 expected { 0.3f, 0.3f, 0.3f, 0.3f };

        REQUIRE(vec4Approx(result, expected));
    }

    SECTION("src.alpha >= (1 - dst.alpha)")
    {
        Vec4 src { 1.0f, 1.0f, 1.0f, 0.8f };
        Vec4 dst { 0.0f, 0.0f, 0.0f, 0.7f };

        // f = min(0.8, 0.3) = 0.3
        // result = src * (0.3, 0.3, 0.3, 1.0) = (0.3, 0.3, 0.3, 0.8)
        Vec4 result = blend.blend(src, dst);
        Vec4 expected { 0.3f, 0.3f, 0.3f, 0.8f };

        REQUIRE(vec4Approx(result, expected));
    }
}

TEST_CASE("BlendFunc classic alpha blending (SRC_ALPHA, ONE_MINUS_SRC_ALPHA)", "[BlendFunc]")
{
    rr::softwarerasterizer::BlendFunc blend;
    blend.setEnable(true);
    blend.setSFactor(rr::BlendFunc::SRC_ALPHA);
    blend.setDFactor(rr::BlendFunc::ONE_MINUS_SRC_ALPHA);

    Vec4 src { 1.0f, 0.0f, 0.0f, 0.5f }; // Red, 50% opacity
    Vec4 dst { 0.0f, 0.0f, 1.0f, 1.0f }; // Blue, fully opaque

    // result = src * src.a + dst * (1 - src.a)
    // result = (1,0,0,0.5) * 0.5 + (0,0,1,1) * 0.5
    // result = (0.5, 0, 0, 0.25) + (0, 0, 0.5, 0.5) = (0.5, 0, 0.5, 0.75)
    Vec4 result = blend.blend(src, dst);
    Vec4 expected { 0.5f, 0.0f, 0.5f, 0.75f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("BlendFunc additive blending (ONE, ONE)", "[BlendFunc]")
{
    rr::softwarerasterizer::BlendFunc blend;
    blend.setEnable(true);
    blend.setSFactor(rr::BlendFunc::ONE);
    blend.setDFactor(rr::BlendFunc::ONE);

    Vec4 src { 0.3f, 0.0f, 0.0f, 1.0f };
    Vec4 dst { 0.0f, 0.3f, 0.0f, 1.0f };

    // result = src + dst (clamped)
    Vec4 result = blend.blend(src, dst);
    Vec4 expected { 0.3f, 0.3f, 0.0f, 1.0f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("BlendFunc result clamping", "[BlendFunc]")
{
    rr::softwarerasterizer::BlendFunc blend;
    blend.setEnable(true);
    blend.setSFactor(rr::BlendFunc::ONE);
    blend.setDFactor(rr::BlendFunc::ONE);

    Vec4 src { 0.8f, 0.8f, 0.8f, 0.8f };
    Vec4 dst { 0.8f, 0.8f, 0.8f, 0.8f };

    // result = src + dst = 1.6 -> clamped to 1.0
    Vec4 result = blend.blend(src, dst);
    Vec4 expected { 1.0f, 1.0f, 1.0f, 1.0f };

    REQUIRE(vec4Approx(result, expected));
}

TEST_CASE("BlendFunc multiplicative blending (DST_COLOR, ZERO)", "[BlendFunc]")
{
    rr::softwarerasterizer::BlendFunc blend;
    blend.setEnable(true);
    blend.setSFactor(rr::BlendFunc::DST_COLOR);
    blend.setDFactor(rr::BlendFunc::ZERO);

    Vec4 src { 0.5f, 0.5f, 0.5f, 1.0f };
    Vec4 dst { 0.8f, 0.6f, 0.4f, 1.0f };

    // result = src * dst = (0.4, 0.3, 0.2, 1.0)
    Vec4 result = blend.blend(src, dst);
    Vec4 expected { 0.4f, 0.3f, 0.2f, 1.0f };

    REQUIRE(vec4Approx(result, expected));
}
