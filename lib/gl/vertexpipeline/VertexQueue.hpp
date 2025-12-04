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

#ifndef VERTEXQUEUE_HPP_
#define VERTEXQUEUE_HPP_

#include "Enums.hpp"
#include "RenderObj.hpp"
#include "math/Vec.hpp"
#include <vector>

namespace rr
{
class VertexQueue
{
public:
    void setActiveTexture(const std::size_t tmu) { m_tmu = tmu; }
    std::size_t getActiveTexture() const { return m_tmu; }

    void begin(const DrawMode drawMode)
    {
        m_beginMode = drawMode;
        m_textureVertexBuffer.clear();
        m_vertexBuffer.clear();
        m_normalVertexBuffer.clear();
        m_colorVertexBuffer.clear();
    }
    void addVertex(const Vec4& vertex)
    {
        m_vertexBuffer.push_back(vertex);
        m_normalVertexBuffer.push_back(m_normal);
        m_textureVertexBuffer.push_back(m_textureCoord);
        m_colorVertexBuffer.push_back(m_vertexColor);
    }
    void setColor(const Vec4& color) { m_vertexColor = color; }
    void setNormal(const Vec3& normal) { m_normal = normal; }
    void setTexCoord(const Vec4& texCoord) { m_textureCoord[0] = texCoord; }
    void setMultiTexCoord(const std::size_t tmu, const Vec4& texCoord) { m_textureCoord[tmu] = texCoord; }
    Vec4 getColor() const { return m_vertexColor; }
    Vec3 getNormal() const { return m_normal; }
    Vec4 getTexCoord() const { return m_textureCoord[0]; }
    Vec4 getMultiTexCoord(const std::size_t tmu) const { return m_textureCoord[tmu]; }

    const RenderObj& end()
    {
        m_objBeginEnd.reset();

        m_objBeginEnd.enableVertexArray(!m_vertexBuffer.empty());
        m_objBeginEnd.setVertexSize(4);
        m_objBeginEnd.setVertexType(Type::FLOAT);
        m_objBeginEnd.setVertexStride(0);
        m_objBeginEnd.setVertexPointer(m_vertexBuffer.data());

        for (std::size_t i = 0; i < RenderObj::MAX_TMU_COUNT; i++)
        {
            m_objBeginEnd.enableTexCoordArray(i, !m_textureVertexBuffer.empty());
            m_objBeginEnd.setTexCoordSize(i, 4);
            m_objBeginEnd.setTexCoordType(i, Type::FLOAT);
            m_objBeginEnd.setTexCoordStride(i, RenderObj::MAX_TMU_COUNT * sizeof(Vec4));
            m_objBeginEnd.setTexCoordPointer(i, m_textureVertexBuffer.data() + i);
        }

        m_objBeginEnd.enableNormalArray(!m_normalVertexBuffer.empty());
        m_objBeginEnd.setNormalType(Type::FLOAT);
        m_objBeginEnd.setNormalStride(0);
        m_objBeginEnd.setNormalPointer(m_normalVertexBuffer.data());

        m_objBeginEnd.enableColorArray(!m_colorVertexBuffer.empty());
        m_objBeginEnd.setColorSize(4);
        m_objBeginEnd.setColorType(Type::FLOAT);
        m_objBeginEnd.setColorStride(0);
        m_objBeginEnd.setColorPointer(m_colorVertexBuffer.data());

        m_objBeginEnd.enableIndices(false);
        m_objBeginEnd.setDrawMode(m_beginMode);
        m_objBeginEnd.setCount(m_vertexBuffer.size());

        return m_objBeginEnd;
    }

    const Vec4 color() const { return m_vertexColor; }

private:
    // Buffer
    std::vector<Vec4> m_vertexBuffer;
    std::vector<std::array<Vec4, RenderObj::MAX_TMU_COUNT>> m_textureVertexBuffer;
    std::vector<Vec3> m_normalVertexBuffer;
    std::vector<Vec4> m_colorVertexBuffer;

    // State values
    Vec4 m_vertexColor {};
    std::array<Vec4, RenderObj::MAX_TMU_COUNT> m_textureCoord {};
    Vec3 m_normal {};
    DrawMode m_beginMode { DrawMode::TRIANGLES };

    // Render Object
    RenderObj m_objBeginEnd {};

    std::size_t m_tmu { 0 };
};

} // namespace rr
#endif // VERTEXQUEUE_HPP_
