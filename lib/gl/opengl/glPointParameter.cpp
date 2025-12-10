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

GLAPI void APIENTRY impl_glPointParameterf(GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glPointParameterf pname 0x{:X} param {} called", pname, param);

    auto& setter = RIXGL::getInstance().pipeline().getPointAssembly();
    switch (pname)
    {
    case GL_POINT_SIZE_MIN:
        if (param < 0.0f)
        {
            SPDLOG_ERROR("glPointParameterf GL_POINT_SIZE_MIN < 0");
            RIXGL::getInstance().setError(GL_INVALID_VALUE);
        }
        else
        {
            setter.setPointSizeMin(param);
        }
        break;
    case GL_POINT_SIZE_MAX:
        if (param < 0.0f)
        {
            SPDLOG_ERROR("glPointParameterf GL_POINT_SIZE_MAX < 0");
            RIXGL::getInstance().setError(GL_INVALID_VALUE);
        }
        else
        {
            setter.setPointSizeMax(param);
        }
        break;
    case GL_POINT_FADE_THRESHOLD_SIZE:
        if (param < 0.0f)
        {
            SPDLOG_ERROR("glPointParameterf GL_POINT_FADE_THRESHOLD_SIZE < 0");
            RIXGL::getInstance().setError(GL_INVALID_VALUE);
        }
        else
        {
            setter.setPointFadeThresholdSize(param);
        }
        break;
    case GL_POINT_DISTANCE_ATTENUATION:
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glPointParameterf: GL_POINT_DISTANCE_ATTENUATION not allowed here.");
        break;
    default:
        SPDLOG_WARN("glPointParameterf: Unknown pname 0x{:X}", pname);
        break;
    }
}

GLAPI void APIENTRY impl_glPointParameterfv(GLenum pname, const GLfloat* params)
{
    SPDLOG_DEBUG("glPointParameterfv pname 0x{:X} and params[0] {} called", pname, params[0]);

    auto& setter = RIXGL::getInstance().pipeline().getPointAssembly();
    switch (pname)
    {
    case GL_POINT_DISTANCE_ATTENUATION:
        setter.setPointDistanceAttenuation(Vec3 { params[0], params[1], params[2] });
        break;
    case GL_POINT_SIZE_MIN:
    case GL_POINT_SIZE_MAX:
    case GL_POINT_FADE_THRESHOLD_SIZE:
        SPDLOG_DEBUG("glPointParameterfv pname 0x{:X} redirected to glPointParameterf", pname);
        impl_glPointParameterf(pname, params[0]);
        break;
    default:
        SPDLOG_WARN("glPointParameterfv: Unknown pname 0x{:X}", pname);
        break;
    }
}