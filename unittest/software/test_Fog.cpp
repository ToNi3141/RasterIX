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
#include "renderer/softwarerasterizer/Fog.hpp"

using namespace rr;
using namespace rr::softwarerasterizer;

// Helper to create a simple linear fog LUT
// Maps w values to fog factors linearly within the LUT range
Fog::FogLut createLinearFogLut()
{
    Fog::FogLut lut {};
    // Create a LUT where each entry linearly decreases fog factor
    // Entry i covers w values from 2^i to 2^(i+1)
    for (std::size_t i = 0; i < lut.size(); ++i)
    {
        // Linear interpolation: factor goes from 1.0 to 0.0 as index increases
        float startFactor = 1.0f - (static_cast<float>(i) / static_cast<float>(lut.size()));
        float endFactor = 1.0f - (static_cast<float>(i + 1) / static_cast<float>(lut.size()));
        lut[i].b = startFactor;
        lut[i].m = endFactor - startFactor;
    }
    return lut;
}

// Helper to create a constant fog LUT (all entries return the same factor)
Fog::FogLut createConstantFogLut(float factor)
{
    Fog::FogLut lut {};
    for (auto& entry : lut)
    {
        entry.m = 0.0f;
        entry.b = factor;
    }
    return lut;
}

TEST_CASE("Fog disabled returns original color", "[Fog]")
{
    Fog fog;
    fog.setEnable(false);
    fog.setFogColor(Vec4 { 0.5f, 0.5f, 0.5f, 1.0f });

    const Vec4 inputColor { 1.0f, 0.0f, 0.0f, 1.0f };
    const Vec4 result = fog.calculateFog(100.0f, inputColor);

    REQUIRE(rr::ut::vec4Approx(result, inputColor));
}

TEST_CASE("Fog enabled with w below lower bound returns original color", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    fog.setFogColor(Vec4 { 0.5f, 0.5f, 0.5f, 1.0f });
    fog.setFogLut(createConstantFogLut(0.0f), 10.0f, 100.0f);

    const Vec4 inputColor { 1.0f, 0.0f, 0.0f, 1.0f };
    // w = 5.0 is below lower bound of 10.0, so factor should be 1.0 (no fog)
    const Vec4 result = fog.calculateFog(5.0f, inputColor);

    REQUIRE(rr::ut::vec4Approx(result, inputColor));
}

TEST_CASE("Fog enabled with w above upper bound returns fog color", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    const Vec4 fogColor { 0.5f, 0.5f, 0.5f, 1.0f };
    fog.setFogColor(fogColor);
    fog.setFogLut(createConstantFogLut(0.5f), 10.0f, 100.0f);

    const Vec4 inputColor { 1.0f, 0.0f, 0.0f, 0.8f };
    // w = 200.0 is above upper bound of 100.0, so factor should be 0.0 (full fog)
    const Vec4 result = fog.calculateFog(200.0f, inputColor);

    // Full fog means result should be fog color but with original alpha
    const Vec4 expected { fogColor[0], fogColor[1], fogColor[2], inputColor[3] };
    REQUIRE(rr::ut::vec4Approx(result, expected));
}

TEST_CASE("Fog preserves alpha channel", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    fog.setFogColor(Vec4 { 0.0f, 0.0f, 0.0f, 0.0f });
    fog.setFogLut(createConstantFogLut(0.5f), 1.0f, 1000.0f);

    SECTION("Alpha 1.0 preserved")
    {
        const Vec4 inputColor { 1.0f, 1.0f, 1.0f, 1.0f };
        const Vec4 result = fog.calculateFog(10.0f, inputColor);
        REQUIRE(result[3] == Approx(1.0f));
    }

    SECTION("Alpha 0.5 preserved")
    {
        const Vec4 inputColor { 1.0f, 1.0f, 1.0f, 0.5f };
        const Vec4 result = fog.calculateFog(10.0f, inputColor);
        REQUIRE(result[3] == Approx(0.5f));
    }

    SECTION("Alpha 0.0 preserved")
    {
        const Vec4 inputColor { 1.0f, 1.0f, 1.0f, 0.0f };
        const Vec4 result = fog.calculateFog(10.0f, inputColor);
        REQUIRE(result[3] == Approx(0.0f));
    }
}

TEST_CASE("Fog factor 1.0 returns original color (RGB)", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    fog.setFogColor(Vec4 { 0.0f, 0.0f, 0.0f, 1.0f });
    fog.setFogLut(createConstantFogLut(1.0f), 1.0f, 1000.0f);

    const Vec4 inputColor { 0.8f, 0.6f, 0.4f, 1.0f };
    const Vec4 result = fog.calculateFog(10.0f, inputColor);

    // Factor 1.0 means full original color
    REQUIRE(rr::ut::vec4Approx(result, inputColor));
}

TEST_CASE("Fog factor 0.0 returns fog color (RGB)", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    const Vec4 fogColor { 0.3f, 0.3f, 0.3f, 1.0f };
    fog.setFogColor(fogColor);
    fog.setFogLut(createConstantFogLut(0.0f), 1.0f, 1000.0f);

    const Vec4 inputColor { 0.8f, 0.6f, 0.4f, 0.7f };
    const Vec4 result = fog.calculateFog(10.0f, inputColor);

    // Factor 0.0 means full fog color, but alpha preserved
    const Vec4 expected { fogColor[0], fogColor[1], fogColor[2], inputColor[3] };
    REQUIRE(rr::ut::vec4Approx(result, expected));
}

TEST_CASE("Fog factor 0.5 blends colors equally", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    const Vec4 fogColor { 0.0f, 0.0f, 0.0f, 1.0f };
    fog.setFogColor(fogColor);
    fog.setFogLut(createConstantFogLut(0.5f), 1.0f, 1000.0f);

    const Vec4 inputColor { 1.0f, 1.0f, 1.0f, 1.0f };
    const Vec4 result = fog.calculateFog(10.0f, inputColor);

    // Factor 0.5 means 50% blend: (0.5 * fogColor) + (0.5 * inputColor)
    const Vec4 expected { 0.5f, 0.5f, 0.5f, 1.0f };
    REQUIRE(rr::ut::vec4Approx(result, expected));
}

TEST_CASE("Fog result is clamped to [0,1]", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    // Use extreme fog color values
    fog.setFogColor(Vec4 { 2.0f, -1.0f, 1.5f, 1.0f });
    fog.setFogLut(createConstantFogLut(0.0f), 1.0f, 1000.0f);

    const Vec4 inputColor { 0.5f, 0.5f, 0.5f, 1.0f };
    const Vec4 result = fog.calculateFog(10.0f, inputColor);

    // Result should be clamped
    REQUIRE(result[0] >= 0.0f);
    REQUIRE(result[0] <= 1.0f);
    REQUIRE(result[1] >= 0.0f);
    REQUIRE(result[1] <= 1.0f);
    REQUIRE(result[2] >= 0.0f);
    REQUIRE(result[2] <= 1.0f);
}

TEST_CASE("Fog LUT interpolation uses log2 of w", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    fog.setFogColor(Vec4 { 0.0f, 0.0f, 0.0f, 1.0f });

    // Create a LUT where entry 3 (w in range 8-16) returns factor 0.75
    Fog::FogLut lut = createConstantFogLut(0.0f);
    lut[3].m = 0.0f;
    lut[3].b = 0.75f;
    fog.setFogLut(lut, 1.0f, 10000.0f);

    const Vec4 inputColor { 1.0f, 1.0f, 1.0f, 1.0f };

    // w = 8.0 -> log2(8) = 3.0 -> index 3, frac 0.0
    const Vec4 result = fog.calculateFog(8.0f, inputColor);

    // Factor 0.75 means 75% original + 25% fog
    const Vec4 expected { 0.75f, 0.75f, 0.75f, 1.0f };
    REQUIRE(rr::ut::vec4Approx(result, expected));
}

TEST_CASE("Fog LUT with slope interpolates within entry", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    fog.setFogColor(Vec4 { 0.0f, 0.0f, 0.0f, 1.0f });

    // Create a LUT where entry 3 interpolates from 1.0 to 0.5
    Fog::FogLut lut = createConstantFogLut(0.0f);
    lut[3].b = 1.0f; // Start at 1.0
    lut[3].m = -0.5f; // End at 0.5 (b + m*1.0 = 0.5)
    fog.setFogLut(lut, 1.0f, 10000.0f);

    const Vec4 inputColor { 1.0f, 1.0f, 1.0f, 1.0f };

    SECTION("At start of range (frac=0)")
    {
        // w = 8.0 -> log2(8) = 3.0 -> index 3, frac 0.0
        // factor = 1.0 + (-0.5 * 0.0) = 1.0
        const Vec4 result = fog.calculateFog(8.0f, inputColor);
        REQUIRE(rr::ut::vec4Approx(result, inputColor));
    }

    SECTION("At middle of range (frac=0.5)")
    {
        // w = 8 * sqrt(2) ≈ 11.31 -> log2 ≈ 3.5 -> index 3, frac 0.5
        // factor = 1.0 + (-0.5 * 0.5) = 0.75
        const float w = 8.0f * std::sqrt(2.0f);
        const Vec4 result = fog.calculateFog(w, inputColor);
        const Vec4 expected { 0.75f, 0.75f, 0.75f, 1.0f };
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }
}

TEST_CASE("Fog with different colors", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    fog.setFogLut(createConstantFogLut(0.0f), 1.0f, 1000.0f);

    const Vec4 inputColor { 1.0f, 0.5f, 0.25f, 1.0f };

    SECTION("White fog")
    {
        fog.setFogColor(Vec4 { 1.0f, 1.0f, 1.0f, 1.0f });
        const Vec4 result = fog.calculateFog(10.0f, inputColor);
        const Vec4 expected { 1.0f, 1.0f, 1.0f, 1.0f };
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("Black fog")
    {
        fog.setFogColor(Vec4 { 0.0f, 0.0f, 0.0f, 1.0f });
        const Vec4 result = fog.calculateFog(10.0f, inputColor);
        const Vec4 expected { 0.0f, 0.0f, 0.0f, 1.0f };
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("Blue fog")
    {
        fog.setFogColor(Vec4 { 0.0f, 0.0f, 1.0f, 1.0f });
        const Vec4 result = fog.calculateFog(10.0f, inputColor);
        const Vec4 expected { 0.0f, 0.0f, 1.0f, 1.0f };
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }
}
