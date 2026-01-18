// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2023 ToNi3141

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

// #define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
// #include "../Unittests/3rdParty/catch.hpp"

#include "general.hpp"

// Include model header, generated from Verilating "top.v"
#include "VLodCalculator.h"

TEST_CASE("Check max lod", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->m_ready = 1;

    t->confEnable = true;

    t->s_textureSizeWidth = 8;
    t->s_textureSizeHeight = 8;

    t->s_texelS = 0x1 << (15 - 8);
    t->s_texelT = 0x1 << (15 - 8);
    t->s_texelSxy = 0x1 << (15 + 16);
    t->s_texelTxy = 0x1 << (15 + 16);
    rr::ut::clk(t);

    REQUIRE(t->m_lod == 8);
}

TEST_CASE("Check lod xy", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->m_ready = 1;

    t->confEnable = true;

    t->s_textureSizeWidth = 8;
    t->s_textureSizeHeight = 8;

    t->s_texelS = 0x1 << (15 - 8);
    t->s_texelT = 0x1 << (15 - 8);
    for (uint32_t i = 0; i < 8; i++)
    {
        t->s_texelSxy = 0x1 << (15 - i);
        t->s_texelTxy = 0x1 << (15 - i);

        rr::ut::clk(t);

        REQUIRE(t->m_lod == 8 - i);
    }
}

TEST_CASE("Check lod x", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->m_ready = 1;

    t->confEnable = true;

    t->s_textureSizeWidth = 8;
    t->s_textureSizeHeight = 8;

    t->s_texelS = 0x1 << (15 - 8);
    t->s_texelT = 0x1 << (15 - 8);

    for (uint32_t i = 0; i < 8; i++)
    {
        t->s_texelSxy = 0x1 << (15 - i);
        t->s_texelTxy = 0x1 << (15 - 8);

        rr::ut::clk(t);

        REQUIRE(t->m_lod == 8 - i);
    }
}

TEST_CASE("Check lod y", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->m_ready = 1;

    t->confEnable = true;

    t->s_textureSizeWidth = 8;
    t->s_textureSizeHeight = 8;

    t->s_texelS = 0x1 << (15 - 8);
    t->s_texelT = 0x1 << (15 - 8);

    for (uint32_t i = 0; i < 8; i++)
    {
        t->s_texelSxy = 0x1 << (15 - 8);
        t->s_texelTxy = 0x1 << (15 - i);

        rr::ut::clk(t);

        REQUIRE(t->m_lod == 8 - i);
    }
}

TEST_CASE("Check lod xy negative", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->m_ready = 1;

    t->confEnable = true;

    t->s_textureSizeWidth = 8;
    t->s_textureSizeHeight = 8;

    t->s_texelS = 0x80000000 | (0x1 << (15 - 8));
    t->s_texelT = 0x80000000 | (0x1 << (15 - 8));

    for (uint32_t i = 0; i < 8; i++)
    {
        t->s_texelSxy = 0x80000000 | (0x1 << (15 - i));
        t->s_texelTxy = 0x80000000 | (0x1 << (15 - i));

        rr::ut::clk(t);

        REQUIRE(t->m_lod == 8 - i);
    }
}

TEST_CASE("Check lod xy max with varying texture size", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->m_ready = 1;

    t->confEnable = true;

    for (uint32_t i = 0; i < 8; i++)
    {
        t->s_textureSizeWidth = i;
        t->s_textureSizeHeight = i;

        t->s_texelS = 0x1 << (15 - i);
        t->s_texelT = 0x1 << (15 - i);

        t->s_texelSxy = 0x1 << (15 + 0);
        t->s_texelTxy = 0x1 << (15 + 0);

        rr::ut::clk(t);

        REQUIRE(t->m_lod == i);
    }
}

TEST_CASE("Check lod xy 0 with varying texture size", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->m_ready = 1;

    t->confEnable = true;

    for (uint32_t i = 0; i < 8; i++)
    {
        t->s_textureSizeWidth = i;
        t->s_textureSizeHeight = i;

        t->s_texelS = 0x1 << (15 - i);
        t->s_texelT = 0x1 << (15 - i);

        t->s_texelSxy = 0x1 << (15 - i);
        t->s_texelTxy = 0x1 << (15 - i);

        rr::ut::clk(t);

        REQUIRE(t->m_lod == 0);
    }
}

TEST_CASE("Check lod xy 0 with varying values", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->m_ready = 1;

    t->confEnable = true;

    t->s_textureSizeWidth = 4;
    t->s_textureSizeHeight = 4;

    t->s_texelS = 0x1 << (15 - 4);
    t->s_texelT = 0x1 << (15 - 4);

    for (uint32_t i = 0; i < 16; i++)
    {
        t->s_texelSxy = (0x1 << (15 - 4)) + ((0x1 << (15 - 8)) * i);
        t->s_texelTxy = (0x1 << (15 - 4)) + ((0x1 << (15 - 8)) * i);
        rr::ut::clk(t);
        REQUIRE(t->m_lod == 0);
    }

    // Check that the overflow causes a LOD increment
    t->s_texelSxy = (0x1 << (15 - 4)) + ((0x1 << (15 - 8)) * 17);
    t->s_texelTxy = (0x1 << (15 - 4)) + ((0x1 << (15 - 8)) * 17);
    rr::ut::clk(t);
    REQUIRE(t->m_lod == 1);
}

TEST_CASE("Check lod xy when calculation is disabled", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->m_ready = 1;

    t->confEnable = false;

    for (uint32_t i = 0; i < 8; i++)
    {
        t->s_textureSizeWidth = i;
        t->s_textureSizeHeight = i;

        t->s_texelS = 0x1 << (15 - i);
        t->s_texelT = 0x1 << (15 - i);

        t->s_texelSxy = 0x1 << (15 + 0);
        t->s_texelTxy = 0x1 << (15 + 0);

        rr::ut::clk(t);

        REQUIRE(t->m_lod == 0);
    }
}

TEST_CASE("Check stall", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->m_ready = 1;

    t->confEnable = true;

    t->s_textureSizeWidth = 8;
    t->s_textureSizeHeight = 8;

    t->s_texelS = 0x1 << (15 - 8);
    t->s_texelT = 0x1 << (15 - 8);
    t->s_texelSxy = 0x1 << (15 + 16);
    t->s_texelTxy = 0x1 << (15 + 16);
    t->s_valid = 1;
    t->s_user = 1;
    rr::ut::clk(t);
    REQUIRE(t->m_lod == 8);
    REQUIRE(t->m_user == 1);
    REQUIRE(t->m_valid == 1);
    REQUIRE(t->s_ready == 1);

    t->s_texelS = 0;
    t->s_texelT = 0;
    t->s_texelSxy = 0;
    t->s_texelTxy = 0;
    t->m_ready = 0;
    t->s_valid = 0;
    t->s_user = 0;
    rr::ut::clk(t);
    REQUIRE(t->m_lod == 8); // Stays at 8
    REQUIRE(t->m_user == 1);
    REQUIRE(t->m_valid == 1);
    REQUIRE(t->s_ready == 0);

    t->m_ready = 1;
    rr::ut::clk(t);
    REQUIRE(t->m_lod == 0);
    REQUIRE(t->m_user == 0);
    REQUIRE(t->m_valid == 0);
    REQUIRE(t->s_ready == 1);
}