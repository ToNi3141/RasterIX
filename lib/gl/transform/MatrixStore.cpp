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
    switch (m_matrixMode)
    {
    case MatrixMode::MODELVIEW:
        m_data.modelView.translate(x, y, z);
        break;
    case MatrixMode::PROJECTION:
        m_data.projection.translate(x, y, z);
        break;
    case MatrixMode::TEXTURE:
        m_data.texture[m_tmu].translate(x, y, z);
        break;
    case MatrixMode::COLOR:
        m_data.color.translate(x, y, z);
        break;
    default:
        break;
    }
}

void MatrixStore::scale(const float x, const float y, const float z)
{
    switch (m_matrixMode)
    {
    case MatrixMode::MODELVIEW:
        m_data.modelView.scale(x, y, z);
        break;
    case MatrixMode::PROJECTION:
        m_data.projection.scale(x, y, z);
        break;
    case MatrixMode::TEXTURE:
        m_data.texture[m_tmu].scale(x, y, z);
        break;
    case MatrixMode::COLOR:
        m_data.color.scale(x, y, z);
        break;
    default:
        break;
    }
}

void MatrixStore::rotate(const float angle, const float x, const float y, const float z)
{
    switch (m_matrixMode)
    {
    case MatrixMode::MODELVIEW:
        m_data.modelView.rotate(angle, x, y, z);
        break;
    case MatrixMode::PROJECTION:
        m_data.projection.rotate(angle, x, y, z);
        break;
    case MatrixMode::TEXTURE:
        m_data.texture[m_tmu].rotate(angle, x, y, z);
        break;
    case MatrixMode::COLOR:
        m_data.color.rotate(angle, x, y, z);
        break;
    default:
        break;
    }
}

void MatrixStore::frustum(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar)
{
    switch (m_matrixMode)
    {
    case MatrixMode::MODELVIEW:
        m_data.modelView.frustum(left, right, bottom, top, zNear, zFar);
        break;
    case MatrixMode::PROJECTION:
        m_data.projection.frustum(left, right, bottom, top, zNear, zFar);
        break;
    case MatrixMode::TEXTURE:
        m_data.texture[m_tmu].frustum(left, right, bottom, top, zNear, zFar);
        break;
    case MatrixMode::COLOR:
        m_data.color.frustum(left, right, bottom, top, zNear, zFar);
        break;
    default:
        break;
    }
}

void MatrixStore::ortho(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar)
{
    switch (m_matrixMode)
    {
    case MatrixMode::MODELVIEW:
        m_data.modelView.ortho(left, right, bottom, top, zNear, zFar);
        break;
    case MatrixMode::PROJECTION:
        m_data.projection.ortho(left, right, bottom, top, zNear, zFar);
        break;
    case MatrixMode::TEXTURE:
        m_data.texture[m_tmu].ortho(left, right, bottom, top, zNear, zFar);
        break;
    case MatrixMode::COLOR:
        m_data.color.ortho(left, right, bottom, top, zNear, zFar);
        break;
    default:
        break;
    }
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
