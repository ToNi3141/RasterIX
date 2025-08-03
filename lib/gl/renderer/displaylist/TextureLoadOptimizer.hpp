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

#ifndef TEXTURELOADOPTIMIZER_HPP
#define TEXTURELOADOPTIMIZER_HPP

#include "RIXDisplayListAssembler.hpp"
#include "renderer/commands/DrawNewElementCmd.hpp"
#include "renderer/commands/TextureStreamCmd.hpp"
#include "renderer/commands/TriangleStreamCmd.hpp"
#include <algorithm>
#include <array>
#include <bitset>
#include <stdint.h>
#include <tcb/span.hpp>

namespace rr::displaylist
{
// Optimization for texture loading: To avoid unecessary texture loads, track if a texture was used by a triangle.
// If the texture wasn't used, then it replaces previous texture load with NOPs.
template <std::size_t TMU_COUNT, typename TDisplayList>
class TextureLoadOptimizer
{
public:
    TextureLoadOptimizer(TDisplayList& displayList)
        : m_displayList { displayList }
    {
        reset();
    }

    void reset()
    {
        m_textureCommandFlag.reset();
    }

    template <typename TCommand>
    void optimize(const TCommand& cmd)
    {
        markTriangleCommand<TCommand>();
        optimizeTextureLoad(cmd);
    }

private:
    template <typename TCommand>
    void markTriangleCommand()
    {
        if constexpr (std::is_same<TCommand, TriangleStreamCmd>::value
            || std::is_same<TCommand, DrawNewElementCmd>::value)
        {
            m_textureCommandFlag.reset();
        }
    }

    template <typename TCommand>
    void optimizeTextureLoad(const TCommand& cmd)
    {
        if constexpr (std::is_same<TCommand, TextureStreamCmd>::value)
        {
            const std::size_t tmu = cmd.getTmu();
            if (tmu >= m_textureCommandFlag.size())
            {
                return;
            }
            if (m_textureCommandFlag[tmu])
            {
                m_displayList.initArea(m_texPosInDisplayList[tmu], m_texSizeInDisplayList[tmu]);
            }
            m_textureCommandFlag.set(tmu);
            m_texPosInDisplayList[tmu] = m_displayList.getCurrentWritePos();
            m_texSizeInDisplayList[tmu] = RIXDisplayListAssembler<TDisplayList>::template getCommandSize(cmd);
        }
    }

    TDisplayList& m_displayList;

    std::bitset<TMU_COUNT> m_textureCommandFlag {};
    std::array<uint32_t, TMU_COUNT> m_texPosInDisplayList {};
    std::array<uint32_t, TMU_COUNT> m_texSizeInDisplayList {};
};

} // namespace rr::displaylist

#endif // TEXTURELOADOPTIMIZER_HPP
