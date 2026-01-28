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

#define CATCH_CONFIG_MAIN
#include "../3rdParty/catch.hpp"
#include "renderer/softwarerasterizer/TestFunc.hpp"

TEST_CASE("TestFunc disabled always returns true", "[TestFunc]")
{
    rr::softwarerasterizer::TestFunc<int> testFunc;
    testFunc.setEnable(false);
    testFunc.setFunction(rr::TestFunc::NEVER);
    testFunc.setReferenceValue(100);

    REQUIRE(testFunc.check(0) == true);
    REQUIRE(testFunc.check(50) == true);
    REQUIRE(testFunc.check(100) == true);
    REQUIRE(testFunc.check(200) == true);
}

TEST_CASE("TestFunc ALWAYS", "[TestFunc]")
{
    rr::softwarerasterizer::TestFunc<int> testFunc;
    testFunc.setEnable(true);
    testFunc.setFunction(rr::TestFunc::ALWAYS);
    testFunc.setReferenceValue(100);

    REQUIRE(testFunc.check(0) == true);
    REQUIRE(testFunc.check(50) == true);
    REQUIRE(testFunc.check(100) == true);
    REQUIRE(testFunc.check(200) == true);
}

TEST_CASE("TestFunc NEVER", "[TestFunc]")
{
    rr::softwarerasterizer::TestFunc<int> testFunc;
    testFunc.setEnable(true);
    testFunc.setFunction(rr::TestFunc::NEVER);
    testFunc.setReferenceValue(100);

    REQUIRE(testFunc.check(0) == false);
    REQUIRE(testFunc.check(50) == false);
    REQUIRE(testFunc.check(100) == false);
    REQUIRE(testFunc.check(200) == false);
}

TEST_CASE("TestFunc LESS", "[TestFunc]")
{
    rr::softwarerasterizer::TestFunc<int> testFunc;
    testFunc.setEnable(true);
    testFunc.setFunction(rr::TestFunc::LESS);
    testFunc.setReferenceValue(100);

    REQUIRE(testFunc.check(50) == true);
    REQUIRE(testFunc.check(99) == true);
    REQUIRE(testFunc.check(100) == false);
    REQUIRE(testFunc.check(101) == false);
}

TEST_CASE("TestFunc EQUAL", "[TestFunc]")
{
    rr::softwarerasterizer::TestFunc<int> testFunc;
    testFunc.setEnable(true);
    testFunc.setFunction(rr::TestFunc::EQUAL);
    testFunc.setReferenceValue(100);

    REQUIRE(testFunc.check(50) == false);
    REQUIRE(testFunc.check(99) == false);
    REQUIRE(testFunc.check(100) == true);
    REQUIRE(testFunc.check(101) == false);
}

TEST_CASE("TestFunc LEQUAL", "[TestFunc]")
{
    rr::softwarerasterizer::TestFunc<int> testFunc;
    testFunc.setEnable(true);
    testFunc.setFunction(rr::TestFunc::LEQUAL);
    testFunc.setReferenceValue(100);

    REQUIRE(testFunc.check(50) == true);
    REQUIRE(testFunc.check(99) == true);
    REQUIRE(testFunc.check(100) == true);
    REQUIRE(testFunc.check(101) == false);
}

TEST_CASE("TestFunc GREATER", "[TestFunc]")
{
    rr::softwarerasterizer::TestFunc<int> testFunc;
    testFunc.setEnable(true);
    testFunc.setFunction(rr::TestFunc::GREATER);
    testFunc.setReferenceValue(100);

    REQUIRE(testFunc.check(50) == false);
    REQUIRE(testFunc.check(100) == false);
    REQUIRE(testFunc.check(101) == true);
    REQUIRE(testFunc.check(200) == true);
}

TEST_CASE("TestFunc NOTEQUAL", "[TestFunc]")
{
    rr::softwarerasterizer::TestFunc<int> testFunc;
    testFunc.setEnable(true);
    testFunc.setFunction(rr::TestFunc::NOTEQUAL);
    testFunc.setReferenceValue(100);

    REQUIRE(testFunc.check(50) == true);
    REQUIRE(testFunc.check(99) == true);
    REQUIRE(testFunc.check(100) == false);
    REQUIRE(testFunc.check(101) == true);
}

TEST_CASE("TestFunc GEQUAL", "[TestFunc]")
{
    rr::softwarerasterizer::TestFunc<int> testFunc;
    testFunc.setEnable(true);
    testFunc.setFunction(rr::TestFunc::GEQUAL);
    testFunc.setReferenceValue(100);

    REQUIRE(testFunc.check(50) == false);
    REQUIRE(testFunc.check(99) == false);
    REQUIRE(testFunc.check(100) == true);
    REQUIRE(testFunc.check(101) == true);
}

TEST_CASE("TestFunc with float type", "[TestFunc]")
{
    rr::softwarerasterizer::TestFunc<float> testFunc;
    testFunc.setEnable(true);
    testFunc.setFunction(rr::TestFunc::LESS);
    testFunc.setReferenceValue(0.5f);

    REQUIRE(testFunc.check(0.25f) == true);
    REQUIRE(testFunc.check(0.49f) == true);
    REQUIRE(testFunc.check(0.5f) == false);
    REQUIRE(testFunc.check(0.75f) == false);
}

TEST_CASE("TestFunc with uint8_t type (stencil buffer)", "[TestFunc]")
{
    rr::softwarerasterizer::TestFunc<uint8_t> testFunc;
    testFunc.setEnable(true);
    testFunc.setFunction(rr::TestFunc::EQUAL);
    testFunc.setReferenceValue(5);

    REQUIRE(testFunc.check(4) == false);
    REQUIRE(testFunc.check(5) == true);
    REQUIRE(testFunc.check(6) == false);
}
