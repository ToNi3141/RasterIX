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
#include "ImageConverter.hpp"
#include "MipMapGenerator.hpp"
#include "RIXGL.hpp"
#include "TypeConverters.hpp"
#include "vertexpipeline/VertexPipeline.hpp"
#include <spdlog/spdlog.h>

using namespace rr;

GLAPI void APIENTRY impl_glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
    SPDLOG_DEBUG("glGetTexParameterfv redirected to glGetTexParameteriv");
    impl_glGetTexParameteriv(target, pname, reinterpret_cast<GLint*>(params));
}

GLAPI void APIENTRY impl_glGetTexParameteriv(GLenum target, GLenum pname, GLint* params)
{
    SPDLOG_DEBUG("glGetTexParameteriv called for target 0x{:X} pname 0x{:X}", target, pname);
    if (target != GL_TEXTURE_2D)
    {
        SPDLOG_ERROR("glGetTexParameteriv only GL_TEXTURE_2D supported");
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }
    switch (pname)
    {
    case GL_TEXTURE_MIN_FILTER:
        *params = convertBoolToGLboolean(RIXGL::getInstance().pipeline().texture().minFilterEnabled());
        break;
    case GL_TEXTURE_MAG_FILTER:
        *params = convertBoolToGLboolean(RIXGL::getInstance().pipeline().texture().magFilterEnabled());
        break;
    case GL_TEXTURE_WRAP_S:
        *params = convertTextureWrapModeToOpenGL(RIXGL::getInstance().pipeline().texture().getTexWrapModeS());
        break;
    case GL_TEXTURE_WRAP_T:
        *params = convertTextureWrapModeToOpenGL(RIXGL::getInstance().pipeline().texture().getTexWrapModeT());
        break;
    case GL_GENERATE_MIPMAP:
        *params = convertBoolToGLboolean(RIXGL::getInstance().mipMapGenerator().getEnableMipMapGeneration());
        break;
    default:
        SPDLOG_ERROR("glGetTexParameteriv pname 0x{:X} not supported", pname);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}