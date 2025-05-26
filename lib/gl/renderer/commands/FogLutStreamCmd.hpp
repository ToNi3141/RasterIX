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

#ifndef _FOG_LUT_STREAM_CMD_HPP_
#define _FOG_LUT_STREAM_CMD_HPP_

#include <array>
#include <cstdint>
#include <tcb/span.hpp>

namespace rr
{

class FogLutStreamCmd
{
    static constexpr std::size_t LUT_SIZE { 66 }; // upper bound, lower bound, 32 * (m and b)
    static constexpr uint32_t FOG_LUT_STREAM { 0x4000'0000 };
    static constexpr uint32_t OP_MASK { 0xF000'0000 };

public:
    using PayloadType = tcb::span<const uint32_t>;
    using CommandType = uint32_t;

    FogLutStreamCmd() = default;
    FogLutStreamCmd(const std::array<float, 33>& fogLut, const float start, const float end)
    {
        // The verilog code is not able to handle float values smaller than 1.0f.
        // So, if start is smaller than 1.0f, set the lower bound to 1.0f which will
        // the set x to 1.
        const float lutLowerBound = start < 1.0f ? 1.0f : start;
        const float lutUpperBound = end;

        // Add bounds to the lut value
        setBounds(lutLowerBound, lutUpperBound);

        // Calculate the lut entries
        for (std::size_t i = 0; i < fogLut.size() - 1; i++)
        {
            float f = fogLut[i];
            float fn = fogLut[i + 1];

            const float diff = fn - f;
            const float step = diff / 256.0f;

            const float m = step * powf(2, 30);
            const float b = f * powf(2, 30);

            setLutValue(i, m, b);
        }

        m_payload = { m_lut };
    }

    FogLutStreamCmd(const CommandType, const PayloadType& payload, const bool) // TODO: Find another way to make in unambiguous
    {
        const std::size_t texSize = payload.size();
        for (std::size_t i = 0; i < texSize; i++)
        {
            m_lut[i] = payload[i];
        }
        m_payload = { m_lut };
    }

    const PayloadType& payload() const { return m_payload; }
    static constexpr CommandType command() { return FOG_LUT_STREAM; }

    static std::size_t getNumberOfElementsInPayloadByCommand(const CommandType) { return LUT_SIZE; }
    static bool isThis(const CommandType cmd) { return (cmd & OP_MASK) == FOG_LUT_STREAM; }

private:
    void setBounds(const float lower, const float upper)
    {
        std::memcpy(&(m_lut[0]), &lower, sizeof(m_lut[0]));
        std::memcpy(&(m_lut[1]), &upper, sizeof(m_lut[1]));
    }

    void setLutValue(const std::size_t index, const float m, const float b)
    {
        m_lut[((index + 1) * 2)] = static_cast<int32_t>(m);
        m_lut[((index + 1) * 2) + 1] = static_cast<int32_t>(b);
    }

    std::array<uint32_t, LUT_SIZE> m_lut;
    PayloadType m_payload;
};

} // namespace rr

#endif // _FOG_LUT_STREAM_CMD_HPP_
