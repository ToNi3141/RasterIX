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
#include "VFramebufferWriterStrobeGen.h"

TEST_CASE("Check mask gating", "[FramebufferWriterStrobeGen]")
{
    VFramebufferWriterStrobeGen* t = rr::ut::makeTop<VFramebufferWriterStrobeGen>();

    t->s_frag_tvalid = 1;
    t->s_frag_tlast = 0;
    t->m_frag_tready = 1;
    t->s_frag_tdata = 0xABCD;
    t->s_frag_taddr = 0x100;

    // tstrb=1 passes confMask through
    t->confMask = 0x3;
    t->s_frag_tstrb = 1;
    t->eval();
    CHECK(t->m_frag_tstrb == 0x3);
    CHECK(t->m_frag_tvalid == 1);
    CHECK(t->m_frag_tlast == 0);
    CHECK(t->s_frag_tready == 1);
    CHECK(t->m_frag_tdata == 0xABCD);
    CHECK(t->m_frag_taddr == 0x100);

    t->confMask = 0xA;
    t->s_frag_tstrb = 1;
    t->eval();
    CHECK(t->m_frag_tstrb == 0xA);

    // tstrb=0 zeroes the mask
    t->confMask = 0xF;
    t->s_frag_tstrb = 0;
    t->eval();
    CHECK(t->m_frag_tstrb == 0x0);

    t->confMask = 0x3;
    t->s_frag_tstrb = 0;
    t->eval();
    CHECK(t->m_frag_tstrb == 0x0);

    delete t;
}
