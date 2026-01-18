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

#include <algorithm>
#include <array>
#include <math.h>

// Include model header, generated from Verilating "top.v"
#include "VFunctionInterpolator.h"

float clamp(float v, float lo, float hi)
{
    if (v < lo)
        v = lo;
    if (v > hi)
        v = hi;
    return v;
}

float calculateLinearValue(const float start, const float end, const float z)
{
    float f = (end - z) / (end - start);
    return f;
}

void generateLinearTable(VFunctionInterpolator* t, const float start, const float end)
{
    static constexpr std::size_t LUT_SIZE = 32;
    uint64_t table[LUT_SIZE];
    // The verilog code is not able to handle float values smaller than 1.0f.
    // So, if start is smaller than 1.0f, set the lower bound to 1.0f which will
    // the set x to 1.
    const float startInc = start < 1.0f ? 1.0f : start;
    const float lutLowerBound = startInc;
    const float lutUpperBound = end;

    t->s_axis_tvalid = 1;
    t->s_axis_tlast = 0;
    t->s_axis_tdata = *reinterpret_cast<const uint32_t*>(&lutLowerBound);
    rr::ut::clk(t);
    t->s_axis_tdata = *reinterpret_cast<const uint32_t*>(&lutUpperBound);
    rr::ut::clk(t);
    // printf("lowerBound %d, upperBound %d, bounds: 0x%llX\r\n", lutLowerBound, lutUpperBound, bounds.axiVal);
    for (int i = 0; i < (int)LUT_SIZE; i++)
    {
        float z = powf(2, i);

        float f = calculateLinearValue(start, end, powf(2, i));
        float fn = calculateLinearValue(start, end, powf(2, i + 1));

        float diff = fn - f;
        float step = diff / 256.0f;

        // printf("%d z: %f f: %f fn: %f step: %f axi: 0x%llX\r\n", i, z, f, fn, step, lutEntry.axiVal);

        t->s_axis_tdata = static_cast<int32_t>(step * powf(2, 30));
        rr::ut::clk(t);
        t->s_axis_tlast = (i + 1 < (int)LUT_SIZE) ? 0 : 1;
        t->s_axis_tdata = static_cast<int32_t>(f * powf(2, 30));
        rr::ut::clk(t);
    }
    t->s_axis_tlast = 0;
    t->s_axis_tvalid = 0;
}

TEST_CASE("Check interpolation of the values", "[FunctionInterpolator]")
{
    const float start = 0;
    const float end = 100000;
    VFunctionInterpolator* top = new VFunctionInterpolator();
    rr::ut::reset(top);
    top->ce = 1;
    generateLinearTable(top, start, end);

    uint32_t pipelineSteps = 0;
    static constexpr float STEPS = 0.1;
    static constexpr float PIPELINE_STEPS = 4;
    for (float i = start; i < (end + 200); i += STEPS)
    {
        top->x = *((uint32_t*)&i);
        rr::ut::clk(top);

        pipelineSteps++;
        if (pipelineSteps >= PIPELINE_STEPS)
        {
            const float ref = clamp(calculateLinearValue(start, end, i - (PIPELINE_STEPS * STEPS)), 0.0f, 1.0f);
            const float actual = (float)(top->fx) / powf(2, 22);
            // printf("i: %f, ref %f, actual: %f\r\n", i, ref, actual);
            REQUIRE(ref == Approx(actual).margin(0.005));

            REQUIRE(actual <= 1.0f);
            REQUIRE(actual >= 0.0f);
        }
    }

    // Destroy model
    delete top;
}

TEST_CASE("Check stall", "[FunctionInterpolator]")
{
    const float start = 0;
    const float end = 100000;
    VFunctionInterpolator* top = new VFunctionInterpolator();
    rr::ut::reset(top);
    top->ce = 1;
    generateLinearTable(top, start, end);

    float x = 50000.0f;
    top->x = *((uint32_t*)&x);
    rr::ut::clk(top);

    top->x = 0;
    rr::ut::clk(top);
    top->ce = 0;
    rr::ut::clk(top);
    rr::ut::clk(top);

    top->ce = 1;
    rr::ut::clk(top);
    rr::ut::clk(top);

    const float fx = (float)(top->fx) / powf(2, 22); // Note: Needs to be divided by 22 because the resulting fix point number has 22 fraction bits (S1.22)
    REQUIRE(0.5f == Approx(fx).margin(0.005));

    // Destroy model
    delete top;
}
