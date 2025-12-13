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
#include "Helpers.hpp"
#include "RIXGL.hpp"
#include "TypeConverters.hpp"
#include "pixelpipeline/PixelPipeline.hpp"
#include "vertexpipeline/VertexArray.hpp"
#include "vertexpipeline/VertexBuffer.hpp"
#include "vertexpipeline/VertexPipeline.hpp"
#include "vertexpipeline/VertexQueue.hpp"
#include <cmath>
#include <cstring>
#include <spdlog/spdlog.h>

using namespace rr;

GLAPI void APIENTRY impl_glFogf(GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glFogf pname 0x{:X} param {} called", pname, param);

    switch (pname)
    {
    case GL_FOG_MODE:
    {
        FogMode fogMode {};
        const GLenum ret = convertFogMode(fogMode, static_cast<GLenum>(param));
        if (ret == GL_NO_ERROR)
        {
            RIXGL::getInstance().pipeline().fog().setFogMode(fogMode);
        }
        else
        {
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
        }
    }
    break;
    case GL_FOG_DENSITY:
        if (param >= 0.0f)
        {
            RIXGL::getInstance().pipeline().fog().setFogDensity(param);
        }
        else
        {
            RIXGL::getInstance().setError(GL_INVALID_VALUE);
        }
        break;
    case GL_FOG_START:
        RIXGL::getInstance().pipeline().fog().setFogStart(param);
        break;
    case GL_FOG_END:
        RIXGL::getInstance().pipeline().fog().setFogEnd(param);
        break;
    default:
        SPDLOG_ERROR("Unknown pname 0x{:X} received. Deactivate fog to avoid artifacts.");
        impl_glDisable(GL_FOG);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI void APIENTRY impl_glFogfv(GLenum pname, const GLfloat* params)
{
    SPDLOG_DEBUG("glFogfv {} called", pname);

    switch (pname)
    {
    case GL_FOG_MODE:
    case GL_FOG_DENSITY:
    case GL_FOG_START:
    case GL_FOG_END:
        SPDLOG_DEBUG("glFogfv redirected to glFogf");
        impl_glFogf(pname, *params);
        break;
    case GL_FOG_COLOR:
    {
        RIXGL::getInstance().pipeline().fog().setFogColor({ params[0], params[1], params[2], params[3] });
    }
    break;
    default:
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI void APIENTRY impl_glFogi(GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glFogi redirected to glFogf");
    impl_glFogf(pname, static_cast<float>(param));
}

GLAPI void APIENTRY impl_glFogiv(GLenum pname, const GLint* params)
{
    SPDLOG_DEBUG("glFogiv pname 0x{:X} and params called", pname);

    switch (pname)
    {
    case GL_FOG_MODE:
    case GL_FOG_DENSITY:
    case GL_FOG_START:
    case GL_FOG_END:
        SPDLOG_DEBUG("glFogiv redirected to glFogi");
        impl_glFogi(pname, *params);
        break;
    case GL_FOG_COLOR:
    {
        Vec4 fogColor = Vec4::createFromArray<GLint, 4>(params);
        fogColor.div(255);
        RIXGL::getInstance().pipeline().fog().setFogColor({ fogColor[0], fogColor[1], fogColor[2], fogColor[3] });
        break;
    }
    default:
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}
