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

#include "SoftwareRasterizer.hpp"
#include <variant>

namespace rr::softwarerasterizer
{

SoftwareRasterizer::SoftwareRasterizer(IBusConnector& busConnector)
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

void SoftwareRasterizer::streamDisplayList(const uint8_t index, const uint32_t size)
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

bool SoftwareRasterizer::handleCommand(const FramebufferCmd& cmd)
{
    if (cmd.getSwapFramebuffer())
    {
        if (cmd.getSelectColorBuffer())
        {
            m_busConnector.writeData(0, cmd.getFramebufferSizeInPixel() * 2, m_colorBuffer.getAddress());
        }
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
        // No need to commit an framebuffer. The software rasterizer always works on a full framebuffer.
        return true;
    }
    // Load
    if (cmd.getLoadFramebuffer())
    {
        // No need to load an framebuffer. The software rasterizer always works on a full framebuffer.
        return true;
    }
    SPDLOG_CRITICAL("FramebufferCmd was not correctly handled and is ignored. This might cause the renderer to crash ...");
    return true;
}

bool SoftwareRasterizer::handleCommand(const FogLutStreamCmd& cmd)
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

bool SoftwareRasterizer::handleCommand(const TriangleStreamCmd& cmd)
{
    const TriangleStreamTypes::TriangleDesc attributesData = cmd.payload()[0];
    m_rasterizer.init(attributesData);
    while (!m_rasterizer.isDone())
    {
        const FragmentData fmd = m_rasterizer.hit();
        if (fmd.hit)
        {
            const InterpolatedAttributesData interpolatedAttributes = m_attributeInterpolator.interpolate(attributesData, fmd.bbx, fmd.bby);
            const uint16_t depth = m_depthBuffer.readFragment(fmd.index);
            const uint8_t stencil = m_stencilBuffer.readFragment(fmd.index);
            m_depthFunc.setReferenceValue(depth);
            const uint16_t depthZ16 = softwarerasterizerhelpers::serializeDepth(interpolatedAttributes.depthZ);
            const bool zPass = m_depthFunc.check(depthZ16);
            const bool stencilPass = m_stencilFunc.check(stencil);
            if (m_stencilOp.getEnable())
            {
                const uint8_t newStencilValue = m_stencilOp.op(stencil, zPass, stencilPass);
                m_stencilBuffer.writeFragment(newStencilValue, fmd.index, fmd.spx, fmd.spy);
            }

            if (zPass && stencilPass)
            {
                // Fragment processing
                const Vec4 texel0 = m_textureMapper[0].getTexel(interpolatedAttributes.tex[0].s, interpolatedAttributes.tex[0].t);
                const Vec4 texel1 = m_textureMapper[1].getTexel(interpolatedAttributes.tex[1].s, interpolatedAttributes.tex[1].t);
                const Vec4 texEnvTexel0 = m_texEnv[0].apply(interpolatedAttributes.color, texel0, interpolatedAttributes.color);
                const Vec4 texEnvTexel1 = m_texEnv[1].apply(texEnvTexel0, texel1, interpolatedAttributes.color);
                if (m_alphaFunc.check(texEnvTexel1[3]))
                {
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
                    m_depthBuffer.writeFragment(depthZ16, fmd.index, fmd.spx, fmd.spy);
                    m_colorBuffer.writeFragment(softwarerasterizerhelpers::serializeToRgb565(finalColor), fmd.index, fmd.spx, fmd.spy);
                }
            }
        }
        m_rasterizer.walk();
    }
    return true;
}

bool SoftwareRasterizer::handleCommand(const PushVertexCmd&)
{
    SPDLOG_WARN("PushVertexCmd is not implemented in the software rasterizer and is ignored.");
    return true;
}

bool SoftwareRasterizer::handleCommand(const SetElementGlobalCtxCmd&)
{
    SPDLOG_WARN("SetElementGlobalCtxCmd is not implemented in the software rasterizer and is ignored.");
    return true;
}

bool SoftwareRasterizer::handleCommand(const SetElementLocalCtxCmd&)
{
    SPDLOG_WARN("SetElementLocalCtxCmd is not implemented in the software rasterizer and is ignored.");
    return true;
}

bool SoftwareRasterizer::handleCommand(const SetLightingCtxCmd&)
{
    SPDLOG_WARN("SetLightingCtxCmd is not implemented in the software rasterizer and is ignored.");
    return true;
}

bool SoftwareRasterizer::handleCommand(const DrawNewElementCmd&)
{
    SPDLOG_WARN("DrawNewElementCmd is not implemented in the software rasterizer and is ignored.");
    return true;
}

bool SoftwareRasterizer::handleCommand(const NopCmd&)
{
    return true;
}

bool SoftwareRasterizer::handleCommand(const TextureStreamCmd& cmd)
{
    m_textureMapper[cmd.getTmu()].setPages(cmd.payload());
    return true;
}

bool SoftwareRasterizer::handleCommand(const WriteRegisterCmd& cmd)
{
    return std::visit(
        [this](const auto& reg)
        {
            return handleRegister(reg);
        },
        cmd.getRegister());
}

bool SoftwareRasterizer::handleRegister(const ColorBufferAddrReg& reg)
{
    m_colorBuffer.setAddress(reg.getValue());
    return true;
}

bool SoftwareRasterizer::handleRegister(const ColorBufferClearColorReg& reg)
{
    m_colorBuffer.setClearColor(softwarerasterizerhelpers::serializeToRgb565(reg.getColorf()));
    return true;
}

bool SoftwareRasterizer::handleRegister(const DepthBufferAddrReg& reg)
{
    m_depthBuffer.setAddress(reg.getValue());
    return true;
}

bool SoftwareRasterizer::handleRegister(const DepthBufferClearDepthReg& reg)
{
    m_depthBuffer.setClearColor(reg.getValue());
    return true;
}

bool SoftwareRasterizer::handleRegister(const FeatureEnableReg& reg)
{
    m_scissorData.enabled = reg.getEnableScissor();
    m_alphaFunc.setEnable(reg.getEnableAlphaTest());
    m_depthFunc.setEnable(reg.getEnableDepthTest());
    m_stencilFunc.setEnable(reg.getEnableStencilTest());
    m_textureMapper[0].setEnable(reg.getEnableTmu(0));
    m_textureMapper[1].setEnable(reg.getEnableTmu(1));
    m_texEnv[0].setEnable(reg.getEnableTmu(0));
    m_texEnv[1].setEnable(reg.getEnableTmu(1));
    m_fog.setEnable(reg.getEnableFog());
    m_blendFunc.setEnable(reg.getEnableBlending());
    m_logicOp.setEnable(reg.getEnableLogicOp());
    m_stencilOp.setEnable(reg.getEnableStencilTest());
    m_attributeInterpolator.setEnableTMU(0, reg.getEnableTmu(0));
    m_attributeInterpolator.setEnableTMU(1, reg.getEnableTmu(1));
    return true;
}

bool SoftwareRasterizer::handleRegister(const FogColorReg& reg)
{
    m_fog.setFogColor(reg.getColorf());
    return true;
}

bool SoftwareRasterizer::handleRegister(const FragmentPipelineReg& reg)
{
    m_alphaFunc.setFunction(reg.getAlphaFunc());
    m_alphaFunc.setReferenceValue(static_cast<float>(reg.getRefAlphaValue()) / 255.0f);
    m_depthFunc.setFunction(reg.getDepthFunc());
    m_blendFunc.setSFactor(reg.getBlendFuncSFactor());
    m_blendFunc.setDFactor(reg.getBlendFuncDFactor());
    m_logicOp.setLogicOp(reg.getLogicOp());
    m_colorBuffer.setMask(softwarerasterizerhelpers::convertColorMask(
        reg.getColorMaskR(),
        reg.getColorMaskG(),
        reg.getColorMaskB(),
        reg.getColorMaskA(),
        DevicePixelFormat::RGB565));
    m_depthBuffer.setMask(softwarerasterizerhelpers::convertDepthMask(reg.getDepthMask()));
    return true;
}

bool SoftwareRasterizer::handleRegister(RenderResolutionReg reg)
{
    m_resolutionData.x = reg.getX();
    m_resolutionData.y = reg.getY();
    return true;
}

bool SoftwareRasterizer::handleRegister(const ScissorEndReg& reg)
{
    m_scissorData.endX = reg.getX();
    m_scissorData.endY = reg.getY();
    return true;
}

bool SoftwareRasterizer::handleRegister(const ScissorStartReg& reg)
{
    m_scissorData.startX = reg.getX();
    m_scissorData.startY = reg.getY();
    return true;
}

bool SoftwareRasterizer::handleRegister(const StencilBufferAddrReg& reg)
{
    m_stencilBuffer.setAddress(reg.getValue());
    return true;
}

bool SoftwareRasterizer::handleRegister(const StencilReg& reg)
{
    m_stencilBuffer.setClearColor(reg.getClearStencil());
    m_stencilBuffer.setMask(reg.getStencilMask());
    m_stencilFunc.setReferenceValue(reg.getRef() & reg.getStencilMask());
    m_stencilFunc.setFunction(reg.getTestFunc());
    m_stencilOp.setFailOp(reg.getOpFail());
    m_stencilOp.setZFailOp(reg.getOpZFail());
    m_stencilOp.setZPassOp(reg.getOpZPass());
    m_stencilOp.setRefValue(reg.getRef());
    return true;
}

bool SoftwareRasterizer::handleRegister(const TexEnvColorReg& reg)
{
    m_texEnv[reg.getTmuFromAddr()].setEnvColor(reg.getColorf());
    return true;
}

bool SoftwareRasterizer::handleRegister(const TexEnvReg& reg)
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

bool SoftwareRasterizer::handleRegister(const TmuTextureReg& reg)
{
    m_textureMapper[reg.getTmuFromAddr()].setTextureSize(reg.getTextureWidth(), reg.getTextureHeight());
    m_textureMapper[reg.getTmuFromAddr()].setWrapMode(reg.getWrapModeS(), reg.getWrapModeT());
    m_textureMapper[reg.getTmuFromAddr()].setEnableMagFilter(reg.getEnableMagFilter());
    m_textureMapper[reg.getTmuFromAddr()].setEnableMinFilter(reg.getEnableMinFilter());
    m_textureMapper[reg.getTmuFromAddr()].setPixelFormat(reg.getPixelFormat());
    return true;
}

bool SoftwareRasterizer::handleRegister(const YOffsetReg& reg)
{
    m_rasterizer.setYOffset(reg.getY());
    m_colorBuffer.setYOffset(reg.getY());
    m_depthBuffer.setYOffset(reg.getY());
    m_stencilBuffer.setYOffset(reg.getY());
    return true;
}

bool SoftwareRasterizer::handleRegister(const std::monostate&)
{
    return true;
}

} // namespace rr::softwarerasterizer
