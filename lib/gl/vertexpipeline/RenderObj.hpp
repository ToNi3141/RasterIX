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

    void initGetters();
    inline bool vertexArrayEnabled() const { return m_vertexArrayEnabled; }
    Vec4 getVertex(const std::size_t index) const { return m_getVertexFunc(m_vertexPointer, index, m_vertexSize, m_vertexStride, m_vertex); }
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
    std::size_t getVertexSize() const { return m_vertexSize; }
    Type getVertexType() const { return m_vertexType; }
    std::size_t getVertexStride() const { return m_vertexStride; }

    void enableTexCoordArray(const std::size_t tmu, bool enable) { m_texCoordArrayEnabled[tmu] = enable; }
    void setTexCoordSize(const std::size_t tmu, std::size_t size) { m_texCoordSize[tmu] = size; }
    void setTexCoordType(const std::size_t tmu, Type type) { m_texCoordType[tmu] = type; }
    void setTexCoordStride(const std::size_t tmu, std::size_t stride) { m_texCoordStride[tmu] = stride; }
    void setTexCoordPointer(const std::size_t tmu, const void* ptr) { m_texCoordPointer[tmu] = ptr; }
    void setTexCoord(const std::size_t tmu, const Vec4& texCoord) { m_texCoord[tmu] = texCoord; }
    std::size_t getTexCoordSize(const std::size_t tmu) const { return m_texCoordSize[tmu]; }
    Type getTexCoordType(const std::size_t tmu) const { return m_texCoordType[tmu]; }
    std::size_t getTexCoordStride(const std::size_t tmu) const { return m_texCoordStride[tmu]; }

    void enableNormalArray(bool enable) { m_normalArrayEnabled = enable; }
    void setNormalType(Type type) { m_normalType = type; }
    void setNormalStride(std::size_t stride) { m_normalStride = stride; }
    void setNormalPointer(const void* ptr) { m_normalPointer = ptr; }
    void setNormal(const Vec3& normal) { m_normal = normal; }
    Type getNormalType() const { return m_normalType; }
    std::size_t getNormalStride() const { return m_normalStride; }

    void enableColorArray(bool enable) { m_colorArrayEnabled = enable; }
    void setColorSize(std::size_t size) { m_colorSize = size; }
    void setColorType(Type type) { m_colorType = type; }
    void setColorStride(std::size_t stride) { m_colorStride = stride; }
    void setColorPointer(const void* ptr) { m_colorPointer = ptr; }
    void setVertexColor(const Vec4& color) { m_vertexColor = color; }
    std::size_t getColorSize() const { return m_colorSize; }
    Type getColorType() const { return m_colorType; }
    std::size_t getColorStride() const { return m_colorStride; }

    void enablePointSizeArray(bool enable) { m_pointSizeArrayEnabled = enable; }
    void setPointSizeType(Type type) { m_pointSizeType = type; }
    void setPointSizeStride(std::size_t stride) { m_pointSizeStride = stride; }
    void setPointSizePointer(const void* ptr) { m_pointSizePointer = ptr; }
    void setPointSize(float pointSize) { m_pointSize.fromArray(&pointSize, 1); }
    Type getPointSizeType() const { return m_pointSizeType; }
    std::size_t getPointSizeStride() const { return m_pointSizeStride; }
    float getPointSize() const { return m_pointSize[0]; }

    void setDrawMode(DrawMode mode) { m_drawMode = mode; }

    void enableIndices(bool enable) { m_indicesEnabled = enable; }
    void setCount(std::size_t count) { m_count = count; }
    void setIndicesType(Type type) { m_indicesType = type; }
    void setIndicesPointer(const void* ptr) { m_indicesPointer = ptr; }
    void setArrayOffset(std::size_t offset) { m_arrayOffset = offset; }

private:
    template <typename T, typename TTYPE>
    static T getFromArrayNoStride(const void* arr, const std::size_t index, const std::size_t size, const std::size_t, const T& defaultValue)
    {
        T vec = defaultValue;
        const TTYPE* a = reinterpret_cast<const TTYPE*>(arr);
        const std::size_t indexWithStride = index * size;
        vec.fromArray(a + indexWithStride, size);
        return vec;
    }

    template <typename T, typename TTYPE>
    static T getFromArrayWithStride(const void* arr, const std::size_t index, const std::size_t size, const std::size_t stride, const T& defaultValue)
    {
        T vec = defaultValue;
        const uint8_t* a = reinterpret_cast<const uint8_t*>(arr);
        const uint32_t indexWithStride = index * stride;
        vec.fromArray(reinterpret_cast<const TTYPE*>(a + indexWithStride), size);
        return vec;
    }

    static Vec4 getFromArrayDefault4(const void*, std::size_t, std::size_t, std::size_t, const Vec4& defaultValue)
    {
        return defaultValue;
    }

    static Vec4 getHomogeneous4(const void*, std::size_t, std::size_t, std::size_t, const Vec4&)
    {
        return Vec4::createHomogeneous();
    }

    static Vec4 getFromArrayNoStrideByte4(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec4& defaultValue)
    {
        return getFromArrayNoStride<Vec4, int8_t>(arr, index, size, 0, defaultValue);
    }

    static Vec4 getFromArrayNoStrideUnsignedByte4(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec4& defaultValue)
    {
        return getFromArrayNoStride<Vec4, uint8_t>(arr, index, size, 0, defaultValue);
    }

    static Vec4 getFromArrayNoStrideShort4(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec4& defaultValue)
    {
        return getFromArrayNoStride<Vec4, int16_t>(arr, index, size, 0, defaultValue);
    }

    static Vec4 getFromArrayNoStrideUnsignedShort4(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec4& defaultValue)
    {
        return getFromArrayNoStride<Vec4, uint16_t>(arr, index, size, 0, defaultValue);
    }

    static Vec4 getFromArrayNoStrideUnsignedInt4(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec4& defaultValue)
    {
        return getFromArrayNoStride<Vec4, uint32_t>(arr, index, size, 0, defaultValue);
    }

    static Vec4 getFromArrayNoStrideFloat4(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec4& defaultValue)
    {
        return getFromArrayNoStride<Vec4, float>(arr, index, size, 0, defaultValue);
    }

    static Vec4 getFromArrayWithStrideByte4(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec4& defaultValue)
    {
        return getFromArrayWithStride<Vec4, int8_t>(arr, index, size, stride, defaultValue);
    }

    static Vec4 getFromArrayWithStrideUnsignedByte4(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec4& defaultValue)
    {
        return getFromArrayWithStride<Vec4, uint8_t>(arr, index, size, stride, defaultValue);
    }

    static Vec4 getFromArrayWithStrideShort4(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec4& defaultValue)
    {
        return getFromArrayWithStride<Vec4, int16_t>(arr, index, size, stride, defaultValue);
    }

    static Vec4 getFromArrayWithStrideUnsignedShort4(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec4& defaultValue)
    {
        return getFromArrayWithStride<Vec4, uint16_t>(arr, index, size, stride, defaultValue);
    }

    static Vec4 getFromArrayWithStrideUnsignedInt4(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec4& defaultValue)
    {
        return getFromArrayWithStride<Vec4, uint32_t>(arr, index, size, stride, defaultValue);
    }

    static Vec4 getFromArrayWithStrideFloat4(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec4& defaultValue)
    {
        return getFromArrayWithStride<Vec4, float>(arr, index, size, stride, defaultValue);
    }

    static Vec3 getFromArrayDefault3(const void*, std::size_t, std::size_t, std::size_t, const Vec3& defaultValue)
    {
        return defaultValue;
    }

    static Vec3 getHomogeneous3(const void*, std::size_t, std::size_t, std::size_t, const Vec3&)
    {
        return Vec3::createHomogeneous();
    }

    static Vec3 getFromArrayNoStrideByte3(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec3& defaultValue)
    {
        return getFromArrayNoStride<Vec3, int8_t>(arr, index, size, 0, defaultValue);
    }

    static Vec3 getFromArrayNoStrideUnsignedByte3(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec3& defaultValue)
    {
        return getFromArrayNoStride<Vec3, uint8_t>(arr, index, size, 0, defaultValue);
    }

    static Vec3 getFromArrayNoStrideShort3(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec3& defaultValue)
    {
        return getFromArrayNoStride<Vec3, int16_t>(arr, index, size, 0, defaultValue);
    }

    static Vec3 getFromArrayNoStrideUnsignedShort3(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec3& defaultValue)
    {
        return getFromArrayNoStride<Vec3, uint16_t>(arr, index, size, 0, defaultValue);
    }

    static Vec3 getFromArrayNoStrideUnsignedInt3(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec3& defaultValue)
    {
        return getFromArrayNoStride<Vec3, uint32_t>(arr, index, size, 0, defaultValue);
    }

    static Vec3 getFromArrayNoStrideFloat3(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec3& defaultValue)
    {
        return getFromArrayNoStride<Vec3, float>(arr, index, size, 0, defaultValue);
    }

    static Vec3 getFromArrayWithStrideByte3(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec3& defaultValue)
    {
        return getFromArrayWithStride<Vec3, int8_t>(arr, index, size, stride, defaultValue);
    }

    static Vec3 getFromArrayWithStrideUnsignedByte3(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec3& defaultValue)
    {
        return getFromArrayWithStride<Vec3, uint8_t>(arr, index, size, stride, defaultValue);
    }

    static Vec3 getFromArrayWithStrideShort3(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec3& defaultValue)
    {
        return getFromArrayWithStride<Vec3, int16_t>(arr, index, size, stride, defaultValue);
    }

    static Vec3 getFromArrayWithStrideUnsignedShort3(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec3& defaultValue)
    {
        return getFromArrayWithStride<Vec3, uint16_t>(arr, index, size, stride, defaultValue);
    }

    static Vec3 getFromArrayWithStrideUnsignedInt3(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec3& defaultValue)
    {
        return getFromArrayWithStride<Vec3, uint32_t>(arr, index, size, stride, defaultValue);
    }

    static Vec3 getFromArrayWithStrideFloat3(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec3& defaultValue)
    {
        return getFromArrayWithStride<Vec3, float>(arr, index, size, stride, defaultValue);
    }

    static Vec1 getFromArrayDefault1(const void*, std::size_t, std::size_t, std::size_t, const Vec1& defaultValue)
    {
        return defaultValue;
    }

    static Vec1 getHomogeneous1(const void*, std::size_t, std::size_t, std::size_t, const Vec1&)
    {
        return Vec1::createHomogeneous();
    }

    static Vec1 getFromArrayNoStrideByte1(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec1& defaultValue)
    {
        return getFromArrayNoStride<Vec1, int8_t>(arr, index, size, 0, defaultValue);
    }

    static Vec1 getFromArrayNoStrideUnsignedByte1(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec1& defaultValue)
    {
        return getFromArrayNoStride<Vec1, uint8_t>(arr, index, size, 0, defaultValue);
    }

    static Vec1 getFromArrayNoStrideShort1(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec1& defaultValue)
    {
        return getFromArrayNoStride<Vec1, int16_t>(arr, index, size, 0, defaultValue);
    }

    static Vec1 getFromArrayNoStrideUnsignedShort1(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec1& defaultValue)
    {
        return getFromArrayNoStride<Vec1, uint16_t>(arr, index, size, 0, defaultValue);
    }

    static Vec1 getFromArrayNoStrideUnsignedInt1(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec1& defaultValue)
    {
        return getFromArrayNoStride<Vec1, uint32_t>(arr, index, size, 0, defaultValue);
    }

    static Vec1 getFromArrayNoStrideFloat1(const void* arr, std::size_t index, std::size_t size, std::size_t, const Vec1& defaultValue)
    {
        return getFromArrayNoStride<Vec1, float>(arr, index, size, 0, defaultValue);
    }

    static Vec1 getFromArrayWithStrideByte1(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec1& defaultValue)
    {
        return getFromArrayWithStride<Vec1, int8_t>(arr, index, size, stride, defaultValue);
    }

    static Vec1 getFromArrayWithStrideUnsignedByte1(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec1& defaultValue)
    {
        return getFromArrayWithStride<Vec1, uint8_t>(arr, index, size, stride, defaultValue);
    }

    static Vec1 getFromArrayWithStrideShort1(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec1& defaultValue)
    {
        return getFromArrayWithStride<Vec1, int16_t>(arr, index, size, stride, defaultValue);
    }

    static Vec1 getFromArrayWithStrideUnsignedShort1(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec1& defaultValue)
    {
        return getFromArrayWithStride<Vec1, uint16_t>(arr, index, size, stride, defaultValue);
    }

    static Vec1 getFromArrayWithStrideUnsignedInt1(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec1& defaultValue)
    {
        return getFromArrayWithStride<Vec1, uint32_t>(arr, index, size, stride, defaultValue);
    }

    static Vec1 getFromArrayWithStrideFloat1(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec1& defaultValue)
    {
        return getFromArrayWithStride<Vec1, float>(arr, index, size, stride, defaultValue);
    }

    template <typename T>
    static auto getFromArrayFuncVec(Vec1 (*func1)(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec1& defaultValue),
        Vec3 (*func3)(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec3& defaultValue),
        Vec4 (*func4)(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec4& defaultValue))
    {
        if constexpr (std::is_same_v<T, Vec1>)
            return func1;
        else if constexpr (std::is_same_v<T, Vec3>)
            return func3;
        else if constexpr (std::is_same_v<T, Vec4>)
            return func4;
    }

    template <typename T>
    static T (*getFromArrayFunc(const Type type, const void* arr, std::size_t stride))(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const T& defaultValue)
    {
        if (arr)
        {
            if (stride == 0)
            {
                switch (type)
                {
                case Type::BYTE:
                    return getFromArrayFuncVec<T>(&getFromArrayNoStrideByte1, &getFromArrayNoStrideByte3, &getFromArrayNoStrideByte4);
                case Type::UNSIGNED_BYTE:
                    return getFromArrayFuncVec<T>(&getFromArrayNoStrideUnsignedByte1, &getFromArrayNoStrideUnsignedByte3, &getFromArrayNoStrideUnsignedByte4);
                case Type::SHORT:
                    return getFromArrayFuncVec<T>(&getFromArrayNoStrideShort1, &getFromArrayNoStrideShort3, &getFromArrayNoStrideShort4);
                case Type::UNSIGNED_SHORT:
                    return getFromArrayFuncVec<T>(&getFromArrayNoStrideUnsignedShort1, &getFromArrayNoStrideUnsignedShort3, &getFromArrayNoStrideUnsignedShort4);
                case Type::UNSIGNED_INT:
                    return getFromArrayFuncVec<T>(&getFromArrayNoStrideUnsignedInt1, &getFromArrayNoStrideUnsignedInt3, &getFromArrayNoStrideUnsignedInt4);
                default:
                    return getFromArrayFuncVec<T>(&getFromArrayNoStrideFloat1, &getFromArrayNoStrideFloat3, &getFromArrayNoStrideFloat4);
                }
            }
            else
            {
                switch (type)
                {
                case Type::BYTE:
                    return getFromArrayFuncVec<T>(&getFromArrayWithStrideByte1, &getFromArrayWithStrideByte3, &getFromArrayWithStrideByte4);
                case Type::UNSIGNED_BYTE:
                    return getFromArrayFuncVec<T>(&getFromArrayWithStrideUnsignedByte1, &getFromArrayWithStrideUnsignedByte3, &getFromArrayWithStrideUnsignedByte4);
                case Type::SHORT:
                    return getFromArrayFuncVec<T>(&getFromArrayWithStrideShort1, &getFromArrayWithStrideShort3, &getFromArrayWithStrideShort4);
                case Type::UNSIGNED_SHORT:
                    return getFromArrayFuncVec<T>(&getFromArrayWithStrideUnsignedShort1, &getFromArrayWithStrideUnsignedShort3, &getFromArrayWithStrideUnsignedShort4);
                case Type::UNSIGNED_INT:
                    return getFromArrayFuncVec<T>(&getFromArrayWithStrideUnsignedInt1, &getFromArrayWithStrideUnsignedInt3, &getFromArrayWithStrideUnsignedInt4);
                default:
                    return getFromArrayFuncVec<T>(&getFromArrayWithStrideFloat1, &getFromArrayWithStrideFloat3, &getFromArrayWithStrideFloat4);
                }
            }
        }
        return getFromArrayFuncVec<T>(&getHomogeneous1, &getHomogeneous3, &getHomogeneous4);
    }

    const char* drawModeToString(const DrawMode drawMode) const;
    const char* typeToString(const Type type) const;

    Vec4 (*m_getVertexFunc)(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec4& defaultValue);
    Vec4 (*m_getTexCoordFunc[MAX_TMU_COUNT])(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec4& defaultValue);
    Vec4 (*m_getColorFunc)(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec4& defaultValue);
    Vec1 (*m_getPointSizeFunc)(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec1& defaultValue);
    Vec3 (*m_getNormalFunc)(const void* arr, std::size_t index, std::size_t size, std::size_t stride, const Vec3& defaultValue);
    float m_colorValueCorrection { 1.0f };

    DrawMode m_drawMode;
    std::size_t m_count;

    bool m_vertexArrayEnabled;
    std::size_t m_vertexSize;
    Type m_vertexType;
    std::size_t m_vertexStride;
    const void* m_vertexPointer;
    Vec4 m_vertex { { 0.0f, 0.0f, 0.0f, 1.0f } };

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
    Type m_pointSizeType;
    std::size_t m_pointSizeStride;
    const void* m_pointSizePointer;
    Vec1 m_pointSize { { 1.0f } };

    bool m_indicesEnabled;
    Type m_indicesType;
    const void* m_indicesPointer;

    std::size_t m_arrayOffset;

    mutable std::size_t m_fetchCount { 0 };
};
} // namespace rr
#endif // RENDEROBJ_HPP
