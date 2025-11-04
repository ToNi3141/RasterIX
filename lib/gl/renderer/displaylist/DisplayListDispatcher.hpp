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

#ifndef DISPLAYLISTDISPATCHER_HPP
#define DISPLAYLISTDISPATCHER_HPP

#include <stdint.h>
#include <tcb/span.hpp>

namespace rr::displaylist
{

template <typename RenderConfig, typename TDisplayListAssembler>
class DisplayListDispatcher
{
public:
    DisplayListDispatcher(TDisplayListAssembler& displayListAssembler)
        : m_displayListAssembler { displayListAssembler }
    {
    }

    template <typename Command>
    bool addCommand(const std::size_t index, const Command& cmd)
    {
        return m_displayListAssembler[index].addCommand(cmd);
    }

    template <typename Command>
    bool addLastCommand(const Command& cmd)
    {
        const std::size_t lastLine = reverseDisplayListIndex(m_displayLines - 1);
        return m_displayListAssembler[lastLine].addCommand(cmd);
    }

    template <typename Command>
    bool addCommand(const Command& cmd)
    {
        bool ret = true;
        for (std::size_t i = 0; i < m_displayLines; i++)
        {
            ret = ret && addCommand(reverseDisplayListIndex(i), cmd);
        }
        return ret;
    }

    template <typename Factory, typename Pred>
    bool addCommandWithFactory_if(const Factory& commandFactory, const Pred& pred)
    {
        bool ret = true;
        for (std::size_t i = 0; i < m_displayLines; i++)
        {
            if (pred(reverseDisplayListIndex(i), m_displayLines, m_xResolution, m_yLineResolution))
            {
                ret = ret && addCommand(reverseDisplayListIndex(i), commandFactory(reverseDisplayListIndex(i), m_displayLines, m_xResolution, m_yLineResolution));
            }
        }
        return ret;
    }

    template <typename Factory, typename Pred>
    bool addLastCommandWithFactory_if(const Factory& commandFactory, const Pred& pred)
    {
        bool ret = true;
        const std::size_t lastLine = reverseDisplayListIndex(m_displayLines - 1);
        if (pred(lastLine, m_displayLines, m_xResolution, m_yLineResolution))
        {
            ret = ret && addCommand(lastLine, commandFactory(lastLine, m_displayLines, m_xResolution, m_yLineResolution));
        }
        return ret;
    }

    template <typename Function>
    bool displayListLooper(const Function& func)
    {
        bool ret = true;
        for (std::size_t i = 0; i < m_displayLines; i++)
        {
            ret = ret && func(*this, reverseDisplayListIndex(i), m_displayLines, m_xResolution, m_yLineResolution);
        }
        return ret;
    }

    bool setResolution(const std::size_t x, const std::size_t y)
    {
        const std::size_t framebufferSize = x * y;
        const std::size_t framebufferLines = (framebufferSize / RenderConfig::FRAMEBUFFER_SIZE_IN_PIXEL)
            + ((framebufferSize % RenderConfig::FRAMEBUFFER_SIZE_IN_PIXEL) ? 1 : 0);
        if (framebufferLines > RenderConfig::getDisplayLines())
        {
            // More lines required than lines available
            return false;
        }

        m_yLineResolution = y / framebufferLines;
        m_xResolution = x;
        m_displayLines = framebufferLines;
        return true;
    }

    std::size_t getYLineResolution() const
    {
        return m_yLineResolution;
    }

    static constexpr bool singleList()
    {
        return RenderConfig::getDisplayLines() == 1;
    }

    void clearDisplayListAssembler()
    {
        for (std::size_t i = 0; i < m_displayLines; i++)
        {
            m_displayListAssembler[i].clearAssembler();
        }
    }

    std::size_t getDisplayListBufferId(std::size_t displayList) const
    {
        return m_displayListAssembler[displayList].getDisplayListBufferId();
    }

    std::size_t getDisplayListSize(std::size_t displayList) const
    {
        return m_displayListAssembler[displayList].getDisplayListSize();
    }

private:
    std::size_t reverseDisplayListIndex(const std::size_t i) const
    {
        // Reverse the order of the display list to get a continuous stream from the internal framebuffer to a
        // AXIS display. Otherwise the partial framebuffers appear in the wrong order on the screen.
        // It does not matter for memory mapped framebuffers. A memory mapped framebuffer supports any order.
        return m_displayLines - i - 1;
    }

    std::size_t m_yLineResolution { 128 };
    std::size_t m_xResolution { 640 };
    std::size_t m_displayLines { RenderConfig::getDisplayLines() };
    TDisplayListAssembler& m_displayListAssembler;
};

} // namespace rr::displaylist
#endif // DISPLAYLISTDISPATCHER_HPP
