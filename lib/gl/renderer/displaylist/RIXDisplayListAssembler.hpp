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
    bool addCommand(const TCommand& cmd)
    {
        using CommandType = typename TCommand::CommandType;
        using SerializedPayloadType = typename TCommand::SerializedPayloadType;

        const std::size_t writePos = m_displayList.getCurrentWritePos();

        // Set op
        CommandType* const op = m_displayList.template create<CommandType>();
        if (op == nullptr)
        {
            return false;
        }
        *op = cmd.command();

        // Set payload
        const std::size_t numberOfElements = cmd.getNumberOfPayloadElements();
        for (std::size_t i = 0; i < numberOfElements; i++)
        {
            SerializedPayloadType* const element = m_displayList.template create<SerializedPayloadType>();
            if (element == nullptr)
            {
                m_displayList.setCurrentSize(writePos);
                return false;
            }
            if (!cmd.serializePayload(*element, i))
            {
                // Drops the slot if it fails to serialize the payload
                m_displayList.setCurrentSize(writePos);
                return true;
            }
        }
        return true;
    }

private:
    TDisplayList& m_displayList;
};

} // namespace rr::displaylist

#endif // RIXDISPLAYLISTASSEMBLER_HPP
