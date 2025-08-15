// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

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

#include "VertexPipeline.hpp"
#include "math/Vec.hpp"
#include "math/Veci.hpp"
#include <cmath>
#include <spdlog/spdlog.h>
#include <stdlib.h>
#include <string.h>

// The Arduino IDE will produce compile errors when using std::min and std::max
#include <algorithm> // std::max
#define max std::max
#define min std::min

namespace rr
{
VertexPipeline::VertexPipeline(PixelPipeline& renderer)
    : m_renderer { renderer }
{
    for (std::size_t i = 0; i < m_texGen.size(); i++)
    {
        m_texGen[i].setTexGenData(m_elementGlobalData.texGen[i], m_matrixStore.getModelView());
    }
    setEnableNormalizing(false);
}

VertexParameter VertexPipeline::fetch(const RenderObj& obj, std::size_t i)
{
    VertexParameter parameter;
    const std::size_t pos = obj.getIndex(i);
    parameter.vertex = obj.getVertex(pos);
    parameter.normal = obj.getNormal(pos);
    parameter.color = obj.getColor(pos);
    for (std::size_t tu = 0; tu < RenderConfig::TMU_COUNT; tu++)
    {
        parameter.tex[tu] = obj.getTexCoord(tu, pos);
    }
    return parameter;
}

void VertexPipeline::updateGlobalElementContext()
{
    if (m_lighting.dataHasChanged())
    {
        m_renderer.setLightingContext(m_lightingData);
        m_lighting.clearDataChangedFlag();
    }

    if (std::memcmp(&m_elementGlobalDataTransferred, &m_elementGlobalData, sizeof(m_elementGlobalDataTransferred)) != 0)
    {
        m_renderer.setElementGlobalContext(m_elementGlobalData);
        m_elementGlobalDataTransferred = m_elementGlobalData;
    }
}

bool VertexPipeline::drawObj(const RenderObj& obj)
{
    if (!obj.vertexArrayEnabled())
    {
        SPDLOG_INFO("drawObj(): Vertex array disabled. No primitive is rendered.");
        return true;
    }

    updateGlobalElementContext();

    if (!updatePipeline())
    {
        SPDLOG_ERROR("drawObj(): Cannot update pixel pipeline");
        return false;
    }
    obj.logCurrentConfig();

    m_primitiveAssembler.setDrawMode(obj.getDrawMode());
    m_primitiveAssembler.setExpectedPrimitiveCount(obj.getCount());

    for (std::size_t i = 0; i < RenderConfig::TMU_COUNT; i++)
    {
        m_elementLocalData.tmuEnabled[i] = m_renderer.featureEnable().getEnableTmu(i);
    }

    m_renderer.setElementLocalContext(m_elementLocalData);
    m_renderer.drawNewElement();

    std::size_t count = obj.getCount();
    for (std::size_t it = 0; it < count; it++)
    {
        m_renderer.pushVertex(fetch(obj, it));
    }

    return true;
}

bool VertexPipeline::updatePipeline()
{
    bool ret = m_renderer.updatePipeline();
    ret = ret && stencil().update();
    return ret;
}

bool VertexPipeline::clearFramebuffer(const bool frameBuffer, const bool zBuffer, const bool stencilBuffer)
{
    bool ret = updatePipeline();
    ret = ret && m_renderer.clearFramebuffer(frameBuffer, zBuffer, stencilBuffer);
    return ret;
}

} // namespace rr
