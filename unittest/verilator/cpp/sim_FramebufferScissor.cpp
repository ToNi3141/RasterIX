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
#include "VFramebufferScissor.h"

TEST_CASE("Scissor disabled passes strobe through", "[FramebufferScissor]")
{
    VFramebufferScissor* t = rr::ut::makeTop<VFramebufferScissor>();

    t->confEnableScissor = 0;
    t->confScissorStartX = 0;
    t->confScissorStartY = 0;
    t->confScissorEndX = 0;
    t->confScissorEndY = 0;

    // Pixel outside any rect but scissor disabled -> pass through
    t->s_frag_tvalid = 1;
    t->s_frag_tlast = 0;
    t->m_frag_tready = 1;
    t->s_frag_tdata = 0x1234;
    t->s_frag_tstrb = 1;
    t->s_frag_taddr = 0x42;
    t->s_frag_txpos = 100;
    t->s_frag_typos = 100;
    t->eval();
    CHECK(t->m_frag_tstrb == 1);
    CHECK(t->m_frag_tvalid == 1);
    CHECK(t->m_frag_tlast == 0);
    CHECK(t->s_frag_tready == 1);
    CHECK(t->m_frag_tdata == 0x1234);
    CHECK(t->m_frag_taddr == 0x42);

    // strb=0 stays 0 even with scissor disabled
    t->s_frag_tstrb = 0;
    t->eval();
    CHECK(t->m_frag_tstrb == 0);

    delete t;
}

TEST_CASE("Scissor enabled clips pixels outside rect", "[FramebufferScissor]")
{
    VFramebufferScissor* t = rr::ut::makeTop<VFramebufferScissor>();

    t->confEnableScissor = 1;
    t->confScissorStartX = 2;
    t->confScissorStartY = 3;
    t->confScissorEndX = 5;
    t->confScissorEndY = 7;
    t->s_frag_tvalid = 1;
    t->s_frag_tlast = 0;
    t->m_frag_tready = 1;
    t->s_frag_tdata = 0;
    t->s_frag_taddr = 0;
    t->s_frag_tstrb = 1;

    // Inside scissor rect
    t->s_frag_txpos = 2;
    t->s_frag_typos = 3;
    t->eval();
    CHECK(t->m_frag_tstrb == 1);

    t->s_frag_txpos = 4;
    t->s_frag_typos = 6;
    t->eval();
    CHECK(t->m_frag_tstrb == 1);

    // On the exclusive end boundary -> outside
    t->s_frag_txpos = 5;
    t->s_frag_typos = 3;
    t->eval();
    CHECK(t->m_frag_tstrb == 0);

    t->s_frag_txpos = 2;
    t->s_frag_typos = 7;
    t->eval();
    CHECK(t->m_frag_tstrb == 0);

    // Below start -> outside
    t->s_frag_txpos = 1;
    t->s_frag_typos = 3;
    t->eval();
    CHECK(t->m_frag_tstrb == 0);

    t->s_frag_txpos = 2;
    t->s_frag_typos = 2;
    t->eval();
    CHECK(t->m_frag_tstrb == 0);

    // Inside but strb=0 -> stays 0
    t->s_frag_tstrb = 0;
    t->s_frag_txpos = 3;
    t->s_frag_typos = 4;
    t->eval();
    CHECK(t->m_frag_tstrb == 0);

    delete t;
}
