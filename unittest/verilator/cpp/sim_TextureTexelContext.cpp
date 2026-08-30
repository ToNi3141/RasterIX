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

#include "general.hpp"

#include <algorithm>
#include <array>
#include <math.h>

// Include model header, generated from Verilating "top.v"
#include "VTextureTexelContext.h"

void createContext(VTextureTexelContext* t, std::array<uint32_t, 4> texels)
{
    t->m_ready = 1;

    t->s_texel_pos = 0b00;
    t->s_texel = texels[0];
    t->s_cmd = 0;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 0);
    CHECK(t->s_ready == 1);

    t->s_texel_pos = 0b01;
    t->s_texel = texels[1];
    t->s_cmd = 0;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 0);
    CHECK(t->s_ready == 1);

    t->s_texel_pos = 0b10;
    t->s_texel = texels[2];
    t->s_cmd = 0;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 0);
    CHECK(t->s_ready == 1);

    t->s_texel_pos = 0b11;
    t->s_texel = texels[3];
    t->s_cmd = 0;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 0);
    CHECK(t->s_ready == 1);
}

TEST_CASE("Create context and send it, no stall", "[TextureTexelContext]")
{
    VTextureTexelContext* t = rr::ut::makeTop<VTextureTexelContext>();
    rr::ut::reset(t);
    t->m_ready = 1;

    t->s_texel_pos = 0b00;
    t->s_texel = 10;
    t->s_cmd = 0;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 0);
    CHECK(t->s_ready == 1);

    t->s_texel_pos = 0b01;
    t->s_texel = 20;
    t->s_cmd = 0;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 0);
    CHECK(t->s_ready == 1);

    t->s_texel_pos = 0b10;
    t->s_texel = 30;
    t->s_cmd = 0;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 0);
    CHECK(t->s_ready == 1);

    t->s_texel_pos = 0b11;
    t->s_texel = 40;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 1);
    CHECK(t->m_texel00 == 10);
    CHECK(t->m_texel01 == 20);
    CHECK(t->m_texel10 == 30);
    CHECK(t->m_texel11 == 40);

    t->s_valid = 0;
    rr::ut::clk(t);
    CHECK(t->m_valid == 0);
    CHECK(t->s_ready == 1);

    // Destroy model
    delete t;
}

TEST_CASE("Update context, with stall", "[TextureTexelContext]")
{
    VTextureTexelContext* t = rr::ut::makeTop<VTextureTexelContext>();
    rr::ut::reset(t);

    createContext(t, { 10, 20, 30, 40 });
    CHECK(t->s_ready == 1);

    t->m_ready = 0;
    t->s_texel_pos = 0b00;
    t->s_texel = 12;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 1);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 20);
    CHECK(t->m_texel10 == 30);
    CHECK(t->m_texel11 == 40);

    t->m_ready = 0;
    t->s_texel_pos = 0b01;
    t->s_texel = 22;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 0);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 20);
    CHECK(t->m_texel10 == 30);
    CHECK(t->m_texel11 == 40);

    t->m_ready = 1;
    t->s_texel_pos = 0b10;
    t->s_texel = 32;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 1);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 22);
    CHECK(t->m_texel10 == 30);
    CHECK(t->m_texel11 == 40);

    t->m_ready = 0;
    t->s_texel_pos = 0b10;
    t->s_texel = 32;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 0);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 22);
    CHECK(t->m_texel10 == 30);
    CHECK(t->m_texel11 == 40);

    t->m_ready = 1;
    t->s_texel_pos = 0b11;
    t->s_texel = 42;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 1);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 22);
    CHECK(t->m_texel10 == 32);
    CHECK(t->m_texel11 == 40);

    t->m_ready = 0;
    t->s_texel_pos = 0b11;
    t->s_texel = 42;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 0);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 22);
    CHECK(t->m_texel10 == 32);
    CHECK(t->m_texel11 == 40);

    t->m_ready = 1;
    t->s_valid = 0;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 1);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 22);
    CHECK(t->m_texel10 == 32);
    CHECK(t->m_texel11 == 42);

    t->m_ready = 0;
    t->s_valid = 0;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 1);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 22);
    CHECK(t->m_texel10 == 32);
    CHECK(t->m_texel11 == 42);

    t->m_ready = 1;
    t->s_valid = 0;
    rr::ut::clk(t);
    CHECK(t->m_valid == 0);
    CHECK(t->s_ready == 1);

    // Destroy model
    delete t;
}

TEST_CASE("Update context, send after context, with stall", "[TextureTexelContext]")
{
    VTextureTexelContext* t = rr::ut::makeTop<VTextureTexelContext>();
    rr::ut::reset(t);

    createContext(t, { 10, 20, 30, 40 });
    CHECK(t->s_ready == 1);

    // Only update context
    t->m_ready = 0;
    t->s_texel_pos = 0b00;
    t->s_texel = 12;
    t->s_cmd = 0;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 0);
    CHECK(t->s_ready == 1);

    // Update context and send
    t->m_ready = 0;
    t->s_texel_pos = 0b01;
    t->s_texel = 22;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 1);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 22);
    CHECK(t->m_texel10 == 30);
    CHECK(t->m_texel11 == 40);

    t->m_ready = 1;
    t->s_texel_pos = 0b10;
    t->s_texel = 32;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 1);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 22);
    CHECK(t->m_texel10 == 32);
    CHECK(t->m_texel11 == 40);

    // Destroy model
    delete t;
}

TEST_CASE("send context and stall, update context, send context", "[TextureTexelContext]")
{
    VTextureTexelContext* t = rr::ut::makeTop<VTextureTexelContext>();
    rr::ut::reset(t);

    createContext(t, { 10, 20, 30, 40 });
    CHECK(t->s_ready == 1);

    // Send context
    t->m_ready = 0;
    t->s_texel_pos = 0b00;
    t->s_texel = 12;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 1);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 20);
    CHECK(t->m_texel10 == 30);
    CHECK(t->m_texel11 == 40);

    // Update context
    t->m_ready = 0;
    t->s_texel_pos = 0b01;
    t->s_texel = 22;
    t->s_cmd = 0;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 0);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 20);
    CHECK(t->m_texel10 == 30);
    CHECK(t->m_texel11 == 40);

    // Update context and send
    t->m_ready = 0;
    t->s_texel_pos = 0b10;
    t->s_texel = 32;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 0);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 20);
    CHECK(t->m_texel10 == 30);
    CHECK(t->m_texel11 == 40);

    t->m_ready = 1;
    t->s_texel_pos = 0b10;
    t->s_texel = 32;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 0);
    CHECK(t->s_ready == 1);

    t->m_ready = 1;
    t->s_texel_pos = 0b10;
    t->s_texel = 32;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 1);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 22);
    CHECK(t->m_texel10 == 32);
    CHECK(t->m_texel11 == 40);

    t->m_ready = 1;
    t->s_valid = 0;
    rr::ut::clk(t);
    CHECK(t->m_valid == 0);
    CHECK(t->s_ready == 1);

    // Destroy model
    delete t;
}

TEST_CASE("Accepted sample remains valid while output is stalled", "[TextureTexelContext]")
{
    VTextureTexelContext* t = rr::ut::makeTop<VTextureTexelContext>();
    rr::ut::reset(t);

    createContext(t, { 10, 20, 30, 40 });

    // s_ready is sampled before the rising edge. The request is therefore
    // accepted even though m_ready has dropped and the output will stall.
    CHECK(t->s_ready == 1);
    t->m_ready = 0;
    t->s_texel_pos = 0b11;
    t->s_texel = 40;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 1);
    CHECK(t->m_texel00 == 10);
    CHECK(t->m_texel01 == 20);
    CHECK(t->m_texel10 == 30);
    CHECK(t->m_texel11 == 40);

    // The input transaction completed on the preceding edge. An AXI master
    // may now advance while the module retains the stalled output.
    t->s_valid = 0;
    t->s_cmd = 0;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->s_ready == 1);
    CHECK(t->m_texel00 == 10);
    CHECK(t->m_texel01 == 20);
    CHECK(t->m_texel10 == 30);
    CHECK(t->m_texel11 == 40);

    delete t;
}

TEST_CASE("Input remains stable until accepted after skid buffer drains", "[TextureTexelContext]")
{
    VTextureTexelContext* t = rr::ut::makeTop<VTextureTexelContext>();
    rr::ut::reset(t);

    createContext(t, { 10, 20, 30, 40 });

    // Create a stalled output and fill the one-entry skid buffer.
    t->m_ready = 0;
    t->s_texel_pos = 0b00;
    t->s_texel = 12;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);

    t->s_texel_pos = 0b01;
    t->s_texel = 22;
    t->s_cmd = 1;
    rr::ut::clk(t);
    CHECK(t->s_ready == 0);

    // This transaction is not accepted yet. Keep its payload unchanged until
    // the skid entry is promoted to the output on the next clock edge.
    t->m_ready = 1;
    t->s_texel_pos = 0b10;
    t->s_texel = 32;
    rr::ut::clk(t);
    CHECK(t->s_ready == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 22);
    CHECK(t->m_texel10 == 30);
    CHECK(t->m_texel11 == 40);

    t->m_ready = 0;
    rr::ut::clk(t);
    CHECK(t->s_ready == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 22);
    CHECK(t->m_texel10 == 30);
    CHECK(t->m_texel11 == 40);

    // Drain the current output and promote the held third transaction.
    t->m_ready = 1;
    rr::ut::clk(t);
    CHECK(t->s_ready == 1);
    CHECK(t->m_valid == 1);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 22);
    CHECK(t->m_texel10 == 32);
    CHECK(t->m_texel11 == 40);

    delete t;
}

TEST_CASE("Buffered store-only command updates context without producing output", "[TextureTexelContext]")
{
    VTextureTexelContext* t = rr::ut::makeTop<VTextureTexelContext>();
    rr::ut::reset(t);

    createContext(t, { 10, 20, 30, 40 });

    // Stall a sample output, then queue a store-only update behind it.
    t->m_ready = 0;
    t->s_texel_pos = 0b00;
    t->s_texel = 12;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);

    t->s_texel_pos = 0b01;
    t->s_texel = 22;
    t->s_cmd = 0;
    rr::ut::clk(t);
    CHECK(t->s_ready == 0);
    CHECK(t->m_valid == 1);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 20);
    CHECK(t->m_texel10 == 30);
    CHECK(t->m_texel11 == 40);

    // The queued store is processed as the prior output is consumed.
    t->m_ready = 1;
    t->s_valid = 0;
    rr::ut::clk(t);
    CHECK(t->m_valid == 0);
    CHECK(t->s_ready == 1);

    // A following sample observes the context update from the buffered store.
    t->m_ready = 0;
    t->s_texel_pos = 0b10;
    t->s_texel = 32;
    t->s_cmd = 1;
    t->s_valid = 1;
    rr::ut::clk(t);
    CHECK(t->m_valid == 1);
    CHECK(t->m_texel00 == 12);
    CHECK(t->m_texel01 == 22);
    CHECK(t->m_texel10 == 32);
    CHECK(t->m_texel11 == 40);

    delete t;
}
