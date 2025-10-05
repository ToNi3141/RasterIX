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

#ifndef RENDEROBJ_HPP
#define RENDEROBJ_HPP

#include "Enums.hpp"
#include "RenderConfigs.hpp"
#include "math/Vec.hpp"
#include <array>
#include <bitset>
#include <optional>

namespace rr
{
class RenderObj
{
public:
    static constexpr std::size_t MAX_TMU_COUNT { RenderConfig::TMU_COUNT };

    RenderObj();

    void logCurrentConfig() const;

    inline bool vertexArrayEnabled() const { return m_vertexArrayEnabled; }
    Vec4 getVertex(const std::size_t index) const { return getFromArray<Vec4>(m_vertexType, m_vertexPointer, m_vertexStride, m_vertexSize, index); }
    inline const std::bitset<MAX_TMU_COUNT>& texCoordArrayEnabled() const { return m_texCoordArrayEnabled; }
    Vec4 getTexCoord(const std::size_t tmu, const std::size_t index) const;
    inline bool colorArrayEnabled() const { return m_colorArrayEnabled; }
    Vec4 getColor(const std::size_t index) const;
    inline const Vec4& getVertexColor() const { return m_vertexColor; }
    inline bool normalArrayEnabled() const { return m_normalArrayEnabled; }
    inline bool indicesEnabled() const { return m_indicesEnabled; }
    Vec3 getNormal(const std::size_t index) const;
    bool isLine() const;
    float getPointSize(const std::size_t index) const;
    inline bool pointSizeArrayEnabled() const { return m_pointSizeArrayEnabled; }

    std::size_t getIndex(const std::size_t index) const;
    inline DrawMode getDrawMode() const { return m_drawMode; }
    inline std::size_t getCount() const { return m_count; }
    inline void reset() { m_fetchCount = 0; }
    inline std::size_t getFetchCount() const { return m_fetchCount; }
    inline void incFetchCount() const { m_fetchCount++; }

    void enableVertexArray(bool enable) { m_vertexArrayEnabled = enable; }
    void setVertexSize(std::size_t size) { m_vertexSize = size; }
    void setVertexType(Type type) { m_vertexType = type; }
    void setVertexStride(std::size_t stride) { m_vertexStride = stride; }
    void setVertexPointer(const void* ptr) { m_vertexPointer = ptr; }

    void enableTexCoordArray(const std::size_t tmu, bool enable) { m_texCoordArrayEnabled[tmu] = enable; }
    void setTexCoordSize(const std::size_t tmu, std::size_t size) { m_texCoordSize[tmu] = size; }
    void setTexCoordType(const std::size_t tmu, Type type) { m_texCoordType[tmu] = type; }
    void setTexCoordStride(const std::size_t tmu, std::size_t stride) { m_texCoordStride[tmu] = stride; }
    void setTexCoordPointer(const std::size_t tmu, const void* ptr) { m_texCoordPointer[tmu] = ptr; }
    void setTexCoord(const std::size_t tmu, const Vec4& texCoord) { m_texCoord[tmu] = texCoord; }

    void enableNormalArray(bool enable) { m_normalArrayEnabled = enable; }
    void setNormalType(Type type) { m_normalType = type; }
    void setNormalStride(std::size_t stride) { m_normalStride = stride; }
    void setNormalPointer(const void* ptr) { m_normalPointer = ptr; }
    void setNormal(const Vec3& normal) { m_normal = normal; }

    void enableColorArray(bool enable) { m_colorArrayEnabled = enable; }
    void setColorSize(std::size_t size) { m_colorSize = size; }
    void setColorType(Type type) { m_colorType = type; }
    void setColorStride(std::size_t stride) { m_colorStride = stride; }
    void setColorPointer(const void* ptr) { m_colorPointer = ptr; }
    void setVertexColor(const Vec4& color) { m_vertexColor = color; }

    void enablePointSizeArray(bool enable) { m_pointSizeArrayEnabled = enable; }
    void setPointSizeSize(std::size_t size) { m_pointSizeSize = size; }
    void setPointSizeType(Type type) { m_pointSizeType = type; }
    void setPointSizeStride(std::size_t stride) { m_pointSizeStride = stride; }
    void setPointSizePointer(const void* ptr) { m_pointSizePointer = ptr; }
    void setPointSize(float pointSize) { m_pointSize = pointSize; }

    void setDrawMode(DrawMode mode) { m_drawMode = mode; }

    void enableIndices(bool enable) { m_indicesEnabled = enable; }
    void setCount(std::size_t count) { m_count = count; }
    void setIndicesType(Type type) { m_indicesType = type; }
    void setIndicesPointer(const void* ptr) { m_indicesPointer = ptr; }
    void setArrayOffset(std::size_t offset) { m_arrayOffset = offset; }

private:
    template <typename T>
    T getFromArray(const Type type, const void* arr, const std::size_t stride, std::size_t size, const std::size_t index) const
    {
        T vec = T::createHomogeneous();
        if (arr)
        {
            const int8_t* a = reinterpret_cast<const int8_t*>(arr);
            if (stride == 0)
            {
                const std::size_t indexWithStride = index * size;
                switch (type)
                {
                case Type::BYTE:
                    vec.fromArray(a + (indexWithStride * 1), size);
                    break;
                case Type::UNSIGNED_BYTE:
                    vec.fromArray(reinterpret_cast<const uint8_t*>(a + (indexWithStride * 1)), size);
                    break;
                case Type::SHORT:
                    vec.fromArray(reinterpret_cast<const int16_t*>(a + (indexWithStride * 2)), size);
                    break;
                case Type::UNSIGNED_SHORT:
                    vec.fromArray(reinterpret_cast<const uint16_t*>(a + (indexWithStride * 2)), size);
                    break;
                case Type::UNSIGNED_INT:
                    vec.fromArray(reinterpret_cast<const int32_t*>(a + (indexWithStride * 4)), size);
                    break;
                default:
                    vec.fromArray(reinterpret_cast<const float*>(a + (indexWithStride * 4)), size);
                    break;
                }
            }
            else
            {
                const uint32_t indexWithStride = index * stride;
                switch (type)
                {
                case Type::BYTE:
                    vec.fromArray(a + indexWithStride, size);
                    break;
                case Type::UNSIGNED_BYTE:
                    vec.fromArray(reinterpret_cast<const uint8_t*>(a + indexWithStride), size);
                    break;
                case Type::SHORT:
                    vec.fromArray(reinterpret_cast<const int16_t*>(a + indexWithStride), size);
                    break;
                case Type::UNSIGNED_SHORT:
                    vec.fromArray(reinterpret_cast<const uint16_t*>(a + indexWithStride), size);
                    break;
                case Type::UNSIGNED_INT:
                    vec.fromArray(reinterpret_cast<const int32_t*>(a + indexWithStride), size);
                    break;
                default:
                    vec.fromArray(reinterpret_cast<const float*>(a + indexWithStride), size);
                    break;
                }
            }
        }
        return vec;
    }

    const char* drawModeToString(const DrawMode drawMode) const;
    const char* typeToString(const Type type) const;

    DrawMode m_drawMode;
    std::size_t m_count;

    bool m_vertexArrayEnabled;
    std::size_t m_vertexSize;
    Type m_vertexType;
    std::size_t m_vertexStride;
    const void* m_vertexPointer;

    std::bitset<MAX_TMU_COUNT> m_texCoordArrayEnabled;
    std::array<std::size_t, MAX_TMU_COUNT> m_texCoordSize;
    std::array<Type, MAX_TMU_COUNT> m_texCoordType;
    std::array<std::size_t, MAX_TMU_COUNT> m_texCoordStride;
    std::array<const void*, MAX_TMU_COUNT> m_texCoordPointer;
    std::array<Vec4, MAX_TMU_COUNT> m_texCoord {};

    bool m_normalArrayEnabled;
    Type m_normalType;
    std::size_t m_normalStride;
    const void* m_normalPointer;
    Vec3 m_normal { { 0.0f, 0.0f, 1.0f } };

    bool m_colorArrayEnabled;
    std::size_t m_colorSize;
    Type m_colorType;
    std::size_t m_colorStride;
    const void* m_colorPointer;
    Vec4 m_vertexColor { { 1.0f, 1.0f, 1.0f, 1.0f } };

    bool m_pointSizeArrayEnabled;
    std::size_t m_pointSizeSize;
    Type m_pointSizeType;
    std::size_t m_pointSizeStride;
    const void* m_pointSizePointer;
    float m_pointSize { 1.0f };

    bool m_indicesEnabled;
    Type m_indicesType;
    const void* m_indicesPointer;

    std::size_t m_arrayOffset;

    mutable std::size_t m_fetchCount { 0 };
};
} // namespace rr
#endif // RENDEROBJ_HPP
