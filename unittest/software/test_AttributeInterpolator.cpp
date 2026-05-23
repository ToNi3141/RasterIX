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
using rr::Vec4iColorRGBA;
using rr::softwarerasterizer::AttributeInterpolator;
using rr::softwarerasterizer::InterpolatedAttributesData;
using rr::TriangleStreamTypes::TriangleDesc;
using rr::ut::vec4Approx;
using rr::ut::vec4i16Approx;

// Helper functions to convert fixed-point to float for comparison
inline float depthZToFloat(int32_t depthZ)
{
    // depthZ is Sx.16 format
    return static_cast<float>(depthZ) / static_cast<float>(1u << 16);
}

inline float texCoordToFloat(int32_t coord)
{
    // Texture coordinates are S16.15 format
    return static_cast<float>(coord) / static_cast<float>(1 << 15);
}

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
    const Vec4iColorRGBA expected = Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.5f, 0.5f, 0.5f, 1.0f });
    REQUIRE(vec4i16Approx(result.color, expected));
}

TEST_CASE("AttributeInterpolator color interpolation in X direction", "[AttributeInterpolator]")
{
    AttributeInterpolator interpolator;
    interpolator.setEnableTMU(0, false);

    TriangleDesc desc = createTestTriangleDesc();

    // Interpolate at (10, 0)
    InterpolatedAttributesData result = interpolator.interpolate(desc, 10, 0);

    // Color should be start + 10 * colorXInc = 0.5 + 10 * 0.01 = 0.6
    const Vec4iColorRGBA expected = Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.6f, 0.6f, 0.6f, 1.0f });
    REQUIRE(vec4i16Approx(result.color, expected));
}

TEST_CASE("AttributeInterpolator color interpolation in Y direction", "[AttributeInterpolator]")
{
    AttributeInterpolator interpolator;
    interpolator.setEnableTMU(0, false);

    TriangleDesc desc = createTestTriangleDesc();

    // Interpolate at (0, 10)
    InterpolatedAttributesData result = interpolator.interpolate(desc, 0, 10);

    // Color should be start + 10 * colorYInc = 0.5 + 10 * 0.005 = 0.55
    const Vec4iColorRGBA expected = Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.55f, 0.55f, 0.55f, 1.0f });
    REQUIRE(vec4i16Approx(result.color, expected));
}

TEST_CASE("AttributeInterpolator combined X and Y color interpolation", "[AttributeInterpolator]")
{
    AttributeInterpolator interpolator;
    interpolator.setEnableTMU(0, false);

    TriangleDesc desc = createTestTriangleDesc();

    // Interpolate at (10, 10)
    InterpolatedAttributesData result = interpolator.interpolate(desc, 10, 10);

    // Color should be start + 10*xInc + 10*yInc = 0.5 + 0.1 + 0.05 = 0.65
    const Vec4iColorRGBA expected = Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.65f, 0.65f, 0.65f, 1.0f });
    REQUIRE(vec4i16Approx(result.color, expected));
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
        REQUIRE(depthZToFloat(result.depthZ) == Approx(0.5f));
        REQUIRE(result.depthW == Approx(1.0f));
    }

    SECTION("At (10, 10)")
    {
        InterpolatedAttributesData result = interpolator.interpolate(desc, 10, 10);

        // depthZ = 0.5 + 10*0.001 + 10*0.001 = 0.52
        REQUIRE(depthZToFloat(result.depthZ) == Approx(0.52f).margin(0.001f));
    }
}

TEST_CASE("AttributeInterpolator depthW interpolation", "[AttributeInterpolator]")
{
    AttributeInterpolator interpolator;
    interpolator.setEnableTMU(0, false);

    TriangleDesc desc = createTestTriangleDesc();

    SECTION("depthW at origin")
    {
        // depthZw[1] = 1.0 -> depthW = 1.0
        InterpolatedAttributesData result = interpolator.interpolate(desc, 0, 0);
        REQUIRE(result.depthW == Approx(1.0f).margin(0.001f));
    }

    SECTION("depthW interpolation along X")
    {
        // W starts at 1.0, increments by 0.1 per pixel in X
        desc.param.depthZwXInc = Vec2 { 0.0f, 0.1f };
        // At x=5: W = 1.0 + 5*0.1 = 1.5
        InterpolatedAttributesData result = interpolator.interpolate(desc, 5, 0);
        REQUIRE(result.depthW == Approx(1.5f).margin(0.001f));
    }

    SECTION("depthW interpolation along Y")
    {
        // W starts at 1.0, increments by 0.05 per pixel in Y
        desc.param.depthZwYInc = Vec2 { 0.0f, 0.05f };
        // At y=4: W = 1.0 + 4*0.05 = 1.2
        InterpolatedAttributesData result = interpolator.interpolate(desc, 0, 4);
        REQUIRE(result.depthW == Approx(1.2f).margin(0.001f));
    }

    SECTION("depthW interpolation along X and Y")
    {
        // W starts at 1.0, increments by 0.1 in X and 0.05 in Y
        desc.param.depthZwXInc = Vec2 { 0.0f, 0.1f };
        desc.param.depthZwYInc = Vec2 { 0.0f, 0.05f };
        // At (3, 4): W = 1.0 + 3*0.1 + 4*0.05 = 1.5
        InterpolatedAttributesData result = interpolator.interpolate(desc, 3, 4);
        REQUIRE(result.depthW == Approx(1.5f).margin(0.001f));
    }

    SECTION("depthW with negative increment")
    {
        // W starts at 1.0, decrements by 0.02 per pixel in X
        desc.param.depthZwXInc = Vec2 { 0.0f, -0.02f };
        // At x=10: W = 1.0 + 10*(-0.02) = 0.8
        InterpolatedAttributesData result = interpolator.interpolate(desc, 10, 0);
        REQUIRE(result.depthW == Approx(0.8f).margin(0.001f));
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
        REQUIRE(texCoordToFloat(result.tex[0].s) == Approx(0.0f).margin(0.001f));
        REQUIRE(texCoordToFloat(result.tex[0].t) == Approx(0.0f).margin(0.001f));
        REQUIRE(texCoordToFloat(result.tex[0].q) == Approx(1.0f).margin(0.001f));
    }

    SECTION("At (10, 0)")
    {
        InterpolatedAttributesData result = interpolator.interpolate(desc, 10, 0);

        // S = (0 + 10*0.01) * (1/Q) = 0.1 (Q=1)
        REQUIRE(texCoordToFloat(result.tex[0].s) == Approx(0.1f).margin(0.001f));
        REQUIRE(texCoordToFloat(result.tex[0].t) == Approx(0.0f).margin(0.001f));
    }

    SECTION("At (0, 10)")
    {
        InterpolatedAttributesData result = interpolator.interpolate(desc, 0, 10);

        // T = (0 + 10*0.01) * (1/Q) = 0.1 (Q=1)
        REQUIRE(texCoordToFloat(result.tex[0].s) == Approx(0.0f).margin(0.001f));
        REQUIRE(texCoordToFloat(result.tex[0].t) == Approx(0.1f).margin(0.001f));
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
        const Vec4iColorRGBA expected = Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.65f, 0.65f, 0.65f, 1.0f });
        REQUIRE(vec4i16Approx(result.color, expected));
    }

    SECTION("TMU 0 enabled interpolates texture")
    {
        interpolator.setEnableTMU(0, true);
        InterpolatedAttributesData result = interpolator.interpolate(desc, 10, 10);

        // Both S and T should be 0.1
        REQUIRE(texCoordToFloat(result.tex[0].s) == Approx(0.1f).margin(0.001f));
        REQUIRE(texCoordToFloat(result.tex[0].t) == Approx(0.1f).margin(0.001f));
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
        const Vec4iColorRGBA expected = Vec4iColorRGBA::createFromVecToInt(Vec4 { 1.0f, 1.0f, 1.0f, 1.0f });
        REQUIRE(vec4i16Approx(result.color, expected));
    }

    SECTION("Color clamped to 0.0")
    {
        // Set up color to go below 0.0 after interpolation
        desc.param.color = Vec4 { 0.1f, 0.1f, 0.1f, 1.0f };
        desc.param.colorXInc = Vec4 { -0.1f, -0.1f, -0.1f, 0.0f };

        InterpolatedAttributesData result = interpolator.interpolate(desc, 10, 0);

        // 0.1 + 10*(-0.1) = -0.9 -> clamped to 0.0
        const Vec4iColorRGBA expected = Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.0f, 0.0f, 0.0f, 1.0f });
        REQUIRE(vec4i16Approx(result.color, expected));
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
    REQUIRE(texCoordToFloat(result.tex[0].s) == Approx(0.0f).margin(0.001f));
    REQUIRE(texCoordToFloat(result.tex[0].t) == Approx(0.0f).margin(0.001f));
    REQUIRE(texCoordToFloat(result.tex[0].q) == Approx(0.5f).margin(0.001f));
}
