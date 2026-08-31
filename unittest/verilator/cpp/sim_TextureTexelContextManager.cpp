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
#include "VTextureTexelContextManager.h"

void makeCacheHot(VTextureTexelContextManager* t,
    const std::array<uint32_t, 4>& texels)
{
    t->invalidate = 1;
    rr::ut::clk(t);

    t->invalidate = 0;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = texels[0];
    t->s_ar_texel01 = texels[1];
    t->s_ar_texel10 = texels[2];
    t->s_ar_texel11 = texels[3];
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == texels[0]);
    CHECK(t->s_ar_ready == 0);

    t->m_ready = 1;
    t->s_ar_valid = 0;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == texels[1]);
    CHECK(t->s_ar_ready == 0);

    t->m_ready = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == texels[2]);
    CHECK(t->s_ar_ready == 0);

    t->m_ready = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == texels[3]);
    CHECK(t->s_ar_ready == 1);

    t->m_ready = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 0);
    CHECK(t->s_ar_ready == 1);
}

TEST_CASE("Test Cold Cache, no stalling", "[TextureTexelContextManager]")
{
    VTextureTexelContextManager* t = rr::ut::makeTop<VTextureTexelContextManager>();
    rr::ut::reset(t);

    t->invalidate = 0;
    CHECK(t->m_valid == 0);
    CHECK(t->s_ar_ready == 1);

    t->m_ready = 1;
    t->s_ar_texel00 = 10;
    t->s_ar_texel01 = 20;
    t->s_ar_texel10 = 30;
    t->s_ar_texel11 = 40;
    t->s_ar_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b00);
    CHECK(t->m_cmd == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 10);
    CHECK(t->s_ar_ready == 0);

    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b01);
    CHECK(t->m_cmd == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 20);
    CHECK(t->s_ar_ready == 0);

    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b10);
    CHECK(t->m_cmd == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 30);
    CHECK(t->s_ar_ready == 0);

    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b11);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 40);
    CHECK(t->s_ar_ready == 1);

    // New cycle
    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b00);
    CHECK(t->m_cmd == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 12);
    CHECK(t->s_ar_ready == 0);

    // Destroy model
    delete t;
}

TEST_CASE("Single miss, no stalling", "[TextureTexelContextManager]")
{
    VTextureTexelContextManager* t = rr::ut::makeTop<VTextureTexelContextManager>();
    rr::ut::reset(t);

    makeCacheHot(t, { 10, 20, 30, 40 });

    // First texel cold
    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 20;
    t->s_ar_texel10 = 30;
    t->s_ar_texel11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b00);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 12);
    CHECK(t->s_ar_ready == 1);

    // Second texel cold
    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 30;
    t->s_ar_texel11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b01);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 22);
    CHECK(t->s_ar_ready == 1);

    // Third texel cold
    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b10);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 32);
    CHECK(t->s_ar_ready == 1);

    // Fourth texel cold
    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b11);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 42);
    CHECK(t->s_ar_ready == 1);

    // Destroy model
    delete t;
}

TEST_CASE("Two misses, no stalling", "[TextureTexelContextManager]")
{
    VTextureTexelContextManager* t = rr::ut::makeTop<VTextureTexelContextManager>();
    rr::ut::reset(t);

    makeCacheHot(t, { 10, 20, 30, 40 });

    // First texel cold
    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 10;
    t->s_ar_texel01 = 20;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b10);
    CHECK(t->m_cmd == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 32);
    CHECK(t->s_ar_ready == 0);

    // Second texel cold
    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b11);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 42);
    CHECK(t->s_ar_ready == 1);

    // -------------------

    // First texel cold
    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b00);
    CHECK(t->m_cmd == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 12);
    CHECK(t->s_ar_ready == 0);

    // Second texel cold
    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 14;
    t->s_ar_texel01 = 24;
    t->s_ar_texel10 = 34;
    t->s_ar_texel11 = 44;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b01);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 22);
    CHECK(t->s_ar_ready == 1);

    // Destroy model
    delete t;
}

TEST_CASE("Test Cold Cache, with stalling", "[TextureTexelContextManager]")
{
    VTextureTexelContextManager* t = rr::ut::makeTop<VTextureTexelContextManager>();
    rr::ut::reset(t);

    t->invalidate = 0;
    CHECK(t->m_valid == 0);
    CHECK(t->s_ar_ready == 1);

    t->m_ready = 0;
    t->s_ar_texel00 = 10;
    t->s_ar_texel01 = 20;
    t->s_ar_texel10 = 30;
    t->s_ar_texel11 = 40;
    t->s_ar_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b00);
    CHECK(t->m_cmd == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 10);
    CHECK(t->s_ar_ready == 0);

    t->m_ready = 0;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    t->s_ar_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b00);
    CHECK(t->m_cmd == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 10);
    CHECK(t->s_ar_ready == 0);

    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b01);
    CHECK(t->m_cmd == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 20);
    CHECK(t->s_ar_ready == 0);

    t->m_ready = 0;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b01);
    CHECK(t->m_cmd == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 20);
    CHECK(t->s_ar_ready == 0);

    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b10);
    CHECK(t->m_cmd == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 30);
    CHECK(t->s_ar_ready == 0);

    t->m_ready = 0;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b10);
    CHECK(t->m_cmd == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 30);
    CHECK(t->s_ar_ready == 0);

    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b11);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 40);
    CHECK(t->s_ar_ready == 1);

    t->m_ready = 0;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b11);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 40);
    CHECK(t->s_ar_ready == 0);

    // New cycle
    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b00);
    CHECK(t->m_cmd == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 12);
    CHECK(t->s_ar_ready == 0);

    t->m_ready = 0;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b00);
    CHECK(t->m_cmd == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 12);
    CHECK(t->s_ar_ready == 0);

    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b01);
    CHECK(t->m_cmd == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 22);
    CHECK(t->s_ar_ready == 0);

    // Destroy model
    delete t;
}

TEST_CASE("Single miss, with stalling", "[TextureTexelContextManager]")
{
    VTextureTexelContextManager* t = rr::ut::makeTop<VTextureTexelContextManager>();
    rr::ut::reset(t);

    makeCacheHot(t, { 10, 20, 30, 40 });

    // First texel cold
    t->m_ready = 0;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 20;
    t->s_ar_texel10 = 30;
    t->s_ar_texel11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b00);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 12);
    CHECK(t->s_ar_ready == 1);

    t->m_ready = 0;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 30;
    t->s_ar_texel11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b00);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 12);
    CHECK(t->s_ar_ready == 0);

    // Second texel cold
    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b01);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 22);
    CHECK(t->s_ar_ready == 1);

    t->m_ready = 0;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b01);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 22);
    CHECK(t->s_ar_ready == 0);

    // Third texel cold
    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b10);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 32);
    CHECK(t->s_ar_ready == 1);

    t->m_ready = 0;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b10);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 32);
    CHECK(t->s_ar_ready == 0);

    // Fourth texel cold
    t->m_ready = 1;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 14;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b11);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 42);
    CHECK(t->s_ar_ready == 1);

    t->m_ready = 0;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 14;
    t->s_ar_texel01 = 22;
    t->s_ar_texel10 = 32;
    t->s_ar_texel11 = 42;
    rr::ut::clk(t);
    CHECK(t->m_texel_pos == 0b11);
    CHECK(t->m_cmd == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 42);
    CHECK(t->s_ar_ready == 0);

    // Destroy model
    delete t;
}

TEST_CASE("Output remains valid while stalled after source withdraws valid", "[TextureTexelContextManager]")
{
    VTextureTexelContextManager* t = rr::ut::makeTop<VTextureTexelContextManager>();
    rr::ut::reset(t);

    makeCacheHot(t, { 10, 20, 30, 40 });

    t->m_ready = 0;
    t->s_ar_valid = 1;
    t->s_ar_texel00 = 12;
    t->s_ar_texel01 = 20;
    t->s_ar_texel10 = 30;
    t->s_ar_texel11 = 40;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 12);
    CHECK(t->m_texel_pos == 0b00);
    CHECK(t->m_cmd == 1);
    CHECK(t->s_ar_ready == 1);

    t->s_ar_valid = 0;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->m_araddr == 12);
    CHECK(t->m_texel_pos == 0b00);
    CHECK(t->m_cmd == 1);
    CHECK(t->s_ar_ready == 1);

    t->m_ready = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 0);
    CHECK(t->s_ar_ready == 1);

    delete t;
}

TEST_CASE("Invalidate", "[TextureTexelContextManager]")
{
    VTextureTexelContextManager* t = rr::ut::makeTop<VTextureTexelContextManager>();
    rr::ut::reset(t);

    // The makeCacheHot already invalidates the cache.
    // When invalidation correctly works, then two makeCacheHot must also work.
    makeCacheHot(t, { 10, 20, 30, 40 });
    makeCacheHot(t, { 10, 20, 30, 40 });

    // Destroy model
    delete t;
}