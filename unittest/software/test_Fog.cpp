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
#include <cmath>

using namespace rr;
using namespace rr::softwarerasterizer;

// Helper functions to convert float values to the fixed-point format used by the fog LUT
static int32_t floatToLutM(float m)
{
    return static_cast<int32_t>(m * std::pow(2.0f, 14.0f));
}

static int32_t floatToLutB(float b)
{
    return static_cast<int32_t>(b * std::pow(2.0f, 22.0f));
}

// Helper to create a simple linear fog LUT
// Maps recip_w values to fog factors linearly within the LUT range
Fog::FogLut createLinearFogLut()
{
    Fog::FogLut lut {};
    // Create a LUT where each entry linearly decreases fog factor
    // Entry i covers recip_w values from 2^-(i+1) to 2^-i
    // b = factor at near end (xs=0, high recip_w = close), factor at far end (xs=256, low recip_w = far) = b + m*256
    for (std::size_t i = 0; i < lut.size(); ++i)
    {
        // Linear interpolation: factor goes from 1.0 to 0.0 as index increases
        float startFactor = 1.0f - (static_cast<float>(i) / static_cast<float>(lut.size()));
        float endFactor = 1.0f - (static_cast<float>(i + 1) / static_cast<float>(lut.size()));
        lut[i].b = floatToLutB(startFactor);
        lut[i].m = floatToLutM(endFactor - startFactor);
    }
    return lut;
}

// Helper to create a constant fog LUT (all entries return the same factor)
Fog::FogLut createConstantFogLut(float factor)
{
    Fog::FogLut lut {};
    for (auto& entry : lut)
    {
        entry.m = floatToLutM(0.0f);
        entry.b = floatToLutB(factor);
    }
    return lut;
}

TEST_CASE("Fog disabled returns original color", "[Fog]")
{
    Fog fog;
    fog.setEnable(false);
    fog.setFogColor(Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.5f, 0.5f, 0.5f, 1.0f }));

    const Vec4iColorRGBA inputColor = Vec4iColorRGBA::createFromVecToInt(Vec4 { 1.0f, 0.0f, 0.0f, 1.0f });
    const Vec4iColorRGBA result = fog.calculateFog(0.01f, inputColor);

    REQUIRE(rr::ut::vec4i16Approx(result, inputColor, 2));
}

TEST_CASE("Fog enabled with recip_w above lower bound returns original color", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    fog.setFogColor(Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.5f, 0.5f, 0.5f, 1.0f }));
    fog.setFogLut(createConstantFogLut(0.0f));

    const Vec4iColorRGBA inputColor = Vec4iColorRGBA::createFromVecToInt(Vec4 { 1.0f, 0.0f, 0.0f, 1.0f });
    // recip_w = 1.0 >= lower bound 0.5 (2^-1), so factor should be 1.0 (no fog)
    const Vec4iColorRGBA result = fog.calculateFog(1.0f, inputColor);

    REQUIRE(rr::ut::vec4i16Approx(result, inputColor, 2));
}

TEST_CASE("Fog enabled with recip_w below upper bound returns fog color", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    fog.setFogColor(Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.5f, 0.5f, 0.5f, 1.0f }));
    fog.setFogLut(createConstantFogLut(0.5f));

    const Vec4iColorRGBA inputColor = Vec4iColorRGBA::createFromVecToInt(Vec4 { 1.0f, 0.0f, 0.0f, 0.8f });
    // recip_w = 2^-33 <= upper bound 2^-32, so factor should be 0.0 (full fog)
    const Vec4iColorRGBA result = fog.calculateFog(std::pow(2.0f, -33.0f), inputColor);

    // Full fog means result should be fog color but with original alpha
    const Vec4iColorRGBA fogColor = Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.5f, 0.5f, 0.5f, 1.0f });
    const Vec4iColorRGBA expected { fogColor[0], fogColor[1], fogColor[2], inputColor[3] };
    REQUIRE(rr::ut::vec4i16Approx(result, expected, 2));
}

TEST_CASE("Fog preserves alpha channel", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    fog.setFogColor(Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.0f, 0.0f, 0.0f, 0.0f }));
    fog.setFogLut(createConstantFogLut(0.5f));

    SECTION("Alpha 1.0 preserved")
    {
        const Vec4iColorRGBA inputColor = Vec4iColorRGBA::createFromVecToInt(Vec4 { 1.0f, 1.0f, 1.0f, 1.0f });
        const Vec4iColorRGBA result = fog.calculateFog(0.1f, inputColor);
        REQUIRE(result[3] == Approx(inputColor[3]).margin(2));
    }

    SECTION("Alpha 0.5 preserved")
    {
        const Vec4iColorRGBA inputColor = Vec4iColorRGBA::createFromVecToInt(Vec4 { 1.0f, 1.0f, 1.0f, 0.5f });
        const Vec4iColorRGBA result = fog.calculateFog(0.1f, inputColor);
        REQUIRE(result[3] == Approx(inputColor[3]).margin(2));
    }

    SECTION("Alpha 0.0 preserved")
    {
        const Vec4iColorRGBA inputColor = Vec4iColorRGBA::createFromVecToInt(Vec4 { 1.0f, 1.0f, 1.0f, 0.0f });
        const Vec4iColorRGBA result = fog.calculateFog(0.1f, inputColor);
        REQUIRE(result[3] == Vec4iColorRGBA::Zero);
    }
}

TEST_CASE("Fog factor 1.0 returns original color (RGB)", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    fog.setFogColor(Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.0f, 0.0f, 0.0f, 1.0f }));
    fog.setFogLut(createConstantFogLut(1.0f));

    const Vec4iColorRGBA inputColor = Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.8f, 0.6f, 0.4f, 1.0f });
    const Vec4iColorRGBA result = fog.calculateFog(0.1f, inputColor);

    // Factor 1.0 means full original color
    REQUIRE(rr::ut::vec4i16Approx(result, inputColor, 2));
}

TEST_CASE("Fog factor 0.0 returns fog color (RGB)", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    fog.setFogColor(Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.3f, 0.3f, 0.3f, 1.0f }));
    fog.setFogLut(createConstantFogLut(0.0f));

    const Vec4iColorRGBA inputColor = Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.8f, 0.6f, 0.4f, 0.7f });
    const Vec4iColorRGBA result = fog.calculateFog(0.1f, inputColor);

    // Factor 0.0 means full fog color, but alpha preserved
    const Vec4iColorRGBA fogColor = Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.3f, 0.3f, 0.3f, 1.0f });
    const Vec4iColorRGBA expected { fogColor[0], fogColor[1], fogColor[2], inputColor[3] };
    REQUIRE(rr::ut::vec4i16Approx(result, expected, 2));
}

TEST_CASE("Fog factor 0.5 blends colors equally", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    fog.setFogColor(Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.0f, 0.0f, 0.0f, 1.0f }));
    fog.setFogLut(createConstantFogLut(0.5f));

    const Vec4iColorRGBA inputColor = Vec4iColorRGBA::createFromVecToInt(Vec4 { 1.0f, 1.0f, 1.0f, 1.0f });
    const Vec4iColorRGBA result = fog.calculateFog(0.1f, inputColor);

    // Factor 0.5 means 50% blend: (0.5 * fogColor) + (0.5 * inputColor)
    const Vec4iColorRGBA expected = Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.5f, 0.5f, 0.5f, 1.0f });
    REQUIRE(rr::ut::vec4i16Approx(result, expected, 2));
}

TEST_CASE("Fog result is clamped to [0,1]", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    // Use extreme fog color values
    fog.setFogColor(Vec4iColorRGBA::createFromVecToInt(Vec4 { 2.0f, -1.0f, 1.5f, 1.0f }));
    fog.setFogLut(createConstantFogLut(0.0f));

    const Vec4iColorRGBA inputColor = Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.5f, 0.5f, 0.5f, 1.0f });
    const Vec4iColorRGBA result = fog.calculateFog(0.1f, inputColor);

    // Result should be clamped
    REQUIRE(result[0] >= Vec4iColorRGBA::Zero);
    REQUIRE(result[0] <= Vec4iColorRGBA::FracMax);
    REQUIRE(result[1] >= Vec4iColorRGBA::Zero);
    REQUIRE(result[1] <= Vec4iColorRGBA::FracMax);
    REQUIRE(result[2] >= Vec4iColorRGBA::Zero);
    REQUIRE(result[2] <= Vec4iColorRGBA::FracMax);
}

TEST_CASE("Fog LUT interpolation uses log2 of w", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    fog.setFogColor(Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.0f, 0.0f, 0.0f, 1.0f }));

    // Create a LUT where entry 3 (recip_w in range 8-16) returns factor 0.75
    Fog::FogLut lut = createConstantFogLut(0.0f);
    lut[3].m = floatToLutM(0.0f);
    lut[3].b = floatToLutB(0.75f);
    fog.setFogLut(lut);

    const Vec4iColorRGBA inputColor = Vec4iColorRGBA::createFromVecToInt(Vec4 { 1.0f, 1.0f, 1.0f, 1.0f });

    // recip_w = 0.0625 = 2^-4 -> index = 126 - 123 = 3, frac 0.0
    const Vec4iColorRGBA result = fog.calculateFog(0.0625f, inputColor);

    // Factor 0.75 means 75% original + 25% fog
    const Vec4iColorRGBA expected = Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.75f, 0.75f, 0.75f, 1.0f });
    REQUIRE(rr::ut::vec4i16Approx(result, expected, 2));
}

TEST_CASE("Fog LUT with slope interpolates within entry", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    fog.setFogColor(Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.0f, 0.0f, 0.0f, 1.0f }));

    // Create a LUT where entry 3 interpolates: b=0.5, m=0.5
    // xs = 256 - (mantissa >> 15), so factor = (m*xs + b) >> 14
    // At xs=256 (recip_w = 2^-4, mantissa=0): factor = (0.5*256 + 0.5) scaled = 1.0
    // At xs=128 (recip_w = 1.5*2^-4, mantissa=0x400000): factor = 0.75
    Fog::FogLut lut = createConstantFogLut(0.0f);
    lut[3].b = floatToLutB(0.5f);
    lut[3].m = floatToLutM(0.5f);
    fog.setFogLut(lut);

    const Vec4iColorRGBA inputColor = Vec4iColorRGBA::createFromVecToInt(Vec4 { 1.0f, 1.0f, 1.0f, 1.0f });

    SECTION("At start of range (xs=256)")
    {
        // recip_w = 0.0625 = 2^-4 -> index 3, mantissa=0, xs=256, factor = 1.0
        const Vec4iColorRGBA result = fog.calculateFog(0.0625f, inputColor);
        REQUIRE(rr::ut::vec4i16Approx(result, inputColor, 2));
    }

    SECTION("At middle of range (xs=128)")
    {
        // recip_w = 0.09375 = 1.5 * 2^-4 -> index 3, mantissa=0x400000, xs=128, factor = 0.75
        const float w = 0.09375f;
        const Vec4iColorRGBA result = fog.calculateFog(w, inputColor);
        const Vec4iColorRGBA expected = Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.75f, 0.75f, 0.75f, 1.0f });
        REQUIRE(rr::ut::vec4i16Approx(result, expected, 2));
    }
}

TEST_CASE("Fog with different colors", "[Fog]")
{
    Fog fog;
    fog.setEnable(true);
    fog.setFogLut(createConstantFogLut(0.0f));

    const Vec4iColorRGBA inputColor = Vec4iColorRGBA::createFromVecToInt(Vec4 { 1.0f, 0.5f, 0.25f, 1.0f });

    SECTION("White fog")
    {
        fog.setFogColor(Vec4iColorRGBA::createFromVecToInt(Vec4 { 1.0f, 1.0f, 1.0f, 1.0f }));
        const Vec4iColorRGBA result = fog.calculateFog(0.1f, inputColor);
        const Vec4iColorRGBA expected = Vec4iColorRGBA::createFromVecToInt(Vec4 { 1.0f, 1.0f, 1.0f, 1.0f });
        REQUIRE(rr::ut::vec4i16Approx(result, expected, 2));
    }

    SECTION("Black fog")
    {
        fog.setFogColor(Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.0f, 0.0f, 0.0f, 1.0f }));
        const Vec4iColorRGBA result = fog.calculateFog(0.1f, inputColor);
        const Vec4iColorRGBA expected = Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.0f, 0.0f, 0.0f, 1.0f });
        REQUIRE(rr::ut::vec4i16Approx(result, expected, 2));
    }

    SECTION("Blue fog")
    {
        fog.setFogColor(Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.0f, 0.0f, 1.0f, 1.0f }));
        const Vec4iColorRGBA result = fog.calculateFog(0.1f, inputColor);
        const Vec4iColorRGBA expected = Vec4iColorRGBA::createFromVecToInt(Vec4 { 0.0f, 0.0f, 1.0f, 1.0f });
        REQUIRE(rr::ut::vec4i16Approx(result, expected, 2));
    }
}
