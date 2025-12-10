// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2025 ToNi3141

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

#include "RIXGL.hpp"
#include "glHelpers.hpp"
#include "glImpl.h"
#include "glTypeConverters.h"
#include "pixelpipeline/PixelPipeline.hpp"
#include "vertexpipeline/VertexArray.hpp"
#include "vertexpipeline/VertexBuffer.hpp"
#include "vertexpipeline/VertexPipeline.hpp"
#include "vertexpipeline/VertexQueue.hpp"
#include <cmath>
#include <cstring>
#include <spdlog/spdlog.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"

using namespace rr;

GLAPI void APIENTRY impl_glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    SPDLOG_DEBUG("glFrustum redirected to glFrustumf");
    impl_glFrustumf(
        static_cast<GLfloat>(left),
        static_cast<GLfloat>(right),
        static_cast<GLfloat>(bottom),
        static_cast<GLfloat>(top),
        static_cast<GLfloat>(zNear),
        static_cast<GLfloat>(zFar));
}

GLAPI void APIENTRY impl_glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
    SPDLOG_DEBUG("glFrustumf left {} right {} bottom {} top {} zNear {} zFar {} called", left, right, bottom, top, zNear, zFar);

    if ((zNear <= 0.0f) || (zFar <= 0.0f) || (left == right) || (bottom == top) || (zNear == zFar))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glFrustumf called with invalid values");
        return;
    }

    RIXGL::getInstance().pipeline().getMatrixStore().getCurrentMatrix().frustum(left, right, bottom, top, zNear, zFar);
}