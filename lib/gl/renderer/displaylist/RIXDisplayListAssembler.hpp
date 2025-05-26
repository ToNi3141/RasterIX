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

#ifndef RIXDISPLAYLISTASSEMBLER_HPP
#define RIXDISPLAYLISTASSEMBLER_HPP

#include <algorithm>
#include <array>
#include <stdint.h>
#include <tcb/span.hpp>

namespace rr::displaylist
{

template <typename TDisplayList>
class RIXDisplayListAssembler
{
public:
    RIXDisplayListAssembler(TDisplayList& displayList)
        : m_displayList { displayList }
    {
    }

    template <typename TCommand>
    static std::size_t getCommandSize(const std::size_t payloadSize)
    {
        using PayloadType = typename std::remove_const<typename std::remove_reference<decltype(TCommand {}.payload()[0])>::type>::type;
        std::size_t expectedSize = 0;
        expectedSize += TDisplayList::template sizeOf<typename TCommand::CommandType>();
        expectedSize += TDisplayList::template sizeOf<PayloadType>() * payloadSize;
        return expectedSize;
    }

    template <typename TCommand>
    static std::size_t getCommandSize(const TCommand& cmd)
    {
        return getCommandSize<TCommand>(cmd.payload().size());
    }

    template <typename TCommand>
    bool addCommand(const TCommand& cmd)
    {
        if (getCommandSize<TCommand>(cmd) >= m_displayList.getFreeSpace())
        {
            return false;
        }
        writeCommand(cmd);
        return true;
    }

private:
    template <typename TCommand>
    void writeCommand(const TCommand& cmd)
    {
        writeCommand<typename TCommand::CommandType, typename TCommand::PayloadType>(cmd.command(), cmd.payload());
    }

    template <typename TCommandType, typename TPayloadType>
    void writeCommand(const TCommandType& op, const TPayloadType& payload)
    {
        *(m_displayList.template create<TCommandType>()) = op;
        for (auto& a : payload)
        {
            *(m_displayList.template create<typename TPayloadType::value_type>()) = a;
        }
    }

    TDisplayList& m_displayList;
};

} // namespace rr::displaylist

#endif // RIXDISPLAYLISTASSEMBLER_HPP
