// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2026 ToNi3141

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

#ifndef FLOATV3T2C3RENDERER_HPP
#define FLOATV3T2C3RENDERER_HPP

#include "RenderObj.hpp"
#include "pixelpipeline/PixelPipeline.hpp"
#include <cstddef>
#include <cstdint>

namespace rr
{
// Fast path for objects with float vertex (3), float tex coord (2) and float color (3) arrays,
// no normals and no point sizes, drawn without indices or with 16 bit indices.
class FloatV3T2C3Renderer
{
public:
    FloatV3T2C3Renderer(const RenderObj& obj, PixelPipeline& renderer)
        : m_obj { obj }
        , m_renderer { renderer }
        , m_vertices { static_cast<const uint8_t*>(obj.getVertexPointer()) }
        , m_texCoords { static_cast<const uint8_t*>(obj.getTexCoordPointer(0)) }
        , m_colors { static_cast<const uint8_t*>(obj.getColorPointer()) }
        , m_vertexStride { obj.getVertexStride() == 0 ? 3 * sizeof(float) : obj.getVertexStride() }
        , m_texCoordStride { obj.getTexCoordStride(0) == 0 ? 2 * sizeof(float) : obj.getTexCoordStride(0) }
        , m_colorStride { obj.getColorStride() == 0 ? 3 * sizeof(float) : obj.getColorStride() }
    {
    }

    // Returns true if the object can be drawn with this renderer. Otherwise the generic path has to be used.
    static bool isApplicable(const RenderObj& obj)
    {
        bool applicable = obj.getVertexType() == Type::FLOAT && obj.getVertexSize() == 3
            && obj.texCoordArrayEnabled()[0] && obj.getTexCoordType(0) == Type::FLOAT && obj.getTexCoordSize(0) == 2
            && obj.colorArrayEnabled() && obj.getColorType() == Type::FLOAT && obj.getColorSize() == 3
            && !obj.normalArrayEnabled() && !obj.pointSizeArrayEnabled();
        for (std::size_t tu = 1; tu < RenderConfig::TMU_COUNT; tu++)
        {
            applicable = applicable && !obj.texCoordArrayEnabled()[tu];
        }
        return applicable
            && (!obj.indicesEnabled() || obj.getIndicesType() == Type::UNSIGNED_SHORT || obj.getIndicesType() == Type::SHORT);
    }

    inline void render()
    {
        if (m_obj.indicesEnabled())
        {
            draw<UInt16IndexReader>();
        }
        else
        {
            draw<SequentialIndexReader>();
        }
    }

private:
    struct SequentialIndexReader
    {
        std::size_t arrayOffset;

        explicit SequentialIndexReader(const RenderObj& obj)
            : arrayOffset { obj.getArrayOffset() }
        {
        }

        std::size_t get(const std::size_t index) const { return index + arrayOffset; }
    };

    struct UInt16IndexReader
    {
        const uint16_t* indices;

        explicit UInt16IndexReader(const RenderObj& obj)
            : indices { static_cast<const uint16_t*>(obj.getIndicesPointer()) }
        {
        }

        std::size_t get(const std::size_t index) const { return indices[index]; }
    };

    static inline const float* floatAt(const uint8_t* data, const std::size_t stride, const std::size_t index)
    {
        return reinterpret_cast<const float*>(data + index * stride);
    }

    template <typename IndexReader>
    inline void draw()
    {
        const IndexReader indexReader { m_obj };
        const uint8_t* const vertices = m_vertices;
        const uint8_t* const texCoords = m_texCoords;
        const uint8_t* const colors = m_colors;
        const std::size_t vertexStride = m_vertexStride;
        const std::size_t texCoordStride = m_texCoordStride;
        const std::size_t colorStride = m_colorStride;
        const std::size_t count = m_obj.getCount();

        const auto fetchVertex = [&](VertexParameter& parameter, const std::size_t i)
        {
            const std::size_t pos = indexReader.get(i);
            parameter.sourceIndex = pos;
            parameter.vertex.fromArray(floatAt(vertices, vertexStride, pos), 3);
            parameter.vertex[3] = 1.0f;
            parameter.color.fromArray(floatAt(colors, colorStride, pos), 3);
            parameter.color[3] = 1.0f;
            parameter.tex[0].fromArray(floatAt(texCoords, texCoordStride, pos), 2);
            parameter.tex[0][2] = 0.0f;
            parameter.tex[0][3] = 1.0f;
            for (std::size_t tu = 1; tu < RenderConfig::TMU_COUNT; tu++)
            {
                parameter.tex[tu] = m_obj.getTexCoordDefault(tu);
            }
        };

        if (m_obj.getDrawMode() == DrawMode::TRIANGLES)
        {
            // Incomplete trailing triangles are ignored, like in OpenGL
            const std::size_t vertexCount = count - (count % 3);
            for (std::size_t i = 0; i < vertexCount; i += 3)
            {
                VertexParameter p0;
                VertexParameter p1;
                VertexParameter p2;
                fetchVertex(p0, i);
                fetchVertex(p1, i + 1);
                fetchVertex(p2, i + 2);
                m_renderer.pushTriangle(p0, p1, p2);
            }
            return;
        }

        for (std::size_t i = 0; i < count; i++)
        {
            VertexParameter parameter;
            fetchVertex(parameter, i);
            m_renderer.pushVertex(parameter);
        }
    }

    const RenderObj& m_obj;
    PixelPipeline& m_renderer;
    const uint8_t* m_vertices;
    const uint8_t* m_texCoords;
    const uint8_t* m_colors;
    std::size_t m_vertexStride;
    std::size_t m_texCoordStride;
    std::size_t m_colorStride;
};

} // namespace rr
#endif // FLOATV3T2C3RENDERER_HPP
