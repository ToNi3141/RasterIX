#pragma once

#include <array>
#include <cstdint>

namespace labyrinth
{
static constexpr uint32_t TextureSize = 32;
using TexturePixels = std::array<uint8_t, TextureSize * TextureSize * 3>;

struct TextureImage
{
    uint32_t size = TextureSize;
    TexturePixels pixels {};
};

class WallTexture
{
public:
    static TextureImage create()
    {
        TextureImage textureImage;
        for (uint32_t y = 0; y < TextureSize; ++y)
        {
            for (uint32_t x = 0; x < TextureSize; ++x)
            {
                writePixel(textureImage.pixels, x, y);
            }
        }
        return textureImage;
    }

private:
    static void writePixel(TexturePixels& pixels, const uint32_t x, const uint32_t y)
    {
        const uint32_t brickY = y / 8;
        const uint32_t shiftedX = x + ((brickY & 1) != 0 ? 8 : 0);
        const bool mortar = (y % 8 == 0) || (shiftedX % 16 == 0);
        const uint32_t brickNoise = ((x * 13 + y * 19) & 15);
        const uint32_t offset = (y * TextureSize + x) * 3;
        pixels[offset + 0] = mortar ? 202 : static_cast<uint8_t>(142 + brickNoise);
        pixels[offset + 1] = mortar ? 198 : static_cast<uint8_t>(48 + brickNoise / 2);
        pixels[offset + 2] = mortar ? 188 : static_cast<uint8_t>(36 + brickNoise / 3);
    }
};

class FloorTexture
{
public:
    static TextureImage create()
    {
        TextureImage textureImage;
        for (uint32_t y = 0; y < TextureSize; ++y)
        {
            for (uint32_t x = 0; x < TextureSize; ++x)
            {
                writePixel(textureImage.pixels, x, y);
            }
        }
        return textureImage;
    }

private:
    static void writePixel(TexturePixels& pixels, const uint32_t x, const uint32_t y)
    {
        const uint32_t noise = ((x * 23 + y * 37 + ((x ^ y) * 11)) & 31);
        const uint32_t fineNoise = ((x * 5 + y * 3) & 7);
        const uint32_t offset = (y * TextureSize + x) * 3;
        pixels[offset + 0] = static_cast<uint8_t>(150 + noise);
        pixels[offset + 1] = static_cast<uint8_t>(126 + noise / 2 + fineNoise);
        pixels[offset + 2] = static_cast<uint8_t>(42 + noise / 4);
    }
};

class CeilingTexture
{
public:
    static TextureImage create()
    {
        TextureImage textureImage;
        for (uint32_t y = 0; y < TextureSize; ++y)
        {
            for (uint32_t x = 0; x < TextureSize; ++x)
            {
                writePixel(textureImage.pixels, x, y);
            }
        }
        return textureImage;
    }

private:
    static void writePixel(TexturePixels& pixels, const uint32_t x, const uint32_t y)
    {
        const bool mortar = (y % 8 == 0) || (x % 16 == ((y / 8) % 2) * 8);
        const uint32_t brickNoise = ((x * 17 + y * 11) & 15);
        const uint32_t offset = (y * TextureSize + x) * 3;
        pixels[offset + 0] = mortar ? 58 : static_cast<uint8_t>(112 + brickNoise);
        pixels[offset + 1] = mortar ? 55 : static_cast<uint8_t>(92 + brickNoise / 2);
        pixels[offset + 2] = mortar ? 55 : static_cast<uint8_t>(82 + brickNoise / 3);
    }
};

class LightMarkerTexture
{
public:
    static TextureImage create()
    {
        TextureImage textureImage;
        for (uint32_t i = 0; i < textureImage.pixels.size(); ++i)
        {
            textureImage.pixels[i] = 255;
        }
        return textureImage;
    }
};
} // namespace labyrinth
