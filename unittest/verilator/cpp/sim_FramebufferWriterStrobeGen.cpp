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

TEST_CASE("Check stream concatenation", "[FramebufferWriterStrobeGen]")
{
    VFramebufferWriterStrobeGen* t = new VFramebufferWriterStrobeGen();

    t->mask = 0xA;
    t->val = 0;
    t->eval();
    CHECK(t->strobe == 0x000A);

    t->mask = 0xF;
    t->val = 1;
    t->eval();
    CHECK(t->strobe == 0x00F0);

    t->mask = 0xF;
    t->val = 2;
    t->eval();
    CHECK(t->strobe == 0x0F00);

    t->mask = 0xF;
    t->val = 3;
    t->eval();
    CHECK(t->strobe == 0xF000);

    delete t;
}
