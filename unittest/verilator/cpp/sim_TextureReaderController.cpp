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

#include <algorithm>
#include <array>
#include <math.h>

// Include model header, generated from Verilating "top.v"
#include "VTextureReaderController.h"

void makeCacheHot(VTextureReaderController* t,
    const std::array<uint32_t, 4>& texels)
{
    t->invalidate = 1;
    rr::ut::clk(t);

    t->invalidate = 0;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = texels[0];
    t->s_tr_texel_01 = texels[1];
    t->s_tr_texel_10 = texels[2];
    t->s_tr_texel_11 = texels[3];
    rr::ut::clk(t);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (texels[0] << 1));
    CHECK(t->s_tr_ready == 0);

    t->m_tr_ready = 1;
    t->s_tr_valid = 0;
    rr::ut::clk(t);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (texels[1] << 1));
    CHECK(t->s_tr_ready == 0);

    t->m_tr_ready = 1;
    rr::ut::clk(t);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (texels[2] << 1));
    CHECK(t->s_tr_ready == 0);

    t->m_tr_ready = 1;
    rr::ut::clk(t);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (texels[3] << 1));
    CHECK(t->s_tr_ready == 1);

    t->m_tr_ready = 1;
    rr::ut::clk(t);
    CHECK(t->m_tr_valid == 0);
    CHECK(t->s_tr_ready == 1);
}

TEST_CASE("Test Cold Cache, no stalling", "[TextureReaderController]")
{
    VTextureReaderController* t = rr::ut::makeTop<VTextureReaderController>();
    rr::ut::reset(t);
    t->nearest = 0;

    t->invalidate = 0;
    CHECK(t->m_tr_valid == 0);
    CHECK(t->s_tr_ready == 1);

    t->m_tr_ready = 1;
    t->s_tr_texel_00 = 10;
    t->s_tr_texel_01 = 20;
    t->s_tr_texel_10 = 30;
    t->s_tr_texel_11 = 40;
    t->s_tr_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b00);
    CHECK(t->m_tr_cmd == 0);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (10 << 1));
    CHECK(t->s_tr_ready == 0);

    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b01);
    CHECK(t->m_tr_cmd == 0);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (20 << 1));
    CHECK(t->s_tr_ready == 0);

    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b10);
    CHECK(t->m_tr_cmd == 0);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (30 << 1));
    CHECK(t->s_tr_ready == 0);

    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b11);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (40 << 1));
    CHECK(t->s_tr_ready == 1);

    // New cycle
    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b00);
    CHECK(t->m_tr_cmd == 0);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (12 << 1));
    CHECK(t->s_tr_ready == 0);

    // Destroy model
    delete t;
}

TEST_CASE("Single miss, no stalling", "[TextureReaderController]")
{
    VTextureReaderController* t = rr::ut::makeTop<VTextureReaderController>();
    rr::ut::reset(t);
    t->nearest = 0;

    makeCacheHot(t, { 10, 20, 30, 40 });

    // First texel cold
    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 20;
    t->s_tr_texel_10 = 30;
    t->s_tr_texel_11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b00);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (12 << 1));
    CHECK(t->s_tr_ready == 1);

    // Second texel cold
    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 30;
    t->s_tr_texel_11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b01);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (22 << 1));
    CHECK(t->s_tr_ready == 1);

    // Third texel cold
    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b10);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (32 << 1));
    CHECK(t->s_tr_ready == 1);

    // Fourth texel cold
    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b11);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (42 << 1));
    CHECK(t->s_tr_ready == 1);

    // Destroy model
    delete t;
}

TEST_CASE("Two misses, no stalling", "[TextureReaderController]")
{
    VTextureReaderController* t = rr::ut::makeTop<VTextureReaderController>();
    rr::ut::reset(t);
    t->nearest = 0;

    makeCacheHot(t, { 10, 20, 30, 40 });

    // First texel cold
    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 10;
    t->s_tr_texel_01 = 20;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b10);
    CHECK(t->m_tr_cmd == 0);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (32 << 1));
    CHECK(t->s_tr_ready == 0);

    // Second texel cold
    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b11);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (42 << 1));
    CHECK(t->s_tr_ready == 1);

    // -------------------

    // First texel cold
    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b00);
    CHECK(t->m_tr_cmd == 0);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (12 << 1));
    CHECK(t->s_tr_ready == 0);

    // Second texel cold
    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 14;
    t->s_tr_texel_01 = 24;
    t->s_tr_texel_10 = 34;
    t->s_tr_texel_11 = 44;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b01);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (22 << 1));
    CHECK(t->s_tr_ready == 1);

    // Destroy model
    delete t;
}

TEST_CASE("Nearest filtering only reads texel00", "[TextureReaderController]")
{
    VTextureReaderController* t = rr::ut::makeTop<VTextureReaderController>();
    rr::ut::reset(t);

    t->nearest = 1;
    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 10;
    t->s_tr_texel_01 = 20;
    t->s_tr_texel_10 = 30;
    t->s_tr_texel_11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b00);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (10 << 1));
    CHECK(t->s_tr_ready == 1);

    // Changing only the other texels must not trigger reads or backpressure.
    t->s_tr_texel_00 = 10;
    t->s_tr_texel_01 = 21;
    t->s_tr_texel_10 = 31;
    t->s_tr_texel_11 = 41;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b00);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (10 << 1));
    CHECK(t->s_tr_ready == 1);

    // Texel00 is still checked and remains the only possible memory read.
    t->s_tr_texel_00 = 11;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b00);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (11 << 1));
    CHECK(t->s_tr_ready == 1);

    delete t;
}

TEST_CASE("Test Cold Cache, with stalling", "[TextureReaderController]")
{
    VTextureReaderController* t = rr::ut::makeTop<VTextureReaderController>();
    rr::ut::reset(t);
    t->nearest = 0;

    t->invalidate = 0;
    CHECK(t->m_tr_valid == 0);
    CHECK(t->s_tr_ready == 1);

    t->m_tr_ready = 0;
    t->s_tr_texel_00 = 10;
    t->s_tr_texel_01 = 20;
    t->s_tr_texel_10 = 30;
    t->s_tr_texel_11 = 40;
    t->s_tr_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b00);
    CHECK(t->m_tr_cmd == 0);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (10 << 1));
    CHECK(t->s_tr_ready == 0);

    t->m_tr_ready = 0;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    t->s_tr_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b00);
    CHECK(t->m_tr_cmd == 0);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (10 << 1));
    CHECK(t->s_tr_ready == 0);

    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b01);
    CHECK(t->m_tr_cmd == 0);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (20 << 1));
    CHECK(t->s_tr_ready == 0);

    t->m_tr_ready = 0;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b01);
    CHECK(t->m_tr_cmd == 0);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (20 << 1));
    CHECK(t->s_tr_ready == 0);

    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b10);
    CHECK(t->m_tr_cmd == 0);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (30 << 1));
    CHECK(t->s_tr_ready == 0);

    t->m_tr_ready = 0;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b10);
    CHECK(t->m_tr_cmd == 0);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (30 << 1));
    CHECK(t->s_tr_ready == 0);

    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b11);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (40 << 1));
    CHECK(t->s_tr_ready == 1);

    t->m_tr_ready = 0;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b11);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (40 << 1));
    CHECK(t->s_tr_ready == 0);

    // New cycle
    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b00);
    CHECK(t->m_tr_cmd == 0);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (12 << 1));
    CHECK(t->s_tr_ready == 0);

    t->m_tr_ready = 0;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b00);
    CHECK(t->m_tr_cmd == 0);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (12 << 1));
    CHECK(t->s_tr_ready == 0);

    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b01);
    CHECK(t->m_tr_cmd == 0);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (22 << 1));
    CHECK(t->s_tr_ready == 0);

    // Destroy model
    delete t;
}

TEST_CASE("Single miss, with stalling", "[TextureReaderController]")
{
    VTextureReaderController* t = rr::ut::makeTop<VTextureReaderController>();
    rr::ut::reset(t);
    t->nearest = 0;

    makeCacheHot(t, { 10, 20, 30, 40 });

    // First texel cold
    t->m_tr_ready = 0;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 20;
    t->s_tr_texel_10 = 30;
    t->s_tr_texel_11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b00);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (12 << 1));
    CHECK(t->s_tr_ready == 1);

    t->m_tr_ready = 0;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 30;
    t->s_tr_texel_11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b00);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (12 << 1));
    CHECK(t->s_tr_ready == 0);

    // Second texel cold
    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b01);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (22 << 1));
    CHECK(t->s_tr_ready == 1);

    t->m_tr_ready = 0;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b01);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (22 << 1));
    CHECK(t->s_tr_ready == 0);

    // Third texel cold
    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b10);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (32 << 1));
    CHECK(t->s_tr_ready == 1);

    t->m_tr_ready = 0;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b10);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (32 << 1));
    CHECK(t->s_tr_ready == 0);

    // Fourth texel cold
    t->m_tr_ready = 1;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 14;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b11);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (42 << 1));
    CHECK(t->s_tr_ready == 1);

    t->m_tr_ready = 0;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 14;
    t->s_tr_texel_01 = 22;
    t->s_tr_texel_10 = 32;
    t->s_tr_texel_11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_tr_texel_pos == 0b11);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (42 << 1));
    CHECK(t->s_tr_ready == 0);

    // Destroy model
    delete t;
}

TEST_CASE("Output remains valid while stalled after source withdraws valid", "[TextureTexelContextManager]")
{
    VTextureReaderController* t = rr::ut::makeTop<VTextureReaderController>();
    rr::ut::reset(t);
    t->nearest = 0;

    makeCacheHot(t, { 10, 20, 30, 40 });

    t->m_tr_ready = 0;
    t->s_tr_valid = 1;
    t->s_tr_texel_00 = 12;
    t->s_tr_texel_01 = 20;
    t->s_tr_texel_10 = 30;
    t->s_tr_texel_11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (12 << 1));
    CHECK(t->m_tr_texel_pos == 0b00);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->s_tr_ready == 1);

    t->s_tr_valid = 0;
    rr::ut::clk(t);
    CHECK(t->m_tr_valid == 1);
    CHECK(t->m_tr_addr == (12 << 1));
    CHECK(t->m_tr_texel_pos == 0b00);
    CHECK(t->m_tr_cmd == 1);
    CHECK(t->s_tr_ready == 1);

    t->m_tr_ready = 1;
    rr::ut::clk(t);
    CHECK(t->m_tr_valid == 0);
    CHECK(t->s_tr_ready == 1);

    delete t;
}

TEST_CASE("Invalidate", "[TextureTexelContextManager]")
{
    VTextureReaderController* t = rr::ut::makeTop<VTextureReaderController>();
    rr::ut::reset(t);
    t->nearest = 0;

    // The makeCacheHot already invalidates the cache.
    // When invalidation correctly works, then two makeCacheHot must also work.
    makeCacheHot(t, { 10, 20, 30, 40 });
    makeCacheHot(t, { 10, 20, 30, 40 });

    // Destroy model
    delete t;
}