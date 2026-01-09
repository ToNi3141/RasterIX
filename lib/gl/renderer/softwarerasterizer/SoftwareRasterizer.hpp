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
#include "TestFunc.hpp"
#include "TexEnv.hpp"
#include "TextureMap.hpp"

#include <spdlog/spdlog.h>

namespace rr::softwarerasterizer
{

class SoftwareRasterizer : public IDevice
{
public:
    SoftwareRasterizer(IBusConnector& busConnector)
        : m_busConnector { busConnector }
    {
        m_gram = busConnector.requestWriteBuffer(0);
        m_colorBuffer.setGRAM(m_gram);
        m_depthBuffer.setGRAM(m_gram);
        m_stencilBuffer.setGRAM(m_gram);
        m_textureMapper[0].setGRAM(m_gram);
        m_textureMapper[1].setGRAM(m_gram);
        SPDLOG_INFO("Software rasterization enabled");
    }

    void deinit()
    {
    }

    void streamDisplayList(const uint8_t index, const uint32_t size) override
    {
        displaylist::DisplayList srcList {};
        srcList.setBuffer(requestDisplayListBuffer(index));
        srcList.resetGet();
        srcList.setCurrentSize(size);
        displaylist::DisplayListDisassembler disassembler { srcList };

        while (disassembler.hasNextCommand())
        {
            const bool ret = std::visit([this](const auto& cmd)
                { return handleCommand(cmd); },
                disassembler.getNextCommand());
            if (!ret)
            {
                SPDLOG_ERROR("Failed to handle command in display list. This might cause the renderer to crash ...");
            }
        }
    }

    bool writeToDeviceMemory(tcb::span<const uint8_t> data, const uint32_t addr) override
    {
        std::copy(data.begin(), data.end(), m_gram.data() + addr - RenderConfig::GRAM_MEMORY_LOC);
        return true;
    }

    bool readFromDeviceMemory(tcb::span<uint8_t> data, const uint32_t addr) override
    {
        return true;
    }

    void blockUntilDeviceIsIdle() override
    {
    }

    tcb::span<uint8_t> requestDisplayListBuffer(const uint8_t index) override
    {
        return { m_buffer[index] };
    }

    uint8_t getDisplayListBufferCount() const override
    {
        return m_buffer.size();
    }

private:
    bool handleCommand(const FramebufferCmd& cmd)
    {
        if (cmd.getSwapFramebuffer())
        {
            return true;
        }
        // Clear
        if (cmd.getEnableMemset())
        {
            if (cmd.getSelectColorBuffer())
            {
                m_colorBuffer.clear();
            }
            if (cmd.getSelectDepthBuffer())
            {
                m_depthBuffer.clear();
            }
            if (cmd.getSelectStencilBuffer())
            {
                m_stencilBuffer.clear();
            }
            return true;
        }
        // Commit
        if (cmd.getCommitFramebuffer())
        {
            if (cmd.getSelectColorBuffer())
            {
                m_busConnector.writeData(0, m_resolutionData.x * m_resolutionData.y * 2, m_colorBuffer.getAddress());
            }
            return true;
        }
        // Load
        if (cmd.getLoadFramebuffer())
        {
            return true;
        }
        SPDLOG_CRITICAL("FramebufferCmd was not correctly handled and is ignored. This might cause the renderer to crash ...");
        return true;
    }

    bool handleCommand(const FogLutStreamCmd& cmd)
    {
        Fog::FogLut lut {};
        for (std::size_t i = 0; i < lut.size(); i++)
        {
            lut[i].m = cmd.getLutM(i);
            lut[i].b = cmd.getLutB(i);
        }
        m_fog.setFogLut(lut, cmd.getLowerBound(), cmd.getUpperBound());
        return true;
    }

    bool handleCommand(const TriangleStreamCmd& cmd)
    {
        m_attributesData = cmd.payload()[0];
        m_rasterizer.init(m_attributesData);
        while (!m_rasterizer.isDone())
        {
            const FragmentData fmd = m_rasterizer.hit();
            if (fmd.hit)
            {
                InterpolatedAttributesData interpolatedAttributes = m_attributeInterpolator.interpolate(fmd.bbx, fmd.bby);
                const float depth = softwarerasterizerhelpers::deserializeDepth(m_depthBuffer.readFragment(fmd.index));
                const uint8_t stencil = m_stencilBuffer.readFragment(fmd.index);
                m_depthFunc.setReferenceValue(depth);

                if (m_depthFunc.check(interpolatedAttributes.depthZ) && m_stencilFunc.check(stencil))
                {
                    // Fragment processing
                    const Vec4 texel0 = m_textureMapper[0].getTexel(interpolatedAttributes.tex[0].s, interpolatedAttributes.tex[0].t);
                    const Vec4 texel1 = m_textureMapper[1].getTexel(interpolatedAttributes.tex[1].s, interpolatedAttributes.tex[1].t);
                    const Vec4 texEnvTexel0 = m_texEnv[0].apply(interpolatedAttributes.color, texel0, interpolatedAttributes.color);
                    const Vec4 texEnvTexel1 = m_texEnv[1].apply(texEnvTexel0, texel1, interpolatedAttributes.color);
                    const Vec4 foggedColor = m_fog.calculateFog(interpolatedAttributes.depthW, texEnvTexel1);
                    const Vec4 destColor = softwarerasterizerhelpers::deserializeFromRgb565(m_colorBuffer.readFragment(fmd.index));
                    Vec4 finalColor;
                    if (m_logicOp.getEnable())
                    {
                        finalColor = m_logicOp.op(foggedColor, destColor);
                    }
                    else
                    {
                        finalColor = m_blendFunc.blend(foggedColor, destColor);
                    }
                    m_depthBuffer.writeFragment(softwarerasterizerhelpers::serializeDepth(interpolatedAttributes.depthZ), fmd.index);
                    m_colorBuffer.writeFragment(softwarerasterizerhelpers::serializeToRgb565(finalColor), fmd.index);
                }
            }
            m_rasterizer.walk();
        }
        return true;
    }

    bool handleCommand(const PushVertexCmd& cmd)
    {
        return true;
    }

    bool handleCommand(const SetElementGlobalCtxCmd& cmd)
    {
        return true;
    }

    bool handleCommand(const SetElementLocalCtxCmd& cmd)
    {
        return true;
        ;
    }

    bool handleCommand(const SetLightingCtxCmd& cmd)
    {
        return true;
    }

    bool handleCommand(const DrawNewElementCmd&)
    {
        return true;
    }

    bool handleCommand(const NopCmd& cmd)
    {
        return true;
    }

    bool handleCommand(const TextureStreamCmd& cmd)
    {
        m_textureMapper[cmd.getTmu()].setPages(cmd.payload());
        return true;
    }

    bool handleCommand(const WriteRegisterCmd& cmd)
    {
        return std::visit(
            [this](const auto& reg)
            {
                return handleRegister(reg);
            },
            cmd.getRegister());
    }

    bool handleRegister(const ColorBufferAddrReg& reg)
    {
        m_colorBuffer.setAddress(reg.getValue());
        return true;
    }

    bool handleRegister(const ColorBufferClearColorReg& reg)
    {
        const uint16_t clearColor = ((static_cast<uint16_t>(reg.getColor()[0]) >> 3) << 11)
            | ((static_cast<uint16_t>(reg.getColor()[1]) >> 2) << 5)
            | ((static_cast<uint16_t>(reg.getColor()[2]) >> 3) << 0);
        m_colorBuffer.setClearColor(clearColor);
        return true;
    }

    bool handleRegister(const DepthBufferAddrReg& reg)
    {
        m_depthBuffer.setAddress(reg.getValue());
        return true;
    }

    bool handleRegister(const DepthBufferClearDepthReg& reg)
    {
        m_depthBuffer.setClearColor(reg.getValue());
        return true;
    }

    bool handleRegister(const FeatureEnableReg& reg)
    {
        m_scissorData.enabled = reg.getEnableScissor();
        m_alphaFunc.setEnable(reg.getEnableAlphaTest());
        m_depthFunc.setEnable(reg.getEnableDepthTest());
        m_depthBuffer.setEnable(reg.getEnableDepthTest());
        m_stencilFunc.setEnable(reg.getEnableStencilTest());
        m_stencilBuffer.setEnable(reg.getEnableStencilTest());
        m_textureMapper[0].setEnable(reg.getEnableTmu(0));
        m_textureMapper[1].setEnable(reg.getEnableTmu(1));
        m_texEnv[0].setEnable(reg.getEnableTmu(0));
        m_texEnv[1].setEnable(reg.getEnableTmu(1));
        m_fog.setEnable(reg.getEnableFog());
        m_blendFunc.setEnable(reg.getEnableBlending());
        m_logicOp.setEnable(reg.getEnableLogicOp());
        return true;
    }

    bool handleRegister(const FogColorReg& reg)
    {
        m_fog.setFogColor(reg.getColorf());
        return true;
    }

    bool handleRegister(const FragmentPipelineReg& reg)
    {
        m_alphaFunc.setFunction(reg.getAlphaFunc());
        m_alphaFunc.setReferenceValue(static_cast<float>(reg.getRefAlphaValue()) / 255.0f);
        m_depthFunc.setFunction(reg.getDepthFunc());
        m_blendFunc.setSFactor(reg.getBlendFuncSFactor());
        m_blendFunc.setDFactor(reg.getBlendFuncDFactor());
        m_logicOp.setLogicOp(reg.getLogicOp());
        // todo: depth and color mask
        return true;
    }

    bool handleRegister(RenderResolutionReg reg)
    {
        m_resolutionData.x = reg.getX();
        m_resolutionData.y = reg.getY();
        return true;
    }

    bool handleRegister(const ScissorEndReg& reg)
    {
        m_scissorData.endX = reg.getX();
        m_scissorData.endY = reg.getY();
        return true;
    }

    bool handleRegister(const ScissorStartReg& reg)
    {
        m_scissorData.startX = reg.getX();
        m_scissorData.startY = reg.getY();
        return true;
    }

    bool handleRegister(const StencilBufferAddrReg& reg)
    {
        m_stencilBuffer.setAddress(reg.getValue());
        return true;
    }

    bool handleRegister(const StencilReg& reg)
    {
        m_stencilBuffer.setClearColor(reg.getClearStencil());
        m_stencilFunc.setReferenceValue(reg.getRef());
        m_stencilFunc.setFunction(reg.getTestFunc());
        // TODO: stencil mask
        return true;
    }

    bool handleRegister(const TexEnvColorReg& reg)
    {
        m_texEnv[reg.getTmuFromAddr()].setEnvColor(reg.getColorf());
        return true;
    }

    bool handleRegister(const TexEnvReg& reg)
    {
        m_texEnv[reg.getTmuFromAddr()].setCombineRgb(reg.getCombineRgb());
        m_texEnv[reg.getTmuFromAddr()].setCombineAlpha(reg.getCombineAlpha());
        m_texEnv[reg.getTmuFromAddr()].setSrcRegRgb0(reg.getSrcRegRgb0());
        m_texEnv[reg.getTmuFromAddr()].setSrcRegRgb1(reg.getSrcRegRgb1());
        m_texEnv[reg.getTmuFromAddr()].setSrcRegRgb2(reg.getSrcRegRgb2());
        m_texEnv[reg.getTmuFromAddr()].setSrcRegAlpha0(reg.getSrcRegAlpha0());
        m_texEnv[reg.getTmuFromAddr()].setSrcRegAlpha1(reg.getSrcRegAlpha1());
        m_texEnv[reg.getTmuFromAddr()].setSrcRegAlpha2(reg.getSrcRegAlpha2());
        m_texEnv[reg.getTmuFromAddr()].setOperandRgb0(reg.getOperandRgb0());
        m_texEnv[reg.getTmuFromAddr()].setOperandRgb1(reg.getOperandRgb1());
        m_texEnv[reg.getTmuFromAddr()].setOperandRgb2(reg.getOperandRgb2());
        m_texEnv[reg.getTmuFromAddr()].setOperandAlpha0(reg.getOperandAlpha0());
        m_texEnv[reg.getTmuFromAddr()].setOperandAlpha1(reg.getOperandAlpha1());
        m_texEnv[reg.getTmuFromAddr()].setOperandAlpha2(reg.getOperandAlpha2());
        m_texEnv[reg.getTmuFromAddr()].setShiftRgb(reg.getShiftRgb());
        m_texEnv[reg.getTmuFromAddr()].setShiftAlpha(reg.getShiftAlpha());
        return true;
    }

    bool handleRegister(const TmuTextureReg& reg)
    {
        m_textureMapper[reg.getTmuFromAddr()].setTextureSize(reg.getTextureWidth(), reg.getTextureHeight());
        m_textureMapper[reg.getTmuFromAddr()].setWrapMode(reg.getWrapModeS(), reg.getWrapModeT());
        m_textureMapper[reg.getTmuFromAddr()].setEnableMagFilter(reg.getEnableMagFilter());
        m_textureMapper[reg.getTmuFromAddr()].setEnableMinFilter(reg.getEnableMinFilter());
        m_textureMapper[reg.getTmuFromAddr()].setPixelFormat(reg.getPixelFormat());
        return true;
    }

    bool handleRegister(const YOffsetReg&)
    {
        return true;
    }

    bool handleRegister(const std::monostate&)
    {
        return true;
    }

    IBusConnector& m_busConnector;

    std::array<std::array<uint8_t, RenderConfig::THREADED_RASTERIZATION_DISPLAY_LIST_BUFFER_SIZE>, 2> m_buffer;

    tcb::span<uint8_t> m_gram {};

    ScissorData m_scissorData {};
    ResolutionData m_resolutionData {};
    TriangleStreamTypes::TriangleDesc m_attributesData {};

    Framebuffer<uint16_t> m_colorBuffer { m_scissorData, m_resolutionData };
    Framebuffer<uint16_t> m_depthBuffer { m_scissorData, m_resolutionData };
    Framebuffer<uint8_t> m_stencilBuffer { m_scissorData, m_resolutionData };
    TestFunc<float> m_depthFunc {};
    TestFunc<uint8_t> m_stencilFunc {};
    TestFunc<float> m_alphaFunc {};
    Rasterizer m_rasterizer { m_resolutionData };
    AttributeInterpolator m_attributeInterpolator { m_attributesData };
    std::array<TextureMap, 2> m_textureMapper {};
    std::array<TexEnv, 2> m_texEnv {};
    Fog m_fog {};
    BlendFunc m_blendFunc {};
    LogicOp m_logicOp {};
};

} // namespace rr::softwarerasterizer

#endif // _SOFTWARE_RASTERIZER_HPP_
