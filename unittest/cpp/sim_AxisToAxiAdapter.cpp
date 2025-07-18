// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2025 ToNi3141

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
#include "VAxisToAxiAdapter.h"

static constexpr std::size_t BEAT_SIZE = 4;
static constexpr std::size_t NUMBER_OF_BEATS = 16;

TEST_CASE("check address channel", "[VAxisToAxiAdapter]")
{

    VAxisToAxiAdapter* t = new VAxisToAxiAdapter();

    t->s_xvalid = true;
    rr::ut::reset(t);
    CHECK(t->s_aready == false);

    t->s_avalid = true;
    t->s_aaddr = 0x1000'0000;
    t->s_abytes = 0x100;
    t->enableAxiLastSignal = true;
    t->m_axready = true;
    t->m_xready = true;
    rr::ut::clk(t);
    CHECK(t->s_aready == false);

    rr::ut::clk(t);
    for (std::size_t i = 0; i < t->s_abytes; i += NUMBER_OF_BEATS * BEAT_SIZE)
    {
        rr::ut::clk(t);
        CHECK(t->m_axvalid == true);
        CHECK(t->m_axaddr == (t->s_aaddr + i));
        CHECK(t->m_axlen == (NUMBER_OF_BEATS - 1));
        CHECK(t->m_axsize == 2);
        CHECK(t->m_axburst == 1);
        CHECK(t->s_aready == false);
    }

    rr::ut::clk(t);
    CHECK(t->m_axvalid == false);
    CHECK(t->s_aready == false);

    // Destroy model
    delete t;
}

TEST_CASE("check mem write", "[VAxisToAxiAdapter]")
{
    VAxisToAxiAdapter* t = new VAxisToAxiAdapter();

    t->s_xvalid = 1;
    rr::ut::reset(t);
    CHECK(t->s_aready == false);

    t->s_avalid = true;
    t->s_aaddr = 0x1000'0000;
    t->s_abytes = 0x100;
    t->enableAxiLastSignal = true;
    t->m_axready = true;
    t->m_xready = true;
    rr::ut::clk(t);
    CHECK(t->s_aready == false);

    for (std::size_t i = 0; i < t->s_abytes; i += BEAT_SIZE)
    {
        t->s_xdata = i;
        t->s_xstrb = i % NUMBER_OF_BEATS;
        t->s_xvalid = true;
        rr::ut::clk(t);
        INFO(std::string("i ") + std::to_string(i));
        CHECK(t->m_xdata == i);
        CHECK(t->m_xvalid == true);
        CHECK(t->m_xlast == ((i % (NUMBER_OF_BEATS * BEAT_SIZE)) == ((NUMBER_OF_BEATS - 1) * BEAT_SIZE)));
        CHECK(t->m_xstrb == (i % NUMBER_OF_BEATS));
        CHECK(t->s_aready == false);
    }

    rr::ut::clk(t);
    CHECK(t->m_xvalid == false);
    CHECK(t->s_aready == true);

    t->s_avalid = false;
    rr::ut::clk(t);
    CHECK(t->m_xvalid == false);
    CHECK(t->s_aready == false);

    // Check that it stays done
    rr::ut::clk(t);
    CHECK(t->m_xvalid == false);
    CHECK(t->s_aready == false);

    // Destroy model
    delete t;
}

TEST_CASE("check mem read", "[VAxisToAxiAdapter]")
{
    VAxisToAxiAdapter* t = new VAxisToAxiAdapter();

    t->s_xvalid = 1;
    rr::ut::reset(t);
    CHECK(t->s_aready == false);

    t->s_avalid = true;
    t->s_aaddr = 0x1000'0000;
    t->s_abytes = 0x100;
    t->enableAxiLastSignal = false;
    t->m_axready = true;
    t->m_xready = true;
    rr::ut::clk(t);
    CHECK(t->s_aready == false);

    for (std::size_t i = 0; i < t->s_abytes; i += BEAT_SIZE)
    {
        t->s_xdata = i;
        t->s_xstrb = i % NUMBER_OF_BEATS;
        t->s_xvalid = true;
        rr::ut::clk(t);
        INFO(std::string("i ") + std::to_string(i));
        CHECK(t->m_xdata == i);
        CHECK(t->m_xvalid == true);
        CHECK(t->m_xlast == ((i + BEAT_SIZE) == t->s_abytes));
        CHECK(t->m_xstrb == (i % NUMBER_OF_BEATS));
        CHECK(t->s_aready == false);
    }

    rr::ut::clk(t);
    CHECK(t->m_xvalid == false);
    CHECK(t->s_aready == true);

    t->s_avalid = false;
    rr::ut::clk(t);
    CHECK(t->m_xvalid == false);
    CHECK(t->s_aready == false);

    // Destroy model
    delete t;
}

TEST_CASE("interrupted mem write", "[VAxisToAxiAdapter]")
{
    VAxisToAxiAdapter* t = new VAxisToAxiAdapter();

    t->s_xvalid = true;
    rr::ut::reset(t);
    CHECK(t->s_aready == false);

    t->s_avalid = true;
    t->s_aaddr = 0x1000'0000;
    t->s_abytes = 0x100;
    t->enableAxiLastSignal = true;
    t->m_axready = true;
    t->m_xready = false;
    rr::ut::clk(t);
    CHECK(t->s_aready == false);
    CHECK(t->s_xready == true);

    t->s_xdata = 0;
    t->s_xstrb = 0;
    t->s_xvalid = true;
    t->m_xready = false;
    rr::ut::clk(t);
    CHECK(t->m_xdata == 0);
    CHECK(t->m_xstrb == 0);
    CHECK(t->m_xvalid == true);
    CHECK(t->m_xlast == 0);
    CHECK(t->s_xready == true);
    CHECK(t->s_aready == false);

    t->s_xdata = BEAT_SIZE;
    t->s_xstrb = BEAT_SIZE;
    t->s_xvalid = true;
    t->m_xready = false;
    rr::ut::clk(t);
    CHECK(t->m_xdata == 0);
    CHECK(t->m_xstrb == 0);
    CHECK(t->m_xvalid == true);
    CHECK(t->m_xlast == 0);
    CHECK(t->s_xready == false);
    CHECK(t->s_aready == false);

    for (std::size_t i = 0; i < t->s_abytes; i += BEAT_SIZE)
    {
        const std::size_t index = i + BEAT_SIZE;

        t->s_xdata = index;
        t->s_xstrb = index % NUMBER_OF_BEATS;
        t->s_xvalid = true;
        t->m_xready = false;
        rr::ut::clk(t);
        CHECK(t->m_xdata == i);
        CHECK(t->m_xstrb == (i % NUMBER_OF_BEATS));
        CHECK(t->m_xvalid == true);
        CHECK(t->m_xlast == ((i % (NUMBER_OF_BEATS * BEAT_SIZE)) == ((NUMBER_OF_BEATS - 1) * BEAT_SIZE)));
        CHECK(t->s_xready == false);
        CHECK(t->s_aready == false);

        t->m_xready = true;
        rr::ut::clk(t);
        INFO(std::string("i ") + std::to_string(i));
        CHECK(t->m_xdata == index);
        CHECK(t->m_xstrb == (index % NUMBER_OF_BEATS));
        CHECK(t->m_xvalid == true);
        CHECK(t->m_xlast == ((index % (NUMBER_OF_BEATS * BEAT_SIZE)) == ((NUMBER_OF_BEATS - 1) * BEAT_SIZE)));
        CHECK(t->s_xready == (index < (t->s_abytes - BEAT_SIZE))); // It's one pixel behind here because one is in the skid buffer
        CHECK(t->s_aready == false);
    }

    rr::ut::clk(t);
    CHECK(t->m_xvalid == false);
    CHECK(t->s_aready == true);

    t->s_avalid = false;
    rr::ut::clk(t);
    CHECK(t->m_xvalid == false);
    CHECK(t->s_aready == false);

    // Destroy model
    delete t;
}