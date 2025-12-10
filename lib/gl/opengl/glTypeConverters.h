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

[[maybe_unused]] static float cv(const GLclampf val)
{
    return std::clamp(val, 0.0f, 1.0f);
}

[[maybe_unused]] static GLint convertTexEnvMode(TexEnvMode& mode, const GLint param)
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

[[maybe_unused]] static GLint convertCombine(Combine& conv, GLint val, bool alpha)
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

[[maybe_unused]] static GLenum convertCombineToOpenGL(const Combine conf)
{
    switch (conf)
    {
    case Combine::REPLACE:
        return GL_REPLACE;
    case Combine::MODULATE:
        return GL_MODULATE;
    case Combine::ADD:
        return GL_ADD;
    case Combine::ADD_SIGNED:
        return GL_ADD_SIGNED;
    case Combine::INTERPOLATE:
        return GL_INTERPOLATE;
    case Combine::SUBTRACT:
        return GL_SUBTRACT;
    case Combine::DOT3_RGB:
        return GL_DOT3_RGB;
    case Combine::DOT3_RGBA:
        return GL_DOT3_RGBA;
    default:
        SPDLOG_WARN("convertCombineToOpenGL invalid Combine {}", static_cast<int>(conf));
        return GL_REPLACE;
    }
}

[[maybe_unused]] static GLint convertOperand(Operand& conf, GLint val, bool alpha)
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

[[maybe_unused]] static GLenum convertOperandToOpenGL(const Operand conf)
{
    switch (conf)
    {
    case Operand::SRC_ALPHA:
        return GL_SRC_ALPHA;
    case Operand::ONE_MINUS_SRC_ALPHA:
        return GL_ONE_MINUS_SRC_ALPHA;
    case Operand::SRC_COLOR:
        return GL_SRC_COLOR;
    case Operand::ONE_MINUS_SRC_COLOR:
        return GL_ONE_MINUS_SRC_COLOR;
    default:
        SPDLOG_WARN("convertOperandToOpenGL invalid Operand {}", static_cast<int>(conf));
        return GL_SRC_ALPHA;
    }
}

[[maybe_unused]] static GLint convertSrcReg(SrcReg& conf, GLint val)
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

[[maybe_unused]] static GLenum convertSrcRegToOpenGL(const SrcReg conf)
{
    switch (conf)
    {
    case SrcReg::TEXTURE:
        return GL_TEXTURE;
    case SrcReg::CONSTANT:
        return GL_CONSTANT;
    case SrcReg::PRIMARY_COLOR:
        return GL_PRIMARY_COLOR;
    case SrcReg::PREVIOUS:
        return GL_PREVIOUS;
    default:
        SPDLOG_WARN("convertSrcRegToOpenGL invalid SrcReg {}", static_cast<int>(conf));
        return GL_TEXTURE;
    }
}

[[maybe_unused]] static BlendFunc convertBlendFunc(const GLenum blendFunc)
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
        SPDLOG_WARN("convertBlendFunc 0x{:X} not suppored", blendFunc);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return BlendFunc::ZERO;
    }
    RIXGL::getInstance().setError(GL_INVALID_ENUM);
    return BlendFunc::ZERO;
}

[[maybe_unused]] static GLenum convertBlendFuncToOpenGL(const BlendFunc blendFunc)
{
    switch (blendFunc)
    {
    case BlendFunc::ZERO:
        return GL_ZERO;
    case BlendFunc::ONE:
        return GL_ONE;
    case BlendFunc::DST_COLOR:
        return GL_DST_COLOR;
    case BlendFunc::SRC_COLOR:
        return GL_SRC_COLOR;
    case BlendFunc::ONE_MINUS_DST_COLOR:
        return GL_ONE_MINUS_DST_COLOR;
    case BlendFunc::ONE_MINUS_SRC_COLOR:
        return GL_ONE_MINUS_SRC_COLOR;
    case BlendFunc::SRC_ALPHA:
        return GL_SRC_ALPHA;
    case BlendFunc::ONE_MINUS_SRC_ALPHA:
        return GL_ONE_MINUS_SRC_ALPHA;
    case BlendFunc::DST_ALPHA:
        return GL_DST_ALPHA;
    case BlendFunc::ONE_MINUS_DST_ALPHA:
        return GL_ONE_MINUS_DST_ALPHA;
    case BlendFunc::SRC_ALPHA_SATURATE:
        return GL_SRC_ALPHA_SATURATE;
    default:
        SPDLOG_WARN("convertBlendFuncToOpenGL invalid BlendFunc {}", static_cast<int>(blendFunc));
        return GL_ZERO;
    }
}

[[maybe_unused]] static void setClientState(const GLenum array, bool enable)
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
    case GL_POINT_SIZE_ARRAY_OES:
        SPDLOG_DEBUG("setClientState GL_POINT_SIZE_ARRAY_OES {}", enable);
        RIXGL::getInstance().vertexArray().enablePointSizeArray(enable);
        break;
    default:
        SPDLOG_WARN("setClientState 0x{:X} 0x{:X} not suppored", array, enable);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

[[maybe_unused]] static Type convertType(GLenum type)
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

[[maybe_unused]] static GLenum convertTypeToOpenGL(Type type)
{
    switch (type)
    {
    case Type::BYTE:
        return GL_BYTE;
    case Type::UNSIGNED_BYTE:
        return GL_UNSIGNED_BYTE;
    case Type::SHORT:
        return GL_SHORT;
    case Type::UNSIGNED_SHORT:
        return GL_UNSIGNED_SHORT;
    case Type::FLOAT:
        return GL_FLOAT;
    case Type::UNSIGNED_INT:
        return GL_UNSIGNED_INT;
    default:
        SPDLOG_WARN("convertTypeToOpenGL invalid Type {}", static_cast<int>(type));
        return GL_BYTE;
    }
}

[[maybe_unused]] static DrawMode convertDrawMode(GLenum drawMode)
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
    case GL_POINTS:
        return DrawMode::POINTS;
    default:
        SPDLOG_WARN("convertDrawMode 0x{:X} not suppored", drawMode);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return DrawMode::TRIANGLES;
    }
}

[[maybe_unused]] static GLenum convertTextureWrapMode(TextureWrapMode& conf, const GLenum mode)
{
    switch (mode)
    {
    case GL_CLAMP:
        SPDLOG_WARN("GL_CLAMP is not fully supported and emulated with GL_CLAMP_TO_EDGE");
        [[fallthrough]];
    case GL_CLAMP_TO_EDGE:
        conf = TextureWrapMode::CLAMP_TO_EDGE;
        break;
    case GL_REPEAT:
        conf = TextureWrapMode::REPEAT;
        break;
    default:
        SPDLOG_WARN("convertTextureWrapMode 0x{:X} not suppored", mode);
        conf = TextureWrapMode::REPEAT;
        return GL_INVALID_ENUM;
    }
    return GL_NO_ERROR;
}

[[maybe_unused]] static GLenum convertTextureWrapModeToOpenGL(const TextureWrapMode conf)
{
    switch (conf)
    {
    case TextureWrapMode::CLAMP_TO_EDGE:
        return GL_CLAMP_TO_EDGE;
    case TextureWrapMode::REPEAT:
        return GL_REPEAT;
    default:
        SPDLOG_WARN("convertTextureWrapModeToOpenGL invalid TextureWrapMode {}", static_cast<int>(conf));
        return GL_REPEAT;
    }
}

[[maybe_unused]] static GLenum convertTestFunc(TestFunc& conf, const GLenum mode)
{
    switch (mode)
    {
    case GL_ALWAYS:
        conf = TestFunc::ALWAYS;
        break;
    case GL_NEVER:
        conf = TestFunc::NEVER;
        break;
    case GL_LESS:
        conf = TestFunc::LESS;
        break;
    case GL_EQUAL:
        conf = TestFunc::EQUAL;
        break;
    case GL_LEQUAL:
        conf = TestFunc::LEQUAL;
        break;
    case GL_GREATER:
        conf = TestFunc::GREATER;
        break;
    case GL_NOTEQUAL:
        conf = TestFunc::NOTEQUAL;
        break;
    case GL_GEQUAL:
        conf = TestFunc::GEQUAL;
        break;

    default:
        SPDLOG_WARN("convertTestFunc 0x{:X} not suppored", mode);
        conf = TestFunc::ALWAYS;
        return GL_INVALID_ENUM;
    }
    return GL_NO_ERROR;
}

[[maybe_unused]] static GLenum convertTestFuncToOpenGL(const TestFunc conf)
{
    switch (conf)
    {
    case TestFunc::ALWAYS:
        return GL_ALWAYS;
    case TestFunc::NEVER:
        return GL_NEVER;
    case TestFunc::LESS:
        return GL_LESS;
    case TestFunc::EQUAL:
        return GL_EQUAL;
    case TestFunc::LEQUAL:
        return GL_LEQUAL;
    case TestFunc::GREATER:
        return GL_GREATER;
    case TestFunc::NOTEQUAL:
        return GL_NOTEQUAL;
    case TestFunc::GEQUAL:
        return GL_GEQUAL;
    default:
        SPDLOG_WARN("convertTestFuncToOpenGL invalid TestFunc {}", static_cast<int>(conf));
        return GL_ALWAYS;
    }
}

[[maybe_unused]] static GLenum convertStencilOp(StencilOp& conf, const GLenum mode)
{
    switch (mode)
    {
    case GL_KEEP:
        conf = StencilOp::KEEP;
        break;
    case GL_ZERO:
        conf = StencilOp::ZERO;
        break;
    case GL_REPLACE:
        conf = StencilOp::REPLACE;
        break;
    case GL_INCR:
        conf = StencilOp::INCR;
        break;
    case GL_INCR_WRAP_EXT:
        conf = StencilOp::INCR_WRAP;
        break;
    case GL_DECR:
        conf = StencilOp::DECR;
        break;
    case GL_DECR_WRAP_EXT:
        conf = StencilOp::DECR_WRAP;
        break;
    case GL_INVERT:
        conf = StencilOp::INVERT;
        break;

    default:
        SPDLOG_WARN("convertStencilOp 0x{:X} not suppored", mode);
        conf = StencilOp::KEEP;
        return GL_INVALID_ENUM;
    }
    return GL_NO_ERROR;
}

[[maybe_unused]] static GLenum convertStencilOpToOpenGL(const StencilOp conf)
{
    switch (conf)
    {
    case StencilOp::KEEP:
        return GL_KEEP;
    case StencilOp::ZERO:
        return GL_ZERO;
    case StencilOp::REPLACE:
        return GL_REPLACE;
    case StencilOp::INCR:
        return GL_INCR;
    case StencilOp::INCR_WRAP:
        return GL_INCR_WRAP_EXT;
    case StencilOp::DECR:
        return GL_DECR;
    case StencilOp::DECR_WRAP:
        return GL_DECR_WRAP_EXT;
    case StencilOp::INVERT:
        return GL_INVERT;
    default:
        SPDLOG_WARN("convertStencilOpToOpenGL invalid StencilOp {}", static_cast<int>(conf));
        return GL_KEEP;
    }
}

[[maybe_unused]] static GLenum convertLogicOp(LogicOp& conf, const GLenum opcode)
{
    switch (opcode)
    {
    case GL_CLEAR:
        conf = LogicOp::CLEAR;
        break;
    case GL_SET:
        conf = LogicOp::SET;
        break;
    case GL_COPY:
        conf = LogicOp::COPY;
        break;
    case GL_COPY_INVERTED:
        conf = LogicOp::COPY_INVERTED;
        break;
    case GL_NOOP:
        conf = LogicOp::NOOP;
        break;
    case GL_INVERT:
        conf = LogicOp::INVERT;
        break;
    case GL_AND:
        conf = LogicOp::AND;
        break;
    case GL_NAND:
        conf = LogicOp::NAND;
        break;
    case GL_OR:
        conf = LogicOp::OR;
        break;
    case GL_NOR:
        conf = LogicOp::NOR;
        break;
    case GL_XOR:
        conf = LogicOp::XOR;
        break;
    case GL_EQUIV:
        conf = LogicOp::EQUIV;
        break;
    case GL_AND_REVERSE:
        conf = LogicOp::AND_REVERSE;
        break;
    case GL_AND_INVERTED:
        conf = LogicOp::AND_INVERTED;
        break;
    case GL_OR_REVERSE:
        conf = LogicOp::OR_REVERSE;
        break;
    case GL_OR_INVERTED:
        conf = LogicOp::OR_INVERTED;
        break;
    default:
        SPDLOG_WARN("convertLogicOp 0x{:X} not suppored", opcode);
        conf = LogicOp::COPY;
        return GL_INVALID_ENUM;
    }
    return GL_NO_ERROR;
}

[[maybe_unused]] static GLenum convertLogicOpToOpenGL(const LogicOp conf)
{
    switch (conf)
    {
    case LogicOp::CLEAR:
        return GL_CLEAR;
    case LogicOp::SET:
        return GL_SET;
    case LogicOp::COPY:
        return GL_COPY;
    case LogicOp::COPY_INVERTED:
        return GL_COPY_INVERTED;
    case LogicOp::NOOP:
        return GL_NOOP;
    case LogicOp::INVERT:
        return GL_INVERT;
    case LogicOp::AND:
        return GL_AND;
    case LogicOp::NAND:
        return GL_NAND;
    case LogicOp::OR:
        return GL_OR;
    case LogicOp::NOR:
        return GL_NOR;
    case LogicOp::XOR:
        return GL_XOR;
    case LogicOp::EQUIV:
        return GL_EQUIV;
    case LogicOp::AND_REVERSE:
        return GL_AND_REVERSE;
    case LogicOp::AND_INVERTED:
        return GL_AND_INVERTED;
    case LogicOp::OR_REVERSE:
        return GL_OR_REVERSE;
    case LogicOp::OR_INVERTED:
        return GL_OR_INVERTED;
    default:
        SPDLOG_WARN("convertLogicOpToOpenGL invalid LogicOp {}", static_cast<int>(conf));
        return GL_COPY;
    }
}

[[maybe_unused]] static GLenum convertFace(Face& conf, const GLenum mode)
{
    switch (mode)
    {
    case GL_FRONT:
        conf = Face::FRONT;
        break;
    case GL_BACK:
        conf = Face::BACK;
        break;
    case GL_FRONT_AND_BACK:
        conf = Face::FRONT_AND_BACK;
        break;
    default:
        SPDLOG_WARN("convertFace 0x{:X} not suppored", mode);
        conf = Face::FRONT;
        return GL_INVALID_ENUM;
    }
    return GL_NO_ERROR;
}

[[maybe_unused]] static GLenum convertFaceToOpenGL(const Face conf)
{
    switch (conf)
    {
    case Face::FRONT:
        return GL_FRONT;
    case Face::BACK:
        return GL_BACK;
    case Face::FRONT_AND_BACK:
        return GL_FRONT_AND_BACK;
    default:
        SPDLOG_WARN("convertFaceToOpenGL invalid Face {}", static_cast<int>(conf));
        return GL_FRONT;
    }
}

[[maybe_unused]] static GLenum convertFogMode(FogMode& conf, const GLenum mode)
{
    switch (mode)
    {
    case GL_LINEAR:
        conf = FogMode::LINEAR;
        break;
    case GL_EXP:
        conf = FogMode::EXP;
        break;
    case GL_EXP2:
        conf = FogMode::EXP2;
        break;
    default:
        SPDLOG_WARN("convertFogMode 0x{:X} not suppored", mode);
        conf = FogMode::LINEAR;
        return GL_INVALID_ENUM;
    }
    return GL_NO_ERROR;
}

[[maybe_unused]] static GLenum convertFogModeToOpenGL(const FogMode conf)
{
    switch (conf)
    {
    case FogMode::LINEAR:
        return GL_LINEAR;
    case FogMode::EXP:
        return GL_EXP;
    case FogMode::EXP2:
        return GL_EXP2;
    default:
        SPDLOG_WARN("convertFogModeToOpenGL invalid FogMode {}", static_cast<int>(conf));
        return GL_LINEAR;
    }
}

[[maybe_unused]] static GLenum convertFrontFace(Orientation& conf, const GLenum mode)
{
    switch (mode)
    {
    case GL_CW:
        conf = Orientation::CW;
        break;
    case GL_CCW:
        conf = Orientation::CCW;
        break;
    default:
        SPDLOG_WARN("convertFrontFace 0x{:X} not supported", mode);
        conf = Orientation::CCW;
        return GL_INVALID_ENUM;
    }
    return GL_NO_ERROR;
}

[[maybe_unused]] static GLenum convertFrontFaceToOpenGL(const Orientation conf)
{
    switch (conf)
    {
    case Orientation::CW:
        return GL_CW;
    case Orientation::CCW:
        return GL_CCW;
    default:
        SPDLOG_WARN("convertFrontFaceToOpenGL invalid Orientation {}", static_cast<int>(conf));
        return GL_CCW;
    }
}

[[maybe_unused]] static GLenum convertMatrixMode(MatrixMode& conf, const GLenum mode)
{
    switch (mode)
    {
    case GL_MODELVIEW:
        conf = MatrixMode::MODELVIEW;
        break;
    case GL_PROJECTION:
        conf = MatrixMode::PROJECTION;
        break;
    case GL_TEXTURE:
        conf = MatrixMode::TEXTURE;
        break;
    case GL_COLOR:
        conf = MatrixMode::COLOR;
        break;
    default:
        SPDLOG_WARN("convertMatrixMode 0x{:X} not suppored", mode);
        conf = MatrixMode::MODELVIEW;
        return GL_INVALID_ENUM;
    }
    return GL_NO_ERROR;
}

[[maybe_unused]] static GLenum convertMatrixModeToOpenGL(const MatrixMode conf)
{
    switch (conf)
    {
    case MatrixMode::MODELVIEW:
        return GL_MODELVIEW;
    case MatrixMode::PROJECTION:
        return GL_PROJECTION;
    case MatrixMode::TEXTURE:
        return GL_TEXTURE;
    case MatrixMode::COLOR:
        return GL_COLOR;
    default:
        SPDLOG_WARN("convertMatrixModeToOpenGL invalid MatrixMode {}", static_cast<int>(conf));
        return GL_MODELVIEW;
    }
}

[[maybe_unused]] static GLenum convertBoolToGLboolean(const bool val)
{
    return val ? GL_TRUE : GL_FALSE;
}

} // namespace rr

#endif // GL_TYPE_CONVERTERS_H
