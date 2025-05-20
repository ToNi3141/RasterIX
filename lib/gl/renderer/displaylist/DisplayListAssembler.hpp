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

#ifndef DISPLAYLISTASSEMBLER_HPP
#define DISPLAYLISTASSEMBLER_HPP

#include "RIXDisplayListAssembler.hpp"
#include "TextureLoadOptimizer.hpp"
#include <algorithm>
#include <array>
#include <bitset>
#include <stdint.h>
#include <tcb/span.hpp>

namespace rr::displaylist
{

template <std::size_t TMU_COUNT, typename TDisplayList, bool Optimize = true>
class DisplayListAssembler
{
public:
    void setBuffer(tcb::span<uint8_t> buffer, std::size_t bufferId)
    {
        m_displayListBufferId = bufferId;
        m_displayList.setBuffer(buffer);
    }

    std::size_t getDisplayListSize() const
    {
        return m_displayList.getSize();
    }

    std::size_t getFreeSpace() const
    {
        return m_displayList.getFreeSpace();
    }

    std::size_t getDisplayListBufferId() const
    {
        return m_displayListBufferId;
    }

    void clearAssembler()
    {
        m_displayList.clear();
        m_textureLoadOptimizer.reset();
    }

    template <typename TCommand>
    std::size_t getCommandSize(std::size_t i) const
    {
        return m_rixDisplayListAssembler.template getCommandSize<TCommand>(i);
    }

    template <typename TCommand>
    std::size_t getCommandSize(const TCommand& cmd) const
    {
        return m_rixDisplayListAssembler.getCommandSize(cmd);
    }

    template <typename TCommand>
    bool copyCommand(TDisplayList& src)
    {
        return m_rixDisplayListAssembler.template copyCommand<TCommand>(src);
    }

    template <typename TCommand>
    bool addCommand(const TCommand& cmd)
    {
        if constexpr (Optimize)
        {
            m_textureLoadOptimizer.optimize(cmd);
        }
        return m_rixDisplayListAssembler.addCommand(cmd);
    }

private:
    template <typename TCommand>
    bool hasDisplayListEnoughSpace(const TCommand& cmd)
    {
        if (getCommandSize(cmd) >= m_displayList.getFreeSpace())
        {
            return false;
        }
        return true;
    }

    TDisplayList m_displayList {};
    RIXDisplayListAssembler<TDisplayList> m_rixDisplayListAssembler { m_displayList };
    TextureLoadOptimizer<TMU_COUNT, TDisplayList> m_textureLoadOptimizer { m_displayList };
    std::size_t m_displayListBufferId { 0 };
};

} // namespace rr::displaylist

#endif // DISPLAYLISTASSEMBLER_HPP
