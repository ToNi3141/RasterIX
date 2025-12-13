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

GLAPI void APIENTRY impl_glGetTexEnvfv(GLenum target, GLenum pname, GLfloat* params)
{
    SPDLOG_DEBUG("glGetTexEnvfv target 0x{:X} pname 0x{:X} called", target, pname);
    switch (target)
    {
    case GL_TEXTURE_ENV:
        switch (pname)
        {
        case GL_TEXTURE_ENV_COLOR:
        {
            const Vec4& v = RIXGL::getInstance().pipeline().texture().getTexEnvColor();
            std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
        }
        break;
        default:
            SPDLOG_INFO("glGetTexEnvfv redirected to glGetTexEnviv");
            impl_glGetTexEnviv(target, pname, reinterpret_cast<GLint*>(params));
            break;
        }
        break;
    default:
        SPDLOG_INFO("glGetTexEnvfv redirected to glGetTexEnviv");
        impl_glGetTexEnviv(target, pname, reinterpret_cast<GLint*>(params));
        break;
    }
}

GLAPI void APIENTRY impl_glGetTexEnviv(GLenum target, GLenum pname, GLint* params)
{
    SPDLOG_DEBUG("glGetTexEnviv target 0x{:X} pname 0x{:X} called", target, pname);
    switch (target)
    {
    case GL_TEXTURE_ENV:
        switch (pname)
        {
        case GL_TEXTURE_ENV_MODE:
            *reinterpret_cast<TexEnvMode*>(params) = RIXGL::getInstance().pipeline().texture().getTexEnvMode();
            break;
        case GL_TEXTURE_ENV_COLOR:
        {
            Vec4 v = RIXGL::getInstance().pipeline().texture().getTexEnvColor();
            v.mul(std::numeric_limits<GLint>::max());
            params[0] = static_cast<GLint>(v[0]);
            params[1] = static_cast<GLint>(v[1]);
            params[2] = static_cast<GLint>(v[2]);
            params[3] = static_cast<GLint>(v[3]);
        }
        break;
        case GL_COMBINE_RGB:
            *params = convertCombineToOpenGL(RIXGL::getInstance().pipeline().texture().getCombineRgb());
            break;
        case GL_COMBINE_ALPHA:
            *params = convertCombineToOpenGL(RIXGL::getInstance().pipeline().texture().getCombineAlpha());
            break;
        case GL_SRC0_RGB:
            *params = convertSrcRegToOpenGL(RIXGL::getInstance().pipeline().texture().getSrcRegRgb0());
            break;
        case GL_SRC1_RGB:
            *params = convertSrcRegToOpenGL(RIXGL::getInstance().pipeline().texture().getSrcRegRgb1());
            break;
        case GL_SRC2_RGB:
            *params = convertSrcRegToOpenGL(RIXGL::getInstance().pipeline().texture().getSrcRegRgb2());
            break;
        case GL_SRC0_ALPHA:
            *params = convertSrcRegToOpenGL(RIXGL::getInstance().pipeline().texture().getSrcRegAlpha0());
            break;
        case GL_SRC1_ALPHA:
            *params = convertSrcRegToOpenGL(RIXGL::getInstance().pipeline().texture().getSrcRegAlpha1());
            break;
        case GL_SRC2_ALPHA:
            *params = convertSrcRegToOpenGL(RIXGL::getInstance().pipeline().texture().getSrcRegAlpha2());
            break;
        case GL_OPERAND0_RGB:
            *params = convertOperandToOpenGL(RIXGL::getInstance().pipeline().texture().getOperandRgb0());
            break;
        case GL_OPERAND1_RGB:
            *params = convertOperandToOpenGL(RIXGL::getInstance().pipeline().texture().getOperandRgb1());
            break;
        case GL_OPERAND2_RGB:
            *params = convertOperandToOpenGL(RIXGL::getInstance().pipeline().texture().getOperandRgb2());
            break;
        case GL_OPERAND0_ALPHA:
            *params = convertOperandToOpenGL(RIXGL::getInstance().pipeline().texture().getOperandAlpha0());
            break;
        case GL_OPERAND1_ALPHA:
            *params = convertOperandToOpenGL(RIXGL::getInstance().pipeline().texture().getOperandAlpha1());
            break;
        case GL_OPERAND2_ALPHA:
            *params = convertOperandToOpenGL(RIXGL::getInstance().pipeline().texture().getOperandAlpha2());
            break;
        case GL_RGB_SCALE:
            *params = std::pow(2.0f, static_cast<GLfloat>(RIXGL::getInstance().pipeline().texture().getShiftRgb()));
            break;
        case GL_ALPHA_SCALE:
            *params = std::pow(2.0f, static_cast<GLfloat>(RIXGL::getInstance().pipeline().texture().getShiftAlpha()));
            break;
        default:
            SPDLOG_ERROR("glGetTexEnviv pname 0x{:X} not supported for target GL_TEXTURE_ENV", pname);
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
            break;
        }
        break;
    case GL_POINT_SPRITE_OES:
        switch (pname)
        {
        case GL_COORD_REPLACE_OES:
            *params = convertBoolToGLboolean(RIXGL::getInstance().pipeline().getPointAssembly().getEnablePointSprite());
            break;
        default:
            SPDLOG_ERROR("glGetTexEnviv pname 0x{:X} not supported for target GL_POINT_SPRITE_OES", pname);
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
            break;
        }
        break;
    default:
        SPDLOG_ERROR("glGetTexEnviv target 0x{:X} not supported", target);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}