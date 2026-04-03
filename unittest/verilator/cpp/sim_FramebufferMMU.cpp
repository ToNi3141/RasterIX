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

// Include model header, generated from Verilating "top.v"
#include "VFramebufferMMU.h"

TEST_CASE("Address translation", "[FramebufferMMU]")
{
    VFramebufferMMU* t = new VFramebufferMMU();

    t->s_frag_tvalid = 1;
    t->s_frag_tlast = 0;
    t->m_frag_tready = 1;
    t->s_frag_tdata = 0x5678;
    t->s_frag_tstrb = 0x3;

    // Zero base address: pixel index 0x42 -> byte addr 0x84
    t->confAddr = 0x0000'0000;
    t->s_frag_taddr = 0x0000'0042;
    t->eval();
    CHECK(t->m_frag_taddr == 0x0000'0084);
    CHECK(t->m_frag_tvalid == 1);
    CHECK(t->m_frag_tlast == 0);
    CHECK(t->s_frag_tready == 1);
    CHECK(t->m_frag_tdata == 0x5678);
    CHECK(t->m_frag_tstrb == 0x3);

    // Non-zero base address: output = (pixel_index << 1) + confAddr
    t->confAddr = 0x1000'0000;
    t->s_frag_taddr = 0x0000'0000;
    t->eval();
    CHECK(t->m_frag_taddr == 0x1000'0000);

    t->confAddr = 0x1000'0000;
    t->s_frag_taddr = 0x0000'0002;
    t->eval();
    CHECK(t->m_frag_taddr == 0x1000'0004);

    t->confAddr = 0x2000'0000;
    t->s_frag_taddr = 0x0000'0006;
    t->eval();
    CHECK(t->m_frag_taddr == 0x2000'000C);

    delete t;
}
