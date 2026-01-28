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
#include "renderer/softwarerasterizer/AttributeInterpolator.hpp"

using rr::Vec2;
using rr::Vec3;
using rr::Vec4;
using rr::softwarerasterizer::AttributeInterpolator;
using rr::softwarerasterizer::InterpolatedAttributesData;
using rr::TriangleStreamTypes::TriangleDesc;
using rr::ut::vec4Approx;

// Helper to create a simple triangle description for testing
TriangleDesc createTestTriangleDesc()
{
    TriangleDesc desc {};

    // Set up bounding box
    desc.param.bbStartX = 0;
    desc.param.bbStartY = 0;
    desc.param.bbEndX = 100;
    desc.param.bbEndY = 100;

    // Color: start at (0.5, 0.5, 0.5, 1.0)
    desc.param.color = Vec4 { 0.5f, 0.5f, 0.5f, 1.0f };
    // Color X increment: +0.01 per pixel for RGB
    desc.param.colorXInc = Vec4 { 0.01f, 0.01f, 0.01f, 0.0f };
    // Color Y increment: +0.005 per pixel for RGB
    desc.param.colorYInc = Vec4 { 0.005f, 0.005f, 0.005f, 0.0f };

    // Depth: Z=0.5, W=1.0 (so depthW will become 1.0)
    desc.param.depthZw = Vec2 { 0.5f, 1.0f };
    desc.param.depthZwXInc = Vec2 { 0.001f, 0.0f };
    desc.param.depthZwYInc = Vec2 { 0.001f, 0.0f };

    // Texture coordinates for TMU 0: S=0, T=0, Q=1
    desc.texture[0].texStq = Vec3 { 0.0f, 0.0f, 1.0f };
    desc.texture[0].texStqXInc = Vec3 { 0.01f, 0.0f, 0.0f };
    desc.texture[0].texStqYInc = Vec3 { 0.0f, 0.01f, 0.0f };

    return desc;
}

TEST_CASE("AttributeInterpolator color interpolation at origin", "[AttributeInterpolator]")
{
    AttributeInterpolator interpolator;
    interpolator.setEnableTMU(0, false);

    TriangleDesc desc = createTestTriangleDesc();

    // Interpolate at origin (0, 0)
    InterpolatedAttributesData result = interpolator.interpolate(desc, 0, 0);

    // At origin, color should be the start value (0.5, 0.5, 0.5, 1.0)
    REQUIRE(vec4Approx(result.color, Vec4 { 0.5f, 0.5f, 0.5f, 1.0f }));
}

TEST_CASE("AttributeInterpolator color interpolation in X direction", "[AttributeInterpolator]")
{
    AttributeInterpolator interpolator;
    interpolator.setEnableTMU(0, false);

    TriangleDesc desc = createTestTriangleDesc();

    // Interpolate at (10, 0)
    InterpolatedAttributesData result = interpolator.interpolate(desc, 10, 0);

    // Color should be start + 10 * colorXInc = 0.5 + 10 * 0.01 = 0.6
    REQUIRE(vec4Approx(result.color, Vec4 { 0.6f, 0.6f, 0.6f, 1.0f }));
}

TEST_CASE("AttributeInterpolator color interpolation in Y direction", "[AttributeInterpolator]")
{
    AttributeInterpolator interpolator;
    interpolator.setEnableTMU(0, false);

    TriangleDesc desc = createTestTriangleDesc();

    // Interpolate at (0, 10)
    InterpolatedAttributesData result = interpolator.interpolate(desc, 0, 10);

    // Color should be start + 10 * colorYInc = 0.5 + 10 * 0.005 = 0.55
    REQUIRE(vec4Approx(result.color, Vec4 { 0.55f, 0.55f, 0.55f, 1.0f }));
}

TEST_CASE("AttributeInterpolator combined X and Y color interpolation", "[AttributeInterpolator]")
{
    AttributeInterpolator interpolator;
    interpolator.setEnableTMU(0, false);

    TriangleDesc desc = createTestTriangleDesc();

    // Interpolate at (10, 10)
    InterpolatedAttributesData result = interpolator.interpolate(desc, 10, 10);

    // Color should be start + 10*xInc + 10*yInc = 0.5 + 0.1 + 0.05 = 0.65
    REQUIRE(vec4Approx(result.color, Vec4 { 0.65f, 0.65f, 0.65f, 1.0f }));
}

TEST_CASE("AttributeInterpolator depth interpolation", "[AttributeInterpolator]")
{
    AttributeInterpolator interpolator;
    interpolator.setEnableTMU(0, false);

    TriangleDesc desc = createTestTriangleDesc();

    SECTION("At origin")
    {
        InterpolatedAttributesData result = interpolator.interpolate(desc, 0, 0);

        // depthZ should be 0.5, depthW should be 1/1.0 = 1.0
        REQUIRE(result.depthZ == Approx(0.5f));
        REQUIRE(result.depthW == Approx(1.0f));
    }

    SECTION("At (10, 10)")
    {
        InterpolatedAttributesData result = interpolator.interpolate(desc, 10, 10);

        // depthZ = 0.5 + 10*0.001 + 10*0.001 = 0.52
        REQUIRE(result.depthZ == Approx(0.52f));
    }
}

TEST_CASE("AttributeInterpolator texture interpolation", "[AttributeInterpolator]")
{
    AttributeInterpolator interpolator;
    interpolator.setEnableTMU(0, true);

    TriangleDesc desc = createTestTriangleDesc();

    SECTION("At origin")
    {
        InterpolatedAttributesData result = interpolator.interpolate(desc, 0, 0);

        // S=0, T=0 at origin (perspective corrected with Q=1)
        REQUIRE(result.tex[0].s == Approx(0.0f).margin(0.001f));
        REQUIRE(result.tex[0].t == Approx(0.0f).margin(0.001f));
        REQUIRE(result.tex[0].q == Approx(1.0f).margin(0.001f));
    }

    SECTION("At (10, 0)")
    {
        InterpolatedAttributesData result = interpolator.interpolate(desc, 10, 0);

        // S = (0 + 10*0.01) * (1/Q) = 0.1 (Q=1)
        REQUIRE(result.tex[0].s == Approx(0.1f).margin(0.001f));
        REQUIRE(result.tex[0].t == Approx(0.0f).margin(0.001f));
    }

    SECTION("At (0, 10)")
    {
        InterpolatedAttributesData result = interpolator.interpolate(desc, 0, 10);

        // T = (0 + 10*0.01) * (1/Q) = 0.1 (Q=1)
        REQUIRE(result.tex[0].s == Approx(0.0f).margin(0.001f));
        REQUIRE(result.tex[0].t == Approx(0.1f).margin(0.001f));
    }
}

TEST_CASE("AttributeInterpolator TMU enable/disable", "[AttributeInterpolator]")
{
    AttributeInterpolator interpolator;

    TriangleDesc desc = createTestTriangleDesc();

    SECTION("TMU 0 disabled skips texture interpolation")
    {
        interpolator.setEnableTMU(0, false);
        // Should not crash, texture values will be uninitialized/zero
        InterpolatedAttributesData result = interpolator.interpolate(desc, 10, 10);
        // Color should still be interpolated
        REQUIRE(vec4Approx(result.color, Vec4 { 0.65f, 0.65f, 0.65f, 1.0f }));
    }

    SECTION("TMU 0 enabled interpolates texture")
    {
        interpolator.setEnableTMU(0, true);
        InterpolatedAttributesData result = interpolator.interpolate(desc, 10, 10);

        // Both S and T should be 0.1
        REQUIRE(result.tex[0].s == Approx(0.1f).margin(0.001f));
        REQUIRE(result.tex[0].t == Approx(0.1f).margin(0.001f));
    }
}

TEST_CASE("AttributeInterpolator color clamping", "[AttributeInterpolator]")
{
    AttributeInterpolator interpolator;
    interpolator.setEnableTMU(0, false);

    TriangleDesc desc = createTestTriangleDesc();

    SECTION("Color clamped to 1.0")
    {
        // Set up color to exceed 1.0 after interpolation
        desc.param.color = Vec4 { 0.9f, 0.9f, 0.9f, 1.0f };
        desc.param.colorXInc = Vec4 { 0.1f, 0.1f, 0.1f, 0.0f };

        InterpolatedAttributesData result = interpolator.interpolate(desc, 10, 0);

        // 0.9 + 10*0.1 = 1.9 -> clamped to 1.0
        REQUIRE(result.color[0] == Approx(1.0f));
        REQUIRE(result.color[1] == Approx(1.0f));
        REQUIRE(result.color[2] == Approx(1.0f));
    }

    SECTION("Color clamped to 0.0")
    {
        // Set up color to go below 0.0 after interpolation
        desc.param.color = Vec4 { 0.1f, 0.1f, 0.1f, 1.0f };
        desc.param.colorXInc = Vec4 { -0.1f, -0.1f, -0.1f, 0.0f };

        InterpolatedAttributesData result = interpolator.interpolate(desc, 10, 0);

        // 0.1 + 10*(-0.1) = -0.9 -> clamped to 0.0
        REQUIRE(result.color[0] == Approx(0.0f));
        REQUIRE(result.color[1] == Approx(0.0f));
        REQUIRE(result.color[2] == Approx(0.0f));
    }
}

TEST_CASE("AttributeInterpolator perspective correct texture", "[AttributeInterpolator]")
{
    AttributeInterpolator interpolator;
    interpolator.setEnableTMU(0, true);

    TriangleDesc desc {};
    // Set up perspective texture coordinates with varying Q
    desc.texture[0].texStq = Vec3 { 0.0f, 0.0f, 2.0f }; // Q=2 at start
    desc.texture[0].texStqXInc = Vec3 { 0.1f, 0.0f, 0.0f };
    desc.texture[0].texStqYInc = Vec3 { 0.0f, 0.1f, 0.0f };

    // Color values (needed to avoid uninitialized)
    desc.param.color = Vec4 { 0.5f, 0.5f, 0.5f, 1.0f };
    desc.param.depthZw = Vec2 { 0.5f, 1.0f };

    InterpolatedAttributesData result = interpolator.interpolate(desc, 0, 0);

    // At origin: S_raw=0, T_raw=0, Q_raw=2
    // After perspective divide: s = S_raw * (1/Q_raw) = 0 * 0.5 = 0
    // q = 1/Q_raw = 0.5
    REQUIRE(result.tex[0].s == Approx(0.0f).margin(0.001f));
    REQUIRE(result.tex[0].t == Approx(0.0f).margin(0.001f));
    REQUIRE(result.tex[0].q == Approx(0.5f).margin(0.001f));
}
