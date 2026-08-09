// RasterIX
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

#ifndef RASTERIZER_HPP
#define RASTERIZER_HPP
#include "Triangle.hpp"
#include "commands/TriangleStreamTypes.hpp"
#include "math/Vec.hpp"
#include <array>
#include <bitset>
#include <stdint.h>
#include <tcb/span.hpp>

namespace rr
{
class Rasterizer
{
public:
    Rasterizer(const bool enableScaling)
        : m_enableScaling(enableScaling)
    {
    }

    bool rasterize(TriangleStreamTypes::TriangleDesc& __restrict desc,
        const TransformedTriangle& triangle) const;

    void setScissorBox(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height);
    void enableScissor(const bool enable) { m_enableScissor = enable; }
    void enableTmu(const std::size_t tmu, const bool enable) { m_tmuEnable[tmu] = enable; }
    void setScissorStart(const int32_t x, const int32_t y)
    {
        m_scissorStartX = x << EDGE_FUNC_SIZE;
        m_scissorStartY = y << EDGE_FUNC_SIZE;
    }
    void setScissorEnd(const uint32_t width, const uint32_t height)
    {
        m_scissorEndX = (width << EDGE_FUNC_SIZE) + m_scissorStartX;
        m_scissorEndY = (height << EDGE_FUNC_SIZE) + m_scissorStartY;
    }

    static float edgeFunction(const Vec4& a, const Vec4& b, const Vec4& c);

    static bool increment(TriangleStreamTypes::TriangleDesc& desc,
        const std::size_t lineStart,
        const std::size_t lineEnd);

    static bool checkIfTriangleIsInBounds(const TriangleStreamTypes::StaticParams& params,
        const std::size_t lineStart,
        const std::size_t lineEnd)
    {
        // Check if the triangle is in the current area by checking if the end position is below the start line
        // and if the start of the triangle is within this area
        return ((params.bbEndY >= lineStart) && (params.bbStartY < lineEnd));
    }

private:
    static constexpr uint32_t EDGE_FUNC_SIZE = 5;
    static constexpr int32_t EDGE_FUNC_ZERO_P_FIVE = (1 << (EDGE_FUNC_SIZE - 1));
    static constexpr int32_t EDGE_FUNC_ONE_P_ZERO = (1 << EDGE_FUNC_SIZE);

    inline static VecInt edgeFunctionX(const Vec2i& a, const Vec2i& b, const Vec2i& c);

    int32_t m_scissorStartX { 0 };
    int32_t m_scissorStartY { 0 };
    int32_t m_scissorEndX { 0 };
    int32_t m_scissorEndY { 0 };
    bool m_enableScissor { false };
    const bool m_enableScaling { false };
    std::bitset<RenderConfig::TMU_COUNT> m_tmuEnable {};
};

} // namespace rr
#endif // RASTERIZER_HPP
