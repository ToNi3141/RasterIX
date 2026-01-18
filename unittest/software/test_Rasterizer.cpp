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
#include "renderer/softwarerasterizer/Rasterizer.hpp"
#include <set>
#include <vector>

using namespace rr;
using namespace rr::softwarerasterizer;

// Helper to collect all hit fragments from a rasterizer run
std::vector<FragmentData> collectFragments(Rasterizer& rasterizer)
{
    std::vector<FragmentData> fragments;
    while (!rasterizer.isDone())
    {
        FragmentData frag = rasterizer.hit();
        if (frag.hit)
        {
            fragments.push_back(frag);
        }
        rasterizer.walk();
    }
    return fragments;
}

// Helper to collect unique pixel coordinates from fragments
std::set<std::pair<int32_t, int32_t>> collectPixelCoords(const std::vector<FragmentData>& fragments)
{
    std::set<std::pair<int32_t, int32_t>> coords;
    for (const auto& frag : fragments)
    {
        coords.insert({ frag.spx, frag.spy });
    }
    return coords;
}

// Create a simple triangle descriptor
// Edge function values are set up so that the triangle covers a specific region
TriangleStreamTypes::TriangleDesc createTriangleDesc(
    int32_t bbStartX, int32_t bbStartY, int32_t bbEndX, int32_t bbEndY,
    const Vec3i& wInit, const Vec3i& wXInc, const Vec3i& wYInc)
{
    TriangleStreamTypes::TriangleDesc desc {};
    desc.param.bbStartX = static_cast<uint16_t>(bbStartX);
    desc.param.bbStartY = static_cast<uint16_t>(bbStartY);
    desc.param.bbEndX = static_cast<uint16_t>(bbEndX);
    desc.param.bbEndY = static_cast<uint16_t>(bbEndY);
    desc.param.wInit = wInit;
    desc.param.wXInc = wXInc;
    desc.param.wYInc = wYInc;
    return desc;
}

TEST_CASE("Rasterizer simple single pixel triangle", "[Rasterizer]")
{
    ResolutionData resolution { 320, 240 };
    Rasterizer rasterizer(resolution);

    // Create a triangle that covers exactly one pixel at (5, 5)
    // All edge functions positive at (5,5), negative elsewhere in bounding box
    // wInit is evaluated at bbStart (5, 5)
    auto desc = createTriangleDesc(
        5, 5, 6, 6, // Bounding box: single pixel at (5,5)
        Vec3i { 1, 1, 1 }, // wInit: all positive at start
        Vec3i { -10, -10, -10 }, // wXInc: moving right makes it negative
        Vec3i { -10, -10, -10 }); // wYInc: moving down makes it negative

    rasterizer.init(desc);
    auto fragments = collectFragments(rasterizer);

    REQUIRE(fragments.size() == 1);
    REQUIRE(fragments[0].spx == 5);
    REQUIRE(fragments[0].spy == 5);
    REQUIRE(fragments[0].bbx == 0);
    REQUIRE(fragments[0].bby == 0);
}

TEST_CASE("Rasterizer 2x2 pixel square", "[Rasterizer]")
{
    ResolutionData resolution { 320, 240 };
    Rasterizer rasterizer(resolution);

    // Create a triangle that covers a 2x2 pixel area at (10,10) to (11,11)
    // Edge functions stay positive for all 4 pixels
    auto desc = createTriangleDesc(
        10, 10, 12, 12, // Bounding box: 2x2 pixels
        Vec3i { 100, 100, 100 }, // wInit: all positive
        Vec3i { 0, 0, 0 }, // wXInc: no change horizontally
        Vec3i { 0, 0, 0 }); // wYInc: no change vertically

    rasterizer.init(desc);
    auto fragments = collectFragments(rasterizer);
    auto coords = collectPixelCoords(fragments);

    REQUIRE(coords.size() == 4);
    REQUIRE(coords.count({ 10, 10 }) == 1);
    REQUIRE(coords.count({ 11, 10 }) == 1);
    REQUIRE(coords.count({ 10, 11 }) == 1);
    REQUIRE(coords.count({ 11, 11 }) == 1);
}

TEST_CASE("Rasterizer horizontal line of pixels", "[Rasterizer]")
{
    ResolutionData resolution { 320, 240 };
    Rasterizer rasterizer(resolution);

    // Triangle covers a horizontal line: (5,10), (6,10), (7,10)
    // wInit at (5, 10), stays positive for x in [5,7], negative for y != 10
    auto desc = createTriangleDesc(
        5, 10, 8, 11, // Bounding box: 3 pixels wide, 1 pixel tall
        Vec3i { 100, 100, 100 }, // wInit: all positive
        Vec3i { 0, 0, 0 }, // wXInc: stays positive across row
        Vec3i { -1000, -1000, -1000 }); // wYInc: negative when moving to next row

    rasterizer.init(desc);
    auto fragments = collectFragments(rasterizer);
    auto coords = collectPixelCoords(fragments);

    REQUIRE(coords.size() == 3);
    REQUIRE(coords.count({ 5, 10 }) == 1);
    REQUIRE(coords.count({ 6, 10 }) == 1);
    REQUIRE(coords.count({ 7, 10 }) == 1);
}

TEST_CASE("Rasterizer vertical line of pixels", "[Rasterizer]")
{
    // Use smaller resolution to speed up edge walking
    ResolutionData resolution { 20, 20 };
    Rasterizer rasterizer(resolution);

    // Triangle covers a vertical line: (10,5), (10,6), (10,7)
    auto desc = createTriangleDesc(
        10, 5, 11, 8, // Bounding box: 1 pixel wide, 3 pixels tall
        Vec3i { 100, 100, 100 }, // wInit: all positive
        Vec3i { -1000, -1000, -1000 }, // wXInc: negative when moving right
        Vec3i { 0, 0, 0 }); // wYInc: stays positive down the column

    rasterizer.init(desc);
    auto fragments = collectFragments(rasterizer);
    auto coords = collectPixelCoords(fragments);

    REQUIRE(coords.size() == 3);
    REQUIRE(coords.count({ 10, 5 }) == 1);
    REQUIRE(coords.count({ 10, 6 }) == 1);
    REQUIRE(coords.count({ 10, 7 }) == 1);
}

TEST_CASE("Rasterizer empty triangle - no pixels inside", "[Rasterizer]")
{
    ResolutionData resolution { 320, 240 };
    Rasterizer rasterizer(resolution);

    // Triangle where edge functions are all negative at start
    auto desc = createTriangleDesc(
        5, 5, 10, 10, // Bounding box
        Vec3i { -1, -1, -1 }, // wInit: all negative - no pixel inside
        Vec3i { 0, 0, 0 },
        Vec3i { 0, 0, 0 });

    rasterizer.init(desc);
    auto fragments = collectFragments(rasterizer);

    REQUIRE(fragments.empty());
}

TEST_CASE("Rasterizer degenerate zero-size bounding box", "[Rasterizer]")
{
    ResolutionData resolution { 320, 240 };
    Rasterizer rasterizer(resolution);

    // Zero-size bounding box (start == end)
    auto desc = createTriangleDesc(
        5, 5, 5, 5, // Empty bounding box
        Vec3i { 100, 100, 100 },
        Vec3i { 0, 0, 0 },
        Vec3i { 0, 0, 0 });

    rasterizer.init(desc);
    auto fragments = collectFragments(rasterizer);

    REQUIRE(fragments.empty());
}

TEST_CASE("Rasterizer triangle with one negative edge function", "[Rasterizer]")
{
    ResolutionData resolution { 320, 240 };
    Rasterizer rasterizer(resolution);

    // One edge function is negative - pixel should not be hit
    auto desc = createTriangleDesc(
        5, 5, 6, 6,
        Vec3i { 100, 100, -1 }, // Third edge function negative
        Vec3i { 0, 0, 0 },
        Vec3i { 0, 0, 0 });

    rasterizer.init(desc);
    auto fragments = collectFragments(rasterizer);

    REQUIRE(fragments.empty());
}

TEST_CASE("Rasterizer fragment index calculation", "[Rasterizer]")
{
    ResolutionData resolution { 320, 240 };
    Rasterizer rasterizer(resolution);

    // Single pixel at (100, 50)
    auto desc = createTriangleDesc(
        100, 50, 101, 51,
        Vec3i { 1, 1, 1 },
        Vec3i { -10, -10, -10 },
        Vec3i { -10, -10, -10 });

    rasterizer.init(desc);
    auto fragments = collectFragments(rasterizer);

    REQUIRE(fragments.size() == 1);
    // Index = ((yLineResolution - 1 - y) * resolutionX) + x
    // y is relative to yOffset (0 here), so y = 50
    // Index = ((240 - 1 - 50) * 320) + 100 = 189 * 320 + 100 = 60580
    std::size_t expectedIndex = ((240 - 1 - 50) * 320) + 100;
    REQUIRE(fragments[0].index == expectedIndex);
}

TEST_CASE("Rasterizer with y offset (tiled rendering)", "[Rasterizer]")
{
    ResolutionData resolution { 320, 120 }; // Half-height tile
    Rasterizer rasterizer(resolution);
    rasterizer.setYOffset(120); // Second tile starts at y=120

    // Triangle at screen position (10, 130) - within the second tile
    // wInit is evaluated at bbStart (10, 130)
    auto desc = createTriangleDesc(
        10, 130, 11, 131, // Screen coordinates
        Vec3i { 1, 1, 1 },
        Vec3i { -10, -10, -10 },
        Vec3i { -10, -10, -10 });

    rasterizer.init(desc);
    auto fragments = collectFragments(rasterizer);

    REQUIRE(fragments.size() == 1);
    REQUIRE(fragments[0].spx == 10);
    REQUIRE(fragments[0].spy == 130); // Screen Y coordinate
}

TEST_CASE("Rasterizer triangle clipped by tile end", "[Rasterizer]")
{
    // Use smaller resolution to speed up edge walking
    ResolutionData resolution { 20, 120 }; // Half-height tile
    Rasterizer rasterizer(resolution);
    rasterizer.setYOffset(0); // First tile starts at y=0

    // Triangle spans y=115 to y=125, but tile only covers y=0 to y=119
    // So only y=115 through y=119 should be rasterized
    auto desc = createTriangleDesc(
        10, 115, 11, 125, // y from 115 to 124 (exclusive end)
        Vec3i { 100, 100, 100 },
        Vec3i { -1000, -1000, -1000 },
        Vec3i { 0, 0, 0 });

    rasterizer.init(desc);
    auto fragments = collectFragments(rasterizer);
    auto coords = collectPixelCoords(fragments);

    // Only y=115 through y=119 are in this tile's range (5 pixels)
    REQUIRE(coords.size() == 5);
    REQUIRE(coords.count({ 10, 115 }) == 1);
    REQUIRE(coords.count({ 10, 116 }) == 1);
    REQUIRE(coords.count({ 10, 117 }) == 1);
    REQUIRE(coords.count({ 10, 118 }) == 1);
    REQUIRE(coords.count({ 10, 119 }) == 1);
}

TEST_CASE("Rasterizer diagonal pattern", "[Rasterizer]")
{
    // Use small resolution to speed up edge walking
    ResolutionData resolution { 10, 10 };
    Rasterizer rasterizer(resolution);

    // Create a diagonal pattern where pixels (0,0), (1,1), (2,2) are inside
    // wInit at (0, 0) = positive
    // Moving right decreases edge function, moving down increases it
    // So (0,0) inside, (1,0) outside, (0,1) outside, (1,1) inside, etc.
    auto desc = createTriangleDesc(
        0, 0, 3, 3,
        Vec3i { 10, 10, 10 }, // wInit at (0,0)
        Vec3i { -15, -15, -15 }, // Moving right decreases by 15
        Vec3i { 15, 15, 15 }); // Moving down increases by 15

    rasterizer.init(desc);
    auto fragments = collectFragments(rasterizer);
    auto coords = collectPixelCoords(fragments);

    // At (0,0): w = 10 >= 0 ✓
    // At (1,0): w = 10 - 15 = -5 < 0 ✗
    // At (2,0): w = 10 - 30 = -20 < 0 ✗
    // At (0,1): w = 10 + 15 = 25 >= 0 ✓
    // At (1,1): w = 10 - 15 + 15 = 10 >= 0 ✓
    // At (2,1): w = 10 - 30 + 15 = -5 < 0 ✗
    // At (0,2): w = 10 + 30 = 40 >= 0 ✓
    // At (1,2): w = 10 - 15 + 30 = 25 >= 0 ✓
    // At (2,2): w = 10 - 30 + 30 = 10 >= 0 ✓

    REQUIRE(coords.count({ 0, 0 }) == 1);
    REQUIRE(coords.count({ 1, 0 }) == 0);
    REQUIRE(coords.count({ 2, 0 }) == 0);
    REQUIRE(coords.count({ 0, 1 }) == 1);
    REQUIRE(coords.count({ 1, 1 }) == 1);
    REQUIRE(coords.count({ 2, 1 }) == 0);
    REQUIRE(coords.count({ 0, 2 }) == 1);
    REQUIRE(coords.count({ 1, 2 }) == 1);
    REQUIRE(coords.count({ 2, 2 }) == 1);
}

TEST_CASE("Rasterizer bbx and bby are relative to bounding box", "[Rasterizer]")
{
    ResolutionData resolution { 320, 240 };
    Rasterizer rasterizer(resolution);

    // Triangle at (50, 100) with 2x2 size
    auto desc = createTriangleDesc(
        50, 100, 52, 102,
        Vec3i { 100, 100, 100 },
        Vec3i { 0, 0, 0 },
        Vec3i { 0, 0, 0 });

    rasterizer.init(desc);
    auto fragments = collectFragments(rasterizer);

    REQUIRE(fragments.size() == 4);

    // Find fragment at screen position (50, 100)
    auto it = std::find_if(fragments.begin(), fragments.end(),
        [](const FragmentData& f)
        { return f.spx == 50 && f.spy == 100; });
    REQUIRE(it != fragments.end());
    REQUIRE(it->bbx == 0); // Relative to bbStartX
    REQUIRE(it->bby == 0); // Relative to bbStartY

    // Find fragment at screen position (51, 101)
    it = std::find_if(fragments.begin(), fragments.end(),
        [](const FragmentData& f)
        { return f.spx == 51 && f.spy == 101; });
    REQUIRE(it != fragments.end());
    REQUIRE(it->bbx == 1);
    REQUIRE(it->bby == 1);
}

TEST_CASE("Rasterizer edge on boundary (w == 0)", "[Rasterizer]")
{
    ResolutionData resolution { 320, 240 };
    Rasterizer rasterizer(resolution);

    // Edge function exactly zero should still be considered inside (>= 0)
    auto desc = createTriangleDesc(
        5, 5, 6, 6,
        Vec3i { 0, 0, 0 }, // All edge functions exactly zero
        Vec3i { 0, 0, 0 },
        Vec3i { 0, 0, 0 });

    rasterizer.init(desc);
    auto fragments = collectFragments(rasterizer);

    // w >= 0 should include w == 0
    REQUIRE(fragments.size() == 1);
    REQUIRE(fragments[0].spx == 5);
    REQUIRE(fragments[0].spy == 5);
}
