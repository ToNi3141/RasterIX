// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

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

#ifndef GL_TYPE_CONVERTERS_H
#define GL_TYPE_CONVERTERS_H

#include "Enums.hpp"
#include "RIXGL.hpp"
#include "gl.h"
#include "pixelpipeline/PixelPipeline.hpp"
#include "pixelpipeline/Texture.hpp"
#include "transform/MatrixStore.hpp"
#include "vertexpipeline/VertexArray.hpp"
#include "vertexpipeline/VertexQueue.hpp"
#include <algorithm>
#include <cstring>
#include <spdlog/spdlog.h>

namespace rr
{

float cv(const GLclampf val)
{
    return std::clamp(val, 0.0f, 1.0f);
}

GLint convertTexEnvMode(TexEnvMode& mode, const GLint param)
{
    GLint ret = GL_NO_ERROR;
    switch (param)
    {
        //    case GL_DISABLE:
        //        mode = TexEnvMode::DISABLE;
        //        break;
    case GL_REPLACE:
        mode = TexEnvMode::REPLACE;
        break;
    case GL_MODULATE:
        mode = TexEnvMode::MODULATE;
        break;
    case GL_DECAL:
        mode = TexEnvMode::DECAL;
        break;
    case GL_BLEND:
        mode = TexEnvMode::BLEND;
        break;
    case GL_ADD:
        mode = TexEnvMode::ADD;
        break;
    case GL_COMBINE:
        mode = TexEnvMode::COMBINE;
        break;
    default:
        SPDLOG_WARN("convertTexEnvMode 0x{:X} not suppored", param);
        ret = GL_INVALID_ENUM;
        mode = TexEnvMode::REPLACE;
        break;
    }
    return ret;
}

GLint convertCombine(Combine& conv, GLint val, bool alpha)
{
    GLint ret = GL_NO_ERROR;
    switch (val)
    {
    case GL_REPLACE:
        conv = Combine::REPLACE;
        break;
    case GL_MODULATE:
        conv = Combine::MODULATE;
        break;
    case GL_ADD:
        conv = Combine::ADD;
        break;
    case GL_ADD_SIGNED:
        conv = Combine::ADD_SIGNED;
        break;
    case GL_INTERPOLATE:
        conv = Combine::INTERPOLATE;
        break;
    case GL_SUBTRACT:
        conv = Combine::SUBTRACT;
        break;
    case GL_DOT3_RGB:
        if (alpha)
        {
            ret = GL_INVALID_ENUM;
        }
        else
        {
            conv = Combine::DOT3_RGB;
        }
        break;
    case GL_DOT3_RGBA:
        if (alpha)
        {
            ret = GL_INVALID_ENUM;
        }
        else
        {
            conv = Combine::DOT3_RGBA;
        }
        break;
    default:
        SPDLOG_WARN("convertCombine 0x{:X} 0x{:X} not suppored", val, alpha);
        ret = GL_INVALID_ENUM;
        break;
    }
    return ret;
}

GLint convertOperand(Operand& conf, GLint val, bool alpha)
{
    GLint ret = GL_NO_ERROR;
    switch (val)
    {
    case GL_SRC_ALPHA:
        conf = Operand::SRC_ALPHA;
        break;
    case GL_ONE_MINUS_SRC_ALPHA:
        conf = Operand::ONE_MINUS_SRC_ALPHA;
        break;
    case GL_SRC_COLOR:
        if (alpha)
        {
            ret = GL_INVALID_ENUM;
        }
        else
        {
            conf = Operand::SRC_COLOR;
        }
        break;
    case GL_ONE_MINUS_SRC_COLOR:
        if (alpha)
        {
            ret = GL_INVALID_ENUM;
        }
        else
        {
            conf = Operand::ONE_MINUS_SRC_COLOR;
        }
        break;
    default:
        SPDLOG_WARN("convertOperand 0x{:X} 0x{:X} not suppored", val, alpha);
        ret = GL_INVALID_ENUM;
    }
    return ret;
}

GLint convertSrcReg(SrcReg& conf, GLint val)
{
    GLint ret = GL_NO_ERROR;
    switch (val)
    {
    case GL_TEXTURE:
        conf = SrcReg::TEXTURE;
        break;
    case GL_CONSTANT:
        conf = SrcReg::CONSTANT;
        break;
    case GL_PRIMARY_COLOR:
        conf = SrcReg::PRIMARY_COLOR;
        break;
    case GL_PREVIOUS:
        conf = SrcReg::PREVIOUS;
        break;
    default:
        SPDLOG_WARN("convertSrcReg 0x{:X} not suppored", val);
        ret = GL_INVALID_ENUM;
        break;
    }
    return ret;
}

BlendFunc convertGlBlendFuncToRenderBlendFunc(const GLenum blendFunc)
{
    switch (blendFunc)
    {
    case GL_ZERO:
        return BlendFunc::ZERO;
    case GL_ONE:
        return BlendFunc::ONE;
    case GL_DST_COLOR:
        return BlendFunc::DST_COLOR;
    case GL_SRC_COLOR:
        return BlendFunc::SRC_COLOR;
    case GL_ONE_MINUS_DST_COLOR:
        return BlendFunc::ONE_MINUS_DST_COLOR;
    case GL_ONE_MINUS_SRC_COLOR:
        return BlendFunc::ONE_MINUS_SRC_COLOR;
    case GL_SRC_ALPHA:
        return BlendFunc::SRC_ALPHA;
    case GL_ONE_MINUS_SRC_ALPHA:
        return BlendFunc::ONE_MINUS_SRC_ALPHA;
    case GL_DST_ALPHA:
        return BlendFunc::DST_ALPHA;
    case GL_ONE_MINUS_DST_ALPHA:
        return BlendFunc::ONE_MINUS_DST_ALPHA;
    case GL_SRC_ALPHA_SATURATE:
        return BlendFunc::SRC_ALPHA_SATURATE;
    default:
        SPDLOG_WARN("convertGlBlendFuncToRenderBlendFunc 0x{:X} not suppored", blendFunc);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return BlendFunc::ZERO;
    }
    RIXGL::getInstance().setError(GL_INVALID_ENUM);
    return BlendFunc::ZERO;
}

void setClientState(const GLenum array, bool enable)
{
    switch (array)
    {
    case GL_COLOR_ARRAY:
        SPDLOG_DEBUG("setClientState GL_COLOR_ARRAY {}", enable);
        RIXGL::getInstance().vertexArray().enableColorArray(enable);
        break;
    case GL_NORMAL_ARRAY:
        SPDLOG_DEBUG("setClientState GL_NORMAL_ARRAY {}", enable);
        RIXGL::getInstance().vertexArray().enableNormalArray(enable);
        break;
    case GL_TEXTURE_COORD_ARRAY:
        SPDLOG_DEBUG("setClientState GL_TEXTURE_COORD_ARRAY {}", enable);
        RIXGL::getInstance().vertexArray().enableTexCoordArray(enable);
        break;
    case GL_VERTEX_ARRAY:
        SPDLOG_DEBUG("setClientState GL_VERTEX_ARRAY {}", enable);
        RIXGL::getInstance().vertexArray().enableVertexArray(enable);
        break;
    default:
        SPDLOG_WARN("setClientState 0x{:X} 0x{:X} not suppored", array, enable);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

Type convertType(GLenum type)
{
    switch (type)
    {
    case GL_BYTE:
        return Type::BYTE;
    case GL_UNSIGNED_BYTE:
        return Type::UNSIGNED_BYTE;
    case GL_SHORT:
        return Type::SHORT;
    case GL_UNSIGNED_SHORT:
        return Type::UNSIGNED_SHORT;
    case GL_FLOAT:
        return Type::FLOAT;
    case GL_UNSIGNED_INT:
        return Type::UNSIGNED_INT;
    default:
        SPDLOG_WARN("convertType 0x{:X} not suppored", type);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return Type::BYTE;
    }
}

DrawMode convertDrawMode(GLenum drawMode)
{
    switch (drawMode)
    {
    case GL_TRIANGLES:
        return DrawMode::TRIANGLES;
    case GL_TRIANGLE_FAN:
        return DrawMode::TRIANGLE_FAN;
    case GL_TRIANGLE_STRIP:
        return DrawMode::TRIANGLE_STRIP;
    case GL_QUAD_STRIP:
        return DrawMode::QUAD_STRIP;
    case GL_QUADS:
        return DrawMode::QUADS;
    case GL_POLYGON:
        return DrawMode::POLYGON;
    case GL_LINES:
        return DrawMode::LINES;
    case GL_LINE_STRIP:
        return DrawMode::LINE_STRIP;
    case GL_LINE_LOOP:
        return DrawMode::LINE_LOOP;
    default:
        SPDLOG_WARN("convertDrawMode 0x{:X} not suppored", drawMode);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return DrawMode::TRIANGLES;
    }
}

TextureWrapMode convertGlTextureWrapMode(const GLenum mode)
{
    switch (mode)
    {
    case GL_CLAMP:
        SPDLOG_WARN("GL_CLAMP is not fully supported and emulated with GL_CLAMP_TO_EDGE");
    case GL_CLAMP_TO_EDGE:
        return TextureWrapMode::CLAMP_TO_EDGE;
    case GL_REPEAT:
        return TextureWrapMode::REPEAT;
    default:
        SPDLOG_WARN("convertGlTextureWarpMode 0x{:X} not suppored", mode);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return TextureWrapMode::REPEAT;
    }
}

TestFunc convertTestFunc(const GLenum mode)
{
    switch (mode)
    {
    case GL_ALWAYS:
        return TestFunc::ALWAYS;
    case GL_NEVER:
        return TestFunc::NEVER;
    case GL_LESS:
        return TestFunc::LESS;
    case GL_EQUAL:
        return TestFunc::EQUAL;
    case GL_LEQUAL:
        return TestFunc::LEQUAL;
    case GL_GREATER:
        return TestFunc::GREATER;
    case GL_NOTEQUAL:
        return TestFunc::NOTEQUAL;
    case GL_GEQUAL:
        return TestFunc::GEQUAL;

    default:
        SPDLOG_WARN("convertTestFunc 0x{:X} not suppored", mode);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return TestFunc::ALWAYS;
    }
}

StencilOp convertStencilOp(const GLenum mode)
{
    switch (mode)
    {
    case GL_KEEP:
        return StencilOp::KEEP;
    case GL_ZERO:
        return StencilOp::ZERO;
    case GL_REPLACE:
        return StencilOp::REPLACE;
    case GL_INCR:
        return StencilOp::INCR;
    case GL_INCR_WRAP_EXT:
        return StencilOp::INCR_WRAP;
    case GL_DECR:
        return StencilOp::DECR;
    case GL_DECR_WRAP_EXT:
        return StencilOp::DECR_WRAP;
    case GL_INVERT:
        return StencilOp::INVERT;

    default:
        SPDLOG_WARN("convertStencilOp 0x{:X} not suppored", mode);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return StencilOp::KEEP;
    }
}

LogicOp convertLogicOp(const GLenum opcode)
{
    switch (opcode)
    {
    case GL_CLEAR:
        return LogicOp::CLEAR;
    case GL_SET:
        return LogicOp::SET;
    case GL_COPY:
        return LogicOp::COPY;
    case GL_COPY_INVERTED:
        return LogicOp::COPY_INVERTED;
    case GL_NOOP:
        return LogicOp::NOOP;
    case GL_INVERT:
        return LogicOp::INVERT;
    case GL_AND:
        return LogicOp::AND;
    case GL_NAND:
        return LogicOp::NAND;
    case GL_OR:
        return LogicOp::OR;
    case GL_NOR:
        return LogicOp::NOR;
    case GL_XOR:
        return LogicOp::XOR;
    case GL_EQUIV:
        return LogicOp::EQUIV;
    case GL_AND_REVERSE:
        return LogicOp::AND_REVERSE;
    case GL_AND_INVERTED:
        return LogicOp::AND_INVERTED;
    case GL_OR_REVERSE:
        return LogicOp::OR_REVERSE;
    case GL_OR_INVERTED:
        return LogicOp::OR_INVERTED;
    default:
        SPDLOG_WARN("convertLogicOp 0x{:X} not suppored", opcode);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return LogicOp::COPY;
    }
}

} // namespace rr

#endif // GL_TYPE_CONVERTERS_H
