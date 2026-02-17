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

#ifndef _SOFTWARE_RASTERIZER_HPP_
#define _SOFTWARE_RASTERIZER_HPP_

#include "IBusConnector.hpp"
#include "renderer/IDevice.hpp"
#include <cstdint>
#include <tcb/span.hpp>
#include <variant>

#include "renderer/commands/CommandVariant.hpp"

#include "renderer/Rasterizer.hpp"
#include "renderer/displaylist/DisplayListDisassembler.hpp"
#include "renderer/registers/BaseColorReg.hpp"
#include "renderer/registers/RegisterVariant.hpp"

#include "AttributeInterpolator.hpp"
#include "BlendFunc.hpp"
#include "Fog.hpp"
#include "Framebuffer.hpp"
#include "LogicOp.hpp"
#include "Rasterizer.hpp"
#include "ResolutionData.hpp"
#include "ScissorData.hpp"
#include "SoftwareRasterizerHelpers.hpp"
#include "StencilOp.hpp"
#include "TestFunc.hpp"
#include "TexEnv.hpp"
#include "TextureMap.hpp"

#include <spdlog/spdlog.h>

namespace rr::softwarerasterizer
{

class SoftwareRasterizationInstance
{
public:
    SoftwareRasterizationInstance(IBusConnector& busConnector);

    void deinit()
    {
    }

    void streamDisplayList(tcb::span<const uint8_t> displayList);
    void setSwapFramebufferEventHandler(std::function<void(const SoftwareRasterizationInstance*)> swapFramebuffer)
    {
        m_swapFramebufferEvent = swapFramebuffer;
    }

private:
    bool handleCommand(const FramebufferCmd& cmd);
    bool handleCommand(const FogLutStreamCmd& cmd);
    bool handleCommand(const TriangleStreamCmd& cmd);
    bool handleCommand(const PushVertexCmd&);
    bool handleCommand(const SetElementGlobalCtxCmd&);
    bool handleCommand(const SetElementLocalCtxCmd&);
    bool handleCommand(const SetLightingCtxCmd&);
    bool handleCommand(const DrawNewElementCmd&);
    bool handleCommand(const NopCmd& cmd);
    bool handleCommand(const TextureStreamCmd& cmd);
    bool handleCommand(const WriteRegisterCmd& cmd);

    bool handleRegister(const ColorBufferAddrReg& reg);
    bool handleRegister(const ColorBufferClearColorReg& reg);
    bool handleRegister(const DepthBufferAddrReg& reg);
    bool handleRegister(const DepthBufferClearDepthReg& reg);
    bool handleRegister(const FeatureEnableReg& reg);
    bool handleRegister(const FogColorReg& reg);
    bool handleRegister(const FragmentPipelineReg& reg);
    bool handleRegister(RenderResolutionReg reg);
    bool handleRegister(const ScissorEndReg& reg);
    bool handleRegister(const ScissorStartReg& reg);
    bool handleRegister(const StencilBufferAddrReg& reg);
    bool handleRegister(const StencilReg& reg);
    bool handleRegister(const TexEnvColorReg& reg);
    bool handleRegister(const TexEnvReg& reg);
    bool handleRegister(const TmuTextureReg& reg);
    bool handleRegister(const YOffsetReg& reg);
    bool handleRegister(const std::monostate&);

    IBusConnector& m_busConnector;

    tcb::span<uint8_t> m_gram {};

    ScissorData m_scissorData {};
    ResolutionData m_resolutionData {};

    Framebuffer<uint16_t> m_colorBuffer { m_scissorData, m_resolutionData };
    Framebuffer<uint16_t> m_depthBuffer { m_scissorData, m_resolutionData };
    Framebuffer<uint8_t> m_stencilBuffer { m_scissorData, m_resolutionData };
    TestFunc<uint16_t> m_depthFunc {};
    TestFunc<uint8_t> m_stencilFunc {};
    TestFunc<float> m_alphaFunc {};
    AttributeInterpolator m_attributeInterpolator {};
    Rasterizer m_rasterizer { m_resolutionData };
    std::array<TextureMap, 2> m_textureMapper {};
    std::array<TexEnv, 2> m_texEnv {};
    Fog m_fog {};
    BlendFunc m_blendFunc {};
    LogicOp m_logicOp {};
    StencilOp m_stencilOp {};
    std::function<void(const SoftwareRasterizationInstance*)> m_swapFramebufferEvent {};
};

} // namespace rr::softwarerasterizer

#endif // _SOFTWARE_RASTERIZER_HPP_
