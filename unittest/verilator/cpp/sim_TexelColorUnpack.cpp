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