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
#include "VStreamMux2.h"

TEST_CASE("StreamMux2 selects port 0", "[StreamMux2]")
{
    VStreamMux2* t = rr::ut::makeTop<VStreamMux2>();
    t->select = 0;
    t->s0_valid = 1;
    t->s0_last = 1;
    t->s0_data = 0x12345678;
    t->s1_valid = 1;
    t->s1_last = 0;
    t->s1_data = 0x87654321;
    t->m_ready = 1;
    t->eval();

    CHECK(t->m_valid == 1);
    CHECK(t->m_last == 1);
    CHECK(t->m_data == 0x12345678);
    CHECK(t->s0_ready == 1);
    CHECK(t->s1_ready == 0);
    delete t;
}

TEST_CASE("StreamMux2 selects port 1 and propagates backpressure", "[StreamMux2]")
{
    VStreamMux2* t = rr::ut::makeTop<VStreamMux2>();
    t->select = 1;
    t->s0_valid = 1;
    t->s0_last = 1;
    t->s0_data = 0x12345678;
    t->s1_valid = 1;
    t->s1_last = 0;
    t->s1_data = 0x87654321;
    t->m_ready = 0;
    t->eval();

    CHECK(t->m_valid == 1);
    CHECK(t->m_last == 0);
    CHECK(t->m_data == 0x87654321);
    CHECK(t->s0_ready == 0);
    CHECK(t->s1_ready == 0);

    t->m_ready = 1;
    t->eval();
    CHECK(t->s0_ready == 0);
    CHECK(t->s1_ready == 1);
    delete t;
}