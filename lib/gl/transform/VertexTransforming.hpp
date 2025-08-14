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
#include "LineAssembly.hpp"
#include "MatrixStore.hpp"
#include "PlaneClipper.hpp"
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
        planeClipper = data.planeClipper;
        lineAssembly = data.lineAssemblyData;
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
    planeclipper::PlaneClipperData planeClipper {};
    lineassembly::LineAssemblyData lineAssembly {};
    bool normalizeLightNormal {};
};

template <typename TDrawTriangleFunc, typename TUpdateStencilFunc>
class VertexTransformingCalc
{
    using Triangle = std::array<VertexParameter, 3>;

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

        const primitiveassembler::PrimitiveAssemblerCalc::Primitive primitive = m_primitiveAssembler.getPrimitive();
        if (primitive.empty())
        {
            return true;
        }

        if (!drawPrimitive(primitive))
        {
            return false;
        }
        m_primitiveAssembler.removePrimitive();

        return true;
    }

    void init()
    {
        m_primitiveAssembler.init();
        if (m_data.lighting.lightingEnabled)
        {
            m_normalMatrix = createNormalMatrix();
        }
    }

private:
    bool clipAtPlaneAndDrawTriangle(const primitiveassembler::PrimitiveAssemblerCalc::Primitive& primitive)
    {
        planeclipper::PlaneClipperCalc::ClipList list;
        planeclipper::PlaneClipperCalc::ClipList listBuffer;

        list[0] = primitive[0];
        list[1] = primitive[1];
        list[2] = primitive[2];

        tcb::span<VertexParameter> clippedVertexParameter = m_planeClipper.clipTriangle(list, listBuffer);

        if (clippedVertexParameter.empty())
        {
            return true;
        }

        bool ret = true;
        for (std::size_t i = 3; i <= clippedVertexParameter.size(); i++)
        {
            const Triangle triangle { clippedVertexParameter[0], clippedVertexParameter[i - 2], clippedVertexParameter[i - 1] };
            ret = ret && drawTriangle(triangle);
        }
        return ret;
    }

    bool clipAtPlaneAndDrawLine(const primitiveassembler::PrimitiveAssemblerCalc::Primitive& primitive)
    {
        planeclipper::PlaneClipperCalc::ClipList list;
        planeclipper::PlaneClipperCalc::ClipList listBuffer;

        list[0] = primitive[0];
        list[1] = primitive[1];

        tcb::span<VertexParameter> clippedVertexParameter = m_planeClipper.clipLine(list, listBuffer);

        if (clippedVertexParameter.empty())
        {
            return true;
        }

        return drawLine({ clippedVertexParameter.data(), 2 });
    }

    bool clipAtPlaneAndDrawPoint(const primitiveassembler::PrimitiveAssemblerCalc::Primitive& primitive)
    {
        // TODO Implement clip and draw point
        return true;
    }

    void transform(VertexParameter& parameter)
    {
        for (std::size_t tu = 0; tu < RenderConfig::TMU_COUNT; tu++)
        {
            if (m_data.tmuEnabled[tu])
            {
                texgen::TexGenCalc { m_data.texGen[tu] }.calculateTexGenCoords(
                    parameter.tex[tu],
                    m_data.transformMatrices.modelView,
                    m_normalMatrix,
                    parameter.vertex,
                    parameter.normal);
                parameter.tex[tu] = m_data.transformMatrices.texture[tu].transform(parameter.tex[tu]);
            }
        }

        // TODO: Check if this required? The standard requires but is it really used?
        // m_c[j].transform(color, color); // Calculate this in one batch to improve performance
        if (m_data.lighting.lightingEnabled)
        {
            Vec3 normal = m_normalMatrix.transform(parameter.normal);

            if (m_data.normalizeLightNormal)
            {
                normal.normalize();
            }
            const Vec4 vl = m_data.transformMatrices.modelView.transform(parameter.vertex);
            const Vec4 c = parameter.color;
            lighting::LightingCalc { m_data.lighting }.calculateLights(parameter.color, c, vl, normal);
        }
        parameter.vertex = m_data.transformMatrices.modelView.transform(parameter.vertex);
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

    bool drawUnclippedTriangle(const Triangle& triangle)
    {
        // Optimized version of the drawTriangle when a triangle is not needed to be clipped.

        Vec4 v0 = triangle[0].vertex;
        Vec4 v1 = triangle[1].vertex;
        Vec4 v2 = triangle[2].vertex;

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
            triangle[0].tex,
            triangle[1].tex,
            triangle[2].tex,
            triangle[0].color,
            triangle[1].color,
            triangle[2].color,
        });
    }

    bool drawClippedTriangle(const Triangle& triangle)
    {
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

    bool drawPreClippedTriangle(const Triangle& triangle)
    {
        if (Clipper::isInside(triangle[0].vertex, triangle[1].vertex, triangle[2].vertex))
        {
            return drawUnclippedTriangle(triangle);
        }

        if (Clipper::isOutside(triangle[0].vertex, triangle[1].vertex, triangle[2].vertex))
        {
            return true;
        }
        return drawClippedTriangle(triangle);
    }

    bool drawTriangle(const primitiveassembler::PrimitiveAssemblerCalc::Primitive& primitive)
    {
        Triangle projectedTriangle { primitive[0], primitive[1], primitive[2] };
        projectedTriangle[0].vertex = m_data.transformMatrices.projection.transform(projectedTriangle[0].vertex);
        projectedTriangle[1].vertex = m_data.transformMatrices.projection.transform(projectedTriangle[1].vertex);
        projectedTriangle[2].vertex = m_data.transformMatrices.projection.transform(projectedTriangle[2].vertex);

        return drawPreClippedTriangle(projectedTriangle);
    }

    bool drawLine(const primitiveassembler::PrimitiveAssemblerCalc::Primitive& primitive)
    {
        VertexParameter vp0 = primitive[0];
        VertexParameter vp1 = primitive[1];

        // Transform vertices to get the projected ones in NDC
        vp0.vertex = m_data.transformMatrices.projection.transform(vp0.vertex);
        vp1.vertex = m_data.transformMatrices.projection.transform(vp1.vertex);

        lineassembly::LineAssemblyCalc::Triangles triangles = m_lineAssembly.createLine(vp0, vp1);

        drawPreClippedTriangle({ triangles[0], triangles[1], triangles[2] });
        // Assume when the first one fails, the second one will also fail.
        return drawPreClippedTriangle({ triangles[3], triangles[4], triangles[5] });
    }

    bool drawPoint(const primitiveassembler::PrimitiveAssemblerCalc::Primitive&)
    {
        // TODO: draw a point
        return true;
    }

    bool drawPrimitive(const primitiveassembler::PrimitiveAssemblerCalc::Primitive& primitive)
    {
        // TODO: Check how to improve the transformation.
        // First problem: Before the clipping plane or everything else can be calculated, a
        // full triangle from the primitive assembler is required.
        // Second problem: Only the clipping plane requires vertices in eye coordinates after
        // primitive assembly. Without clipping plane or after the clipping plane, only
        // projected vertices are required. That means, without clipping plane, the projection
        // could be applied in the transform() method is a precalculated modelViewProjection
        // matrix. A modelViewProjection matrix in the transform() method can safe almost three
        // projection transformations.
        if (primitive.size() == 3)
        {
            if (m_planeClipper.enabled())
            {
                return clipAtPlaneAndDrawTriangle(primitive);
            }
            else
            {
                return drawTriangle(primitive);
            }
        }
        if (primitive.size() == 2)
        {
            if (m_planeClipper.enabled())
            {
                return clipAtPlaneAndDrawLine(primitive);
            }
            else
            {
                return drawLine(primitive);
            }
        }
        if (primitive.size() == 1)
        {
            if (m_planeClipper.enabled())
            {
                return clipAtPlaneAndDrawPoint(primitive);
            }
            else
            {
                return drawPoint(primitive);
            };
        }
        return true;
    }

    Mat44 createModelProjectionMatrix() const
    {
        return m_data.transformMatrices.modelView * m_data.transformMatrices.projection;
    }

    Mat44 createNormalMatrix() const
    {
        Mat44 normalMat = m_data.transformMatrices.modelView;
        normalMat.invert();
        normalMat.transpose();
        return normalMat;
    }

    Mat44 m_normalMatrix {};

    const VertexTransformingData& m_data;
    const TDrawTriangleFunc m_drawTriangleFunc;
    const TUpdateStencilFunc m_updateStencilFunc;
    primitiveassembler::PrimitiveAssemblerCalc m_primitiveAssembler {
        m_data.viewPort,
        m_data.primitiveAssembler,
    };
    planeclipper::PlaneClipperCalc m_planeClipper { m_data.planeClipper };
    lineassembly::LineAssemblyCalc m_lineAssembly {
        m_data.lineAssembly,
        m_data.viewPort.viewportWidth,
        m_data.viewPort.viewportHeight
    };
};

} // namespace rr::vertextransforming
#endif // VERTEXTRANSFORMING_HPP
