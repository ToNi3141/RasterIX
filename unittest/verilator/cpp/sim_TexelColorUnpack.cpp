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

#include "VTexelColorUnpack.h"

struct TexelColorUnpackTestCase
{
    uint8_t pixelFormat;
    uint16_t texel;
    uint32_t expected;
};

TEST_CASE("Unpack texture color formats", "[TexelColorUnpack]")
{
    constexpr uint8_t rgba4444 = 0;
    constexpr uint8_t rgba5551 = 1;
    constexpr uint8_t rgb565 = 2;

    const std::array testCases {
        TexelColorUnpackTestCase { rgba4444, 0x1234, 0x11223344 },
        TexelColorUnpackTestCase { rgba4444, 0xabcd, 0xaabbccdd },
        TexelColorUnpackTestCase { rgba5551, 0xf800, 0xff000000 },
        TexelColorUnpackTestCase { rgba5551, 0x07c0, 0x00ff0000 },
        TexelColorUnpackTestCase { rgba5551, 0x003e, 0x0000ff00 },
        TexelColorUnpackTestCase { rgba5551, 0x0001, 0x000000ff },
        TexelColorUnpackTestCase { rgb565, 0xf800, 0xff0000ff },
        TexelColorUnpackTestCase { rgb565, 0x07e0, 0x00ff00ff },
        TexelColorUnpackTestCase { rgb565, 0x001f, 0x0000ffff },
    };

    VTexelColorUnpack* top = rr::ut::makeTop<VTexelColorUnpack>();
    for (const auto& testCase : testCases)
    {
        top->confPixelFormat = testCase.pixelFormat;
        top->texelInput = testCase.texel;
        top->eval();
        REQUIRE(top->texelOutput == testCase.expected);
    }

    delete top;
}