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

#include "GLImpl.h"
#include "RIXGL.hpp"
#include "vertexpipeline/VertexPipeline.hpp"
#include <spdlog/spdlog.h>

using namespace rr;

GLAPI void APIENTRY impl_glGetClipPlane(GLenum plane, GLdouble* equation)
{
    SPDLOG_DEBUG("glGetClipPlane redirected to glGetClipPlanef");

    GLfloat equationFloat[4] {};
    impl_glGetClipPlanef(plane, equationFloat);
    equation[0] = static_cast<GLdouble>(equationFloat[0]);
    equation[1] = static_cast<GLdouble>(equationFloat[1]);
    equation[2] = static_cast<GLdouble>(equationFloat[2]);
    equation[3] = static_cast<GLdouble>(equationFloat[3]);
}

GLAPI void APIENTRY impl_glGetClipPlanef(GLenum plane, GLfloat* equation)
{
    SPDLOG_DEBUG("glGetClipPlanef plane {} called", plane);

    if (plane != GL_CLIP_PLANE0)
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        SPDLOG_ERROR("glGetClipPlanef only GL_CLIP_PLANE0 supported");
        return;
    }

    const Vec4 equationFloat = RIXGL::getInstance().pipeline().getPlaneClipper().getEquation();
    equation[0] = equationFloat[0];
    equation[1] = equationFloat[1];
    equation[2] = equationFloat[2];
    equation[3] = equationFloat[3];
}
