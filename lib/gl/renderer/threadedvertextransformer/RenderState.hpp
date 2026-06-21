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
#include <type_traits>

namespace rr::threadedvertextransformer
{

template <typename T, typename Variant>
struct is_variant_alternative;

template <typename T, typename... Ts>
struct is_variant_alternative<T, std::variant<Ts...>>
    : std::disjunction<std::is_same<T, Ts>...>
{
};

template <typename T, typename Variant>
inline constexpr bool is_variant_alternative_v = is_variant_alternative<T, Variant>::value;

template <typename T, typename Variant, std::size_t I = 0>
struct variant_index;

template <typename T, typename First, typename... Rest, std::size_t I>
struct variant_index<T, std::variant<First, Rest...>, I>
    : std::conditional_t<std::is_same_v<T, First>,
          std::integral_constant<std::size_t, I>,
          variant_index<T, std::variant<Rest...>, I + 1>>
{
};

template <typename T, typename Variant>
inline constexpr std::size_t variant_index_v = variant_index<T, Variant>::value;

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
        for (const auto& texRegArray : m_state.texRegisters)
        {
            for (const auto& texReg : texRegArray)
            {
                std::visit(regHandlers, texReg);
            }
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
        std::array<std::array<TextureRegisterVariant, std::variant_size_v<TextureRegisterVariant>>, RenderConfig::TMU_COUNT> texRegisters {};
        FogLutStreamCmd fogLut {};
        std::array<TextureStreamCmd, RenderConfig::TMU_COUNT> textureStream {};
    };

    void storeRegister(const WriteRegisterCmd& cmd)
    {
        const auto reg = cmd.getRegister();
        std::visit([&](const auto& r)
            {
            using T = std::decay_t<decltype(r)>;
            if constexpr (is_variant_alternative_v<T, TextureRegisterVariant> && !std::is_same_v<T, std::monostate>)
            {
                m_state.texRegisters[r.getTmuFromAddr()][variant_index_v<T, TextureRegisterVariant>] = r;
            }
            else
            {
                m_state.registers[reg.index()] = reg;
            } },
            reg);
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

} // namespace rr::threadedvertextransformer

#endif // _RENDER_STATE_HPP_
