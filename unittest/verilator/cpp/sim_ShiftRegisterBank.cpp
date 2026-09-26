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
#include "VShiftRegisterBank.h"

TEST_CASE("A complete stream shifts into register order", "[VShiftRegisterBank]")
{
    auto* t = rr::ut::makeTop<VShiftRegisterBank>();
    t->s_axis_tvalid = 0;
    t->s_axis_tlast = 0;
    t->update_acknowledged = 0;
    rr::ut::reset(t);
    CHECK(t->registers_updated == 0);

    // Exercise the default BANK_SIZE=8 and REGISTER_WIDTH=32 without a wrapper.
    for (uint32_t base : { 0x1000U, 0x2000U })
    {
        for (uint32_t i = 0; i < 8; i++)
        {
            t->s_axis_tdata = base + i;
            t->s_axis_tvalid = 1;
            t->s_axis_tlast = i == 7;
            // The final valid beat wins over an acknowledgement on the same clock.
            t->update_acknowledged = i == 7;
            rr::ut::clk(t);

            if (i == 3)
            {
                t->s_axis_tvalid = 0;
                t->s_axis_tlast = 1;
                rr::ut::clk(t);
                CHECK(t->registers_updated == 0);
            }
        }

        CHECK(t->registers_updated == 1);
        for (uint32_t i = 0; i < 8; i++)
        {
            CHECK(t->registers[i] == base + i);
        }

        t->s_axis_tvalid = 0;
        t->s_axis_tlast = 0;
        t->update_acknowledged = 1;
        rr::ut::clk(t);
        CHECK(t->registers_updated == 0);
        for (uint32_t i = 0; i < 8; i++)
        {
            CHECK(t->registers[i] == base + i);
        }
    }

    delete t;
}

TEST_CASE("Overflow beats are ignored until the final beat", "[VShiftRegisterBank]")
{
    auto* t = rr::ut::makeTop<VShiftRegisterBank>();
    t->s_axis_tvalid = 0;
    t->s_axis_tlast = 0;
    t->update_acknowledged = 0;
    rr::ut::reset(t);

    for (uint32_t i = 0; i < 8; i++)
    {
        t->s_axis_tvalid = 1;
        t->s_axis_tlast = 0;
        t->s_axis_tdata = 0x3000 + i;
        rr::ut::clk(t);
    }

    // Invalid cycles must not end the stream or make room for further beats.
    t->s_axis_tvalid = 0;
    t->s_axis_tlast = 1;
    rr::ut::clk(t);
    CHECK(t->registers_updated == 0);

    for (uint32_t i = 0; i < 3; i++)
    {
        t->s_axis_tvalid = 1;
        t->s_axis_tlast = i == 2;
        t->s_axis_tdata = 0xdead0000 + i;
        rr::ut::clk(t);
        for (uint32_t word = 0; word < 8; word++)
        {
            CHECK(t->registers[word] == 0x3000 + word);
        }
    }
    CHECK(t->registers_updated == 1);

    // The overflow's valid tlast resets the counter for the next descriptor.
    t->s_axis_tvalid = 0;
    t->s_axis_tlast = 0;
    t->update_acknowledged = 1;
    rr::ut::clk(t);
    CHECK(t->registers_updated == 0);
    t->update_acknowledged = 0;
    for (uint32_t i = 0; i < 8; i++)
    {
        t->s_axis_tvalid = 1;
        t->s_axis_tlast = i == 7;
        t->s_axis_tdata = 0x4000 + i;
        rr::ut::clk(t);
    }
    CHECK(t->registers_updated == 1);
    for (uint32_t i = 0; i < 8; i++)
    {
        CHECK(t->registers[i] == 0x4000 + i);
    }

    delete t;
}