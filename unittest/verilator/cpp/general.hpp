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

#ifndef GENERAL_HPP
#define GENERAL_HPP

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "../../3rdParty/catch.hpp"

// Include common routines
#include <verilated.h>

namespace rr::ut
{

template <typename T>
void clk(T* t)
{
    t->aclk = 0;
    t->eval();
    t->aclk = 1;
    t->eval();
}

template <typename T>
void reset(T* t)
{
    t->resetn = 0;
    clk(t);
    t->resetn = 1;
    clk(t);
}

void enableVerilatorTracing()
{
    Verilated::traceEverOn(true);
}

// Creates a Verilated model with a fresh VerilatedContext so that simulation
// time always starts at 0, even when multiple TEST_CASEs run in the same
// binary.  Verilator 5.024+ requires time() == 0 at model construction.
// The context is intentionally leaked (acceptable in short-lived test binaries).
template <typename T>
T* makeTop()
{
    auto* ctx = new VerilatedContext;
    return new T { ctx };
}

} // namespace rr::ut

// Needed for verilator when tracing is enabled.
// Returns 0 always so that a fresh VerilatedContext (whose m_time starts at 0)
// reports time() == 0 via the legacy callback path and never blocks model
// construction with "Adding model when time is non-zero".
double sc_time_stamp()
{
    return 0;
}

#endif // GENERAL_HPP
