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

GLAPI void APIENTRY impl_glLightModelf(GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glLightModelf pname 0x{:X} param {} called", pname, param);
    if (pname == GL_LIGHT_MODEL_TWO_SIDE)
    {
        if (param == GL_TRUE)
        {
            RIXGL::getInstance().pipeline().getLighting().enableTwoSideModel(true);
        }
        else if (param == GL_FALSE)
        {
            RIXGL::getInstance().pipeline().getLighting().enableTwoSideModel(false);
        }
        else
        {
            RIXGL::getInstance().setError(GL_INVALID_VALUE);
        }
    }
    else
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
    }
}

GLAPI void APIENTRY impl_glLightModelfv(GLenum pname, const GLfloat* params)
{
    SPDLOG_DEBUG("glLightModelfv pname 0x{:X} called", pname);

    if (pname == GL_LIGHT_MODEL_AMBIENT)
    {
        RIXGL::getInstance().pipeline().getLighting().setAmbientColorScene({ params });
    }
    else
    {
        SPDLOG_DEBUG("glLightModelfv pname 0x{:X} params[0] {} redirected to glLightModelf", pname, params[0]);
        impl_glLightModelf(pname, params[0]);
    }
}

GLAPI void APIENTRY impl_glLightModeli(GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glLightModeli pname 0x{:X} param 0x{:X} redirected to glLightModelf", pname, param);
    impl_glLightModelf(pname, param);
}

GLAPI void APIENTRY impl_glLightModeliv(GLenum pname, const GLint* params)
{
    SPDLOG_DEBUG("glLightModeliv redirected to glLightModefv");
    Vec4 color = Vec4::createFromArray<GLint, 4>(params);
    color.div(255);
    impl_glLightModelfv(pname, color.data());
}
