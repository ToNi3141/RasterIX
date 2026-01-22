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
#include "renderer/softwarerasterizer/TextureMap.hpp"
#include <vector>

using namespace rr;
using namespace rr::softwarerasterizer;

// Helper class to create test textures
class TestTextureHelper
{
public:
    // Create a 2x2 texture with RGBA4444 format
    // Texel layout:
    //   (0,0) top-left     (1,0) top-right
    //   (0,1) bottom-left  (1,1) bottom-right
    static std::vector<uint8_t> create2x2TextureRGBA4444(
        uint16_t texel00, uint16_t texel10,
        uint16_t texel01, uint16_t texel11)
    {
        std::vector<uint8_t> data(8);
        // Row 0: texel00, texel10
        data[0] = texel00 & 0xFF;
        data[1] = (texel00 >> 8) & 0xFF;
        data[2] = texel10 & 0xFF;
        data[3] = (texel10 >> 8) & 0xFF;
        // Row 1: texel01, texel11
        data[4] = texel01 & 0xFF;
        data[5] = (texel01 >> 8) & 0xFF;
        data[6] = texel11 & 0xFF;
        data[7] = (texel11 >> 8) & 0xFF;
        return data;
    }

    // Create a 4x4 texture with RGBA4444 format
    static std::vector<uint8_t> create4x4TextureRGBA4444(const std::array<uint16_t, 16>& texels)
    {
        std::vector<uint8_t> data(32);
        for (std::size_t i = 0; i < 16; ++i)
        {
            data[i * 2] = texels[i] & 0xFF;
            data[i * 2 + 1] = (texels[i] >> 8) & 0xFF;
        }
        return data;
    }

    // Convert RGBA (0-15 each) to RGBA4444 format
    static uint16_t rgba4444(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        return ((r & 0xF) << 12) | ((g & 0xF) << 8) | ((b & 0xF) << 4) | (a & 0xF);
    }

    // Convert RGBA (0.0-1.0) to expected Vec4 from RGBA4444
    // RGBA4444 stores 4-bit values which get expanded to 8-bit by shifting left 4
    static Vec4 expectedColorRGBA4444(uint8_t r4, uint8_t g4, uint8_t b4, uint8_t a4)
    {
        constexpr float inv255 = 1.0f / 255.0f;
        return Vec4 {
            (r4 << 4) * inv255,
            (g4 << 4) * inv255,
            (b4 << 4) * inv255,
            (a4 << 4) * inv255
        };
    }
};

// Setup a simple texture map for testing
TextureMap createTestTextureMap(const tcb::span<const uint8_t> gram, float width, float height)
{
    TextureMap texMap;
    texMap.setGRAM(gram);
    texMap.setTextureSize(width, height);
    texMap.setPixelFormat(DevicePixelFormat::RGBA4444);
    texMap.setEnable(true);

    // Setup pages - simple identity mapping (page 0 at offset 0)
    std::array<uint32_t, RenderConfig::getMaxTexturePages()> pages {};
    pages[0] = 0;
    texMap.setPages(tcb::span<const uint32_t>(pages.data(), pages.size()));

    return texMap;
}

TEST_CASE("TextureMap disabled returns homogeneous vector", "[TextureMap]")
{
    auto textureData = TestTextureHelper::create2x2TextureRGBA4444(
        TestTextureHelper::rgba4444(15, 0, 0, 15),
        TestTextureHelper::rgba4444(0, 15, 0, 15),
        TestTextureHelper::rgba4444(0, 0, 15, 15),
        TestTextureHelper::rgba4444(15, 15, 15, 15));

    TextureMap texMap = createTestTextureMap(tcb::span<const uint8_t>(textureData), 2.0f, 2.0f);
    texMap.setEnable(false);

    Vec4 result = texMap.getTexel(0.25f, 0.25f);

    // Homogeneous vector is (0, 0, 0, 1)
    REQUIRE(rr::ut::vec4Approx(result, Vec4 { 0.0f, 0.0f, 0.0f, 1.0f }));
}

TEST_CASE("TextureMap unfiltered sampling at texel centers", "[TextureMap]")
{
    // Create 2x2 texture: Red, Green, Blue, White
    auto textureData = TestTextureHelper::create2x2TextureRGBA4444(
        TestTextureHelper::rgba4444(15, 0, 0, 15), // (0,0) Red
        TestTextureHelper::rgba4444(0, 15, 0, 15), // (1,0) Green
        TestTextureHelper::rgba4444(0, 0, 15, 15), // (0,1) Blue
        TestTextureHelper::rgba4444(15, 15, 15, 15)); // (1,1) White

    TextureMap texMap = createTestTextureMap(tcb::span<const uint8_t>(textureData), 2.0f, 2.0f);
    texMap.setEnableMagFilter(false);
    texMap.setWrapMode(TextureWrapMode::REPEAT, TextureWrapMode::REPEAT);

    SECTION("Sample texel (0,0) - Red")
    {
        Vec4 result = texMap.getTexel(0.25f, 0.25f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(15, 0, 0, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("Sample texel (1,0) - Green")
    {
        Vec4 result = texMap.getTexel(0.75f, 0.25f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(0, 15, 0, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("Sample texel (0,1) - Blue")
    {
        Vec4 result = texMap.getTexel(0.25f, 0.75f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(0, 0, 15, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("Sample texel (1,1) - White")
    {
        Vec4 result = texMap.getTexel(0.75f, 0.75f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(15, 15, 15, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }
}

TEST_CASE("TextureMap wrap mode REPEAT", "[TextureMap]")
{
    auto textureData = TestTextureHelper::create2x2TextureRGBA4444(
        TestTextureHelper::rgba4444(15, 0, 0, 15), // Red
        TestTextureHelper::rgba4444(0, 15, 0, 15), // Green
        TestTextureHelper::rgba4444(0, 0, 15, 15), // Blue
        TestTextureHelper::rgba4444(15, 15, 15, 15)); // White

    TextureMap texMap = createTestTextureMap(tcb::span<const uint8_t>(textureData), 2.0f, 2.0f);
    texMap.setEnableMagFilter(false);
    texMap.setWrapMode(TextureWrapMode::REPEAT, TextureWrapMode::REPEAT);

    SECTION("s > 1.0 wraps")
    {
        // s=1.25 should wrap to s=0.25 -> texel (0,0)
        Vec4 result = texMap.getTexel(1.25f, 0.25f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(15, 0, 0, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("t > 1.0 wraps")
    {
        // t=1.75 should wrap to t=0.75 -> texel (0,1)
        Vec4 result = texMap.getTexel(0.25f, 1.75f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(0, 0, 15, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("s < 0.0 wraps")
    {
        // s=-0.25 should wrap to s=0.75 -> texel (1,0)
        Vec4 result = texMap.getTexel(-0.25f, 0.25f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(0, 15, 0, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("t < 0.0 wraps")
    {
        // t=-0.25 should wrap to t=0.75 -> texel (0,1)
        Vec4 result = texMap.getTexel(0.25f, -0.25f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(0, 0, 15, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }
}

TEST_CASE("TextureMap wrap mode REPEAT edge cases", "[TextureMap]")
{
    // 2x2 texture layout:
    // (0,0) Red     (1,0) Green    <- t in [0.0, 0.5)
    // (0,1) Blue    (1,1) White    <- t in [0.5, 1.0)
    //   ^             ^
    //   s in [0,0.5)  s in [0.5,1.0)

    auto textureData = TestTextureHelper::create2x2TextureRGBA4444(
        TestTextureHelper::rgba4444(15, 0, 0, 15), // (0,0) Red
        TestTextureHelper::rgba4444(0, 15, 0, 15), // (1,0) Green
        TestTextureHelper::rgba4444(0, 0, 15, 15), // (0,1) Blue
        TestTextureHelper::rgba4444(15, 15, 15, 15)); // (1,1) White

    TextureMap texMap = createTestTextureMap(tcb::span<const uint8_t>(textureData), 2.0f, 2.0f);
    texMap.setEnableMagFilter(false);
    texMap.setWrapMode(TextureWrapMode::REPEAT, TextureWrapMode::REPEAT);

    SECTION("s=1.0 wraps to s=0.0 -> texel (0,0) Red")
    {
        Vec4 result = texMap.getTexel(1.0f, 0.25f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(15, 0, 0, 15);
        INFO("result: " << result[0] << ", " << result[1] << ", " << result[2] << ", " << result[3]);
        INFO("expected: " << expected[0] << ", " << expected[1] << ", " << expected[2] << ", " << expected[3]);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("t=1.0 wraps to t=0.0 -> texel (0,0) Red")
    {
        Vec4 result = texMap.getTexel(0.25f, 1.0f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(15, 0, 0, 15);
        INFO("result: " << result[0] << ", " << result[1] << ", " << result[2] << ", " << result[3]);
        INFO("expected: " << expected[0] << ", " << expected[1] << ", " << expected[2] << ", " << expected[3]);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("s=2.0 wraps to s=0.0 -> texel (0,0) Red")
    {
        Vec4 result = texMap.getTexel(2.0f, 0.25f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(15, 0, 0, 15);
        INFO("result: " << result[0] << ", " << result[1] << ", " << result[2] << ", " << result[3]);
        INFO("expected: " << expected[0] << ", " << expected[1] << ", " << expected[2] << ", " << expected[3]);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("s=1.75 wraps to s=0.75 -> texel (1,0) Green")
    {
        Vec4 result = texMap.getTexel(1.75f, 0.25f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(0, 15, 0, 15);
        INFO("result: " << result[0] << ", " << result[1] << ", " << result[2] << ", " << result[3]);
        INFO("expected: " << expected[0] << ", " << expected[1] << ", " << expected[2] << ", " << expected[3]);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("s=0.0, t=0.0 -> texel (0,0) Red")
    {
        Vec4 result = texMap.getTexel(0.0f, 0.0f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(15, 0, 0, 15);
        INFO("result: " << result[0] << ", " << result[1] << ", " << result[2] << ", " << result[3]);
        INFO("expected: " << expected[0] << ", " << expected[1] << ", " << expected[2] << ", " << expected[3]);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("s=1.0, t=1.0 wraps to (0,0) Red")
    {
        Vec4 result = texMap.getTexel(1.0f, 1.0f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(15, 0, 0, 15);
        INFO("result: " << result[0] << ", " << result[1] << ", " << result[2] << ", " << result[3]);
        INFO("expected: " << expected[0] << ", " << expected[1] << ", " << expected[2] << ", " << expected[3]);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }
}

TEST_CASE("TextureMap wrap mode CLAMP_TO_EDGE", "[TextureMap]")
{
    auto textureData = TestTextureHelper::create2x2TextureRGBA4444(
        TestTextureHelper::rgba4444(15, 0, 0, 15), // Red
        TestTextureHelper::rgba4444(0, 15, 0, 15), // Green
        TestTextureHelper::rgba4444(0, 0, 15, 15), // Blue
        TestTextureHelper::rgba4444(15, 15, 15, 15)); // White

    TextureMap texMap = createTestTextureMap(tcb::span<const uint8_t>(textureData), 2.0f, 2.0f);
    texMap.setEnableMagFilter(false);
    texMap.setWrapMode(TextureWrapMode::CLAMP_TO_EDGE, TextureWrapMode::CLAMP_TO_EDGE);

    // CLAMP_TO_EDGE correctly clamps to the last texel at the edge

    SECTION("s > 1.0 clamps to right edge texel")
    {
        // s=1.5 clamps to edge -> texel (1,0) Green
        Vec4 result = texMap.getTexel(1.5f, 0.25f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(0, 15, 0, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("t > 1.0 clamps to bottom edge texel")
    {
        // t=1.5 clamps to edge -> texel (0,1) Blue
        Vec4 result = texMap.getTexel(0.25f, 1.5f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(0, 0, 15, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("s < 0.0 clamps to left edge")
    {
        // s=-0.5 clamps to s=0.0 -> texel (0,0) Red
        Vec4 result = texMap.getTexel(-0.5f, 0.25f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(15, 0, 0, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("t < 0.0 clamps to top edge")
    {
        // t=-0.5 clamps to t=0.0 -> texel (0,0) Red
        Vec4 result = texMap.getTexel(0.25f, -0.5f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(15, 0, 0, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("s and t > 1.0 clamps to bottom-right corner")
    {
        // Both clamp to edge -> texel (1,1) White
        Vec4 result = texMap.getTexel(1.5f, 1.5f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(15, 15, 15, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }
}

TEST_CASE("TextureMap bilinear filtering", "[TextureMap]")
{
    // Create 2x2 texture with distinct colors for interpolation testing
    // Black (0,0), White (1,0), Black (0,1), White (1,1)
    auto textureData = TestTextureHelper::create2x2TextureRGBA4444(
        TestTextureHelper::rgba4444(0, 0, 0, 15), // (0,0) Black
        TestTextureHelper::rgba4444(15, 15, 15, 15), // (1,0) White
        TestTextureHelper::rgba4444(0, 0, 0, 15), // (0,1) Black
        TestTextureHelper::rgba4444(15, 15, 15, 15)); // (1,1) White

    TextureMap texMap = createTestTextureMap(tcb::span<const uint8_t>(textureData), 2.0f, 2.0f);
    texMap.setEnableMagFilter(true);
    texMap.setWrapMode(TextureWrapMode::REPEAT, TextureWrapMode::REPEAT);

    SECTION("Filtering at texel center returns texel color")
    {
        // When filtering is enabled, getTexel subtracts half texel size
        // So to sample at texel (0,0), we need to account for that
        // For a 2x2 texture, half texel = 0.25
        // Sample point 0.25 becomes 0.0 after subtraction
        Vec4 result = texMap.getTexel(0.25f, 0.25f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(0, 0, 0, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected, 0.1f));
    }

    SECTION("Filtering interpolates between texels horizontally")
    {
        // Sample between texel (0,0) black and texel (1,0) white
        // At s=0.5, we expect 50% blend
        Vec4 result = texMap.getTexel(0.5f, 0.25f);
        // Should be approximately gray
        REQUIRE(result[0] > 0.3f);
        REQUIRE(result[0] < 0.7f);
    }
}

TEST_CASE("TextureMap different pixel formats", "[TextureMap]")
{
    SECTION("RGB565 format")
    {
        // RGB565: RRRRR GGGGGG BBBBB
        // Red = 31, Green = 63, Blue = 31 -> white-ish
        uint16_t whiteTexel = 0xFFFF; // All 1s = max R, G, B
        std::vector<uint8_t> data = { static_cast<uint8_t>(whiteTexel & 0xFF),
            static_cast<uint8_t>((whiteTexel >> 8) & 0xFF),
            static_cast<uint8_t>(whiteTexel & 0xFF),
            static_cast<uint8_t>((whiteTexel >> 8) & 0xFF),
            static_cast<uint8_t>(whiteTexel & 0xFF),
            static_cast<uint8_t>((whiteTexel >> 8) & 0xFF),
            static_cast<uint8_t>(whiteTexel & 0xFF),
            static_cast<uint8_t>((whiteTexel >> 8) & 0xFF) };

        TextureMap texMap = createTestTextureMap(tcb::span<const uint8_t>(data), 2.0f, 2.0f);
        texMap.setPixelFormat(DevicePixelFormat::RGB565);
        texMap.setEnableMagFilter(false);

        Vec4 result = texMap.getTexel(0.25f, 0.25f);

        // RGB565 white has alpha = 1.0 always
        REQUIRE(result[0] > 0.9f); // R
        REQUIRE(result[1] > 0.9f); // G
        REQUIRE(result[2] > 0.9f); // B
        REQUIRE(result[3] == Approx(1.0f)); // A always 1.0 for RGB565
    }

    SECTION("RGBA5551 format")
    {
        // RGBA5551: RRRRR GGGGG BBBBB A
        // All 1s = max R, G, B, A=1
        uint16_t whiteTexel = 0xFFFF;
        std::vector<uint8_t> data = { static_cast<uint8_t>(whiteTexel & 0xFF),
            static_cast<uint8_t>((whiteTexel >> 8) & 0xFF),
            static_cast<uint8_t>(whiteTexel & 0xFF),
            static_cast<uint8_t>((whiteTexel >> 8) & 0xFF),
            static_cast<uint8_t>(whiteTexel & 0xFF),
            static_cast<uint8_t>((whiteTexel >> 8) & 0xFF),
            static_cast<uint8_t>(whiteTexel & 0xFF),
            static_cast<uint8_t>((whiteTexel >> 8) & 0xFF) };

        TextureMap texMap = createTestTextureMap(tcb::span<const uint8_t>(data), 2.0f, 2.0f);
        texMap.setPixelFormat(DevicePixelFormat::RGBA5551);
        texMap.setEnableMagFilter(false);

        Vec4 result = texMap.getTexel(0.25f, 0.25f);

        REQUIRE(result[0] > 0.9f); // R
        REQUIRE(result[1] > 0.9f); // G
        REQUIRE(result[2] > 0.9f); // B
        REQUIRE(result[3] == Approx(1.0f)); // A = 1 bit set
    }
}

TEST_CASE("TextureMap 4x4 texture sampling", "[TextureMap]")
{
    // Create a 4x4 texture with a gradient pattern
    std::array<uint16_t, 16> texels {};
    for (std::size_t i = 0; i < 16; ++i)
    {
        uint8_t val = static_cast<uint8_t>(i);
        texels[i] = TestTextureHelper::rgba4444(val, val, val, 15);
    }
    auto textureData = TestTextureHelper::create4x4TextureRGBA4444(texels);

    TextureMap texMap = createTestTextureMap(tcb::span<const uint8_t>(textureData), 4.0f, 4.0f);
    texMap.setEnableMagFilter(false);
    texMap.setWrapMode(TextureWrapMode::REPEAT, TextureWrapMode::REPEAT);

    SECTION("Sample texel (0,0)")
    {
        Vec4 result = texMap.getTexel(0.125f, 0.125f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(0, 0, 0, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("Sample texel (3,0)")
    {
        Vec4 result = texMap.getTexel(0.875f, 0.125f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(3, 3, 3, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("Sample texel (0,3)")
    {
        Vec4 result = texMap.getTexel(0.125f, 0.875f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(12, 12, 12, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("Sample texel (3,3)")
    {
        Vec4 result = texMap.getTexel(0.875f, 0.875f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(15, 15, 15, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }
}

TEST_CASE("TextureMap exact boundary sampling", "[TextureMap]")
{
    auto textureData = TestTextureHelper::create2x2TextureRGBA4444(
        TestTextureHelper::rgba4444(15, 0, 0, 15),
        TestTextureHelper::rgba4444(0, 15, 0, 15),
        TestTextureHelper::rgba4444(0, 0, 15, 15),
        TestTextureHelper::rgba4444(15, 15, 15, 15));

    TextureMap texMap = createTestTextureMap(tcb::span<const uint8_t>(textureData), 2.0f, 2.0f);
    texMap.setEnableMagFilter(false);
    texMap.setWrapMode(TextureWrapMode::REPEAT, TextureWrapMode::REPEAT);

    SECTION("Sample at s=0.0, t=0.0")
    {
        Vec4 result = texMap.getTexel(0.0f, 0.0f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(15, 0, 0, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("Sample at s=0.5, t=0.0")
    {
        Vec4 result = texMap.getTexel(0.5f, 0.0f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(0, 15, 0, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }

    SECTION("Sample at s=0.0, t=0.5")
    {
        Vec4 result = texMap.getTexel(0.0f, 0.5f);
        Vec4 expected = TestTextureHelper::expectedColorRGBA4444(0, 0, 15, 15);
        REQUIRE(rr::ut::vec4Approx(result, expected));
    }
}
