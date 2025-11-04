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

#ifndef _WRITE_REGISTER_CMD_HPP_
#define _WRITE_REGISTER_CMD_HPP_

#include "Op.hpp"
#include "renderer/registers/RegisterVariant.hpp"
#include <array>
#include <cstdint>
#include <tcb/span.hpp>

namespace rr
{

class WriteRegisterCmd
{
public:
    using PayloadType = tcb::span<const uint32_t>;
    using CommandType = uint32_t;

    WriteRegisterCmd() = default;
    WriteRegisterCmd(const RegisterVariant& reg)
    {
        const CommandType regAddr = std::visit(
            [this](const auto& r) -> CommandType
            {
                if constexpr (!std::is_same_v<std::decay_t<decltype(r)>, std::monostate>)
                {
                    m_val[0] = r.serialize();
                    return r.getAddr();
                }
                else
                {
                    return 0xffffffff;
                }
            },
            reg);
        m_op = op::RENDER_CONFIG | regAddr;
        m_payload = { m_val };
    }

    WriteRegisterCmd(const CommandType op, const PayloadType& payload, const bool)
    {
        m_op = op;
        m_payload = payload;
    }

    const PayloadType& payload() const { return m_payload; }
    CommandType command() const { return m_op; }

    static std::size_t getNumberOfElementsInPayloadByCommand(const uint32_t) { return 1; }
    static bool isThis(const CommandType cmd) { return (cmd & op::MASK) == op::RENDER_CONFIG; }

    static uint32_t getRegAddr(const CommandType cmd) { return cmd & ~op::MASK; };
    uint32_t getRegAddr() const { return getRegAddr(m_op); }

    RegisterVariant getRegister() const
    {
        switch (getRegAddr())
        {
        case ColorBufferAddrReg::getAddr():
            return deserializeRegister<ColorBufferAddrReg>();
        case ColorBufferClearColorReg::getAddr():
            return deserializeRegister<ColorBufferClearColorReg>();
        case DepthBufferAddrReg::getAddr():
            return deserializeRegister<DepthBufferAddrReg>();
        case DepthBufferClearDepthReg::getAddr():
            return deserializeRegister<DepthBufferClearDepthReg>();
        case FeatureEnableReg::getAddr():
            return deserializeRegister<FeatureEnableReg>();
        case FogColorReg::getAddr():
            return deserializeRegister<FogColorReg>();
        case FragmentPipelineReg::getAddr():
            return deserializeRegister<FragmentPipelineReg>();
        case RenderResolutionReg::getAddr():
            return deserializeRegister<RenderResolutionReg>();
        case ScissorEndReg::getAddr():
            return deserializeRegister<ScissorEndReg>();
        case ScissorStartReg::getAddr():
            return deserializeRegister<ScissorStartReg>();
        case StencilBufferAddrReg::getAddr():
            return deserializeRegister<StencilBufferAddrReg>();
        case StencilReg::getAddr():
            return deserializeRegister<StencilReg>();
        case TexEnvColorReg::getAddr(0):
            return deserializeTextureRegister<TexEnvColorReg>(0);
        case TexEnvReg::getAddr(0):
            return deserializeTextureRegister<TexEnvReg>(0);
        case TmuTextureReg::getAddr(0):
            return deserializeTextureRegister<TmuTextureReg>(0);
        case TexEnvColorReg::getAddr(1):
            return deserializeTextureRegister<TexEnvColorReg>(1);
        case TexEnvReg::getAddr(1):
            return deserializeTextureRegister<TexEnvReg>(1);
        case TmuTextureReg::getAddr(1):
            return deserializeTextureRegister<TmuTextureReg>(1);
        case YOffsetReg::getAddr():
            return deserializeRegister<YOffsetReg>();
        default:
            break;
        }
        return RegisterVariant {};
    }

private:
    template <typename T>
    T deserializeRegister() const
    {
        T reg {};
        reg.deserialize(m_payload[0]);
        return reg;
    }

    template <typename T>
    T deserializeTextureRegister(const std::size_t tmu) const
    {
        T reg {};
        reg.deserialize(m_payload[0]);
        reg.setTmu(tmu);
        return reg;
    }

    CommandType m_op {};
    std::array<uint32_t, 1> m_val;
    PayloadType m_payload;
};

} // namespace rr

#endif // _WRITE_REGISTER_CMD_HPP_
