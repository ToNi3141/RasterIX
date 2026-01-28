// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

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
#include "VTriangleStreamF2XConverter.h"

TEST_CASE("Convert triangle stream", "[VTriangleStreamF2XConverter]")
{
    VTriangleStreamF2XConverter* t = new VTriangleStreamF2XConverter();

    struct StreamData
    {
        bool tvalid;
        bool tlast;
        union
        {
            uint32_t u32Data;
            float f32Data;
        } tdata;
    };

    std::vector<StreamData> tstData {};
    std::vector<StreamData> expData {};

    // Vanguard (to sync expected data with the test data)
    expData.push_back(StreamData { .tvalid = false, .tlast = false, .tdata = { .u32Data = 0 } });
    expData.push_back(StreamData { .tvalid = false, .tlast = false, .tdata = { .u32Data = 0 } });

    // Reserved
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 0 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 0 } });
    // Bounding Box
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 1 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 1 } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 2 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 2 } });
    // Triangle Weights
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 3 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 3 } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 4 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 4 } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 5 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 5 } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 6 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 6 } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 7 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 7 } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 8 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 8 } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 9 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 9 } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 10 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 10 } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 11 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 11 } });
    // Color
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 12 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 12 * (1 << 24) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 13 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 13 * (1 << 24) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 14 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 14 * (1 << 24) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 15 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 15 * (1 << 24) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 16 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 16 * (1 << 24) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 17 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 17 * (1 << 24) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 18 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 18 * (1 << 24) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 19 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 19 * (1 << 24) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 20 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 20 * (1 << 24) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 21 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 21 * (1 << 24) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 22 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 22 * (1 << 24) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 23 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 23 * (1 << 24) } });
    // Depth
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 0.1 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 0x06666668 } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 0.2 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 0x0cccccd0 } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 0.3 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 0x13333340 } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 0.4 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 0x199999a0 } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 0.5 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 0x20000000 } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 0.6 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 0x26666680 } });
    // Texture 0
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 0 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 0 * (1 << 28) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 1 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 1 * (1 << 28) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 2 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 2 * (1 << 28) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 3 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 3 * (1 << 28) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 4 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 4 * (1 << 28) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 5 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 5 * (1 << 28) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 6 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 6 * (1 << 28) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 1 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 1 * (1 << 28) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 2 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 2 * (1 << 28) } });
    // Texture 1
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 0 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 0 * (1 << 28) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 1 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 1 * (1 << 28) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 2 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 2 * (1 << 28) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 3 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 3 * (1 << 28) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 4 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 4 * (1 << 28) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 5 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 5 * (1 << 28) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 6 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 6 * (1 << 28) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 1 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 1 * (1 << 28) } });
    tstData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .f32Data = 2 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = false, .tdata = { .u32Data = 2 * (1 << 28) } });
    // Padding
    tstData.push_back(StreamData { .tvalid = true, .tlast = true, .tdata = { .f32Data = 0 } });
    expData.push_back(StreamData { .tvalid = true, .tlast = true, .tdata = { .u32Data = 0 } });

    // Trailing
    tstData.push_back(StreamData { .tvalid = false, .tlast = false, .tdata = { .f32Data = 0 } });
    tstData.push_back(StreamData { .tvalid = false, .tlast = false, .tdata = { .f32Data = 0 } });

    auto testFunc = [&expData, &tstData, t]()
    {
        for (std::size_t i = 0; i < tstData.size(); i++)
        {
            const StreamData& testEntry = tstData[i];
            const StreamData& expectedEntry = expData[i];

            t->s_axis_tvalid = testEntry.tvalid;
            t->s_axis_tlast = testEntry.tlast;
            t->s_axis_tdata = testEntry.tdata.u32Data;
            rr::ut::clk(t);
            CHECK(t->m_axis_tvalid == expectedEntry.tvalid);
            CHECK(t->m_axis_tlast == expectedEntry.tlast);
            CHECK(t->m_axis_tdata == expectedEntry.tdata.u32Data);
        }
    };

    // Run it two times to make sure, that the last handling works
    rr::ut::reset(t);
    testFunc();
    testFunc();

    // Destroy model
    delete t;
}
