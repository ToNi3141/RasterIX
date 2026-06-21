#pragma once

#include <cstdint>

namespace labyrinth
{
struct Cell
{
    uint8_t row;
    uint8_t col;
};

struct Vec2
{
    float x;
    float y;
};

struct Vec3
{
    float x;
    float y;
    float z;
};

struct Vec3RGB
{
    float r;
    float g;
    float b;
};
} // namespace labyrinth
