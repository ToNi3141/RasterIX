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

#ifndef VERTEXTRANSFORMING_HPP
#define VERTEXTRANSFORMING_HPP

#include "Clipper.hpp"
#include "Culling.hpp"
#include "ElementGlobalData.hpp"
#include "ElementLocalData.hpp"
#include "Lighting.hpp"
#include "MatrixStore.hpp"
#include "PrimitiveAssembler.hpp"
#include "RenderConfigs.hpp"
#include "Stencil.hpp"
#include "TexGen.hpp"
#include "ViewPort.hpp"
#include "math/Vec.hpp"
#include <bitset>
#include <tcb/span.hpp>

namespace rr::vertextransforming
{

struct VertexTransformingData
{
    void setElementLocalData(const transform::ElementLocalData& data)
    {
        transformMatrices = data.transformMatrices;
        primitiveAssembler = data.primitiveAssembler;
        tmuEnabled = data.tmuEnabled;
    }

    void setElementGlobalData(const transform::ElementGlobalData& data)
    {
        viewPort = data.viewPort;
        culling = data.culling;
        stencil = data.stencil;
        texGen = data.texGen;
        normalizeLightNormal = data.normalizeLightNormal;
    }

    lighting::LightingData lighting {};
    matrixstore::TransformMatricesData transformMatrices {};
    primitiveassembler::PrimitiveAssemblerData primitiveAssembler {};
    std::bitset<RenderConfig::TMU_COUNT> tmuEnabled {};
    viewport::ViewPortData viewPort {};
    culling::CullingData culling {};
    stencil::StencilData stencil {};
    std::array<texgen::TexGenData, RenderConfig::TMU_COUNT> texGen {};
    bool normalizeLightNormal {};
};

template <typename TDrawTriangleFunc, typename TUpdateStencilFunc>
class VertexTransformingCalc
{
public:
    VertexTransformingCalc(
        const VertexTransformingData& data,
        const TDrawTriangleFunc& drawTriangleFunc,
        const TUpdateStencilFunc& updateStencilFunc)
        : m_data { data }
        , m_drawTriangleFunc { drawTriangleFunc }
        , m_updateStencilFunc { updateStencilFunc }
    {
    }

    bool pushVertex(VertexParameter param)
    {
        transform(param);
        m_primitiveAssembler.pushParameter(param);

        const tcb::span<const primitiveassembler::PrimitiveAssemblerCalc::Triangle> triangles = m_primitiveAssembler.getPrimitive();
        for (const primitiveassembler::PrimitiveAssemblerCalc::Triangle& triangle : triangles)
        {
            if (!drawTriangle(triangle))
            {
                return false;
            }
        }
        if (!triangles.empty())
        {
            m_primitiveAssembler.removePrimitive();
        }
        return true;
    }

    void init()
    {
        m_primitiveAssembler.init();
    }

    void* operator new(size_t, VertexTransformingCalc<TDrawTriangleFunc, TUpdateStencilFunc>* p) { return p; }

private:
    void transform(VertexParameter& parameter)
    {
        for (std::size_t tu = 0; tu < RenderConfig::TMU_COUNT; tu++)
        {
            if (m_data.tmuEnabled[tu])
            {
                texgen::TexGenCalc { m_data.texGen[tu] }.calculateTexGenCoords(
                    parameter.tex[tu],
                    m_data.transformMatrices,
                    parameter.vertex,
                    parameter.normal);
                parameter.tex[tu] = m_data.transformMatrices.texture[tu].transform(parameter.tex[tu]);
            }
        }

        // TODO: Check if this required? The standard requires but is it really used?
        // m_c[j].transform(color, color); // Calculate this in one batch to improve performance
        if (m_data.lighting.lightingEnabled)
        {
            Vec3 normal = m_data.transformMatrices.normal.transform(parameter.normal);

            if (m_data.normalizeLightNormal)
            {
                normal.normalize();
            }
            const Vec4 vl = m_data.transformMatrices.modelView.transform(parameter.vertex);
            const Vec4 c = parameter.color;
            lighting::LightingCalc { m_data.lighting }.calculateLights(parameter.color, c, vl, normal);
        }
        parameter.vertex = m_data.transformMatrices.modelViewProjection.transform(parameter.vertex);
    }

    bool drawClippedTriangleList(tcb::span<VertexParameter> list)
    {
        const std::size_t clippedVertexListSize = list.size();
        for (std::size_t i = 0; i < clippedVertexListSize; i++)
        {
            list[i].vertex.perspectiveDivide();
            viewport::ViewPortCalc { m_data.viewPort }.transform(list[i].vertex);
        }

        // Check only one triangle in the clipped list. The triangles are sub divided, but not rotated. So if one triangle is
        // facing backwards, then all in the clipping list will do this and vice versa.
        if (culling::CullingCalc { m_data.culling }.cull(list[0].vertex, list[1].vertex, list[2].vertex))
        {
            return true;
        }

        if (m_data.stencil.enableTwoSideStencil)
        {
            const StencilReg reg = stencil::StencilCalc { m_data.stencil }.updateStencilFace(list[0].vertex, list[1].vertex, list[2].vertex);
            if (!m_updateStencilFunc(reg))
            {
                return false;
            }
        }

        for (std::size_t i = 3; i <= clippedVertexListSize; i++)
        {
            const bool success = m_drawTriangleFunc({
                list[0].vertex,
                list[i - 2].vertex,
                list[i - 1].vertex,
                list[0].tex,
                list[i - 2].tex,
                list[i - 1].tex,
                list[0].color,
                list[i - 2].color,
                list[i - 1].color,
            });
            if (!success)
            {
                return false;
            }
        }
        return true;
    }

    bool drawUnclippedTriangle(const primitiveassembler::PrimitiveAssemblerCalc::Triangle& triangle)
    {
        // Optimized version of the drawTriangle when a triangle is not needed to be clipped.

        Vec4 v0 = triangle[0].get().vertex;
        Vec4 v1 = triangle[1].get().vertex;
        Vec4 v2 = triangle[2].get().vertex;

        // Perspective division
        v0.perspectiveDivide();
        v1.perspectiveDivide();
        v2.perspectiveDivide();

        // Viewport transformation of the vertex
        viewport::ViewPortCalc { m_data.viewPort }.transform(v0);
        viewport::ViewPortCalc { m_data.viewPort }.transform(v1);
        viewport::ViewPortCalc { m_data.viewPort }.transform(v2);

        // Check only one triangle in the clipped list. The triangles are sub divided, but not rotated. So if one triangle is
        // facing backwards, then all in the clipping list will do this and vice versa.
        if (culling::CullingCalc { m_data.culling }.cull(v0, v1, v2))
        {
            return true;
        }

        if (m_data.stencil.enableTwoSideStencil)
        {
            const StencilReg reg = stencil::StencilCalc { m_data.stencil }.updateStencilFace(v0, v1, v2);
            if (!m_updateStencilFunc(reg))
            {
                return false;
            }
        }

        return m_drawTriangleFunc({
            v0,
            v1,
            v2,
            triangle[0].get().tex,
            triangle[1].get().tex,
            triangle[2].get().tex,
            triangle[0].get().color,
            triangle[1].get().color,
            triangle[2].get().color,
        });
    }

    bool drawTriangle(const primitiveassembler::PrimitiveAssemblerCalc::Triangle& triangle)
    {
        if (Clipper::isInside(triangle[0].get().vertex, triangle[1].get().vertex, triangle[2].get().vertex))
        {
            return drawUnclippedTriangle(triangle);
        }

        if (Clipper::isOutside(triangle[0].get().vertex, triangle[1].get().vertex, triangle[2].get().vertex))
        {
            return true;
        }

        Clipper::ClipList list;
        Clipper::ClipList listBuffer;

        list[0] = triangle[0];
        list[1] = triangle[1];
        list[2] = triangle[2];

        tcb::span<VertexParameter> clippedVertexParameter = Clipper::clip(list, listBuffer);

        if (clippedVertexParameter.empty())
        {
            return true;
        }

        return drawClippedTriangleList(clippedVertexParameter);
    }

    const VertexTransformingData& m_data;
    const TDrawTriangleFunc m_drawTriangleFunc;
    const TUpdateStencilFunc m_updateStencilFunc;
    primitiveassembler::PrimitiveAssemblerCalc m_primitiveAssembler {
        m_data.viewPort,
        m_data.primitiveAssembler,
    };
};

} // namespace rr::vertextransforming
#endif // VERTEXTRANSFORMING_HPP
