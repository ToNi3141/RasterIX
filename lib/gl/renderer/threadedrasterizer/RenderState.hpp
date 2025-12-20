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

#ifndef _RENDER_STATE_HPP_
#define _RENDER_STATE_HPP_

#include "renderer/commands/CommandVariant.hpp"
#include "renderer/registers/RegisterVariant.hpp"
#include <cstdint>
#include <tcb/span.hpp>

namespace rr
{

class RenderState
{
public:
    RenderState()
    {
    }

    template <typename TRegHandlers, typename TCmdHandlers>
    void restoreRenderState(const TRegHandlers& regHandlers, const TCmdHandlers& cmdHandlers)
    {
        for (const auto& r : m_state.registers)
        {
            std::visit(regHandlers, r);
        }
        cmdHandlers(m_state.fogLut);
        for (const auto& ts : m_state.textureStream)
        {
            cmdHandlers(ts);
        }
    }

    void storeCommand(const CommandVariant& cmd)
    {
        if (std::holds_alternative<WriteRegisterCmd>(cmd))
        {
            storeRegister(std::get<WriteRegisterCmd>(cmd));
        }
        else if (std::holds_alternative<TextureStreamCmd>(cmd))
        {
            storeTextureStream(std::get<TextureStreamCmd>(cmd));
        }
        else if (std::holds_alternative<FogLutStreamCmd>(cmd))
        {
            storeFogLut(std::get<FogLutStreamCmd>(cmd));
        }
    }

private:
    struct State
    {
        std::array<RegisterVariant, std::variant_size_v<RegisterVariant>> registers {};
        FogLutStreamCmd fogLut {};
        std::array<TextureStreamCmd, RenderConfig::TMU_COUNT> textureStream {};
    };

    void storeRegister(const WriteRegisterCmd& cmd)
    {
        m_state.registers[cmd.getRegister().index()] = cmd.getRegister();
    }

    void storeTextureStream(const TextureStreamCmd& cmd)
    {
        m_state.textureStream[cmd.getTmu()] = cmd;
    }

    void storeFogLut(const FogLutStreamCmd& cmd)
    {
        m_state.fogLut = cmd;
    }

    State m_state {};
};

} // namespace rr

#endif // _RENDER_STATE_HPP_
