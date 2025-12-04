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

#ifndef VERTEXARRAY_HPP_
#define VERTEXARRAY_HPP_

#include "Enums.hpp"
#include "RenderObj.hpp"
#include "math/Vec.hpp"
#include <cstdint>
#include <vector>

namespace rr
{
class VertexArray
{
public:
    void setActiveTexture(const uint8_t tmu) { m_tmu = tmu; }

    void setColor(const Vec4& color) { m_objPtr.setVertexColor(color); }
    void setNormal(const Vec3& normal) { m_objPtr.setNormal(normal); }
    void setTexCoord(const Vec4& texCoord) { m_objPtr.setTexCoord(m_tmu, texCoord); }
    void setMultiTexCoord(const uint8_t tmu, const Vec4& texCoord) { m_objPtr.setTexCoord(tmu, texCoord); }
    void setPointSize(float pointSize) { m_objPtr.setPointSize(pointSize); }
    float getPointSize() const { return m_objPtr.getPointSize(); }

    const RenderObj& renderObj() const { return m_objPtr; }

    void reset() { m_objPtr.reset(); }

    void enableVertexArray(bool enable) { m_objPtr.enableVertexArray(enable); }
    bool vertexArrayEnabled() const { return m_objPtr.vertexArrayEnabled(); }
    void setVertexSize(uint8_t size) { m_objPtr.setVertexSize(size); }
    void setVertexType(Type type) { m_objPtr.setVertexType(type); }
    void setVertexStride(uint32_t stride) { m_objPtr.setVertexStride(stride); }
    void setVertexPointer(const void* ptr, const std::size_t offset)
    {
        m_objPtr.setVertexPointer(reinterpret_cast<const uint8_t*>(ptr) + offset);
        m_vertexPointer = ptr;
        m_vertexPointerOffset = offset;
    }
    uint8_t getVertexSize() const { return m_objPtr.getVertexSize(); }
    Type getVertexType() const { return m_objPtr.getVertexType(); }
    uint32_t getVertexStride() const { return m_objPtr.getVertexStride(); }

    void enableTexCoordArray(bool enable) { m_objPtr.enableTexCoordArray(m_tmu, enable); }
    bool texCoordArrayEnabled() const { return m_objPtr.texCoordArrayEnabled()[m_tmu]; }
    void setTexCoordSize(uint8_t size) { m_objPtr.setTexCoordSize(m_tmu, size); }
    void setTexCoordType(Type type) { m_objPtr.setTexCoordType(m_tmu, type); }
    void setTexCoordStride(uint32_t stride) { m_objPtr.setTexCoordStride(m_tmu, stride); }
    void setTexCoordPointer(const void* ptr, const std::size_t offset)
    {
        m_objPtr.setTexCoordPointer(m_tmu, reinterpret_cast<const uint8_t*>(ptr) + offset);
        m_texCoordPointer = ptr;
        m_texCoordPointerOffset = offset;
    }
    uint8_t getTexCoordSize() const { return m_objPtr.getTexCoordSize(m_tmu); }
    Type getTexCoordType() const { return m_objPtr.getTexCoordType(m_tmu); }
    uint32_t getTexCoordStride() const { return m_objPtr.getTexCoordStride(m_tmu); }

    void enableNormalArray(bool enable) { m_objPtr.enableNormalArray(enable); }
    bool normalArrayEnabled() const { return m_objPtr.normalArrayEnabled(); }
    void setNormalType(Type type) { m_objPtr.setNormalType(type); }
    void setNormalStride(uint32_t stride) { m_objPtr.setNormalStride(stride); }
    void setNormalPointer(const void* ptr, const std::size_t offset)
    {
        m_objPtr.setNormalPointer(reinterpret_cast<const uint8_t*>(ptr) + offset);
        m_normalPointer = ptr;
        m_normalPointerOffset = offset;
    }
    Type getNormalType() const { return m_objPtr.getNormalType(); }
    uint32_t getNormalStride() const { return m_objPtr.getNormalStride(); }

    void enableColorArray(bool enable) { m_objPtr.enableColorArray(enable); }
    bool colorArrayEnabled() const { return m_objPtr.colorArrayEnabled(); }
    void setColorSize(uint8_t size) { m_objPtr.setColorSize(size); }
    void setColorType(Type type) { m_objPtr.setColorType(type); }
    void setColorStride(uint32_t stride) { m_objPtr.setColorStride(stride); }
    void setColorPointer(const void* ptr, const std::size_t offset)
    {
        m_objPtr.setColorPointer(reinterpret_cast<const uint8_t*>(ptr) + offset);
        m_colorPointer = ptr;
        m_colorPointerOffset = offset;
    }
    uint8_t getColorSize() const { return m_objPtr.getColorSize(); }
    Type getColorType() const { return m_objPtr.getColorType(); }
    uint32_t getColorStride() const { return m_objPtr.getColorStride(); }

    void enablePointSizeArray(bool enable) { m_objPtr.enablePointSizeArray(enable); }
    bool pointSizeArrayEnabled() const { return m_objPtr.pointSizeArrayEnabled(); }
    void setPointSizeType(Type type) { m_objPtr.setPointSizeType(type); }
    void setPointSizeStride(uint32_t stride) { m_objPtr.setPointSizeStride(stride); }
    void setPointSizePointer(const void* ptr, const std::size_t offset)
    {
        m_objPtr.setPointSizePointer(reinterpret_cast<const uint8_t*>(ptr) + offset);
        m_pointSizePointer = ptr;
        m_pointSizePointerOffset = offset;
    }
    Type getPointSizeType() const { return m_objPtr.getPointSizeType(); }
    uint32_t getPointSizeStride() const { return m_objPtr.getPointSizeStride(); }

    void setDrawMode(DrawMode mode) { m_objPtr.setDrawMode(mode); }

    void enableIndices(bool enable) { m_objPtr.enableIndices(enable); }
    bool indicesEnabled() const { return m_objPtr.indicesEnabled(); }
    void setCount(std::size_t count) { m_objPtr.setCount(count); }
    void setIndicesType(Type type) { m_objPtr.setIndicesType(type); }
    void setIndicesPointer(const void* ptr) { m_objPtr.setIndicesPointer(ptr); }
    void setArrayOffset(uint32_t offset) { m_objPtr.setArrayOffset(offset); }

    // This methods are only used for the glGetPointerv function
    const void* getVertexPointer() const { return m_vertexPointer; }
    const void* getTexCoordPointer() const { return m_texCoordPointer; }
    const void* getNormalPointer() const { return m_normalPointer; }
    const void* getColorPointer() const { return m_colorPointer; }
    const void* getPointSizePointer() const { return m_pointSizePointer; }
    std::size_t getVertexPointerOffset() const { return m_vertexPointerOffset; }
    std::size_t getTexCoordPointerOffset() const { return m_texCoordPointerOffset; }
    std::size_t getNormalPointerOffset() const { return m_normalPointerOffset; }
    std::size_t getColorPointerOffset() const { return m_colorPointerOffset; }
    std::size_t getPointSizePointerOffset() const { return m_pointSizePointerOffset; }

    // This methods are used for glGet* and bindings
    // TODO: Rename bindings instead of getBound* to get*BindingID
    std::size_t getBoundVertexBuffer() const { return m_boundVertexBuffer; }
    void setBoundVertexBuffer(const std::size_t buffer) { m_boundVertexBuffer = buffer; }
    std::size_t getBoundTexCoordBuffer() const { return m_boundTexCoordBuffer; }
    void setBoundTexCoordBuffer(const std::size_t buffer) { m_boundTexCoordBuffer = buffer; }
    std::size_t getBoundNormalBuffer() const { return m_boundNormalBuffer; }
    void setBoundNormalBuffer(const std::size_t buffer) { m_boundNormalBuffer = buffer; }
    std::size_t getBoundColorBuffer() const { return m_boundColorBuffer; }
    void setBoundColorBuffer(const std::size_t buffer) { m_boundColorBuffer = buffer; }
    std::size_t getBoundElementBuffer() const { return m_boundElementBuffer; }
    void setBoundElementBuffer(const std::size_t buffer) { m_boundElementBuffer = buffer; }
    std::size_t getBoundPointSizeArrayBuffer() const { return m_boundPointSizeArrayBuffer; }
    void setBoundPointSizeArrayBuffer(const std::size_t buffer) { m_boundPointSizeArrayBuffer = buffer; }

private:
    // Render Object
    RenderObj m_objPtr {};
    std::size_t m_tmu { 0 };

    // This variables are only used for the glGetPointerv function
    const void* m_vertexPointer { nullptr };
    const void* m_texCoordPointer { nullptr };
    const void* m_normalPointer { nullptr };
    const void* m_colorPointer { nullptr };
    const void* m_pointSizePointer { nullptr };
    std::size_t m_vertexPointerOffset { 0 };
    std::size_t m_texCoordPointerOffset { 0 };
    std::size_t m_normalPointerOffset { 0 };
    std::size_t m_colorPointerOffset { 0 };
    std::size_t m_pointSizePointerOffset { 0 };

    // This variables are used for glGet* and bindings
    std::size_t m_boundVertexBuffer { 0 };
    std::size_t m_boundTexCoordBuffer { 0 };
    std::size_t m_boundNormalBuffer { 0 };
    std::size_t m_boundColorBuffer { 0 };
    std::size_t m_boundElementBuffer { 0 };
    std::size_t m_boundPointSizeArrayBuffer { 0 };
};

} // namespace rr
#endif // VERTEXARRAY_HPP_
