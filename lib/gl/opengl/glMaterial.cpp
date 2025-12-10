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

GLAPI void APIENTRY impl_glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glMaterialf face 0x{:X} pname 0x{:X} param {} called", face, pname, param);

    if (face != GL_FRONT_AND_BACK)
    {
        SPDLOG_WARN("impl_glMaterialf face 0x{:X} not supported", face);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    if (pname != GL_SHININESS)
    {
        SPDLOG_WARN("glMaterialf not supported");
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    if ((param < 0.0f) && (param > 128.0f))
    {
        SPDLOG_WARN("glMaterialf param {} out of range", param);
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }

    RIXGL::getInstance().pipeline().getLighting().setSpecularExponentMaterial(param);
}

GLAPI void APIENTRY impl_glMaterialfv(GLenum face, GLenum pname, const GLfloat* params)
{
    SPDLOG_DEBUG("glMaterialfv face 0x{:X} pname 0x{:X} called", face, pname);

    if (face != GL_FRONT_AND_BACK)
    {
        SPDLOG_WARN("glMaterialfv face 0x{:X} not supported", face);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    switch (pname)
    {
    case GL_AMBIENT:
        RIXGL::getInstance().pipeline().getLighting().setAmbientColorMaterial({ params });
        break;
    case GL_DIFFUSE:
        RIXGL::getInstance().pipeline().getLighting().setDiffuseColorMaterial({ params });
        break;
    case GL_AMBIENT_AND_DIFFUSE:
        RIXGL::getInstance().pipeline().getLighting().setAmbientColorMaterial({ params });
        RIXGL::getInstance().pipeline().getLighting().setDiffuseColorMaterial({ params });
        break;
    case GL_SPECULAR:
        RIXGL::getInstance().pipeline().getLighting().setSpecularColorMaterial({ params });
        break;
    case GL_EMISSION:
        RIXGL::getInstance().pipeline().getLighting().setEmissionColorMaterial({ params });
        break;
    default:
        impl_glMaterialf(face, pname, params[0]);
        break;
    }
}

GLAPI void APIENTRY impl_glMateriali(GLenum face, GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glMateriali redirected to glMaterialf");
    impl_glMaterialf(face, pname, static_cast<float>(param));
}

GLAPI void APIENTRY impl_glMaterialiv(GLenum face, GLenum pname, const GLint* params)
{
    SPDLOG_DEBUG("glMaterialiv redirected to glMaterialfv");
    Vec4 color = Vec4::createFromArray<GLint, 4>(params);
    color.div(255);
    impl_glMaterialfv(face, pname, color.data());
}