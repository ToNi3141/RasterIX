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

#include "ImageConverter.hpp"
#include "MipMapGenerator.hpp"
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

GLAPI void APIENTRY impl_glTexGend(GLenum coord, GLenum pname, GLdouble param)
{
    SPDLOG_DEBUG("glTexGend redirected to glTexGenf");
    impl_glTexGenf(coord, pname, static_cast<float>(param));
}

GLAPI void APIENTRY impl_glTexGendv(GLenum coord, GLenum pname, const GLdouble* params)
{
    SPDLOG_DEBUG("glTexGendv redirected to glTexGenfv");
    std::array<float, 4> tmp {};
    tmp[0] = static_cast<float>(params[0]);
    tmp[1] = static_cast<float>(params[1]);
    tmp[2] = static_cast<float>(params[2]);
    tmp[3] = static_cast<float>(params[3]);
    impl_glTexGenfv(coord, pname, tmp.data());
}

GLAPI void APIENTRY impl_glTexGenf(GLenum coord, GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glTexGenf redirected to glTexGeni");
    impl_glTexGeni(coord, pname, static_cast<GLint>(param));
}

GLAPI void APIENTRY impl_glTexGenfv(GLenum coord, GLenum pname, const GLfloat* params)
{
    SPDLOG_DEBUG("glTexGenfv coord 0x{:X} pname 0x{:X} called", coord, pname);
    switch (pname)
    {
    case GL_OBJECT_PLANE:
        switch (coord)
        {
        case GL_S:
            RIXGL::getInstance().pipeline().getTexGen().setTexGenVecObjS({ params });
            break;
        case GL_T:
            RIXGL::getInstance().pipeline().getTexGen().setTexGenVecObjT({ params });
            break;
        case GL_R:
            RIXGL::getInstance().pipeline().getTexGen().setTexGenVecObjR({ params });
            break;
        case GL_Q:
            SPDLOG_WARN("glTexGenfv GL_OBJECT_PLANE GL_Q not implemented");
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
            break;
        default:
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
            break;
        }
        break;
    case GL_EYE_PLANE:
        switch (coord)
        {
        case GL_S:
            RIXGL::getInstance().pipeline().getTexGen().setTexGenVecEyeS({ params });
            break;
        case GL_T:
            RIXGL::getInstance().pipeline().getTexGen().setTexGenVecEyeT({ params });
            break;
        case GL_R:
            RIXGL::getInstance().pipeline().getTexGen().setTexGenVecEyeR({ params });
            break;
        case GL_Q:
            SPDLOG_WARN("glTexGenfv GL_OBJECT_PLANE GL_Q not implemented");
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
            break;
        default:
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
            break;
        }
        break;
    default:
        SPDLOG_DEBUG("glTexGenfv redirected to glTexGenf");
        impl_glTexGenf(coord, pname, params[0]);
        break;
    }
}

GLAPI void APIENTRY impl_glTexGeni(GLenum coord, GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glTexGeni coord 0x{:X} pname 0x{:X} param 0x{:X} called", coord, pname, param);
    TexGenMode mode {};
    bool error { false };
    switch (param)
    {
    case GL_OBJECT_LINEAR:
        mode = TexGenMode::OBJECT_LINEAR;
        break;
    case GL_EYE_LINEAR:
        mode = TexGenMode::EYE_LINEAR;
        break;
    case GL_SPHERE_MAP:
        mode = TexGenMode::SPHERE_MAP;
        break;
    case GL_REFLECTION_MAP:
        mode = TexGenMode::REFLECTION_MAP;
        break;
    default:
        SPDLOG_WARN("glTexGeni param not supported");
        error = true;
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }

    if (!error && (pname == GL_TEXTURE_GEN_MODE))
    {
        switch (coord)
        {
        case GL_S:
            RIXGL::getInstance().pipeline().getTexGen().setTexGenModeS(mode);
            break;
        case GL_T:
            RIXGL::getInstance().pipeline().getTexGen().setTexGenModeT(mode);
            break;
        case GL_R:
            RIXGL::getInstance().pipeline().getTexGen().setTexGenModeR(mode);
            break;
        case GL_Q:
            SPDLOG_WARN("glTexGeni GL_Q not implemented");
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
            break;
        default:
            SPDLOG_WARN("glTexGeni coord not supported");
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
            break;
        }
    }
    else
    {
        SPDLOG_WARN("glTexGeni pname not supported");
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
    }
}

GLAPI void APIENTRY impl_glTexGeniv(GLenum coord, GLenum pname, const GLint* params)
{
    SPDLOG_DEBUG("glTexGeniv redirected to glTexGenfv");
    std::array<float, 4> tmp {};
    tmp[0] = static_cast<float>(params[0]);
    tmp[1] = static_cast<float>(params[1]);
    tmp[2] = static_cast<float>(params[2]);
    tmp[3] = static_cast<float>(params[3]);
    impl_glTexGenfv(coord, pname, tmp.data());
}