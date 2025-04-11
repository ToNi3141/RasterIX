// RRX
// https://github.com/ToNi3141/RRX
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

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "../3rdParty/catch.hpp"

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

} // namespace rr::ut