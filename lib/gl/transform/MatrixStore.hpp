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

#ifndef MATRIXSTORE_HPP
#define MATRIXSTORE_HPP

#include "RenderConfigs.hpp"
#include "Stack.hpp"
#include "math/Mat44.hpp"
#include "math/Vec.hpp"

namespace rr::matrixstore
{

struct TransformMatricesData
{
    std::array<Mat44, RenderConfig::TMU_COUNT> texture {};
    Mat44 modelView {};
    Mat44 projection {};
    Mat44 color {};
};

class MatrixStore
{
public:
    enum MatrixMode
    {
        MODELVIEW,
        PROJECTION,
        TEXTURE,
        COLOR
    };

    MatrixStore(TransformMatricesData& transformMatrices);

    const Mat44& getModelView() const { return m_data.modelView; }
    const Mat44& getProjection() const { return m_data.projection; }
    const Mat44& getTexture(const std::size_t tmu) const { return m_data.texture[tmu]; }
    const Mat44& getColor() const { return m_data.color; }

    void setModelMatrix(const Mat44& m);
    void setProjectionMatrix(const Mat44& m);
    void setTextureMatrix(const Mat44& m);
    void setColorMatrix(const Mat44& m);

    void multiply(const Mat44& mat);
    void translate(const float x, const float y, const float z);
    void scale(const float x, const float y, const float z);
    void rotate(const float angle, const float x, const float y, const float z);
    void loadIdentity();

    bool pushMatrix();
    bool popMatrix();

    void setMatrixMode(const MatrixMode matrixMode);
    void setTmu(const std::size_t tmu);
    bool loadMatrix(const Mat44& m);

    static std::size_t getModelMatrixStackDepth();
    static std::size_t getProjectionMatrixStackDepth();

private:
    static constexpr std::size_t MODEL_MATRIX_STACK_DEPTH { 16 };
    static constexpr std::size_t TEXTURE_MATRIX_STACK_DEPTH { 16 };
    static constexpr std::size_t PROJECTION_MATRIX_STACK_DEPTH { 4 };
    static constexpr std::size_t COLOR_MATRIX_STACK_DEPTH { 16 };

    MatrixMode m_matrixMode { MatrixMode::PROJECTION };
    Stack<Mat44, MODEL_MATRIX_STACK_DEPTH> m_mStack {};
    Stack<Mat44, PROJECTION_MATRIX_STACK_DEPTH> m_pStack {};
    std::array<Stack<Mat44, TEXTURE_MATRIX_STACK_DEPTH>, RenderConfig::TMU_COUNT> m_tmStack {};
    Stack<Mat44, COLOR_MATRIX_STACK_DEPTH> m_cStack {};
    TransformMatricesData& m_data;
    std::size_t m_tmu { 0 };
};

} // namespace rr::matrixstore
#endif // MATRIXSTORE_HPP
