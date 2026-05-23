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

#include "Op.hpp"
#include "renderer/displaylist/DisplayList.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <tcb/span.hpp>

namespace rr
{

class FogLutStreamCmd
{
    static constexpr std::size_t LUT_SIZE { 64 }; // 32 * (m and b)
public:
    using PayloadType = tcb::span<const int32_t>;
    using CommandType = uint32_t;
    using FogLutDesc = std::array<int32_t, LUT_SIZE>;

    FogLutStreamCmd() = default;
    FogLutStreamCmd(const std::array<float, 33>& fogLut)
    {
        // Calculate the lut entries
        for (std::size_t i = 0; i < fogLut.size() - 1; i++)
        {
            const float f = std::clamp(fogLut[i], 0.0f, 1.0f);
            const float fn = std::clamp(fogLut[i + 1], 0.0f, 1.0f);

            const float deltaF = fn - f;

            const float m = deltaF * powf(2, 22);
            const float b = f * powf(2, 30);

            setLutValue(i, m, b);
        }

        m_payload = { m_lut };
    }

    FogLutStreamCmd(const CommandType, const PayloadType& payload, const bool)
    {
        m_payload = payload;
    }

    int32_t getLutM(const std::size_t index) const
    {
        return m_payload[(index * 2)];
    }

    int32_t getLutB(const std::size_t index) const
    {
        return m_payload[(index * 2) + 1];
    }

    const PayloadType& payload() const { return m_payload; }
    static constexpr CommandType command() { return op::FOG_LUT_STREAM | (static_cast<uint32_t>(LUT_SIZE) << 2); }

    static std::size_t getNumberOfElementsInPayloadByCommand(const CommandType) { return LUT_SIZE; }
    static bool isThis(const CommandType cmd) { return (cmd & op::MASK) == op::FOG_LUT_STREAM; }

    FogLutStreamCmd& operator=(const FogLutStreamCmd& rhs)
    {
        std::copy(rhs.m_payload.begin(), rhs.m_payload.end(), m_lut.begin());
        m_payload = { m_lut };
        return *this;
    }

private:
    void setLutValue(const std::size_t index, const float m, const float b)
    {
        m_lut[(index * 2)] = static_cast<int32_t>(m);
        m_lut[(index * 2) + 1] = static_cast<int32_t>(b);
    }

    FogLutDesc m_lut;
    PayloadType m_payload;
};

} // namespace rr

#endif // _FOG_LUT_STREAM_CMD_HPP_
