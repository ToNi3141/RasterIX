#ifndef DISPLAYLISTDISASSEMBLER_HPP_
#define DISPLAYLISTDISASSEMBLER_HPP_

#include "renderer/commands/DisplayListCommand.hpp"
#include "renderer/displaylist/DisplayList.hpp"
#include <spdlog/spdlog.h>

namespace rr::displaylist
{

class DisplayListDisassembler
{
public:
    DisplayListDisassembler(DisplayList& displayList)
        : m_displayList { displayList }
    {
    }

    DisplayListCommand getNextCommand()
    {
        if (m_displayList.atEnd())
        {
            return DisplayListCommand {};
        }

        return decodeCommand(m_displayList);
    }

    bool hasNextCommand() const
    {
        return !m_displayList.atEnd();
    }

private:
    DisplayListCommand decodeCommand(DisplayList& srcList)
    {
        const uint32_t op = *(srcList.lookAhead<uint32_t>());

        if (PushVertexCmd::isThis(op))
        {
            return deserializeCommand<PushVertexCmd>(srcList);
        }
        else if (SetVertexCtxCmd::isThis(op))
        {
            return deserializeCommand<SetVertexCtxCmd>(srcList);
        }
        else if (WriteRegisterCmd<BaseColorReg>::isThis(op))
        {
            return deserializeCommand<WriteRegisterCmd<BaseColorReg>>(srcList);
        }
        else if (NopCmd::isThis(op))
        {
            return deserializeCommand<NopCmd>(srcList);
        }
        else if (TextureStreamCmd::isThis(op))
        {
            return deserializeCommand<TextureStreamCmd>(srcList);
        }
        else if (FramebufferCmd::isThis(op))
        {
            return deserializeCommand<FramebufferCmd>(srcList);
        }
        else if (FogLutStreamCmd::isThis(op))
        {
            return deserializeCommand<FogLutStreamCmd>(srcList);
        }
        else if (TriangleStreamCmd::isThis(op))
        {
            return deserializeCommand<TriangleStreamCmd>(srcList);
        }

        SPDLOG_CRITICAL("Unknown command (0x{:X}) found. This might cause the renderer to crash ...", op);
        return {};
    }

    template <typename TCmd>
    DisplayListCommand deserializeCommand(DisplayList& src)
    {
        using PayloadType = typename std::remove_const<typename std::remove_reference<decltype(TCmd {}.payload()[0])>::type>::type;
        const typename TCmd::CommandType* op = src.getNext<typename TCmd::CommandType>();
        const PayloadType* pl = nullptr;
        const std::size_t numberOfElements = TCmd::getNumberOfElementsInPayloadByCommand(*op);
        if (numberOfElements > 0)
        {
            pl = src.getNext<PayloadType>(); // store start addr
            // push display list to the end of the payload
            for (std::size_t i = 0; i < numberOfElements - 1; i++)
            {
                src.getNext<PayloadType>();
            }
        }
        // The third argument exists because sometimes (TextureStreamCmd) the constructors are ambiguous.
        // The bool enforces the correct constructor
        return TCmd { *op, { pl, numberOfElements }, true };
    }

    DisplayList& m_displayList;
};

} // namespace rr::displaylist
#endif // DISPLAYLISTDISASSEMBLER_HPP_