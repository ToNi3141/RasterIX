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

using namespace rr;

GLAPI void APIENTRY impl_glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glTexParameterf target 0x{:X} pname 0x{:X} param {} redirected to glTexParameteri", target, pname, param);
    impl_glTexParameteri(target, pname, static_cast<GLint>(param));
}

GLAPI void APIENTRY impl_glTexParameterfv([[maybe_unused]] GLenum target, [[maybe_unused]] GLenum pname, [[maybe_unused]] const GLfloat* params)
{
    SPDLOG_WARN("glTexParameterfv not implemented");
}

GLAPI void APIENTRY impl_glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glTexParameteri target 0x{:X} pname 0x{:X} param 0x{:X}", target, pname, param);
    if (target == GL_TEXTURE_2D)
    {
        switch (pname)
        {
        case GL_TEXTURE_WRAP_S:
        {
            TextureWrapMode mode;
            const GLenum error = convertTextureWrapMode(mode, static_cast<GLenum>(param));
            if (error == GL_NO_ERROR)
            {
                RIXGL::getInstance().pipeline().texture().setTexWrapModeS(mode);
            }
            else
            {
                RIXGL::getInstance().setError(error);
                SPDLOG_ERROR("glTexParameteri GL_TEXTURE_WRAP_S param 0x{:X} not supported", param);
            }
            break;
        }
        case GL_TEXTURE_WRAP_T:
        {
            TextureWrapMode mode;
            const GLenum error = convertTextureWrapMode(mode, static_cast<GLenum>(param));
            if (error == GL_NO_ERROR)
            {
                RIXGL::getInstance().pipeline().texture().setTexWrapModeT(mode);
            }
            else
            {
                RIXGL::getInstance().setError(error);
                SPDLOG_ERROR("glTexParameteri GL_TEXTURE_WRAP_T param 0x{:X} not supported", param);
            }
            break;
        }
        case GL_TEXTURE_MAG_FILTER:
            if ((param == GL_LINEAR) || (param == GL_NEAREST))
            {
                RIXGL::getInstance().pipeline().texture().setEnableMagFilter(param == GL_LINEAR);
            }
            else
            {
                RIXGL::getInstance().setError(GL_INVALID_ENUM);
                SPDLOG_ERROR("glTexParameteri GL_TEXTURE_MAG_FILTER param 0x{:X} not supported", param);
            }
            break;
        case GL_TEXTURE_MIN_FILTER:
            switch (param)
            {
            case GL_NEAREST:
            case GL_LINEAR:
                RIXGL::getInstance().pipeline().texture().setEnableMinFilter(false);
                break;
            case GL_NEAREST_MIPMAP_NEAREST:
            case GL_LINEAR_MIPMAP_NEAREST:
            case GL_NEAREST_MIPMAP_LINEAR:
            case GL_LINEAR_MIPMAP_LINEAR:
                if (RIXGL::getInstance().isMipmappingAvailable())
                {
                    RIXGL::getInstance().pipeline().texture().setEnableMinFilter(true);
                }
                break;
            default:
                SPDLOG_ERROR("glTexParameteri GL_TEXTURE_MIN_FILTER param 0x{:X} not supported", param);
                RIXGL::getInstance().setError(GL_INVALID_ENUM);
                break;
            }
            break;
        case GL_GENERATE_MIPMAP:
            if ((param == GL_TRUE) || (param == GL_FALSE))
            {
                RIXGL::getInstance().mipMapGenerator().setEnableMipMapGeneration(param == GL_TRUE);
            }
            else
            {
                SPDLOG_ERROR("glTexParameteri GL_GENERATE_MIPMAP param 0x{:X} not supported", param);
                RIXGL::getInstance().setError(GL_INVALID_ENUM);
            }
        default:
            SPDLOG_WARN("glTexParameteri pname 0x{:X} not supported", pname);
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
            break;
        }
    }
    else
    {
        SPDLOG_WARN("glTexParameteri target 0x{:X} not supported", target);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
    }
}

GLAPI void APIENTRY impl_glTexParameteriv([[maybe_unused]] GLenum target, [[maybe_unused]] GLenum pname, [[maybe_unused]] const GLint* params)
{
    SPDLOG_WARN("glTexParameteriv not implemented");
}