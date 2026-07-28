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

#ifndef VERTEXTRANSFORMER_HPP
#define VERTEXTRANSFORMER_HPP

#include "Cache.hpp"
#include "Clipper.hpp"
#include "Culling.hpp"
#include "ElementGlobalData.hpp"
#include "ElementLocalData.hpp"
#include "Lighting.hpp"
#include "LineAssembly.hpp"
#include "MatrixStore.hpp"
#include "PlaneClipper.hpp"
#include "PointAssembly.hpp"
#include "PolygonOffset.hpp"
#include "PrimitiveAssembler.hpp"
#include "RenderConfigs.hpp"
#include "ShadeModel.hpp"
#include "Stencil.hpp"
#include "TexGen.hpp"
#include "ViewPort.hpp"
#include "math/Vec.hpp"
#include "transform/TransformingVertexParameter.hpp"
#include <bitset>
#include <tcb/span.hpp>

namespace rr::vertextransformer
{

struct VertexTransformerData
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
        lineAssembly = data.lineAssembly;
        pointAssembly = data.pointAssembly;
        polygonOffset = data.polygonOffset;
        shadeModel = data.shadeModel;
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
    pointassembly::PointAssemblyData pointAssembly {};
    polygonoffset::PolygonOffsetData polygonOffset {};
    shademodel::ShadeModelData shadeModel {};
};

template <typename TDrawTriangleFunc, typename TUpdateStencilFunc>
class VertexTransformerCalc
{
    static constexpr std::size_t VERTEX_CACHE_SIZE = 16;

public:
    VertexTransformerCalc(
        const VertexTransformerData& data,
        const TDrawTriangleFunc& drawTriangleFunc,
        const TUpdateStencilFunc& updateStencilFunc)
        : m_data { data }
        , m_drawTriangleFunc { drawTriangleFunc }
        , m_updateStencilFunc { updateStencilFunc }
    {
    }

    bool pushVertex(const VertexParameter& param)
    {
        TransformingVertexParameter transformingVertexParameter;
        if (!m_vertexCache.get(param.sourceIndex, transformingVertexParameter))
        {
            transformingVertexParameter = transform(param);
            m_vertexCache.put(param.sourceIndex, transformingVertexParameter);
        }
        m_primitiveAssembler.pushParameter(transformingVertexParameter);

        const primitiveassembler::PrimitiveAssemblerCalc::Primitive primitive = m_primitiveAssembler.createPrimitive();
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
        m_vertexCache.reset();
        updateNormalMatrix();
        checkForRequiredTextureTransformations();
    }

private:
    void updateNormalMatrix()
    {
        bool calculateNormalMatrix = false;
        for (std::size_t tu = 0; tu < RenderConfig::TMU_COUNT; tu++)
        {
            if (m_data.tmuEnabled[tu])
            {
                calculateNormalMatrix = calculateNormalMatrix || texgen::TexGenCalc { m_data.texGen[tu] }.isEnabled();
            }
        }
        calculateNormalMatrix = calculateNormalMatrix || m_data.lighting.lightingEnabled;
        if (calculateNormalMatrix)
        {
            m_normalMatrix = createNormalMatrix();
            m_lighting.init(m_normalMatrix);
        }
    }

    void checkForRequiredTextureTransformations()
    {
        for (std::size_t tu = 0; tu < RenderConfig::TMU_COUNT; tu++)
        {
            if (m_data.tmuEnabled[tu])
            {
                m_textureTransformationRequired[tu] = !m_data.transformMatrices.texture[tu].isIdentity();
            }
        }
    }

    void clipPlaneProjectiveTransformation(tcb::span<TransformingVertexParameter> list)
    {
        for (std::size_t i = 0; i < list.size(); i++)
        {
            list[i].vertex = m_data.transformMatrices.projection.transform(list[i].vertex);
        }
    }

    bool clipAtPlaneAndDrawTriangle(const primitiveassembler::PrimitiveAssemblerCalc::Primitive& primitive)
    {
        planeclipper::PlaneClipperCalc::ClipList list;
        planeclipper::PlaneClipperCalc::ClipList listBuffer;

        list[0] = primitive[0];
        list[1] = primitive[1];
        list[2] = primitive[2];

        tcb::span<TransformingVertexParameter> clippedVertexParameter = m_planeClipper.clipTriangle(list, listBuffer);

        if (clippedVertexParameter.empty())
        {
            return true;
        }

        // Transform the clipped vertexes to projection space
        clipPlaneProjectiveTransformation({ &clippedVertexParameter[0], clippedVertexParameter.size() });

        bool ret = true;
        for (std::size_t i = 3; i <= clippedVertexParameter.size(); i++)
        {
            std::array<TransformingVertexParameter, 3> triangle {
                clippedVertexParameter[0],
                clippedVertexParameter[i - 2],
                clippedVertexParameter[i - 1]
            };
            tcb::span<TransformingVertexParameter> triangleSpan { triangle.data(), 3 };
            ret = ret && drawTriangle(triangleSpan);
        }
        return ret;
    }

    bool clipAtPlaneAndDrawLine(const primitiveassembler::PrimitiveAssemblerCalc::Primitive& primitive)
    {
        planeclipper::PlaneClipperCalc::ClipList list;
        planeclipper::PlaneClipperCalc::ClipList listBuffer;

        list[0] = primitive[0];
        list[1] = primitive[1];

        tcb::span<TransformingVertexParameter> clippedVertexParameter = m_planeClipper.clipLine(list, listBuffer);

        // Transform the clipped vertexes to projection space
        clipPlaneProjectiveTransformation(clippedVertexParameter);

        if (clippedVertexParameter.empty())
        {
            return true;
        }

        return drawLine({ clippedVertexParameter.data(), 2 });
    }

    bool clipAtPlaneAndDrawPoint(const primitiveassembler::PrimitiveAssemblerCalc::Primitive& primitive)
    {
        planeclipper::PlaneClipperCalc::ClipList list;
        planeclipper::PlaneClipperCalc::ClipList listBuffer;

        list[0] = primitive[0];

        tcb::span<TransformingVertexParameter> clippedVertexParameter = m_planeClipper.clipPoint(list, listBuffer);

        // Transform the clipped vertexes to projection space
        clipPlaneProjectiveTransformation(clippedVertexParameter);

        if (clippedVertexParameter.empty())
        {
            return true;
        }

        return drawPoint({ clippedVertexParameter.data(), 1 });
    }

    TransformingVertexParameter transform(const VertexParameter& parameter)
    {
        TransformingVertexParameter outParam;
        for (std::size_t tu = 0; tu < RenderConfig::TMU_COUNT; tu++)
        {
            if (m_data.tmuEnabled[tu])
            {
                outParam.tex[tu] = parameter.tex[tu];
                if (const texgen::TexGenCalc  texGen { m_data.texGen[tu] }; texGen.isEnabled())
                {
                    texGen.calculateTexGenCoords(
                        outParam.tex[tu],
                        m_data.transformMatrices.modelView,
                        m_normalMatrix,
                        parameter.vertex,
                        parameter.normal);
                }
                if (m_textureTransformationRequired[tu])
                {
                    outParam.tex[tu] = m_data.transformMatrices.texture[tu].transform(outParam.tex[tu]);
                }
            }
        }

        outParam.vertex = m_data.transformMatrices.modelView.transform(parameter.vertex);

        // TODO: Check if this required? The standard requires but is it really used?
        // m_c[j].transform(color, color); // Calculate this in one batch to improve performance

        if (m_data.lighting.lightingEnabled)
        {
            const Vec3 normal = m_normalMatrix.transform(parameter.normal);
            m_lighting.calculateLights(
                outParam.colorFront,
                parameter.color,
                outParam.vertex,
                normal);
            if (m_data.lighting.enableTwoSideModel)
            {
                m_lighting.calculateLights(
                    outParam.colorBack,
                    parameter.color,
                    outParam.vertex,
                    normal * -1.0f);
            }
        }
        else
        {
            outParam.colorFront = parameter.color;
        }

        outParam.pointSize = parameter.pointSize;

        // Optimization: Clipping works in model space. If the clipping is disabled, we can directly transform to projection space.
        if (!m_planeClipper.enabled())
        {
            outParam.vertex = m_data.transformMatrices.projection.transform(outParam.vertex);
        }

        return outParam;
    }

    bool drawClippedTriangleList(const tcb::span<TransformingVertexParameter>& list)
    {
        const std::size_t clippedVertexListSize = list.size();
        for (std::size_t i = 0; i < clippedVertexListSize; i++)
        {
            list[i].vertex.perspectiveDivide();
            viewport::ViewPortCalc { m_data.viewPort }.transform(list[i].vertex);
        }

        // Check only one triangle in the clipped list. The triangles are sub divided, but not rotated. So if one triangle is
        // facing backwards, then all in the clipping list will do this and vice versa.
        const Vec4& v0 = list[0].vertex;
        const Vec4& v1 = list[1].vertex;
        const Vec4& v2 = list[2].vertex;
        if (culling::CullingCalc { m_data.culling }.cull(v0, v1, v2))
        {
            return true;
        }

        if (m_data.lighting.lightingEnabled && m_data.lighting.enableTwoSideModel)
        {
            if (!culling::CullingCalc { m_data.culling }.isFrontFace(v0, v1, v2))
            {
                for (std::size_t i = 0; i < clippedVertexListSize; i++)
                {
                    list[i].colorFront = list[i].colorBack;
                }
            }
        }

        if (m_data.stencil.enableTwoSideStencil)
        {
            const StencilReg reg = stencil::StencilCalc { m_data.stencil }.updateStencilFace(v0, v1, v2);
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
                list[0].colorFront,
                list[i - 2].colorFront,
                list[i - 1].colorFront,
            });
            if (!success)
            {
                return false;
            }
        }
        return true;
    }

    bool drawUnclippedTriangle(const primitiveassembler::PrimitiveAssemblerCalc::Primitive& primitive)
    {
        // Optimized version of the drawTriangle when a triangle is not needed to be clipped.

        // Perspective division
        Vec4& v0 = primitive[0].vertex;
        Vec4& v1 = primitive[1].vertex;
        Vec4& v2 = primitive[2].vertex;

        v0.perspectiveDivide();
        v1.perspectiveDivide();
        v2.perspectiveDivide();

        // Viewport transformation of the vertex
        viewport::ViewPortCalc { m_data.viewPort }.transform(v0);
        viewport::ViewPortCalc { m_data.viewPort }.transform(v1);
        viewport::ViewPortCalc { m_data.viewPort }.transform(v2);

        if (culling::CullingCalc { m_data.culling }.cull(v0, v1, v2))
        {
            return true;
        }

        Vec4& c0 = primitive[0].colorFront;
        Vec4& c1 = primitive[1].colorFront;
        Vec4& c2 = primitive[2].colorFront;
        if (m_data.lighting.lightingEnabled && m_data.lighting.enableTwoSideModel)
        {
            if (!culling::CullingCalc { m_data.culling }.isFrontFace(v0, v1, v2))
            {
                c0 = primitive[0].colorBack;
                c1 = primitive[1].colorBack;
                c2 = primitive[2].colorBack;
            }
        }

        if (m_data.stencil.enableTwoSideStencil)
        {
            const StencilReg reg = stencil::StencilCalc { m_data.stencil }.updateStencilFace(v0, v1, v2);
            if (!m_updateStencilFunc(reg))
            {
                return false;
            }
        }

        if (m_data.polygonOffset.enable)
        {
            polygonoffset::PolygonOffsetCalc { m_data.polygonOffset }.addOffset(v0, v1, v2);
        }

        return m_drawTriangleFunc({
            v0,
            v1,
            v2,
            primitive[0].tex,
            primitive[1].tex,
            primitive[2].tex,
            c0,
            c1,
            c2,
        });
    }

    bool drawClippedTriangle(const primitiveassembler::PrimitiveAssemblerCalc::Primitive& primitive)
    {
        Clipper::ClipList list;
        Clipper::ClipList listBuffer;

        list[0] = primitive[0];
        list[1] = primitive[1];
        list[2] = primitive[2];

        tcb::span<TransformingVertexParameter> clippedVertexParameter = Clipper::clip(list, listBuffer);

        if (clippedVertexParameter.empty())
        {
            return true;
        }

        return drawClippedTriangleList(clippedVertexParameter);
    }

    bool drawPreClippedTriangle(const primitiveassembler::PrimitiveAssemblerCalc::Primitive& primitive)
    {
        switch (Clipper::isIntersecting(primitive[0].vertex, primitive[1].vertex, primitive[2].vertex))
        {
        case Clipper::Position::Inside:
            return drawUnclippedTriangle(primitive);
        case Clipper::Position::Outside:
            return true;
        case Clipper::Position::Intersecting:
            return drawClippedTriangle(primitive);
        }
        return false;
    }

    bool drawTriangle(const primitiveassembler::PrimitiveAssemblerCalc::Primitive& primitive)
    {
        TransformingVertexParameter& p0 = primitive[0];
        TransformingVertexParameter& p1 = primitive[1];
        TransformingVertexParameter& p2 = primitive[2];

        shademodel::ShadeModelCalc { m_data.shadeModel }.updateShadeModelTriangle(p0, p1, p2);

        return drawPreClippedTriangle(primitive);
    }

    bool drawLine(const primitiveassembler::PrimitiveAssemblerCalc::Primitive& primitive)
    {
        TransformingVertexParameter& p0 = primitive[0];
        TransformingVertexParameter& p1 = primitive[1];

        shademodel::ShadeModelCalc { m_data.shadeModel }.updateShadeModelLine(p0, p1);

        lineassembly::LineAssemblyCalc::Triangles triangles = m_lineAssembly.createLine(p0, p1);

        tcb::span<TransformingVertexParameter> triangleSpan { triangles.data(), triangles.size() };
        drawPreClippedTriangle(triangleSpan.subspan(0, 3));
        // Assume when the first one fails, the second one will also fail.
        return drawPreClippedTriangle(triangleSpan.subspan(3, 3));
    }

    bool drawPoint(const primitiveassembler::PrimitiveAssemblerCalc::Primitive& primitive)
    {
        TransformingVertexParameter& p0 = primitive[0];

        pointassembly::PointAssemblyCalc::Triangles triangles = m_pointAssembly.createPoint(p0);

        tcb::span<TransformingVertexParameter> triangleSpan { triangles.data(), triangles.size() };
        drawPreClippedTriangle(triangleSpan.subspan(0, 3));
        // Assume when the first one fails, the second one will also fail.
        return drawPreClippedTriangle(triangleSpan.subspan(3, 3));
    }

    bool drawPrimitive(const primitiveassembler::PrimitiveAssemblerCalc::Primitive& primitive)
    {
        switch (primitive.size())
        {
        case 3:

            if (m_planeClipper.enabled())
            {
                return clipAtPlaneAndDrawTriangle(primitive);
            }
            else
            {
                return drawTriangle(primitive);
            }

        case 2:
            if (m_planeClipper.enabled())
            {
                return clipAtPlaneAndDrawLine(primitive);
            }
            else
            {
                return drawLine(primitive);
            }

        case 1:
            if (m_planeClipper.enabled())
            {
                return clipAtPlaneAndDrawPoint(primitive);
            }
            else
            {
                return drawPoint(primitive);
            }
        default:
            return false;
        }

        return true;
    }

    Mat44 createNormalMatrix() const
    {
        Mat44 normalMat = m_data.transformMatrices.modelView;
        normalMat.invert();
        normalMat.transpose();
        return normalMat;
    }

    Mat44 m_normalMatrix {};

    const VertexTransformerData& m_data;
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
    pointassembly::PointAssemblyCalc m_pointAssembly {
        m_data.pointAssembly,
        m_data.viewPort.viewportWidth,
        m_data.viewPort.viewportHeight
    };
    lighting::LightingCalc m_lighting { m_data.lighting };
    cache::Cache<TransformingVertexParameter, VERTEX_CACHE_SIZE> m_vertexCache {};

    std::bitset<RenderConfig::TMU_COUNT> m_textureTransformationRequired {};
};

} // namespace rr::vertextransformer
#endif // VERTEXTRANSFORMER_HPP
