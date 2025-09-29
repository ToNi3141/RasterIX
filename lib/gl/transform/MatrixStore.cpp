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

Mat44& MatrixStore::getCurrentMatrix()
{
    switch (m_matrixMode)
    {
    case MatrixMode::MODELVIEW:
        return m_data.modelView;
    case MatrixMode::PROJECTION:
        return m_data.projection;
    case MatrixMode::TEXTURE:
        return m_data.texture[m_tmu];
    case MatrixMode::COLOR:
        return m_data.color;
    default:
        return m_data.modelView;
    }
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
