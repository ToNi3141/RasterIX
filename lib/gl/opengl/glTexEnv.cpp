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

using namespace rr;

GLAPI void APIENTRY impl_glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glTexEnvf target 0x{:X} pname 0x{:X} param {} redirected to glTexEnvi", target, pname, param);
    impl_glTexEnvi(target, pname, static_cast<GLint>(param));
}

GLAPI void APIENTRY impl_glTexEnvfv(GLenum target, GLenum pname, const GLfloat* params)
{
    SPDLOG_DEBUG("glTexEnvfv target 0x{:X} param 0x{:X} called", target, pname);

    if ((target == GL_TEXTURE_ENV) && (pname == GL_TEXTURE_ENV_COLOR))
    {
        if (!RIXGL::getInstance().pipeline().texture().setTexEnvColor({ params[0], params[1], params[2], params[3] }))
        {
            RIXGL::getInstance().setError(GL_OUT_OF_MEMORY);
            SPDLOG_ERROR("glTexEnvfv Out Of Memory");
        }
    }
    else
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
    }
}

GLAPI void APIENTRY impl_glTexEnvi(GLenum target, GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glTexEnvi target 0x{:X} pname 0x{:X} param 0x{:X} called", target, pname, param);

    GLenum error { GL_NO_ERROR };
    if (target == GL_TEXTURE_ENV)
    {
        switch (pname)
        {
        case GL_TEXTURE_ENV_MODE:
        {
            TexEnvMode mode {};
            error = convertTexEnvMode(mode, param);
            if (error == GL_NO_ERROR)
                RIXGL::getInstance().pipeline().texture().setTexEnvMode(mode);
            break;
        }
        case GL_COMBINE_RGB:
        {
            Combine c {};
            error = convertCombine(c, param, false);
            if (error == GL_NO_ERROR)
                RIXGL::getInstance().pipeline().texture().setCombineRgb(c);
            break;
        }
        case GL_COMBINE_ALPHA:
        {
            Combine c {};
            error = convertCombine(c, param, true);
            if (error == GL_NO_ERROR)
                RIXGL::getInstance().pipeline().texture().setCombineAlpha(c);
            break;
        }
        case GL_SRC0_RGB:
        {
            SrcReg c {};
            error = convertSrcReg(c, param);
            if (error == GL_NO_ERROR)
                RIXGL::getInstance().pipeline().texture().setSrcRegRgb0(c);
            break;
        }
        case GL_SRC1_RGB:
        {
            SrcReg c {};
            error = convertSrcReg(c, param);
            if (error == GL_NO_ERROR)
                RIXGL::getInstance().pipeline().texture().setSrcRegRgb1(c);
            break;
        }
        case GL_SRC2_RGB:
        {
            SrcReg c {};
            error = convertSrcReg(c, param);
            if (error == GL_NO_ERROR)
                RIXGL::getInstance().pipeline().texture().setSrcRegRgb2(c);
            break;
        }
        case GL_SRC0_ALPHA:
        {
            SrcReg c {};
            error = convertSrcReg(c, param);
            if (error == GL_NO_ERROR)
                RIXGL::getInstance().pipeline().texture().setSrcRegAlpha0(c);
            break;
        }
        case GL_SRC1_ALPHA:
        {
            SrcReg c {};
            error = convertSrcReg(c, param);
            if (error == GL_NO_ERROR)
                RIXGL::getInstance().pipeline().texture().setSrcRegAlpha1(c);
            break;
        }
        case GL_SRC2_ALPHA:
        {
            SrcReg c {};
            error = convertSrcReg(c, param);
            if (error == GL_NO_ERROR)
                RIXGL::getInstance().pipeline().texture().setSrcRegAlpha2(c);
            break;
        }
        case GL_OPERAND0_RGB:
        {
            Operand c {};
            error = convertOperand(c, param, false);
            if (error == GL_NO_ERROR)
                RIXGL::getInstance().pipeline().texture().setOperandRgb0(c);
            break;
        }
        case GL_OPERAND1_RGB:
        {
            Operand c {};
            error = convertOperand(c, param, false);
            if (error == GL_NO_ERROR)
                RIXGL::getInstance().pipeline().texture().setOperandRgb1(c);
            break;
        }
        case GL_OPERAND2_RGB:
        {
            Operand c {};
            error = convertOperand(c, param, false);
            if (error == GL_NO_ERROR)
                RIXGL::getInstance().pipeline().texture().setOperandRgb2(c);
            break;
        }
        case GL_OPERAND0_ALPHA:
        {
            Operand c {};
            error = convertOperand(c, param, true);
            if (error == GL_NO_ERROR)
                RIXGL::getInstance().pipeline().texture().setOperandAlpha0(c);
            break;
        }
        case GL_OPERAND1_ALPHA:
        {
            Operand c {};
            error = convertOperand(c, param, true);
            if (error == GL_NO_ERROR)
                RIXGL::getInstance().pipeline().texture().setOperandAlpha1(c);
            break;
        }
        case GL_OPERAND2_ALPHA:
        {
            Operand c {};
            error = convertOperand(c, param, true);
            if (error == GL_NO_ERROR)
                RIXGL::getInstance().pipeline().texture().setOperandAlpha2(c);
            break;
        }
        case GL_RGB_SCALE:
        {
            const uint8_t shift = log2f(param);
            if (shift <= 2)
            {
                RIXGL::getInstance().pipeline().texture().setShiftRgb(shift);
                error = GL_NO_ERROR;
            }
            else
            {
                error = GL_INVALID_ENUM;
                SPDLOG_ERROR("glTexEnvi: GL_RGB_SCALE param {} invalid", param);
            }
        }
        break;
        case GL_ALPHA_SCALE:
        {
            const uint8_t shift = log2f(param);
            if (shift <= 2)
            {
                RIXGL::getInstance().pipeline().texture().setShiftAlpha(shift);
                error = GL_NO_ERROR;
            }
            else
            {
                error = GL_INVALID_VALUE;
                SPDLOG_ERROR("glTexEnvi: GL_ALPHA_SCALE param {} invalid", param);
            }
        }
        break;
        default:
            error = GL_INVALID_ENUM;
            SPDLOG_ERROR("glTexEnvi: pname 0x{:X} not supported", pname);
            break;
        };
    }
    else if (target == GL_POINT_SPRITE_OES)
    {
        if (pname == GL_COORD_REPLACE_OES)
        {
            if (param == GL_TRUE)
            {
                RIXGL::getInstance().pipeline().getPointAssembly().setTexCoordReplace(true);
            }
            else if (param == GL_FALSE)
            {
                RIXGL::getInstance().pipeline().getPointAssembly().setTexCoordReplace(false);
            }
            else
            {
                error = GL_INVALID_VALUE;
                SPDLOG_ERROR("glTexEnvi: GL_COORD_REPLACE_OES param {} invalid", param);
            }
        }
        else
        {
            error = GL_INVALID_ENUM;
            SPDLOG_ERROR("glTexEnvi: pname 0x{:X} not supported for GL_POINT_SPRITE_OES", pname);
        }
    }
    else
    {
        error = GL_INVALID_ENUM;
        SPDLOG_ERROR("glTexEnvi: target 0x{:X} not supported", target);
    }
    if (error != GL_NO_ERROR)
    {
        RIXGL::getInstance().setError(error);
        SPDLOG_ERROR("glTexEnvi: error 0x{:X}", error);
    }
}

GLAPI void APIENTRY impl_glTexEnviv([[maybe_unused]] GLenum target, [[maybe_unused]] GLenum pname, [[maybe_unused]] const GLint* params)
{
    SPDLOG_WARN("glTexEnviv not implemented");
}