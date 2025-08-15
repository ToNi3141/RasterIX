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

#include "MatrixStore.hpp"
#include <cmath>

namespace rr::matrixstore
{
MatrixStore::MatrixStore(TransformMatricesData& transformMatrices)
    : m_data { transformMatrices }
{
    m_data.modelView.identity();
    m_data.projection.identity();
    for (auto& mat : m_data.texture)
    {
        mat.identity();
    }
    m_data.color.identity();
}

void MatrixStore::setModelMatrix(const Mat44& m)
{
    m_data.modelView = m;
}

void MatrixStore::setProjectionMatrix(const Mat44& m)
{
    m_data.projection = m;
}

void MatrixStore::setColorMatrix(const Mat44& m)
{
    m_data.color = m;
}

void MatrixStore::setTextureMatrix(const Mat44& m)
{
    m_data.texture[m_tmu] = m;
}

void MatrixStore::multiply(const Mat44& mat)
{
    switch (m_matrixMode)
    {
    case MatrixMode::MODELVIEW:
        setModelMatrix(mat * m_data.modelView);
        break;
    case MatrixMode::PROJECTION:
        setProjectionMatrix(mat * m_data.projection);
        break;
    case MatrixMode::TEXTURE:
        setTextureMatrix(mat * m_data.texture[m_tmu]);
        break;
    case MatrixMode::COLOR:
        setColorMatrix(mat * m_data.color);
        break;
    default:
        break;
    }
}

void MatrixStore::translate(const float x, const float y, const float z)
{
    Mat44 m;
    m.identity();
    m[3][0] = x;
    m[3][1] = y;
    m[3][2] = z;
    multiply(m);
}

void MatrixStore::scale(const float x, const float y, const float z)
{
    Mat44 m;
    m.identity();
    m[0][0] = x;
    m[1][1] = y;
    m[2][2] = z;
    multiply(m);
}

void MatrixStore::rotate(const float angle, const float x, const float y, const float z)
{
    static constexpr float PI { 3.14159265358979323846f };
    float angle_rad = angle * (PI / 180.0f);

    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    float t = 1.0f - c;

    // clang-format off
    Mat44 m
    { { {
        { c + x * x * t    , y * x * t + z * s, z * x * t - y * s, 0.0f},
        { x * y * t - z * s, c + y * y * t    , z * y * t + x * s, 0.0f},
        { x * z * t + y * s, y * z * t - x * s, z * z * t + c    , 0.0f},
        { 0.0f             , 0.0f             , 0.0f             , 1.0f}
    } } };
    // clang-format on

    multiply(m);
}

void MatrixStore::loadIdentity()
{
    switch (m_matrixMode)
    {
    case MatrixMode::MODELVIEW:
        m_data.modelView.identity();
        break;
    case MatrixMode::PROJECTION:
        m_data.projection.identity();
        break;
    case MatrixMode::TEXTURE:
        m_data.texture[m_tmu].identity();
        break;
    case MatrixMode::COLOR:
        m_data.color.identity();
        break;
    default:
        break;
    }
}

bool MatrixStore::pushMatrix()
{
    switch (m_matrixMode)
    {
    case MatrixMode::MODELVIEW:
        return m_mStack.push(m_data.modelView);
    case MatrixMode::PROJECTION:
        return m_pStack.push(m_data.projection);
    case MatrixMode::COLOR:
        return m_cStack.push(m_data.color);
    case MatrixMode::TEXTURE:
        return m_tmStack[m_tmu].push(m_data.texture[m_tmu]);
    default:
        return false;
    }
}

bool MatrixStore::popMatrix()
{
    switch (m_matrixMode)
    {
    case MatrixMode::MODELVIEW:
        return m_mStack.pop(m_data.modelView);
    case MatrixMode::PROJECTION:
        return m_pStack.pop(m_data.projection);
    case MatrixMode::COLOR:
        return m_cStack.pop(m_data.color);
    case MatrixMode::TEXTURE:
        return m_tmStack[m_tmu].pop(m_data.texture[m_tmu]);
    default:
        return false;
    }
}

void MatrixStore::setMatrixMode(const MatrixMode matrixMode)
{
    m_matrixMode = matrixMode;
}

void MatrixStore::setTmu(const std::size_t tmu)
{
    m_tmu = tmu;
}

bool MatrixStore::loadMatrix(const Mat44& m)
{
    switch (m_matrixMode)
    {
    case MatrixMode::MODELVIEW:
        setModelMatrix(m);
        return true;
    case MatrixMode::PROJECTION:
        setProjectionMatrix(m);
        return true;
    case MatrixMode::TEXTURE:
        setTextureMatrix(m);
        return true;
    case MatrixMode::COLOR:
        setColorMatrix(m);
        return true;
    default:
        break;
    }
    return false;
}

std::size_t MatrixStore::getModelMatrixStackDepth()
{
    return MODEL_MATRIX_STACK_DEPTH;
}

std::size_t MatrixStore::getProjectionMatrixStackDepth()
{
    return PROJECTION_MATRIX_STACK_DEPTH;
}

} // namespace rr::matrixstore
