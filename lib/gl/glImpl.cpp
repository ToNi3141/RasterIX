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

#include "glImpl.h"
#include "ImageConverter.hpp"
#include "MipMapGenerator.hpp"
#include "RIXGL.hpp"
#include "glHelpers.hpp"
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

GLAPI void APIENTRY impl_glAccum(GLenum op, GLfloat value)
{
    SPDLOG_WARN("glAccum not implemented");
}

GLAPI void APIENTRY impl_glAlphaFunc(GLenum func, GLclampf ref)
{
    SPDLOG_DEBUG("glAlphaFunc func 0x{:X} ref {}", func, ref);

    TestFunc testFunc;
    const GLenum error = convertTestFunc(testFunc, func);
    if (error == GL_NO_ERROR)
    {
        // Convert reference value from float to fix point
        ref = cv(ref);
        uint8_t refFix = ref * (1 << 8);
        if (ref >= 1.0f)
        {
            refFix = 0xff;
        }
        RIXGL::getInstance().pipeline().fragmentPipeline().setAlphaFunc(testFunc);
        RIXGL::getInstance().pipeline().fragmentPipeline().setRefAlphaValue(refFix);
    }
    else
    {
        RIXGL::getInstance().setError(error);
    }
}

GLAPI void APIENTRY impl_glBegin(GLenum mode)
{
    SPDLOG_DEBUG("glBegin 0x{:X} called", mode);
    RIXGL::getInstance().vertexQueue().begin(convertDrawMode(mode));
}

GLAPI void APIENTRY impl_glBitmap(GLsizei width, GLsizei height, GLfloat xOrig, GLfloat yOrig, GLfloat xMove, GLfloat yMove, const GLubyte* bitmap)
{
    SPDLOG_WARN("glBitmap not implemented");
}

GLAPI void APIENTRY impl_glBlendFunc(GLenum srcFactor, GLenum dstFactor)
{
    SPDLOG_DEBUG("glBlendFunc srcFactor 0x{:X} dstFactor 0x{:X} called", srcFactor, dstFactor);

    if (srcFactor == GL_SRC_ALPHA_SATURATE)
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
    }
    else
    {
        RIXGL::getInstance().pipeline().fragmentPipeline().setBlendFuncSFactor(convertBlendFunc(srcFactor));
        RIXGL::getInstance().pipeline().fragmentPipeline().setBlendFuncDFactor(convertBlendFunc(dstFactor));
    }
}

GLAPI void APIENTRY impl_glCallList(GLuint list)
{
    SPDLOG_WARN("glCallList not implemented");
}

GLAPI void APIENTRY impl_glCallLists(GLsizei n, GLenum type, const GLvoid* lists)
{
    SPDLOG_WARN("glCallLists not implemented");
}

GLAPI void APIENTRY impl_glClear(GLbitfield mask)
{
    SPDLOG_DEBUG("glClear mask 0x{:X} called", mask);

    if (!RIXGL::getInstance().pipeline().clearFramebuffer(mask & GL_COLOR_BUFFER_BIT, mask & GL_DEPTH_BUFFER_BIT, mask & GL_STENCIL_BUFFER_BIT))
    {
        RIXGL::getInstance().setError(GL_OUT_OF_MEMORY);
        SPDLOG_ERROR("glClear Out Of Memory");
    }
}

GLAPI void APIENTRY impl_glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    SPDLOG_WARN("glClearAccum not implemented");
}

GLAPI void APIENTRY impl_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    SPDLOG_DEBUG("glClearColor ({}, {}, {}, {}) called", red, green, blue, alpha);
    if (!RIXGL::getInstance().pipeline().setClearColor({ cv(red), cv(green), cv(blue), cv(alpha) }))
    {
        RIXGL::getInstance().setError(GL_OUT_OF_MEMORY);
        SPDLOG_ERROR("glClearColor Out Of Memory");
    }
}

GLAPI void APIENTRY impl_glClearDepth(GLclampd depth)
{
    SPDLOG_DEBUG("glClearDepth {} called", depth);

    if (!RIXGL::getInstance().pipeline().setClearDepth(cv(depth)))
    {
        RIXGL::getInstance().setError(GL_OUT_OF_MEMORY);
        SPDLOG_ERROR("glClearDepth Out Of Memory");
    }
}

GLAPI void APIENTRY impl_glClearIndex(GLfloat c)
{
    SPDLOG_WARN("glClearIndex not implemented");
}

GLAPI void APIENTRY impl_glClearStencil(GLint s)
{
    SPDLOG_DEBUG("glClearStencil {} called", s);

    RIXGL::getInstance().pipeline().getStencil().setClearStencil(s);
}

GLAPI void APIENTRY impl_glClipPlane(GLenum plane, const GLdouble* equation)
{
    SPDLOG_DEBUG("glClipPlane ({}, {}, {}, {}) called",
        equation[0],
        equation[1],
        equation[2],
        equation[3]);
    const Vec4 equationFloat {
        (static_cast<float>(equation[0])),
        (static_cast<float>(equation[1])),
        (static_cast<float>(equation[2])),
        (static_cast<float>(equation[3]))
    };
    RIXGL::getInstance().pipeline().getPlaneClipper().setEquation(equationFloat);
}

GLAPI void APIENTRY impl_glColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
    SPDLOG_DEBUG("glColor3b (0x{:X}, 0x{:X}, 0x{:X}) called", red, green, blue);
    const Vec4 color {
        (static_cast<float>(red) / std::numeric_limits<GLbyte>::max()),
        (static_cast<float>(green) / std::numeric_limits<GLbyte>::max()),
        (static_cast<float>(blue) / std::numeric_limits<GLbyte>::max()),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor3bv(const GLbyte* v)
{
    SPDLOG_DEBUG("glColor3bv (0x{:X}, 0x{:X}, 0x{:X}) called", v[0], v[1], v[2]);
    const Vec4 color {
        (static_cast<float>(v[0]) / std::numeric_limits<GLbyte>::max()),
        (static_cast<float>(v[1]) / std::numeric_limits<GLbyte>::max()),
        (static_cast<float>(v[2]) / std::numeric_limits<GLbyte>::max()),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
    SPDLOG_DEBUG("glColor3d ({}, {}, {}) called",
        static_cast<float>(red),
        static_cast<float>(green),
        static_cast<float>(blue));
    const Vec4 color {
        static_cast<float>(red),
        static_cast<float>(green),
        static_cast<float>(blue),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor3dv(const GLdouble* v)
{
    SPDLOG_DEBUG("glColor3dv ({}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]));
    const Vec4 color {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
    SPDLOG_DEBUG("glColor3f ({}, {}, {}) called", red, green, blue);
    const Vec4 color { red, green, blue, 1.0f };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor3fv(const GLfloat* v)
{
    SPDLOG_DEBUG("glColor3fv ({}, {}, {}) called", v[0], v[1], v[2]);
    const Vec4 color { v[0], v[1], v[2], 1.0f };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor3i(GLint red, GLint green, GLint blue)
{
    SPDLOG_DEBUG("glColor3i ({}, {}, {}) called", red, green, blue);
    const Vec4 color {
        (static_cast<float>(red) / std::numeric_limits<GLint>::max()),
        (static_cast<float>(green) / std::numeric_limits<GLint>::max()),
        (static_cast<float>(blue) / std::numeric_limits<GLint>::max()),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor3iv(const GLint* v)
{
    SPDLOG_DEBUG("glColor3iv ({}, {}, {}) called", v[0], v[1], v[2]);
    const Vec4 color {
        (static_cast<float>(v[0]) / std::numeric_limits<GLint>::max()),
        (static_cast<float>(v[1]) / std::numeric_limits<GLint>::max()),
        (static_cast<float>(v[2]) / std::numeric_limits<GLint>::max()),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor3s(GLshort red, GLshort green, GLshort blue)
{
    SPDLOG_DEBUG("glColor3s ({}, {}, {}) called", red, green, blue);
    const Vec4 color {
        (static_cast<float>(red) / std::numeric_limits<GLshort>::max()),
        (static_cast<float>(green) / std::numeric_limits<GLshort>::max()),
        (static_cast<float>(blue) / std::numeric_limits<GLshort>::max()),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor3sv(const GLshort* v)
{
    SPDLOG_DEBUG("glColor3sv ({}, {}, {}) called", v[0], v[1], v[2]);
    const Vec4 color {
        (static_cast<float>(v[0]) / std::numeric_limits<GLshort>::max()),
        (static_cast<float>(v[1]) / std::numeric_limits<GLshort>::max()),
        (static_cast<float>(v[2]) / std::numeric_limits<GLshort>::max()),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
    SPDLOG_DEBUG("glColor3ub (0x{:X}, 0x{:X}, 0x{:X}) called", red, green, blue);
    const Vec4 color {
        (static_cast<float>(red) / std::numeric_limits<GLubyte>::max()),
        (static_cast<float>(green) / std::numeric_limits<GLubyte>::max()),
        (static_cast<float>(blue) / std::numeric_limits<GLubyte>::max()),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor3ubv(const GLubyte* v)
{
    SPDLOG_DEBUG("glColor3ubv (0x{:X}, 0x{:X}, 0x{:X}) called", v[0], v[1], v[2]);
    const Vec4 color {
        (static_cast<float>(v[0]) / std::numeric_limits<GLubyte>::max()),
        (static_cast<float>(v[1]) / std::numeric_limits<GLubyte>::max()),
        (static_cast<float>(v[2]) / std::numeric_limits<GLubyte>::max()),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor3ui(GLuint red, GLuint green, GLuint blue)
{
    SPDLOG_DEBUG("glColor3ui (0x{:X}, 0x{:X}, 0x{:X}) called", red, green, blue);
    const Vec4 color {
        (static_cast<float>(red) / std::numeric_limits<GLuint>::max()),
        (static_cast<float>(green) / std::numeric_limits<GLuint>::max()),
        (static_cast<float>(blue) / std::numeric_limits<GLuint>::max()),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor3uiv(const GLuint* v)
{
    SPDLOG_DEBUG("glColor3uiv (0x{:X}, 0x{:X}, 0x{:X}) called", v[0], v[1], v[2]);
    const Vec4 color {
        (static_cast<float>(v[0]) / std::numeric_limits<GLuint>::max()),
        (static_cast<float>(v[1]) / std::numeric_limits<GLuint>::max()),
        (static_cast<float>(v[2]) / std::numeric_limits<GLuint>::max()),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor3us(GLushort red, GLushort green, GLushort blue)
{
    SPDLOG_DEBUG("glColor3us (0x{:X}, 0x{:X}, 0x{:X}) called", red, green, blue);
    const Vec4 color {
        (static_cast<float>(red) / std::numeric_limits<GLushort>::max()),
        (static_cast<float>(green) / std::numeric_limits<GLushort>::max()),
        (static_cast<float>(blue) / std::numeric_limits<GLushort>::max()),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor3usv(const GLushort* v)
{
    SPDLOG_DEBUG("glColor3usv (0x{:X}, 0x{:X}, 0x{:X}) called", v[0], v[1], v[2]);
    const Vec4 color {
        (static_cast<float>(v[0]) / std::numeric_limits<GLushort>::max()),
        (static_cast<float>(v[1]) / std::numeric_limits<GLushort>::max()),
        (static_cast<float>(v[2]) / std::numeric_limits<GLushort>::max()),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
    SPDLOG_DEBUG("glColor4b (0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}) called", red, green, blue, alpha);
    const Vec4 color {
        (static_cast<float>(red) / std::numeric_limits<GLbyte>::max()),
        (static_cast<float>(green) / std::numeric_limits<GLbyte>::max()),
        (static_cast<float>(blue) / std::numeric_limits<GLbyte>::max()),
        (static_cast<float>(alpha) / std::numeric_limits<GLbyte>::max())
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor4bv(const GLbyte* v)
{
    SPDLOG_DEBUG("glColor4bv (0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}) called", v[0], v[1], v[2], v[3]);
    const Vec4 color {
        (static_cast<float>(v[0]) / std::numeric_limits<GLbyte>::max()),
        (static_cast<float>(v[1]) / std::numeric_limits<GLbyte>::max()),
        (static_cast<float>(v[2]) / std::numeric_limits<GLbyte>::max()),
        (static_cast<float>(v[3]) / std::numeric_limits<GLbyte>::max())
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
    SPDLOG_DEBUG("glColor4d ({}, {}, {}, {}) called",
        static_cast<float>(red),
        static_cast<float>(green),
        static_cast<float>(blue),
        static_cast<float>(alpha));
    const Vec4 color {
        static_cast<float>(red),
        static_cast<float>(green),
        static_cast<float>(blue),
        static_cast<float>(alpha)
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor4dv(const GLdouble* v)
{
    SPDLOG_DEBUG("glColor4dv ({}, {}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3]));
    const Vec4 color {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3])
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    SPDLOG_DEBUG("glColor4f ({}, {}, {}, {}) called", red, green, blue, alpha);
    const Vec4 color { red, green, blue, alpha };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor4fv(const GLfloat* v)
{
    SPDLOG_DEBUG("glColor4fv ({}, {}, {}, {}) called", v[0], v[1], v[2], v[3]);
    const Vec4 color { v[0], v[1], v[2], v[3] };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor4i(GLint red, GLint green, GLint blue, GLint alpha)
{
    SPDLOG_DEBUG("glColor4i ({}, {}, {}, {}) called", red, green, blue, alpha);
    const Vec4 color {
        (static_cast<float>(red) / std::numeric_limits<GLint>::max()),
        (static_cast<float>(green) / std::numeric_limits<GLint>::max()),
        (static_cast<float>(blue) / std::numeric_limits<GLint>::max()),
        (static_cast<float>(alpha) / std::numeric_limits<GLint>::max())
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor4iv(const GLint* v)
{
    SPDLOG_DEBUG("glColor4iv ({}, {}, {}, {}) called", v[0], v[1], v[2], v[3]);
    const Vec4 color {
        (static_cast<float>(v[0]) / std::numeric_limits<GLint>::max()),
        (static_cast<float>(v[1]) / std::numeric_limits<GLint>::max()),
        (static_cast<float>(v[2]) / std::numeric_limits<GLint>::max()),
        (static_cast<float>(v[3]) / std::numeric_limits<GLint>::max())
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
    SPDLOG_DEBUG("glColor4s ({}, {}, {}, {}) called", red, green, blue, alpha);
    const Vec4 color {
        (static_cast<float>(red) / std::numeric_limits<GLshort>::max()),
        (static_cast<float>(green) / std::numeric_limits<GLshort>::max()),
        (static_cast<float>(blue) / std::numeric_limits<GLshort>::max()),
        (static_cast<float>(alpha) / std::numeric_limits<GLshort>::max())
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor4sv(const GLshort* v)
{
    SPDLOG_DEBUG("glColor4sv ({}, {}, {}, {}) called", v[0], v[1], v[2], v[3]);
    const Vec4 color {
        (static_cast<float>(v[0]) / std::numeric_limits<GLshort>::max()),
        (static_cast<float>(v[1]) / std::numeric_limits<GLshort>::max()),
        (static_cast<float>(v[2]) / std::numeric_limits<GLshort>::max()),
        (static_cast<float>(v[3]) / std::numeric_limits<GLshort>::max())
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    SPDLOG_DEBUG("glColor4ub (0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}) called", red, green, blue, alpha);
    const Vec4 color {
        (static_cast<float>(red) / std::numeric_limits<GLubyte>::max()),
        (static_cast<float>(green) / std::numeric_limits<GLubyte>::max()),
        (static_cast<float>(blue) / std::numeric_limits<GLubyte>::max()),
        (static_cast<float>(alpha) / std::numeric_limits<GLubyte>::max())
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor4ubv(const GLubyte* v)
{
    SPDLOG_DEBUG("glColor4ubv (0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}) called", v[0], v[1], v[2], v[3]);
    const Vec4 color {
        (static_cast<float>(v[0]) / std::numeric_limits<GLubyte>::max()),
        (static_cast<float>(v[1]) / std::numeric_limits<GLubyte>::max()),
        (static_cast<float>(v[2]) / std::numeric_limits<GLubyte>::max()),
        (static_cast<float>(v[3]) / std::numeric_limits<GLubyte>::max())
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
    SPDLOG_DEBUG("glColor4ui (0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}) called", red, green, blue, alpha);
    const Vec4 color {
        (static_cast<float>(red) / std::numeric_limits<GLuint>::max()),
        (static_cast<float>(green) / std::numeric_limits<GLuint>::max()),
        (static_cast<float>(blue) / std::numeric_limits<GLuint>::max()),
        (static_cast<float>(alpha) / std::numeric_limits<GLuint>::max())
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor4uiv(const GLuint* v)
{
    SPDLOG_DEBUG("glColor4uiv (0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}) called", v[0], v[1], v[2], v[3]);
    const Vec4 color {
        (static_cast<float>(v[0]) / std::numeric_limits<GLuint>::max()),
        (static_cast<float>(v[1]) / std::numeric_limits<GLuint>::max()),
        (static_cast<float>(v[2]) / std::numeric_limits<GLuint>::max()),
        (static_cast<float>(v[3]) / std::numeric_limits<GLuint>::max())
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
    SPDLOG_DEBUG("glColor4us (0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}) called", red, green, blue, alpha);
    const Vec4 color {
        (static_cast<float>(red) / std::numeric_limits<GLushort>::max()),
        (static_cast<float>(green) / std::numeric_limits<GLushort>::max()),
        (static_cast<float>(blue) / std::numeric_limits<GLushort>::max()),
        (static_cast<float>(alpha) / std::numeric_limits<GLushort>::max())
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColor4usv(const GLushort* v)
{
    SPDLOG_DEBUG("glColor4usv (0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}) called", v[0], v[1], v[2], v[3]);
    const Vec4 color {
        (static_cast<float>(v[0]) / std::numeric_limits<GLushort>::max()),
        (static_cast<float>(v[1]) / std::numeric_limits<GLushort>::max()),
        (static_cast<float>(v[2]) / std::numeric_limits<GLushort>::max()),
        (static_cast<float>(v[3]) / std::numeric_limits<GLushort>::max())
    };
    RIXGL::getInstance().vertexQueue().setColor(color);
    RIXGL::getInstance().vertexArray().setColor(color);
}

GLAPI void APIENTRY impl_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    SPDLOG_DEBUG("glColorMask red 0x{:X} green 0x{:X} blue 0x{:X} alpha 0x{:X} called", red, green, blue, alpha);
    RIXGL::getInstance().pipeline().fragmentPipeline().setColorMaskR(red == GL_TRUE);
    RIXGL::getInstance().pipeline().fragmentPipeline().setColorMaskG(green == GL_TRUE);
    RIXGL::getInstance().pipeline().fragmentPipeline().setColorMaskB(blue == GL_TRUE);
    RIXGL::getInstance().pipeline().fragmentPipeline().setColorMaskA(alpha == GL_TRUE);
}

GLAPI void APIENTRY impl_glColorMaterial(GLenum face, GLenum mode)
{
    SPDLOG_DEBUG("glColorMaterial face 0x{:X} mode 0x{:X} called", face, mode);

    bool error = false;
    Face faceConverted {};
    switch (face)
    {
    case GL_FRONT:
        faceConverted = Face::FRONT;
        break;
    case GL_BACK:
        faceConverted = Face::BACK;
        break;
    case GL_FRONT_AND_BACK:
        faceConverted = Face::FRONT_AND_BACK;
        break;
    default:
        SPDLOG_WARN("glColorMaterial face 0x{:X} not supported", face);
        faceConverted = Face::FRONT_AND_BACK;
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        error = true;
        break;
    }

    if (!error)
    {
        switch (mode)
        {
        case GL_AMBIENT:
            RIXGL::getInstance().pipeline().getLighting().setColorMaterialTracking(faceConverted, ColorMaterialTracking::AMBIENT);
            break;
        case GL_DIFFUSE:
            RIXGL::getInstance().pipeline().getLighting().setColorMaterialTracking(faceConverted, ColorMaterialTracking::DIFFUSE);
            break;
        case GL_AMBIENT_AND_DIFFUSE:
            RIXGL::getInstance().pipeline().getLighting().setColorMaterialTracking(faceConverted, ColorMaterialTracking::AMBIENT_AND_DIFFUSE);
            break;
        case GL_SPECULAR:
            RIXGL::getInstance().pipeline().getLighting().setColorMaterialTracking(faceConverted, ColorMaterialTracking::SPECULAR);
            break;
        case GL_EMISSION:
            RIXGL::getInstance().pipeline().getLighting().setColorMaterialTracking(faceConverted, ColorMaterialTracking::EMISSION);
            break;
        default:
            SPDLOG_WARN("glColorMaterial mode 0x{:X} not supported", mode);
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
            RIXGL::getInstance().pipeline().getLighting().setColorMaterialTracking(faceConverted, ColorMaterialTracking::AMBIENT_AND_DIFFUSE);
            break;
        }
    }
}

GLAPI void APIENTRY impl_glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
    SPDLOG_WARN("glCopyPixels not implemented");
}

GLAPI void APIENTRY impl_glCullFace(GLenum mode)
{
    SPDLOG_DEBUG("glCullFace mode 0x{:X} called", mode);
    Face face {};
    GLenum error = convertFace(face, mode);
    if (error == GL_NO_ERROR)
    {
        RIXGL::getInstance().pipeline().getCulling().setCullMode(face);
    }
    else
    {
        RIXGL::getInstance().setError(error);
    }
}

GLAPI void APIENTRY impl_glDeleteLists(GLuint list, GLsizei range)
{
    SPDLOG_WARN("glDeleteLists not implemented");
}

GLAPI void APIENTRY impl_glDepthFunc(GLenum func)
{
    SPDLOG_DEBUG("glDepthFunc 0x{:X} called", func);

    TestFunc testFunc;
    const GLenum error = convertTestFunc(testFunc, func);

    if (error == GL_NO_ERROR)
    {
        RIXGL::getInstance().pipeline().fragmentPipeline().setDepthFunc(testFunc);
    }
    else
    {
        RIXGL::getInstance().setError(error);
    }
}

GLAPI void APIENTRY impl_glDepthMask(GLboolean flag)
{
    SPDLOG_DEBUG("glDepthMask flag 0x{:X} called", flag);
    RIXGL::getInstance().pipeline().fragmentPipeline().setDepthMask(flag == GL_TRUE);
}

GLAPI void APIENTRY impl_glDepthRange(GLclampd zNear, GLclampd zFar)
{
    SPDLOG_DEBUG("glDepthRange zNear {} zFar {} called", zNear, zFar);
    RIXGL::getInstance().pipeline().getViewPort().setDepthRange(cv(zNear), cv(zFar));
}

GLAPI void APIENTRY impl_glDisable(GLenum cap)
{
    switch (cap)
    {
    case GL_TEXTURE_2D:
    {
        SPDLOG_DEBUG("glDisable GL_TEXTURE_2D called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableTmu(false);
        break;
    }
    case GL_ALPHA_TEST:
    {
        SPDLOG_DEBUG("glDisable GL_ALPHA_TEST called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableAlphaTest(false);
        break;
    }
    case GL_DEPTH_TEST:
    {
        SPDLOG_DEBUG("glDisable GL_DEPTH_TEST called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableDepthTest(false);
        break;
    }
    case GL_BLEND:
    {
        SPDLOG_DEBUG("glDisable GL_BLEND called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableBlending(false);
        break;
    }
    case GL_LIGHTING:
        SPDLOG_DEBUG("glDisable GL_LIGHTING called");
        RIXGL::getInstance().pipeline().getLighting().enableLighting(false);
        break;
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
        SPDLOG_DEBUG("glDisable GL_LIGHT{} called", cap - GL_LIGHT0);
        RIXGL::getInstance().pipeline().getLighting().enableLight(cap - GL_LIGHT0, false);
        break;
    case GL_TEXTURE_GEN_S:
        SPDLOG_DEBUG("glDisable GL_TEXTURE_GEN_S called");
        RIXGL::getInstance().pipeline().getTexGen().enableTexGenS(false);
        break;
    case GL_TEXTURE_GEN_T:
        SPDLOG_DEBUG("glDisable GL_TEXTURE_GEN_T called");
        RIXGL::getInstance().pipeline().getTexGen().enableTexGenT(false);
        break;
    case GL_TEXTURE_GEN_R:
        SPDLOG_DEBUG("glDisable GL_TEXTURE_GEN_R called");
        RIXGL::getInstance().pipeline().getTexGen().enableTexGenR(false);
        break;
    case GL_CULL_FACE:
        SPDLOG_DEBUG("glDisable GL_CULL_FACE called");
        RIXGL::getInstance().pipeline().getCulling().enableCulling(false);
        break;
    case GL_COLOR_MATERIAL:
        SPDLOG_DEBUG("glDisable GL_COLOR_MATERIAL called");
        RIXGL::getInstance().pipeline().getLighting().enableColorMaterial(false);
        break;
    case GL_FOG:
        SPDLOG_DEBUG("glDisable GL_FOG called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableFog(false);
        break;
    case GL_SCISSOR_TEST:
        SPDLOG_DEBUG("glDisable GL_SCISSOR_TEST called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableScissor(false);
        break;
    case GL_NORMALIZE:
        SPDLOG_DEBUG("glDisable GL_NORMALIZE called");
        RIXGL::getInstance().pipeline().getLighting().setEnableNormalNormalization(false);
        break;
    case GL_STENCIL_TEST:
        SPDLOG_DEBUG("glDisable GL_STENCIL_TEST called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableStencil(false);
        break;
    case GL_STENCIL_TEST_TWO_SIDE_EXT:
        SPDLOG_DEBUG("glDisable GL_STENCIL_TEST_TWO_SIDE_EXT called");
        RIXGL::getInstance().pipeline().getStencil().enableTwoSideStencil(false);
        break;
    case GL_COLOR_LOGIC_OP:
        SPDLOG_DEBUG("glDisable GL_COLOR_LOGIC_OP called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableLogicOp(false);
        break;
    case GL_CLIP_PLANE0:
        SPDLOG_DEBUG("glDisable GL_CLIP_PLANE0 called");
        RIXGL::getInstance().pipeline().getPlaneClipper().setEnable(false);
        break;
    case GL_POLYGON_OFFSET_FILL:
        SPDLOG_DEBUG("glDisable GL_POLYGON_OFFSET_FILL called");
        RIXGL::getInstance().pipeline().getPolygonOffset().setEnableFill(false);
        break;
    case GL_POINT_SPRITE_OES:
        SPDLOG_DEBUG("glDisable GL_POINT_SPRITE_OES called");
        RIXGL::getInstance().pipeline().getPointAssembly().setEnablePointSprite(false);
        break;
    case GL_DITHER:
        SPDLOG_DEBUG("glDisable GL_DITHER called");
        SPDLOG_INFO("glDisable GL_DITHER has no effect in RIXGL");
        break;
    case GL_LINE_SMOOTH:
        SPDLOG_DEBUG("glDisable GL_LINE_SMOOTH called");
        SPDLOG_INFO("glDisable GL_LINE_SMOOTH has no effect in RIXGL");
        break;
    case GL_MULTISAMPLE:
        SPDLOG_DEBUG("glDisable GL_MULTISAMPLE called");
        SPDLOG_INFO("glDisable GL_MULTISAMPLE has no effect in RIXGL");
        break;
    case GL_POINT_SMOOTH:
        SPDLOG_DEBUG("glDisable GL_POINT_SMOOTH called");
        SPDLOG_INFO("glDisable GL_POINT_SMOOTH has no effect in RIXGL");
        break;
    case GL_RESCALE_NORMAL:
        SPDLOG_DEBUG("glDisable GL_RESCALE_NORMAL called");
        RIXGL::getInstance().pipeline().getLighting().setEnableNormalRescale(false);
        break;
    case GL_SAMPLE_ALPHA_TO_COVERAGE:
        SPDLOG_DEBUG("glDisable GL_SAMPLE_ALPHA_TO_COVERAGE called");
        SPDLOG_INFO("glDisable GL_SAMPLE_ALPHA_TO_COVERAGE has no effect in RIXGL");
        break;
    case GL_SAMPLE_COVERAGE:
        SPDLOG_DEBUG("glDisable GL_SAMPLE_COVERAGE called");
        SPDLOG_INFO("glDisable GL_SAMPLE_COVERAGE has no effect in RIXGL");
        break;
    default:
        SPDLOG_WARN("glDisable cap 0x{:X} not supported", cap);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI void APIENTRY impl_glDrawBuffer(GLenum mode)
{
    SPDLOG_WARN("glDrawBuffer not implemented");
}

GLAPI void APIENTRY impl_glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
    SPDLOG_WARN("glDrawPixels not implemented");
}

GLAPI void APIENTRY impl_glEdgeFlag(GLboolean flag)
{
    SPDLOG_WARN("glEdgeFlag not implemented");
}

GLAPI void APIENTRY impl_glEdgeFlagv(const GLboolean* flag)
{
    SPDLOG_WARN("glEdgeFlagv not implemented");
}

GLAPI void APIENTRY impl_glEnable(GLenum cap)
{
    switch (cap)
    {
    case GL_TEXTURE_2D:
        SPDLOG_DEBUG("glEnable GL_TEXTURE_2D called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableTmu(true);
        break;
    case GL_ALPHA_TEST:
        SPDLOG_DEBUG("glEnable GL_ALPHA_TEST called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableAlphaTest(true);
        break;
    case GL_DEPTH_TEST:
        SPDLOG_DEBUG("glEnable GL_DEPTH_TEST called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableDepthTest(true);
        break;
    case GL_BLEND:
        SPDLOG_DEBUG("glEnable GL_BLEND called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableBlending(true);
        break;
    case GL_LIGHTING:
        SPDLOG_DEBUG("glEnable GL_LIGHTING called");
        RIXGL::getInstance().pipeline().getLighting().enableLighting(true);
        break;
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
        SPDLOG_DEBUG("glEnable GL_LIGHT{} called", cap - GL_LIGHT0);
        RIXGL::getInstance().pipeline().getLighting().enableLight(cap - GL_LIGHT0, true);
        break;
    case GL_TEXTURE_GEN_S:
        SPDLOG_DEBUG("glEnable GL_TEXTURE_GEN_S called");
        RIXGL::getInstance().pipeline().getTexGen().enableTexGenS(true);
        break;
    case GL_TEXTURE_GEN_T:
        SPDLOG_DEBUG("glEnable GL_TEXTURE_GEN_T called");
        RIXGL::getInstance().pipeline().getTexGen().enableTexGenT(true);
        break;
    case GL_TEXTURE_GEN_R:
        SPDLOG_DEBUG("glEnable GL_TEXTURE_GEN_R called");
        RIXGL::getInstance().pipeline().getTexGen().enableTexGenR(true);
        break;
    case GL_CULL_FACE:
        SPDLOG_DEBUG("glEnable GL_CULL_FACE called");
        RIXGL::getInstance().pipeline().getCulling().enableCulling(true);
        break;
    case GL_COLOR_MATERIAL:
        SPDLOG_DEBUG("glEnable GL_COLOR_MATERIAL called");
        RIXGL::getInstance().pipeline().getLighting().enableColorMaterial(true);
        break;
    case GL_FOG:
        SPDLOG_DEBUG("glEnable GL_FOG called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableFog(true);
        break;
    case GL_SCISSOR_TEST:
        SPDLOG_DEBUG("glEnable GL_SCISSOR_TEST called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableScissor(true);
        break;
    case GL_NORMALIZE:
        SPDLOG_DEBUG("glEnable GL_NORMALIZE called");
        RIXGL::getInstance().pipeline().getLighting().setEnableNormalNormalization(true);
        break;
    case GL_STENCIL_TEST:
        SPDLOG_DEBUG("glEnable GL_STENCIL_TEST called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableStencil(true);
        break;
    case GL_STENCIL_TEST_TWO_SIDE_EXT:
        SPDLOG_DEBUG("glEnable GL_STENCIL_TEST_TWO_SIDE_EXT called");
        RIXGL::getInstance().pipeline().getStencil().enableTwoSideStencil(true);
        break;
    case GL_COLOR_LOGIC_OP:
        SPDLOG_DEBUG("glEnable GL_COLOR_LOGIC_OP called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableLogicOp(true);
        break;
    case GL_CLIP_PLANE0:
        SPDLOG_DEBUG("glEnable GL_CLIP_PLANE0 called");
        RIXGL::getInstance().pipeline().getPlaneClipper().setEnable(true);
        break;
    case GL_POLYGON_OFFSET_FILL:
        SPDLOG_DEBUG("glEnable GL_POLYGON_OFFSET_FILL called");
        RIXGL::getInstance().pipeline().getPolygonOffset().setEnableFill(true);
        break;
    case GL_POINT_SPRITE_OES:
        SPDLOG_DEBUG("glEnable GL_POINT_SPRITE_OES called");
        RIXGL::getInstance().pipeline().getPointAssembly().setEnablePointSprite(true);
        break;
    case GL_DITHER:
        SPDLOG_DEBUG("glEnable GL_DITHER called");
        SPDLOG_INFO("glEnable GL_DITHER has no effect in RIXGL");
        break;
    case GL_LINE_SMOOTH:
        SPDLOG_DEBUG("glEnable GL_LINE_SMOOTH called");
        SPDLOG_INFO("glEnable GL_LINE_SMOOTH has no effect in RIXGL");
        break;
    case GL_MULTISAMPLE:
        SPDLOG_DEBUG("glEnable GL_MULTISAMPLE called");
        SPDLOG_INFO("glEnable GL_MULTISAMPLE has no effect in RIXGL");
        break;
    case GL_POINT_SMOOTH:
        SPDLOG_DEBUG("glEnable GL_POINT_SMOOTH called");
        SPDLOG_INFO("glEnable GL_POINT_SMOOTH has no effect in RIXGL");
        break;
    case GL_RESCALE_NORMAL:
        SPDLOG_DEBUG("glEnable GL_RESCALE_NORMAL called");
        RIXGL::getInstance().pipeline().getLighting().setEnableNormalRescale(true);
        break;
    case GL_SAMPLE_ALPHA_TO_COVERAGE:
        SPDLOG_DEBUG("glEnable GL_SAMPLE_ALPHA_TO_COVERAGE called");
        SPDLOG_INFO("glEnable GL_SAMPLE_ALPHA_TO_COVERAGE has no effect in RIXGL");
        break;
    case GL_SAMPLE_COVERAGE:
        SPDLOG_DEBUG("glEnable GL_SAMPLE_COVERAGE called");
        SPDLOG_INFO("glEnable GL_SAMPLE_COVERAGE has no effect in RIXGL");
        break;
    default:
        SPDLOG_WARN("glEnable cap 0x{:X} not supported", cap);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI void APIENTRY impl_glEnd(void)
{
    SPDLOG_DEBUG("glEnd called");
    RIXGL::getInstance().pipeline().drawObj(RIXGL::getInstance().vertexQueue().end());
}

GLAPI void APIENTRY impl_glEndList(void)
{
    SPDLOG_WARN("glEndList not implemented");
}

GLAPI void APIENTRY impl_glEvalCoord1d(GLdouble u)
{
    SPDLOG_WARN("glEvalCoord1d not implemented");
}

GLAPI void APIENTRY impl_glEvalCoord1dv(const GLdouble* u)
{
    SPDLOG_WARN("glEvalCoord1dv not implemented");
}

GLAPI void APIENTRY impl_glEvalCoord1f(GLfloat u)
{
    SPDLOG_WARN("glEvalCoord1f not implemented");
}

GLAPI void APIENTRY impl_glEvalCoord1fv(const GLfloat* u)
{
    SPDLOG_WARN("glEvalCoord1fv not implemented");
}

GLAPI void APIENTRY impl_glEvalCoord2d(GLdouble u, GLdouble v)
{
    SPDLOG_WARN("glEvalCoord2d not implemented");
}

GLAPI void APIENTRY impl_glEvalCoord2dv(const GLdouble* u)
{
    SPDLOG_WARN("glEvalCoord2dv not implemented");
}

GLAPI void APIENTRY impl_glEvalCoord2f(GLfloat u, GLfloat v)
{
    SPDLOG_WARN("glEvalCoord2f not implemented");
}

GLAPI void APIENTRY impl_glEvalCoord2fv(const GLfloat* u)
{
    SPDLOG_WARN("glEvalCoord2fv not implemented");
}

GLAPI void APIENTRY impl_glEvalMesh1(GLenum mode, GLint i1, GLint i2)
{
    SPDLOG_WARN("glEvalMesh1 not implemented");
}

GLAPI void APIENTRY impl_glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
    SPDLOG_WARN("glEvalMesh2 not implemented");
}

GLAPI void APIENTRY impl_glEvalPoint1(GLint i)
{
    SPDLOG_WARN("glEvalPoint1 not implemented");
}

GLAPI void APIENTRY impl_glEvalPoint2(GLint i, GLint j)
{
    SPDLOG_WARN("glEvalPoint2 not implemented");
}

GLAPI void APIENTRY impl_glFeedbackBuffer(GLsizei size, GLenum type, GLfloat* buffer)
{
    SPDLOG_WARN("glFeedbackBuffer not implemented");
}

GLAPI void APIENTRY impl_glFinish(void)
{
    SPDLOG_WARN("glFinish not implemented");
}

GLAPI void APIENTRY impl_glFlush(void)
{
    SPDLOG_WARN("glFlush not implemented");
}

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

GLAPI void APIENTRY impl_glFrontFace(GLenum mode)
{
    SPDLOG_DEBUG("glFrontFace mode 0x{:X} called", mode);

    Orientation conf {};
    const GLenum ret = convertFrontFace(conf, mode);

    if (ret == GL_NO_ERROR)
    {
        RIXGL::getInstance().pipeline().getCulling().setFrontFace(conf);
    }
    else
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
    }
}

GLAPI void APIENTRY impl_glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    SPDLOG_DEBUG("glFrustum redirected to glFrustumf");
    impl_glFrustumf(
        static_cast<GLfloat>(left),
        static_cast<GLfloat>(right),
        static_cast<GLfloat>(bottom),
        static_cast<GLfloat>(top),
        static_cast<GLfloat>(zNear),
        static_cast<GLfloat>(zFar));
}

GLAPI void APIENTRY impl_glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
    SPDLOG_DEBUG("glFrustumf left {} right {} bottom {} top {} zNear {} zFar {} called", left, right, bottom, top, zNear, zFar);

    if ((zNear <= 0.0f) || (zFar <= 0.0f) || (left == right) || (bottom == top) || (zNear == zFar))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glFrustumf called with invalid values");
        return;
    }

    RIXGL::getInstance().pipeline().getMatrixStore().getCurrentMatrix().frustum(left, right, bottom, top, zNear, zFar);
}

GLAPI GLuint APIENTRY impl_glGenLists(GLsizei range)
{
    SPDLOG_WARN("glGenLists not implemented");
    return 0;
}

GLAPI void APIENTRY impl_glGetBooleanv(GLenum pname, GLboolean* params)
{
    SPDLOG_DEBUG("glGetBooleanv pname 0x{:X} called", pname);

    GLfloat floatVals[16] = { 0.0f }; // Large enough for all cases
    SPDLOG_DEBUG("glGetIntegerv redirected to glGetFloatv", pname);
    impl_glGetFloatv(pname, floatVals);
    std::size_t count = 1;
    switch (pname)
    {
    case GL_COLOR_CLEAR_VALUE:
    case GL_CURRENT_COLOR:
    case GL_FOG_COLOR:
    case GL_LIGHT_MODEL_AMBIENT:
    case GL_TEXTURE_ENV_COLOR:
    case GL_MAX_VIEWPORT_DIMS:
    case GL_SCISSOR_BOX:
    case GL_COLOR_WRITEMASK:
        count = 4;
        break;
    case GL_MODELVIEW_MATRIX:
    case GL_PROJECTION_MATRIX:
    case GL_TEXTURE_MATRIX:
        count = 16;
        break;
    case GL_POINT_DISTANCE_ATTENUATION:
    case GL_CURRENT_NORMAL:
        count = 3;
        break;
    case GL_DEPTH_RANGE:
    case GL_SMOOTH_LINE_WIDTH_RANGE:
    case GL_SMOOTH_POINT_SIZE_RANGE:
        count = 2;
        break;
    default:
        count = 1;
        break;
    }
    for (std::size_t i = 0; i < count; i++)
        params[i] = convertBoolToGLboolean(floatVals[i] > 0.0f);
}

GLAPI void APIENTRY impl_glGetClipPlane(GLenum plane, GLdouble* equation)
{
    SPDLOG_DEBUG("glGetClipPlane redirected to glGetClipPlanef");

    GLfloat equationFloat[4];
    impl_glGetClipPlanef(plane, equationFloat);
    equation[0] = static_cast<GLdouble>(equationFloat[0]);
    equation[1] = static_cast<GLdouble>(equationFloat[1]);
    equation[2] = static_cast<GLdouble>(equationFloat[2]);
    equation[3] = static_cast<GLdouble>(equationFloat[3]);
}

GLAPI void APIENTRY impl_glGetClipPlanef(GLenum plane, GLfloat* equation)
{
    SPDLOG_DEBUG("glGetClipPlanef plane {} called", plane);

    if (plane != GL_CLIP_PLANE0)
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        SPDLOG_ERROR("glGetClipPlanef only GL_CLIP_PLANE0 supported");
        return;
    }

    const Vec4 equationFloat = RIXGL::getInstance().pipeline().getPlaneClipper().getEquation();
    equation[0] = equationFloat[0];
    equation[1] = equationFloat[1];
    equation[2] = equationFloat[2];
    equation[3] = equationFloat[3];
}

GLAPI void APIENTRY impl_glGetDoublev(GLenum pname, GLdouble* params)
{
    SPDLOG_WARN("glGetDoublev not implemented");
}

GLAPI GLenum APIENTRY impl_glGetError(void)
{
    SPDLOG_DEBUG("glGetError called");
    const GLenum error = RIXGL::getInstance().getError();
    RIXGL::getInstance().resetError();
    return error;
}

GLAPI void APIENTRY impl_glGetFloatv(GLenum pname, GLfloat* params)
{
    SPDLOG_DEBUG("glGetFloatv pname 0x{:X} called", pname);

    switch (pname)
    {
    case GL_ACTIVE_TEXTURE:
        params[0] = static_cast<GLfloat>(GL_TEXTURE0 + RIXGL::getInstance().pipeline().texture().getActiveTmu());
        break;
    case GL_ALIASED_POINT_SIZE_RANGE:
        params[0] = 1.0f;
        params[1] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getPointAssembly().getMaxPointSize());
        break;
    case GL_ALIASED_LINE_WIDTH_RANGE:
        params[0] = 1.0f;
        params[1] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getLineAssembly().getMaxLineWidth());
        break;
    case GL_ALPHA_BITS:
        params[0] = 4.0f;
        break;
    case GL_ALPHA_TEST:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableAlphaTest()));
        break;
    case GL_ALPHA_TEST_FUNC:
        params[0] = static_cast<GLfloat>(convertTestFuncToOpenGL(RIXGL::getInstance().pipeline().fragmentPipeline().getAlphaFunc()));
        break;
    case GL_ALPHA_TEST_REF:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().fragmentPipeline().getRefAlphaValue()) / 255.0f;
        break;
    case GL_ARRAY_BUFFER_BINDING:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getVertexBufferId());
        break;
    case GL_BLEND:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableBlending()));
        break;
    case GL_BLEND_DST:
        params[0] = static_cast<GLfloat>(convertBlendFuncToOpenGL(RIXGL::getInstance().pipeline().fragmentPipeline().getBlendFuncDFactor()));
        break;
    case GL_BLEND_SRC:
        params[0] = static_cast<GLfloat>(convertBlendFuncToOpenGL(RIXGL::getInstance().pipeline().fragmentPipeline().getBlendFuncSFactor()));
        break;
    case GL_BLUE_BITS:
        params[0] = 4.0f;
        break;
    case GL_CLIENT_ACTIVE_TEXTURE:
        params[0] = static_cast<GLfloat>(GL_TEXTURE0 + RIXGL::getInstance().vertexQueue().getActiveTexture());
        break;
    case GL_CLIP_PLANE0:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getPlaneClipper().getEnable()));
        break;
    case GL_COLOR_ARRAY:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().vertexArray().colorArrayEnabled()));
        break;
    case GL_COLOR_ARRAY_BUFFER_BINDING:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getColorBufferId());
        break;
    case GL_COLOR_ARRAY_SIZE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getColorSize());
        break;
    case GL_COLOR_ARRAY_STRIDE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getColorStride());
        break;
    case GL_COLOR_ARRAY_TYPE:
        params[0] = static_cast<GLfloat>(convertTypeToOpenGL(RIXGL::getInstance().vertexArray().getColorType()));
        break;
    case GL_COLOR_CLEAR_VALUE:
    {
        const auto& c = RIXGL::getInstance().pipeline().getClearColor();
        params[0] = c[0];
        params[1] = c[1];
        params[2] = c[2];
        params[3] = c[3];
        break;
    }
    case GL_COLOR_LOGIC_OP:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableLogicOp()));
        break;
    case GL_COLOR_MATERIAL:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().getColorMaterialEnabled()));
        break;
    case GL_COLOR_WRITEMASK:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().fragmentPipeline().getColorMaskR()));
        params[1] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().fragmentPipeline().getColorMaskG()));
        params[2] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().fragmentPipeline().getColorMaskB()));
        params[3] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().fragmentPipeline().getColorMaskA()));
        break;
    case GL_COMPRESSED_TEXTURE_FORMATS:
        params[0] = 0.0f;
        break;
    case GL_CULL_FACE:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getCulling().isCullingEnabled()));
        break;
    case GL_CULL_FACE_MODE:
        params[0] = static_cast<GLfloat>(convertFaceToOpenGL(RIXGL::getInstance().pipeline().getCulling().getCullMode()));
        break;
    case GL_CURRENT_COLOR:
    {
        const auto& c = RIXGL::getInstance().vertexQueue().getColor();
        params[0] = c[0];
        params[1] = c[1];
        params[2] = c[2];
        params[3] = c[3];
        break;
    }
    case GL_CURRENT_NORMAL:
    {
        const auto& n = RIXGL::getInstance().vertexQueue().getNormal();
        params[0] = n[0];
        params[1] = n[1];
        params[2] = n[2];
        break;
    }
    case GL_CURRENT_TEXTURE_COORDS:
    {
        const auto& t = RIXGL::getInstance().vertexQueue().getTexCoord();
        params[0] = t[0];
        params[1] = t[1];
        params[2] = t[2];
        params[3] = t[3];
        break;
    }
    case GL_DEPTH_BITS:
        params[0] = 16.0f;
        break;
    case GL_DEPTH_CLEAR_VALUE:
        params[0] = RIXGL::getInstance().pipeline().getClearDepth();
        break;
    case GL_DEPTH_FUNC:
        params[0] = static_cast<GLfloat>(convertTestFuncToOpenGL(RIXGL::getInstance().pipeline().fragmentPipeline().getDepthFunc()));
        break;
    case GL_DEPTH_RANGE:
        params[0] = RIXGL::getInstance().pipeline().getViewPort().getDepthRangeNear();
        params[1] = RIXGL::getInstance().pipeline().getViewPort().getDepthRangeFar();
        break;
    case GL_DEPTH_TEST:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableDepthTest()));
        break;
    case GL_DEPTH_WRITEMASK:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().fragmentPipeline().getDepthMask()));
        break;
    case GL_ELEMENT_ARRAY_BUFFER_BINDING:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getElementBufferId());
        break;
    case GL_FOG:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableFog()));
        break;
    case GL_FOG_COLOR:
    {
        const auto& c = RIXGL::getInstance().pipeline().fog().getFogColor();
        params[0] = c[0];
        params[1] = c[1];
        params[2] = c[2];
        params[3] = c[3];
        break;
    }
    case GL_FOG_DENSITY:
        params[0] = RIXGL::getInstance().pipeline().fog().getFogDensity();
        break;
    case GL_FOG_END:
        params[0] = RIXGL::getInstance().pipeline().fog().getFogEnd();
        break;
    case GL_FOG_HINT:
        params[0] = static_cast<GLfloat>(GL_DONT_CARE);
        break;
    case GL_FOG_MODE:
        params[0] = static_cast<GLfloat>(convertFogModeToOpenGL(RIXGL::getInstance().pipeline().fog().getFogMode()));
        break;
    case GL_FOG_START:
        params[0] = RIXGL::getInstance().pipeline().fog().getFogStart();
        break;
    case GL_FRONT_FACE:
        params[0] = static_cast<GLfloat>(convertFrontFaceToOpenGL(RIXGL::getInstance().pipeline().getCulling().getFrontFace()));
        break;
    case GL_GREEN_BITS:
        params[0] = 4.0f;
        break;
    case GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES:
        params[0] = static_cast<GLfloat>(GL_RGBA);
        break;
    case GL_IMPLEMENTATION_COLOR_READ_TYPE_OES:
        params[0] = static_cast<GLfloat>(GL_UNSIGNED_BYTE);
        break;
    case GL_LIGHT_MODEL_AMBIENT:
    {
        const auto& c = RIXGL::getInstance().pipeline().getLighting().getAmbientColorScene();
        params[0] = c[0];
        params[1] = c[1];
        params[2] = c[2];
        params[3] = c[3];
        break;
    }
    case GL_LIGHT_MODEL_TWO_SIDE:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().getTwoSideModelEnabled()));
        break;
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().lightEnabled(pname - GL_LIGHT0)));
        break;
    case GL_LIGHTING:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().lightingEnabled()));
        break;
    case GL_LINE_SMOOTH:
        params[0] = static_cast<GLfloat>(GL_FALSE);
        break;
    case GL_LINE_SMOOTH_HINT:
        params[0] = static_cast<GLfloat>(GL_DONT_CARE);
        break;
    case GL_LINE_WIDTH:
        params[0] = RIXGL::getInstance().pipeline().getLineAssembly().getLineWidth();
        break;
    case GL_LOGIC_OP_MODE:
        params[0] = static_cast<GLfloat>(convertLogicOpToOpenGL(RIXGL::getInstance().pipeline().fragmentPipeline().getLogicOp()));
        break;
    case GL_MATRIX_MODE:
        params[0] = static_cast<GLfloat>(convertMatrixModeToOpenGL(RIXGL::getInstance().pipeline().getMatrixStore().getMatrixMode()));
        break;
    case GL_MAX_CLIP_PLANES:
        params[0] = static_cast<GLfloat>(planeclipper::PlaneClipperSetter::getMaxClipPlanes());
        break;
    case GL_MAX_LIGHTS:
        params[0] = static_cast<GLfloat>(lighting::LightingSetter::getMaxLights());
        break;
    case GL_MAX_MODELVIEW_STACK_DEPTH:
        params[0] = static_cast<GLfloat>(matrixstore::MatrixStore::getMaxModelMatrixStackDepth());
        break;
    case GL_MAX_PROJECTION_STACK_DEPTH:
        params[0] = static_cast<GLfloat>(matrixstore::MatrixStore::getMaxProjectionMatrixStackDepth());
        break;
    case GL_MAX_TEXTURE_SIZE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().getMaxTextureSize());
        break;
    case GL_MAX_TEXTURE_STACK_DEPTH:
        params[0] = static_cast<GLfloat>(matrixstore::MatrixStore::getMaxTextureMatrixStackDepth());
        break;
    case GL_MAX_TEXTURE_UNITS:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().getMaxTmuCount());
        break;
    case GL_MAX_VIEWPORT_DIMS:
        params[0] = static_cast<GLfloat>(viewport::ViewPortSetter::getMaxViewportDimsWidth());
        params[1] = static_cast<GLfloat>(viewport::ViewPortSetter::getMaxViewportDimsHeight());
        break;
    case GL_MODELVIEW_MATRIX:
    {
        const auto& m = RIXGL::getInstance().pipeline().getMatrixStore().getModelView();
        std::memcpy(params, m.data(), 16 * sizeof(GLfloat));
        break;
    }
    case GL_MODELVIEW_STACK_DEPTH:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getMatrixStore().getModelMatrixStackDepth());
        break;
    case GL_MULTISAMPLE:
        params[0] = static_cast<GLfloat>(GL_FALSE);
        break;
    case GL_NORMAL_ARRAY:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().vertexArray().normalArrayEnabled()));
        break;
    case GL_NORMAL_ARRAY_BUFFER_BINDING:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getNormalBufferId());
        break;
    case GL_NORMAL_ARRAY_STRIDE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getNormalStride());
        break;
    case GL_NORMAL_ARRAY_TYPE:
        params[0] = static_cast<GLfloat>(convertTypeToOpenGL(RIXGL::getInstance().vertexArray().getNormalType()));
        break;
    case GL_NORMALIZE:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().getNormalNormalizationEnabled()));
        break;
    case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
        params[0] = 0.0f;
        break;
    case GL_PACK_ALIGNMENT:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().imageConverter().getPackAlignment());
        break;
    case GL_PERSPECTIVE_CORRECTION_HINT:
        params[0] = static_cast<GLfloat>(GL_DONT_CARE);
        break;
    case GL_POINT_DISTANCE_ATTENUATION:
    {
        const auto& v = RIXGL::getInstance().pipeline().getPointAssembly().getPointDistanceAttenuation();
        params[0] = v[0];
        params[1] = v[1];
        params[2] = v[2];
        break;
    }
    case GL_POINT_FADE_THRESHOLD_SIZE:
        params[0] = RIXGL::getInstance().pipeline().getPointAssembly().getPointFadeThresholdSize();
        break;
    case GL_POINT_SIZE:
        params[0] = RIXGL::getInstance().vertexArray().getPointSize();
        break;
    case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getPointSizeArrayBufferId());
        break;
    case GL_POINT_SIZE_ARRAY_OES:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().vertexArray().pointSizeArrayEnabled()));
        break;
    case GL_POINT_SIZE_ARRAY_STRIDE_OES:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getPointSizeStride());
        break;
    case GL_POINT_SIZE_ARRAY_TYPE_OES:
        params[0] = static_cast<GLfloat>(convertTypeToOpenGL(RIXGL::getInstance().vertexArray().getPointSizeType()));
        break;
    case GL_POINT_SIZE_MAX:
        params[0] = RIXGL::getInstance().pipeline().getPointAssembly().getPointSizeMax();
        break;
    case GL_POINT_SIZE_MIN:
        params[0] = RIXGL::getInstance().pipeline().getPointAssembly().getPointSizeMin();
        break;
    case GL_POINT_SMOOTH:
        params[0] = static_cast<GLfloat>(GL_FALSE);
        break;
    case GL_POINT_SMOOTH_HINT:
        params[0] = static_cast<GLfloat>(GL_DONT_CARE);
        break;
    case GL_POINT_SPRITE_OES:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getPointAssembly().getEnablePointSprite()));
        break;
    case GL_POLYGON_OFFSET_FACTOR:
        params[0] = RIXGL::getInstance().pipeline().getPolygonOffset().getFactor();
        break;
    case GL_POLYGON_OFFSET_FILL:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getPolygonOffset().getEnableFill()));
        break;
    case GL_POLYGON_OFFSET_UNITS:
        params[0] = RIXGL::getInstance().pipeline().getPolygonOffset().getUnits();
        break;
    case GL_PROJECTION_MATRIX:
    {
        const auto& m = RIXGL::getInstance().pipeline().getMatrixStore().getProjection();
        std::memcpy(params, m.data(), 16 * sizeof(GLfloat));
        break;
    }
    case GL_PROJECTION_STACK_DEPTH:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getMatrixStore().getProjectionMatrixStackDepth());
        break;
    case GL_RED_BITS:
        params[0] = 4.0f;
        break;
    case GL_RESCALE_NORMAL:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().getNormalRescaleEnabled()));
        break;
    case GL_SAMPLE_ALPHA_TO_COVERAGE:
    case GL_SAMPLE_ALPHA_TO_ONE:
    case GL_SAMPLE_COVERAGE:
    case GL_SAMPLE_COVERAGE_INVERT:
        params[0] = static_cast<GLfloat>(GL_FALSE);
        break;
    case GL_SAMPLE_BUFFERS:
    case GL_SAMPLES:
        params[0] = 0.0f;
        break;
    case GL_SAMPLE_COVERAGE_VALUE:
        params[0] = 1.0f;
        break;
    case GL_SCISSOR_BOX:
    {
        const auto& box = RIXGL::getInstance().pipeline().getScissorBox();
        params[0] = static_cast<GLfloat>(std::get<0>(box));
        params[1] = static_cast<GLfloat>(std::get<1>(box));
        params[2] = static_cast<GLfloat>(std::get<2>(box));
        params[3] = static_cast<GLfloat>(std::get<3>(box));
        break;
    }
    case GL_SCISSOR_TEST:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableScissor()));
        break;
    case GL_SHADE_MODEL:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getShadeModel().getShadeModelFlat() ? GL_FLAT : GL_SMOOTH);
        break;
    case GL_SMOOTH_LINE_WIDTH_RANGE:
        params[0] = 1.0f;
        params[1] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getLineAssembly().getMaxLineWidth());
        break;
    case GL_SMOOTH_POINT_SIZE_RANGE:
        params[0] = 1.0f;
        params[1] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getPointAssembly().getMaxPointSize());
        break;
    case GL_STENCIL_BITS:
        params[0] = static_cast<GLfloat>(rr::StencilReg::MAX_STENCIL_VAL);
        break;
    case GL_STENCIL_CLEAR_VALUE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getStencil().getClearStencil());
        break;
    case GL_STENCIL_FAIL:
        params[0] = static_cast<GLfloat>(convertStencilOpToOpenGL(RIXGL::getInstance().pipeline().getStencil().getOpFail()));
        break;
    case GL_STENCIL_FUNC:
        params[0] = static_cast<GLfloat>(convertTestFuncToOpenGL(RIXGL::getInstance().pipeline().getStencil().getTestFunc()));
        break;
    case GL_STENCIL_PASS_DEPTH_FAIL:
        params[0] = static_cast<GLfloat>(convertStencilOpToOpenGL(RIXGL::getInstance().pipeline().getStencil().getOpZFail()));
        break;
    case GL_STENCIL_PASS_DEPTH_PASS:
        params[0] = static_cast<GLfloat>(convertStencilOpToOpenGL(RIXGL::getInstance().pipeline().getStencil().getOpZPass()));
        break;
    case GL_STENCIL_REF:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getStencil().getRef());
        break;
    case GL_STENCIL_TEST:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableStencil()));
        break;
    case GL_STENCIL_VALUE_MASK:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getStencil().getMask());
        break;
    case GL_STENCIL_WRITEMASK:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getStencil().getStencilMask());
        break;
    case GL_SUBPIXEL_BITS:
        params[0] = 4.0f;
        break;
    case GL_TEXTURE_2D:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableTmu()));
        break;
    case GL_TEXTURE_BINDING_2D:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().texture().getBoundTexture());
        break;
    case GL_TEXTURE_COORD_ARRAY:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().vertexArray().texCoordArrayEnabled()));
        break;
    case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getTexCoordBufferId());
        break;
    case GL_TEXTURE_COORD_ARRAY_SIZE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getTexCoordSize());
        break;
    case GL_TEXTURE_COORD_ARRAY_STRIDE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getTexCoordStride());
        break;
    case GL_TEXTURE_COORD_ARRAY_TYPE:
        params[0] = static_cast<GLfloat>(convertTypeToOpenGL(RIXGL::getInstance().vertexArray().getTexCoordType()));
        break;
    case GL_TEXTURE_MATRIX:
    {
        const auto& m = RIXGL::getInstance().pipeline().getMatrixStore().getTexture();
        std::memcpy(params, m.data(), 16 * sizeof(GLfloat));
        break;
    }
    case GL_TEXTURE_STACK_DEPTH:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getMatrixStore().getTextureMatrixStackDepth());
        break;
    case GL_UNPACK_ALIGNMENT:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().imageConverter().getUnpackAlignment());
        break;
    case GL_VIEWPORT:
    {
        const auto& vp = RIXGL::getInstance().pipeline().getViewPort();
        params[0] = static_cast<GLfloat>(vp.getViewportX());
        params[1] = static_cast<GLfloat>(vp.getViewportY());
        params[2] = static_cast<GLfloat>(vp.getViewportWidth());
        params[3] = static_cast<GLfloat>(vp.getViewportHeight());
        break;
    }
    case GL_VERTEX_ARRAY:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().vertexArray().vertexArrayEnabled()));
        break;
    case GL_VERTEX_ARRAY_BUFFER_BINDING:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getVertexBufferId());
        break;
    case GL_VERTEX_ARRAY_SIZE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getVertexSize());
        break;
    case GL_VERTEX_ARRAY_STRIDE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getVertexStride());
        break;
    case GL_VERTEX_ARRAY_TYPE:
        params[0] = static_cast<GLfloat>(convertTypeToOpenGL(RIXGL::getInstance().vertexArray().getVertexType()));
        break;
    default:
        params[0] = 0.0f;
        break;
    }
}

GLAPI void APIENTRY impl_glGetIntegerv(GLenum pname, GLint* params)
{
    SPDLOG_DEBUG("glGetIntegerv pname 0x{:X} called", pname);

    GLfloat floatVals[4] = { 0.0f };
    SPDLOG_DEBUG("glGetIntegerv redirected to glGetFloatv", pname);
    impl_glGetFloatv(pname, floatVals);

    switch (pname)
    {
    // RGBA color components
    case GL_CURRENT_COLOR:
    case GL_COLOR_CLEAR_VALUE:
    case GL_FOG_COLOR:
    case GL_LIGHT_MODEL_AMBIENT:
    case GL_TEXTURE_ENV_COLOR:
        for (std::size_t i = 0; i < 4; i++)
            params[i] = static_cast<GLint>(floatVals[i] * std::numeric_limits<GLint>::max());
        break;

    // DepthRange value
    case GL_DEPTH_RANGE:
        params[0] = static_cast<GLint>(floatVals[0] * std::numeric_limits<GLint>::max());
        params[1] = static_cast<GLint>(floatVals[1] * std::numeric_limits<GLint>::max());
        break;

    // Depth buffer clear value
    case GL_DEPTH_CLEAR_VALUE:
        params[0] = static_cast<GLint>(floatVals[0] * std::numeric_limits<GLint>::max());
        break;

    // Normal coordinate
    case GL_CURRENT_NORMAL:
    case GL_POINT_DISTANCE_ATTENUATION:
        for (std::size_t i = 0; i < 3; i++)
            params[i] = static_cast<GLint>(floatVals[i] * std::numeric_limits<GLint>::max());
        break;

    // Boolean values
    case GL_ALPHA_TEST:
    case GL_BLEND:
    case GL_COLOR_LOGIC_OP:
    case GL_COLOR_MATERIAL:
    case GL_CULL_FACE:
    case GL_DEPTH_TEST:
    case GL_DITHER:
    case GL_FOG:
    case GL_LIGHTING:
    case GL_LINE_SMOOTH:
    case GL_MULTISAMPLE:
    case GL_NORMALIZE:
    case GL_RESCALE_NORMAL:
    case GL_SCISSOR_TEST:
    case GL_STENCIL_TEST:
    case GL_TEXTURE_2D:
        params[0] = convertBoolToGLboolean(floatVals[0] > 0.0f);
        break;

    default:
        // Default: round to nearest integer
        params[0] = static_cast<GLint>(std::lround(floatVals[0]));
        break;
    }
}

GLAPI void APIENTRY impl_glGetLightfv(GLenum light, GLenum pname, GLfloat* params)
{
    SPDLOG_DEBUG("glGetLightfv light 0x{:X} pname 0x{:X} called", light, pname);

    if (light < GL_LIGHT0 || light > GL_LIGHT7)
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    const auto& lighting = RIXGL::getInstance().pipeline().getLighting();
    const int idx = light - GL_LIGHT0;

    switch (pname)
    {
    case GL_AMBIENT:
    {
        const Vec4& v = lighting.getAmbientColorLight(idx);
        std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
    }
    break;
    case GL_DIFFUSE:
    {
        const Vec4& v = lighting.getDiffuseColorLight(idx);
        std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
    }
    break;
    case GL_SPECULAR:
    {
        const Vec4& v = lighting.getSpecularColorLight(idx);
        std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
    }
    break;
    case GL_POSITION:
    {
        const Vec4& v = lighting.getPosLight(idx);
        std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
    }
    break;
    case GL_SPOT_DIRECTION:
    {
        const Vec3& v = lighting.getSpotlightDirection(idx);
        std::memcpy(params, v.data(), 3 * sizeof(GLfloat));
    }
    break;
    case GL_SPOT_EXPONENT:
        params[0] = lighting.getSpotlightExponent(idx);
        break;
    case GL_SPOT_CUTOFF:
        params[0] = lighting.getSpotlightCutoff(idx);
        break;
    case GL_CONSTANT_ATTENUATION:
        params[0] = lighting.getConstantAttenuationLight(idx);
        break;
    case GL_LINEAR_ATTENUATION:
        params[0] = lighting.getLinearAttenuationLight(idx);
        break;
    case GL_QUADRATIC_ATTENUATION:
        params[0] = lighting.getQuadraticAttenuationLight(idx);
        break;
    default:
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI void APIENTRY impl_glGetLightiv(GLenum light, GLenum pname, GLint* params)
{
    SPDLOG_WARN("glGetLightiv not implemented");
}

GLAPI void APIENTRY impl_glGetMapdv(GLenum target, GLenum query, GLdouble* v)
{
    SPDLOG_WARN("glGetMapdv not implemented");
}

GLAPI void APIENTRY impl_glGetMapfv(GLenum target, GLenum query, GLfloat* v)
{
    SPDLOG_WARN("glGetMapfv not implemented");
}

GLAPI void APIENTRY impl_glGetMapiv(GLenum target, GLenum query, GLint* v)
{
    SPDLOG_WARN("glGetMapiv not implemented");
}

GLAPI void APIENTRY impl_glGetMaterialfv(GLenum face, GLenum pname, GLfloat* params)
{
    SPDLOG_WARN("glGetMaterialfv called for face 0x{:X} pname 0x{:X} not implemented", face, pname);

    if (face != GL_FRONT_AND_BACK)
    {
        SPDLOG_WARN("glGetMaterialfv only GL_FRONT_AND_BACK supported");
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    const auto& lighting = RIXGL::getInstance().pipeline().getLighting();

    switch (pname)
    {
    case GL_AMBIENT:
    {
        const Vec4& v = lighting.getAmbientColorMaterial();
        std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
        break;
    }
    case GL_DIFFUSE:
    {
        const Vec4& v = lighting.getDiffuseColorMaterial();
        std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
        break;
    }
    case GL_SPECULAR:
    {
        const Vec4& v = lighting.getSpecularColorMaterial();
        std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
        break;
    }
    case GL_EMISSION:
    {
        const Vec4& v = lighting.getEmissionColorMaterial();
        std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
        break;
    }
    case GL_SHININESS:
        params[0] = lighting.getSpecularExponentMaterial();
        break;

    default:
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        SPDLOG_ERROR("glGetMaterialfv pname 0x{:X} not supported", pname);
        break;
    }
}

GLAPI void APIENTRY impl_glGetMaterialiv(GLenum face, GLenum pname, GLint* params)
{
    SPDLOG_WARN("glGetMaterialiv not implemented");
}

GLAPI void APIENTRY impl_glGetPixelMapfv(GLenum map, GLfloat* values)
{
    SPDLOG_WARN("glGetPixelMapfv not implemented");
}

GLAPI void APIENTRY impl_glGetPixelMapuiv(GLenum map, GLuint* values)
{
    SPDLOG_WARN("glGetPixelMapuiv not implemented");
}

GLAPI void APIENTRY impl_glGetPixelMapusv(GLenum map, GLushort* values)
{
    SPDLOG_WARN("glGetPixelMapusv not implemented");
}

GLAPI void APIENTRY impl_glGetPolygonStipple(GLubyte* mask)
{
    SPDLOG_WARN("glGetPolygonStipple not implemented");
}

GLAPI const GLubyte* APIENTRY impl_glGetString(GLenum name)
{
    SPDLOG_DEBUG("glGetString 0x{:X} called", name);

    switch (name)
    {
    case GL_VENDOR:
        return reinterpret_cast<const GLubyte*>("ToNi3141");
    case GL_RENDERER:
        return reinterpret_cast<const GLubyte*>("RasterIX");
    case GL_VERSION:
        return reinterpret_cast<const GLubyte*>("OpenGL ES-CM 1.1");
    case GL_EXTENSIONS:
        return reinterpret_cast<const GLubyte*>(RIXGL::getInstance().getLibExtensions());
    default:
        SPDLOG_WARN("glGetString 0x{:X} not supported", name);
        break;
    }
    return nullptr;
}

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

GLAPI void APIENTRY impl_glGetTexGendv(GLenum coord, GLenum pname, GLdouble* params)
{
    SPDLOG_WARN("glGetTexGendv not implemented");
}

GLAPI void APIENTRY impl_glGetTexGenfv(GLenum coord, GLenum pname, GLfloat* params)
{
    SPDLOG_WARN("glGetTexGenfv not implemented");
}

GLAPI void APIENTRY impl_glGetTexGeniv(GLenum coord, GLenum pname, GLint* params)
{
    SPDLOG_WARN("glGetTexGeniv not implemented");
}

GLAPI void APIENTRY impl_glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels)
{
    SPDLOG_WARN("glGetTexImage not implemented");
}

GLAPI void APIENTRY impl_glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params)
{
    SPDLOG_WARN("glGetTexLevelParameterfv not implemented");
}

GLAPI void APIENTRY impl_glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params)
{
    SPDLOG_WARN("glGetTexLevelParameteriv not implemented");
}

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

GLAPI void APIENTRY impl_glHint(GLenum target, GLenum mode)
{
    SPDLOG_WARN("glHint not implemented");
}

GLAPI void APIENTRY impl_glIndexMask(GLuint mask)
{
    SPDLOG_WARN("glIndexMask not implemented");
}

GLAPI void APIENTRY impl_glIndexd(GLdouble c)
{
    SPDLOG_WARN("glIndexd not implemented");
}

GLAPI void APIENTRY impl_glIndexdv(const GLdouble* c)
{
    SPDLOG_WARN("glIndexdv not implemented");
}

GLAPI void APIENTRY impl_glIndexf(GLfloat c)
{
    SPDLOG_WARN("glIndexf not implemented");
}

GLAPI void APIENTRY impl_glIndexfv(const GLfloat* c)
{
    SPDLOG_WARN("glIndexfv not implemented");
}

GLAPI void APIENTRY impl_glIndexi(GLint c)
{
    SPDLOG_WARN("glIndexi not implemented");
}

GLAPI void APIENTRY impl_glIndexiv(const GLint* c)
{
    SPDLOG_WARN("glIndexiv not implemented");
}

GLAPI void APIENTRY impl_glIndexs(GLshort c)
{
    SPDLOG_WARN("glIndexs not implemented");
}

GLAPI void APIENTRY impl_glIndexsv(const GLshort* c)
{
    SPDLOG_WARN("glIndexsv not implemented");
}

GLAPI void APIENTRY impl_glInitNames(void)
{
    SPDLOG_WARN("glInitNames not implemented");
}

GLAPI GLboolean APIENTRY impl_glIsEnabled(GLenum cap)
{
    switch (cap)
    {
    case GL_TEXTURE_2D:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableTmu());
    case GL_ALPHA_TEST:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableAlphaTest());
    case GL_DEPTH_TEST:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableDepthTest());
    case GL_BLEND:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableBlending());
    case GL_LIGHTING:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().lightingEnabled());
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().lightEnabled(cap - GL_LIGHT0));
    case GL_TEXTURE_GEN_S:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getTexGen().getTexGenEnableS());
    case GL_TEXTURE_GEN_T:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getTexGen().getTexGenEnableT());
    case GL_TEXTURE_GEN_R:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getTexGen().getTexGenEnableR());
    case GL_CULL_FACE:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getCulling().isCullingEnabled());
    case GL_COLOR_MATERIAL:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().getColorMaterialEnabled());
    case GL_FOG:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableFog());
    case GL_SCISSOR_TEST:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableScissor());
    case GL_NORMALIZE:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().getNormalNormalizationEnabled());
    case GL_STENCIL_TEST:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableStencil());
    case GL_STENCIL_TEST_TWO_SIDE_EXT:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getStencil().getTwoSideStencilEnabled());
    case GL_COLOR_LOGIC_OP:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableLogicOp());
    case GL_CLIP_PLANE0:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getPlaneClipper().getEnable());
    case GL_CLIP_PLANE1:
    case GL_CLIP_PLANE2:
    case GL_CLIP_PLANE3:
    case GL_CLIP_PLANE4:
    case GL_CLIP_PLANE5:
        return GL_FALSE; // Not supported
    case GL_POLYGON_OFFSET_FILL:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getPolygonOffset().getEnableFill());
    case GL_POINT_SPRITE_OES:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getPointAssembly().getEnablePointSprite());
    case GL_DITHER:
        // Dither is always enabled in RIXGL, but has no effect
        return GL_TRUE;
    case GL_LINE_SMOOTH:
    case GL_MULTISAMPLE:
    case GL_POINT_SMOOTH:
    case GL_SAMPLE_ALPHA_TO_COVERAGE:
    case GL_SAMPLE_ALPHA_TO_ONE:
    case GL_SAMPLE_COVERAGE:
        // These are not implemented, always return GL_FALSE
        return GL_FALSE;
    case GL_RESCALE_NORMAL:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().getNormalRescaleEnabled());
    case GL_COLOR_ARRAY:
        return convertBoolToGLboolean(RIXGL::getInstance().vertexArray().colorArrayEnabled());
    case GL_NORMAL_ARRAY:
        return convertBoolToGLboolean(RIXGL::getInstance().vertexArray().normalArrayEnabled());
    case GL_TEXTURE_COORD_ARRAY:
        return convertBoolToGLboolean(RIXGL::getInstance().vertexArray().texCoordArrayEnabled());
    case GL_VERTEX_ARRAY:
        return convertBoolToGLboolean(RIXGL::getInstance().vertexArray().vertexArrayEnabled());
    case GL_POINT_SIZE_ARRAY_OES:
        return convertBoolToGLboolean(RIXGL::getInstance().vertexArray().pointSizeArrayEnabled());
    default:
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return GL_FALSE;
    }
}

GLAPI GLboolean APIENTRY impl_glIsList(GLuint list)
{
    SPDLOG_WARN("glIsList not implemented");
    return false;
}

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

GLAPI void APIENTRY impl_glLightf(GLenum light, GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glLightf light 0x{:X} pname 0x{:X} param {} called", light - GL_LIGHT0, pname, param);

    if (light > GL_LIGHT7)
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
    }

    switch (pname)
    {
    case GL_SPOT_EXPONENT:
        RIXGL::getInstance().pipeline().getLighting().setSpotlightExponent(light - GL_LIGHT0, param);
        break;
    case GL_SPOT_CUTOFF:
        RIXGL::getInstance().pipeline().getLighting().setSpotlightCutoff(light - GL_LIGHT0, param);
        break;
    case GL_CONSTANT_ATTENUATION:
        RIXGL::getInstance().pipeline().getLighting().setConstantAttenuationLight(light - GL_LIGHT0, param);
        break;
    case GL_LINEAR_ATTENUATION:
        RIXGL::getInstance().pipeline().getLighting().setLinearAttenuationLight(light - GL_LIGHT0, param);
        break;
    case GL_QUADRATIC_ATTENUATION:
        RIXGL::getInstance().pipeline().getLighting().setQuadraticAttenuationLight(light - GL_LIGHT0, param);
        break;
    default:
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI void APIENTRY impl_glLightfv(GLenum light, GLenum pname, const GLfloat* params)
{
    SPDLOG_DEBUG("glLightfv light 0x{:X} pname 0x{:X}", light - GL_LIGHT0, pname);

    if (light > GL_LIGHT7)
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    switch (pname)
    {
    case GL_AMBIENT:
        RIXGL::getInstance().pipeline().getLighting().setAmbientColorLight(light - GL_LIGHT0, { params });
        break;
    case GL_DIFFUSE:
        RIXGL::getInstance().pipeline().getLighting().setDiffuseColorLight(light - GL_LIGHT0, { params });
        break;
    case GL_SPECULAR:
        RIXGL::getInstance().pipeline().getLighting().setSpecularColorLight(light - GL_LIGHT0, { params });
        break;
    case GL_POSITION:
    {
        Vec4 lightPos { params };
        Vec4 lightPosTransformed {};
        RIXGL::getInstance().pipeline().getMatrixStore().getModelView().transform(lightPosTransformed, lightPos);
        RIXGL::getInstance().pipeline().getLighting().setPosLight(light - GL_LIGHT0, lightPosTransformed);
        break;
    }
    case GL_SPOT_DIRECTION:
        RIXGL::getInstance().pipeline().getLighting().setSpotlightDirection(light - GL_LIGHT0, { params });
        break;
    default:
        SPDLOG_DEBUG("glLightfv redirected to glLightf");
        impl_glLightf(light, pname, params[0]);
        break;
    }
}

GLAPI void APIENTRY impl_glLighti(GLenum light, GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glLighti redirected to glLightf");
    impl_glLightf(light, pname, static_cast<float>(param));
}

GLAPI void APIENTRY impl_glLightiv(GLenum light, GLenum pname, const GLint* params)
{
    SPDLOG_DEBUG("glLightiv redirected to glLightfv");
    Vec4 color = Vec4::createFromArray<GLint, 4>(params);
    color.div(255);
    impl_glLightfv(light, pname, color.data());
}

GLAPI void APIENTRY impl_glLineStipple(GLint factor, GLushort pattern)
{
    SPDLOG_WARN("glLineStipple not implemented");
}

GLAPI void APIENTRY impl_glLineWidth(GLfloat width)
{
    SPDLOG_DEBUG("glLineWidth {} called", width);
    if (width <= 0.0f)
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
    }
    RIXGL::getInstance().pipeline().getLineAssembly().setLineWidth(width);
}

GLAPI void APIENTRY impl_glListBase(GLuint base)
{
    SPDLOG_WARN("glListBase not implemented");
}

GLAPI void APIENTRY impl_glLoadIdentity(void)
{
    SPDLOG_DEBUG("glLoadIdentity called");
    RIXGL::getInstance().pipeline().getMatrixStore().getCurrentMatrix().identity();
}

GLAPI void APIENTRY impl_glLoadMatrixd(const GLdouble* m)
{
    SPDLOG_WARN("glLoadMatrixd not implemented");
}

GLAPI void APIENTRY impl_glLoadMatrixf(const GLfloat* m)
{
    SPDLOG_DEBUG("glLoadMatrixf called");
    if (m == nullptr)
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }
    RIXGL::getInstance().pipeline().getMatrixStore().getCurrentMatrix() = *reinterpret_cast<const Mat44*>(m);
}

GLAPI void APIENTRY impl_glLoadName(GLuint name)
{
    SPDLOG_WARN("glLoadName not implemented");
}

GLAPI void APIENTRY impl_glLogicOp(GLenum opcode)
{
    SPDLOG_DEBUG("glLogicOp 0x{:X} called", opcode);

    LogicOp logicOp;
    const GLenum error = convertLogicOp(logicOp, opcode);
    if (error == GL_NO_ERROR)
    {
        RIXGL::getInstance().pipeline().fragmentPipeline().setLogicOp(logicOp);
    }
    else
    {
        RIXGL::getInstance().setError(error);
    }
}

GLAPI void APIENTRY impl_glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble* points)
{
    SPDLOG_WARN("glMap1d not implemented");
}

GLAPI void APIENTRY impl_glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat* points)
{
    SPDLOG_WARN("glMap1f not implemented");
}

GLAPI void APIENTRY impl_glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble* points)
{
    SPDLOG_WARN("glMap2d not implemented");
}

GLAPI void APIENTRY impl_glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat* points)
{
    SPDLOG_WARN("glMap2f not implemented");
}

GLAPI void APIENTRY impl_glMapGrid1d(GLint un, GLdouble u1, GLdouble u2)
{
    SPDLOG_WARN("glMapGrid1d not implemented");
}

GLAPI void APIENTRY impl_glMapGrid1f(GLint un, GLfloat u1, GLfloat u2)
{
    SPDLOG_WARN("glMapGrid1f not implemented");
}

GLAPI void APIENTRY impl_glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
    SPDLOG_WARN("glMapGrid2d not implemented");
}

GLAPI void APIENTRY impl_glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
    SPDLOG_WARN("glMapGrid2f not implemented");
}

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

GLAPI void APIENTRY impl_glMatrixMode(GLenum mode)
{
    SPDLOG_DEBUG("glMatrixMode 0x{:X} called", mode);

    MatrixMode matrixMode {};
    const GLenum ret = convertMatrixMode(matrixMode, mode);
    if (ret == GL_NO_ERROR)
    {
        RIXGL::getInstance().pipeline().getMatrixStore().setMatrixMode(matrixMode);
    }
    else
    {
        RIXGL::getInstance().setError(ret);
    }
}

GLAPI void APIENTRY impl_glMultMatrixd(const GLdouble* m)
{
    SPDLOG_DEBUG("glMultMatrixd redirected to glMultMatrixf");
    GLfloat mf[16];
    for (std::size_t i = 0; i < 4 * 4; i++)
    {
        mf[i] = m[i];
    }
    impl_glMultMatrixf(mf);
}

GLAPI void APIENTRY impl_glMultMatrixf(const GLfloat* m)
{
    SPDLOG_DEBUG("glMultMatrixf called");
    const Mat44* m44 = reinterpret_cast<const Mat44*>(m);
    RIXGL::getInstance().pipeline().getMatrixStore().getCurrentMatrix().multiplyLhs(*m44);
}

GLAPI void APIENTRY impl_glNewList(GLuint list, GLenum mode)
{
    SPDLOG_WARN("glNewList not implemented");
}

GLAPI void APIENTRY impl_glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz)
{
    SPDLOG_DEBUG("glNormal3b ({}, {}, {}) called",
        static_cast<float>(nx),
        static_cast<float>(ny),
        static_cast<float>(nz));
    const Vec3 normal {
        static_cast<float>(nx),
        static_cast<float>(ny),
        static_cast<float>(nz)
    };
    RIXGL::getInstance().vertexQueue().setNormal(normal);
    RIXGL::getInstance().vertexArray().setNormal(normal);
}

GLAPI void APIENTRY impl_glNormal3bv(const GLbyte* v)
{
    SPDLOG_DEBUG("glNormal3bv ({}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]));
    const Vec3 normal {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2])
    };
    RIXGL::getInstance().vertexQueue().setNormal(normal);
    RIXGL::getInstance().vertexArray().setNormal(normal);
}

GLAPI void APIENTRY impl_glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz)
{
    SPDLOG_DEBUG("glNormal3d ({}, {}, {}) called",
        static_cast<float>(nx),
        static_cast<float>(ny),
        static_cast<float>(nz));
    const Vec3 normal {
        static_cast<float>(nx),
        static_cast<float>(ny),
        static_cast<float>(nz)
    };
    RIXGL::getInstance().vertexQueue().setNormal(normal);
    RIXGL::getInstance().vertexArray().setNormal(normal);
}

GLAPI void APIENTRY impl_glNormal3dv(const GLdouble* v)
{
    SPDLOG_DEBUG("glNormal3dv ({}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]));
    const Vec3 normal {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2])
    };
    RIXGL::getInstance().vertexQueue().setNormal(normal);
    RIXGL::getInstance().vertexArray().setNormal(normal);
}

GLAPI void APIENTRY impl_glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    SPDLOG_DEBUG("glNormal3f ({}, {}, {}) called", nx, ny, nz);
    const Vec3 normal { nx, ny, nz };
    RIXGL::getInstance().vertexQueue().setNormal(normal);
    RIXGL::getInstance().vertexArray().setNormal(normal);
}

GLAPI void APIENTRY impl_glNormal3fv(const GLfloat* v)
{
    SPDLOG_DEBUG("glNormal3f ({}, {}, {}) called", v[0], v[1], v[2]);
    const Vec3 normal { v[0], v[1], v[2] };
    RIXGL::getInstance().vertexQueue().setNormal(normal);
    RIXGL::getInstance().vertexArray().setNormal(normal);
}

GLAPI void APIENTRY impl_glNormal3i(GLint nx, GLint ny, GLint nz)
{
    SPDLOG_DEBUG("glNormal3i ({}, {}, {}) called",
        static_cast<float>(nx),
        static_cast<float>(ny),
        static_cast<float>(nz));
    const Vec3 normal {
        static_cast<float>(nx),
        static_cast<float>(ny),
        static_cast<float>(nz)
    };
    RIXGL::getInstance().vertexQueue().setNormal(normal);
    RIXGL::getInstance().vertexArray().setNormal(normal);
}

GLAPI void APIENTRY impl_glNormal3iv(const GLint* v)
{
    SPDLOG_DEBUG("glNormal3iv ({}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]));
    const Vec3 normal {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2])
    };
    RIXGL::getInstance().vertexQueue().setNormal(normal);
    RIXGL::getInstance().vertexArray().setNormal(normal);
}

GLAPI void APIENTRY impl_glNormal3s(GLshort nx, GLshort ny, GLshort nz)
{
    SPDLOG_DEBUG("glNormal3s ({}, {}, {}) called",
        static_cast<float>(nx),
        static_cast<float>(ny),
        static_cast<float>(nz));
    const Vec3 normal {
        static_cast<float>(nx),
        static_cast<float>(ny),
        static_cast<float>(nz)
    };
    RIXGL::getInstance().vertexQueue().setNormal(normal);
    RIXGL::getInstance().vertexArray().setNormal(normal);
}

GLAPI void APIENTRY impl_glNormal3sv(const GLshort* v)
{
    SPDLOG_DEBUG("glNormal3sv ({}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]));
    const Vec3 normal {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2])
    };
    RIXGL::getInstance().vertexQueue().setNormal(normal);
    RIXGL::getInstance().vertexArray().setNormal(normal);
}

GLAPI void APIENTRY impl_glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
    SPDLOG_DEBUG("glOrthof left {} right {} bottom {} top {} zNear {} zFar {} called", left, right, bottom, top, zNear, zFar);

    if ((zNear == zFar) || (left == right) || (top == bottom))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glOrthof called with invalid values");
        return;
    }

    RIXGL::getInstance().pipeline().getMatrixStore().getCurrentMatrix().ortho(left, right, bottom, top, zNear, zFar);
}

GLAPI void APIENTRY impl_glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    SPDLOG_DEBUG("glOrtho redirected to glOrthof");
    impl_glOrthof(static_cast<float>(left),
        static_cast<float>(right),
        static_cast<float>(bottom),
        static_cast<float>(top),
        static_cast<float>(zNear),
        static_cast<float>(zFar));
}

GLAPI void APIENTRY impl_glPassThrough(GLfloat token)
{
    SPDLOG_WARN("glPassThrough not implemented");
}

GLAPI void APIENTRY impl_glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat* values)
{
    SPDLOG_WARN("glPixelMapfv not implemented");
}

GLAPI void APIENTRY impl_glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint* values)
{
    SPDLOG_WARN("glPixelMapuiv not implemented");
}

GLAPI void APIENTRY impl_glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort* values)
{
    SPDLOG_WARN("glPixelMapusv not implemented");
}

GLAPI void APIENTRY impl_glPixelStoref(GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glPixelStoref redirected to glPixelStorei");
    impl_glPixelStorei(pname, static_cast<GLint>(param));
}

GLAPI void APIENTRY impl_glPixelStorei(GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glPixelStorei pname 0x{:X} param 0x{:X} called", pname, param);
    switch (param)
    {
    case 1:
    case 2:
    case 4:
    case 8:
        if (pname == GL_UNPACK_ALIGNMENT)
        {
            RIXGL::getInstance().imageConverter().setUnpackAlignment(param);
        }
        else if (pname == GL_PACK_ALIGNMENT)
        {
            RIXGL::getInstance().imageConverter().setPackAlignment(param);
        }
        else
        {
            SPDLOG_WARN("glPixelStorei pname 0x{:X} and param 0x{:X} not supported", pname, param);
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
        }
        break;
    default:
        SPDLOG_ERROR("glPixelStorei pname GL_PACK_ALIGNMENT and param 0x{:X} not supported", param);
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        break;
    }
}

GLAPI void APIENTRY impl_glPixelTransferf(GLenum pname, GLfloat param)
{
    SPDLOG_WARN("glPixelTransferf not implemented");
}

GLAPI void APIENTRY impl_glPixelTransferi(GLenum pname, GLint param)
{
    SPDLOG_WARN("glPixelTransferi not implemented");
}

GLAPI void APIENTRY impl_glPixelZoom(GLfloat xfactor, GLfloat yfactor)
{
    SPDLOG_WARN("glPixelZoom not implemented");
}

GLAPI void APIENTRY impl_glPointSize(GLfloat size)
{
    SPDLOG_DEBUG("glPointSize {} called", size);
    if (size <= 0.0f)
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }
    RIXGL::getInstance().vertexArray().setPointSize(size);
}

GLAPI void APIENTRY impl_glPointParameterf(GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glPointParameterf pname 0x{:X} param {} called", pname, param);

    auto& setter = RIXGL::getInstance().pipeline().getPointAssembly();
    switch (pname)
    {
    case GL_POINT_SIZE_MIN:
        if (param < 0.0f)
        {
            SPDLOG_ERROR("glPointParameterf GL_POINT_SIZE_MIN < 0");
            RIXGL::getInstance().setError(GL_INVALID_VALUE);
        }
        else
        {
            setter.setPointSizeMin(param);
        }
        break;
    case GL_POINT_SIZE_MAX:
        if (param < 0.0f)
        {
            SPDLOG_ERROR("glPointParameterf GL_POINT_SIZE_MAX < 0");
            RIXGL::getInstance().setError(GL_INVALID_VALUE);
        }
        else
        {
            setter.setPointSizeMax(param);
        }
        break;
    case GL_POINT_FADE_THRESHOLD_SIZE:
        if (param < 0.0f)
        {
            SPDLOG_ERROR("glPointParameterf GL_POINT_FADE_THRESHOLD_SIZE < 0");
            RIXGL::getInstance().setError(GL_INVALID_VALUE);
        }
        else
        {
            setter.setPointFadeThresholdSize(param);
        }
        break;
    case GL_POINT_DISTANCE_ATTENUATION:
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glPointParameterf: GL_POINT_DISTANCE_ATTENUATION not allowed here.");
        break;
    default:
        SPDLOG_WARN("glPointParameterf: Unknown pname 0x{:X}", pname);
        break;
    }
}

GLAPI void APIENTRY impl_glPointParameterfv(GLenum pname, const GLfloat* params)
{
    SPDLOG_DEBUG("glPointParameterfv pname 0x{:X} and params[0] {} called", pname, params[0]);

    auto& setter = RIXGL::getInstance().pipeline().getPointAssembly();
    switch (pname)
    {
    case GL_POINT_DISTANCE_ATTENUATION:
        setter.setPointDistanceAttenuation(Vec3 { params[0], params[1], params[2] });
        break;
    case GL_POINT_SIZE_MIN:
    case GL_POINT_SIZE_MAX:
    case GL_POINT_FADE_THRESHOLD_SIZE:
        SPDLOG_DEBUG("glPointParameterfv pname 0x{:X} redirected to glPointParameterf", pname);
        impl_glPointParameterf(pname, params[0]);
        break;
    default:
        SPDLOG_WARN("glPointParameterfv: Unknown pname 0x{:X}", pname);
        break;
    }
}

GLAPI void APIENTRY impl_glPolygonMode(GLenum face, GLenum mode)
{
    SPDLOG_WARN("glPolygonMode not implemented");
}

GLAPI void APIENTRY impl_glPolygonStipple(const GLubyte* mask)
{
    SPDLOG_WARN("glPolygonStipple not implemented");
}

GLAPI void APIENTRY impl_glPopAttrib(void)
{
    SPDLOG_WARN("glPopAttrib not implemented");
}

GLAPI void APIENTRY impl_glPopMatrix(void)
{
    SPDLOG_DEBUG("glPopMatrix called");
    if (!RIXGL::getInstance().pipeline().getMatrixStore().popMatrix())
    {
        RIXGL::getInstance().setError(GL_STACK_OVERFLOW);
    }
}

GLAPI void APIENTRY impl_glPopName(void)
{
    SPDLOG_WARN("glPopName not implemented");
}

GLAPI void APIENTRY impl_glPushAttrib(GLbitfield mask)
{
    SPDLOG_WARN("glPushAttrib not implemented");
}

GLAPI void APIENTRY impl_glPushMatrix(void)
{
    SPDLOG_DEBUG("glPushMatrix called");

    if (!RIXGL::getInstance().pipeline().getMatrixStore().pushMatrix())
    {
        RIXGL::getInstance().setError(GL_STACK_OVERFLOW);
    }
}

GLAPI void APIENTRY impl_glPushName(GLuint name)
{
    SPDLOG_WARN("glPushName not implemented");
}

GLAPI void APIENTRY impl_glRasterPos2d(GLdouble x, GLdouble y)
{
    SPDLOG_WARN("glRasterPos2d not implemented");
}

GLAPI void APIENTRY impl_glRasterPos2dv(const GLdouble* v)
{
    SPDLOG_WARN("glRasterPos2dv not implemented");
}

GLAPI void APIENTRY impl_glRasterPos2f(GLfloat x, GLfloat y)
{
    SPDLOG_WARN("glRasterPos2f not implemented");
}

GLAPI void APIENTRY impl_glRasterPos2fv(const GLfloat* v)
{
    SPDLOG_WARN("glRasterPos2fv not implemented");
}

GLAPI void APIENTRY impl_glRasterPos2i(GLint x, GLint y)
{
    SPDLOG_WARN("glRasterPos2i not implemented");
}

GLAPI void APIENTRY impl_glRasterPos2iv(const GLint* v)
{
    SPDLOG_WARN("glRasterPos2iv not implemented");
}

GLAPI void APIENTRY impl_glRasterPos2s(GLshort x, GLshort y)
{
    SPDLOG_WARN("glRasterPos2s not implemented");
}

GLAPI void APIENTRY impl_glRasterPos2sv(const GLshort* v)
{
    SPDLOG_WARN("glRasterPos2sv not implemented");
}

GLAPI void APIENTRY impl_glRasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
    SPDLOG_WARN("glRasterPos3d not implemented");
}

GLAPI void APIENTRY impl_glRasterPos3dv(const GLdouble* v)
{
    SPDLOG_WARN("glRasterPos3dv not implemented");
}

GLAPI void APIENTRY impl_glRasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
    SPDLOG_WARN("glRasterPos3f not implemented");
}

GLAPI void APIENTRY impl_glRasterPos3fv(const GLfloat* v)
{
    SPDLOG_WARN("glRasterPos3fv not implemented");
}

GLAPI void APIENTRY impl_glRasterPos3i(GLint x, GLint y, GLint z)
{
    SPDLOG_WARN("glRasterPos3i not implemented");
}

GLAPI void APIENTRY impl_glRasterPos3iv(const GLint* v)
{
    SPDLOG_WARN("glRasterPos3iv not implemented");
}

GLAPI void APIENTRY impl_glRasterPos3s(GLshort x, GLshort y, GLshort z)
{
    SPDLOG_WARN("glRasterPos3s not implemented");
}

GLAPI void APIENTRY impl_glRasterPos3sv(const GLshort* v)
{
    SPDLOG_WARN("glRasterPos3sv not implemented");
}

GLAPI void APIENTRY impl_glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    SPDLOG_WARN("glRasterPos4d not implemented");
}

GLAPI void APIENTRY impl_glRasterPos4dv(const GLdouble* v)
{
    SPDLOG_WARN("glRasterPos4dv not implemented");
}

GLAPI void APIENTRY impl_glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    SPDLOG_WARN("glRasterPos4f not implemented");
}

GLAPI void APIENTRY impl_glRasterPos4fv(const GLfloat* v)
{
    SPDLOG_WARN("glRasterPos4fv not implemented");
}

GLAPI void APIENTRY impl_glRasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
    SPDLOG_WARN("glRasterPos4i not implemented");
}

GLAPI void APIENTRY impl_glRasterPos4iv(const GLint* v)
{
    SPDLOG_WARN("glRasterPos4iv not implemented");
}

GLAPI void APIENTRY impl_glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    SPDLOG_WARN("glRasterPos4s not implemented");
}

GLAPI void APIENTRY impl_glRasterPos4sv(const GLshort* v)
{
    SPDLOG_WARN("glRasterPos4sv not implemented");
}

GLAPI void APIENTRY impl_glReadBuffer(GLenum mode)
{
    SPDLOG_WARN("glReadBuffer not implemented");
}

GLAPI void APIENTRY impl_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
    SPDLOG_DEBUG("glReadPixels x {} y {} width {} height {} format 0x{:X} type {:X} called",
        x, y, width, height, format, type);

    if (format != GL_RGBA)
    {
        SPDLOG_WARN("glReadPixels format not supported");
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }
    if (type != GL_UNSIGNED_BYTE)
    {
        SPDLOG_WARN("glReadPixels type not supported");
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }
    if (!((format == GL_RGBA) && (type == GL_UNSIGNED_BYTE)))
    {
        SPDLOG_WARN("glReadPixels only GL_RGBA/GL_UNSIGNED_BYTE is supported");
        RIXGL::getInstance().setError(GL_INVALID_OPERATION);
        return;
    }
    const uint16_t* pixelsDevice = readFromColorBuffer(x, y, width, height, false);
    RIXGL::getInstance().imageConverter().convertRGB565ToPackedRGBA8888(reinterpret_cast<uint8_t*>(pixels), width, height, pixelsDevice);
    delete pixelsDevice;
}

GLAPI void APIENTRY impl_glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
    SPDLOG_DEBUG("glRectd redirected to glRectf");
    impl_glRectf(static_cast<float>(x1), static_cast<float>(y1), static_cast<float>(x2), static_cast<float>(y2));
}

GLAPI void APIENTRY impl_glRectdv(const GLdouble* v1, const GLdouble* v2)
{
    SPDLOG_WARN("glRectdv not implemented");
}

GLAPI void APIENTRY impl_glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    SPDLOG_DEBUG("glRectf constructed with glBegin glVertex2f glEnd");

    impl_glBegin(GL_TRIANGLE_FAN); // The spec says, use GL_POLYGON. GL_POLYGON is not implemented. GL_TRIANGLE_FAN should also do the trick.
    impl_glVertex2f(x1, y1);
    impl_glVertex2f(x2, y1);
    impl_glVertex2f(x2, y2);
    impl_glVertex2f(x1, y2);
    impl_glEnd();
}

GLAPI void APIENTRY impl_glRectfv(const GLfloat* v1, const GLfloat* v2)
{
    SPDLOG_WARN("glRectfv not implemented");
}

GLAPI void APIENTRY impl_glRecti(GLint x1, GLint y1, GLint x2, GLint y2)
{
    SPDLOG_WARN("glRecti not implemented");
}

GLAPI void APIENTRY impl_glRectiv(const GLint* v1, const GLint* v2)
{
    SPDLOG_WARN("glRectiv not implemented");
}

GLAPI void APIENTRY impl_glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
    SPDLOG_WARN("glRects not implemented");
}

GLAPI void APIENTRY impl_glRectsv(const GLshort* v1, const GLshort* v2)
{
    SPDLOG_WARN("glRectsv not implemented");
}

GLAPI GLint APIENTRY impl_glRenderMode(GLenum mode)
{
    SPDLOG_WARN("glRenderMode not implemented");
    return 0;
}

GLAPI void APIENTRY impl_glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    SPDLOG_DEBUG("glRotated ({}, {}, {}, {}) called",
        static_cast<float>(angle),
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z));
    RIXGL::getInstance().pipeline().getMatrixStore().getCurrentMatrix().rotate(static_cast<float>(angle),
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z));
}

GLAPI void APIENTRY impl_glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    SPDLOG_DEBUG("glRotatef ({}, {}, {}, {}) called", angle, x, y, z);
    RIXGL::getInstance().pipeline().getMatrixStore().getCurrentMatrix().rotate(angle, x, y, z);
}

GLAPI void APIENTRY impl_glScaled(GLdouble x, GLdouble y, GLdouble z)
{
    SPDLOG_DEBUG("glScaled ({}, {}, {}) called",
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z));
    RIXGL::getInstance().pipeline().getMatrixStore().getCurrentMatrix().scale(static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z));
}

GLAPI void APIENTRY impl_glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    SPDLOG_DEBUG("glScalef ({}, {}, {}) called", x, y, z);
    RIXGL::getInstance().pipeline().getMatrixStore().getCurrentMatrix().scale(x, y, z);
}

GLAPI void APIENTRY impl_glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    SPDLOG_DEBUG("glScissor x {} y {} width {} height {} called", x, y, width, height);
    if ((width < 0) || (height < 0))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
    }
    else
    {
        RIXGL::getInstance().pipeline().setScissorBox(x, y, width, height);
    }
}

GLAPI void APIENTRY impl_glSelectBuffer(GLsizei size, GLuint* buffer)
{
    SPDLOG_WARN("glSelectBuffer not implemented");
}

GLAPI void APIENTRY impl_glShadeModel(GLenum mode)
{
    SPDLOG_DEBUG("glShadeModel 0x{:X} called", mode);
    if (mode == GL_FLAT)
    {
        RIXGL::getInstance().pipeline().getShadeModel().setShadeModelFlat(true);
    }
    else if (mode == GL_SMOOTH)
    {
        RIXGL::getInstance().pipeline().getShadeModel().setShadeModelFlat(false);
    }
    else
    {
        SPDLOG_ERROR("glShadeModel unknown mode received.");
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
    }
}

GLAPI void APIENTRY impl_glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
    SPDLOG_DEBUG("glStencilFunc func 0x{:X} ref 0x{:X} mask 0x{:X} called", func, ref, mask);

    TestFunc testFunc;
    const GLenum error = convertTestFunc(testFunc, func);

    if (error == GL_NO_ERROR)
    {
        RIXGL::getInstance().pipeline().getStencil().setTestFunc(testFunc);
        RIXGL::getInstance().pipeline().getStencil().setRef(ref);
        RIXGL::getInstance().pipeline().getStencil().setMask(mask);
    }
    else
    {
        RIXGL::getInstance().setError(error);
    }
}

GLAPI void APIENTRY impl_glStencilMask(GLuint mask)
{
    SPDLOG_DEBUG("glStencilMask 0x{:X} called", mask);
    RIXGL::getInstance().pipeline().getStencil().setStencilMask(mask);
}

GLAPI void APIENTRY impl_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    SPDLOG_DEBUG("glStencilOp fail 0x{:X} zfail 0x{:X} zpass 0x{:X} called", fail, zfail, zpass);

    StencilOp failOp;
    StencilOp zfailOp;
    StencilOp zpassOp;

    const GLenum error0 = convertStencilOp(failOp, fail);
    const GLenum error1 = convertStencilOp(zfailOp, zfail);
    const GLenum error2 = convertStencilOp(zpassOp, zpass);

    if ((error0 == GL_NO_ERROR) && (error1 == GL_NO_ERROR) == (error2 == GL_NO_ERROR))
    {
        RIXGL::getInstance().pipeline().getStencil().setOpFail(failOp);
        RIXGL::getInstance().pipeline().getStencil().setOpZFail(zfailOp);
        RIXGL::getInstance().pipeline().getStencil().setOpZPass(zpassOp);
    }
    else
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
    }
}

GLAPI void APIENTRY impl_glTexCoord1d(GLdouble s)
{
    SPDLOG_DEBUG("glTexCoord1d ({}) called", static_cast<float>(s));
    const Vec4 tex { static_cast<float>(s), 0.0f, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord1dv(const GLdouble* v)
{
    SPDLOG_DEBUG("glTexCoord1dv ({}) called", static_cast<float>(v[0]));
    const Vec4 tex { static_cast<float>(v[0]), 0.0f, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord1f(GLfloat s)
{
    SPDLOG_DEBUG("glTexCoord1f ({}) called", s);
    const Vec4 tex { s, 0.0f, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord1fv(const GLfloat* v)
{
    SPDLOG_DEBUG("glTexCoord1fv ({}) called", v[0]);
    const Vec4 tex { v[0], 0.0f, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord1i(GLint s)
{
    SPDLOG_DEBUG("glTexCoord1i ({}) called", static_cast<float>(s));
    const Vec4 tex { static_cast<float>(s), 0.0f, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord1iv(const GLint* v)
{
    SPDLOG_DEBUG("glTexCoord1iv ({}) called", static_cast<float>(v[0]));
    const Vec4 tex { static_cast<float>(v[0]), 0.0f, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord1s(GLshort s)
{
    SPDLOG_DEBUG("glTexCoord1s ({}) called", static_cast<float>(s));
    const Vec4 tex { static_cast<float>(s), 0.0f, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord1sv(const GLshort* v)
{
    SPDLOG_DEBUG("glTexCoord1sv ({}) called", static_cast<float>(v[0]));
    const Vec4 tex { static_cast<float>(v[0]), 0.0f, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord2d(GLdouble s, GLdouble t)
{
    SPDLOG_DEBUG("glTexCoord2d ({}, {}) called",
        static_cast<float>(s),
        static_cast<float>(t));
    const Vec4 tex { static_cast<float>(s), static_cast<float>(t), 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord2dv(const GLdouble* v)
{
    SPDLOG_DEBUG("glTexCoord2dv ({}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]));
    const Vec4 tex { static_cast<float>(v[0]), static_cast<float>(v[1]), 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord2f(GLfloat s, GLfloat t)
{
    SPDLOG_DEBUG("glTexCoord2f ({}, {}) called", s, t);
    const Vec4 tex { s, t, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord2fv(const GLfloat* v)
{
    SPDLOG_DEBUG("glTexCoord2fv ({}, {}) called", v[0], v[1]);
    const Vec4 tex { v[0], v[1], 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord2i(GLint s, GLint t)
{
    SPDLOG_DEBUG("glTexCoord2i ({}, {}) called",
        static_cast<float>(s),
        static_cast<float>(t));
    const Vec4 tex { static_cast<float>(s), static_cast<float>(t), 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord2iv(const GLint* v)
{
    SPDLOG_DEBUG("glTexCoord2iv ({}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]));
    const Vec4 tex { static_cast<float>(v[0]), static_cast<float>(v[1]), 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord2s(GLshort s, GLshort t)
{
    SPDLOG_DEBUG("glTexCoord2s ({}, {}) called",
        static_cast<float>(s),
        static_cast<float>(t));
    const Vec4 tex { static_cast<float>(s), static_cast<float>(t), 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord2sv(const GLshort* v)
{
    SPDLOG_DEBUG("glTexCoord2sv ({}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]));
    const Vec4 tex { static_cast<float>(v[0]), static_cast<float>(v[1]), 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord3d(GLdouble s, GLdouble t, GLdouble r)
{
    SPDLOG_DEBUG("glTexCoord3d ({}, {}, {}) called",
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r));
    const Vec4 tex {
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord3dv(const GLdouble* v)
{
    SPDLOG_DEBUG("glTexCoord3dv ({}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]));
    const Vec4 tex {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord3f(GLfloat s, GLfloat t, GLfloat r)
{
    SPDLOG_DEBUG("glTexCoord3f ({}, {}, {}) called", s, t, r);
    const Vec4 tex { s, t, r, 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord3fv(const GLfloat* v)
{
    SPDLOG_DEBUG("glTexCoord3fv ({}, {}, {}) called", v[0], v[1], v[2]);
    const Vec4 tex { v[0], v[1], v[2], 1.0f };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord3i(GLint s, GLint t, GLint r)
{
    SPDLOG_DEBUG("glTexCoord3i ({}, {}, {}) called",
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r));
    const Vec4 tex {
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord3iv(const GLint* v)
{
    SPDLOG_DEBUG("glTexCoord3iv ({}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]));
    const Vec4 tex {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord3s(GLshort s, GLshort t, GLshort r)
{
    SPDLOG_DEBUG("glTexCoord3s ({}, {}, {}) called",
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r));
    const Vec4 tex {
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord3sv(const GLshort* v)
{
    SPDLOG_DEBUG("glTexCoord3sv ({}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]));
    const Vec4 tex {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    SPDLOG_DEBUG("glTexCoord4d ({}, {}, {}, {}) called",
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        static_cast<float>(q));
    const Vec4 tex {
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        static_cast<float>(q)
    };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord4dv(const GLdouble* v)
{
    SPDLOG_DEBUG("glTexCoord4dv ({}, {}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3]));
    const Vec4 tex {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3])
    };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    SPDLOG_DEBUG("glTexCoord4f ({}, {}, {}, {}) called", s, t, r, q);
    const Vec4 tex { s, t, r, q };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord4fv(const GLfloat* v)
{
    SPDLOG_DEBUG("glTexCoord4fv ({}, {}, {}, {}) called", v[0], v[1], v[2], v[3]);
    const Vec4 tex { v[0], v[1], v[2], v[3] };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord4i(GLint s, GLint t, GLint r, GLint q)
{
    SPDLOG_DEBUG("glTexCoord4i ({}, {}, {}, {}) called",
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        static_cast<float>(q));
    const Vec4 tex {
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        static_cast<float>(q)
    };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord4iv(const GLint* v)
{
    SPDLOG_DEBUG("glTexCoord4iv ({}, {}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3]));
    const Vec4 tex {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3])
    };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)
{
    SPDLOG_DEBUG("glTexCoord4s ({}, {}, {}, {}) called",
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        static_cast<float>(q));
    const Vec4 tex {
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        static_cast<float>(q)
    };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

GLAPI void APIENTRY impl_glTexCoord4sv(const GLshort* v)
{
    SPDLOG_DEBUG("glTexCoord4sv ({}, {}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3]));
    const Vec4 tex {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3])
    };
    RIXGL::getInstance().vertexQueue().setTexCoord(tex);
    RIXGL::getInstance().vertexArray().setTexCoord(tex);
}

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

GLAPI void APIENTRY impl_glTexEnviv(GLenum target, GLenum pname, const GLint* params)
{
    SPDLOG_WARN("glTexEnviv not implemented");
}

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

GLAPI void APIENTRY impl_glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    SPDLOG_WARN("glTexImage1D not implemented");
}

GLAPI void APIENTRY impl_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    SPDLOG_DEBUG("glTexImage2D target 0x{:X} level 0x{:X} internalformat 0x{:X} width {} height {} border 0x{:X} format 0x{:X} type 0x{:X} called", target, level, internalformat, width, height, border, format, type);

    // Border is not supported in OpenGL ES and is ignored. What does border mean: https://stackoverflow.com/questions/913801/what-does-border-mean-in-the-glteximage2d-function
    if ((border != 0) || (level < 0))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glTexImage2D border or level invalid");
        return;
    }

    const std::size_t maxTexSize { RIXGL::getInstance().getMaxTextureSize() };

    if (static_cast<std::size_t>(level) > RIXGL::getInstance().getMaxLOD())
    {
        SPDLOG_ERROR("glTexImage2D invalid lod.");
        return;
    }

    if ((static_cast<std::size_t>(width) > maxTexSize) || (static_cast<std::size_t>(height) > maxTexSize))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glTexImage2D texture is too big.");
        return;
    }

    if (!RIXGL::getInstance().isMipmappingAvailable() && (level != 0))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glTexImage2D mipmapping on hardware not supported.");
        return;
    }

    // It can happen, that a not power of two texture is used. This little hack allows that the texture can sill be used
    // without crashing the software. But it is likely that it will produce graphical errors.
    const std::size_t widthRounded = powf(2.0f, ceilf(logf(width) / logf(2.0f)));
    const std::size_t heightRounded = powf(2.0f, ceilf(logf(height) / logf(2.0f)));

    if ((widthRounded == 0) || (heightRounded == 0))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glTexImage2D texture with invalid size detected ({} (rounded to {}), {} (rounded to {}))", width, widthRounded, height, heightRounded);
        return;
    }

    InternalPixelFormat internalPixelFormat;
    const GLenum error = ImageConverter::convertInternalPixelFormat(internalPixelFormat, internalformat);
    if (error != GL_NO_ERROR)
    {
        RIXGL::getInstance().setError(error);
        SPDLOG_ERROR("glTexImage2D internal Pixel Format 0x{:X} not supported", internalformat);
        return;
    }

    TextureObject& texObj { RIXGL::getInstance().pipeline().texture().getTexture() };

    texObj.setWidth(level, widthRounded);
    texObj.setHeight(level, heightRounded);
    texObj.setInternalPixelFormat(level, internalPixelFormat);

    SPDLOG_DEBUG("glTexImage2D redirect to glTexSubImage2D");
    impl_glTexSubImage2D(target, level, 0, 0, width, height, format, type, pixels);
}

GLAPI void APIENTRY impl_glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glTexParameterf target 0x{:X} pname 0x{:X} param {} redirected to glTexParameteri", target, pname, param);
    impl_glTexParameteri(target, pname, static_cast<GLint>(param));
}

GLAPI void APIENTRY impl_glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
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

GLAPI void APIENTRY impl_glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
    SPDLOG_WARN("glTexParameteriv not implemented");
}

GLAPI void APIENTRY impl_glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
    SPDLOG_DEBUG("glTranslated ({}, {}, {}) called",
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z));
    RIXGL::getInstance().pipeline().getMatrixStore().getCurrentMatrix().translate(
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z));
}

GLAPI void APIENTRY impl_glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
    SPDLOG_DEBUG("glTranslatef ({}, {}, {}) called", x, y, z);
    RIXGL::getInstance().pipeline().getMatrixStore().getCurrentMatrix().translate(x, y, z);
}

GLAPI void APIENTRY impl_glVertex2d(GLdouble x, GLdouble y)
{
    SPDLOG_DEBUG("glVertex2d ({}, {}) called",
        static_cast<float>(x),
        static_cast<float>(y));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(x),
        static_cast<float>(y),
        0.0f,
        1.0f });
}

GLAPI void APIENTRY impl_glVertex2dv(const GLdouble* v)
{
    SPDLOG_DEBUG("glVertex2dv ({}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        0.0f,
        1.0f });
}

GLAPI void APIENTRY impl_glVertex2f(GLfloat x, GLfloat y)
{
    SPDLOG_DEBUG("glVertex2f ({}, {}) called", x, y);
    RIXGL::getInstance().vertexQueue().addVertex({ x, y, 0.0f, 1.0f });
}

GLAPI void APIENTRY impl_glVertex2fv(const GLfloat* v)
{
    SPDLOG_DEBUG("glVertex2fv ({}, {}) called", v[0], v[1]);
    RIXGL::getInstance().vertexQueue().addVertex({ v[0], v[1], 0.0f, 1.0f });
}

GLAPI void APIENTRY impl_glVertex2i(GLint x, GLint y)
{
    SPDLOG_DEBUG("glVertex2i ({}, {}) called",
        static_cast<float>(x),
        static_cast<float>(y));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(x),
        static_cast<float>(y),
        0.0f,
        1.0f });
}

GLAPI void APIENTRY impl_glVertex2iv(const GLint* v)
{
    SPDLOG_DEBUG("glVertex2iv ({}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        0.0f,
        1.0f });
}

GLAPI void APIENTRY impl_glVertex2s(GLshort x, GLshort y)
{
    SPDLOG_DEBUG("glVertex2s ({}, {}) called",
        static_cast<float>(x),
        static_cast<float>(y));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(x),
        static_cast<float>(y),
        0.0f,
        1.0f });
}

GLAPI void APIENTRY impl_glVertex2sv(const GLshort* v)
{
    SPDLOG_DEBUG("glVertex2sv ({}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        0.0f,
        1.0f });
}

GLAPI void APIENTRY impl_glVertex3d(GLdouble x, GLdouble y, GLdouble z)
{
    SPDLOG_DEBUG("glVertex3d ({}, {}, {}) called",
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z),
        1.0f });
}

GLAPI void APIENTRY impl_glVertex3dv(const GLdouble* v)
{
    SPDLOG_DEBUG("glVertex3dv ({}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        1.0f });
}

GLAPI void APIENTRY impl_glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    SPDLOG_DEBUG("glVertex3f ({}, {}, {}) called", x, y, z);
    RIXGL::getInstance().vertexQueue().addVertex({ x, y, z, 1.0f });
}

GLAPI void APIENTRY impl_glVertex3fv(const GLfloat* v)
{
    SPDLOG_DEBUG("glVertex3fv ({}, {}, {}) called", v[0], v[1], v[2]);
    RIXGL::getInstance().vertexQueue().addVertex({ v[0], v[1], v[2], 1.0f });
}

GLAPI void APIENTRY impl_glVertex3i(GLint x, GLint y, GLint z)
{
    SPDLOG_DEBUG("glVertex3i ({}, {}, {}) called",
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z),
        1.0f });
}

GLAPI void APIENTRY impl_glVertex3iv(const GLint* v)
{
    SPDLOG_DEBUG("glVertex3iv ({}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        1.0f });
}

GLAPI void APIENTRY impl_glVertex3s(GLshort x, GLshort y, GLshort z)
{
    SPDLOG_DEBUG("glVertex3s ({}, {}, {}) called",
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z),
        1.0f });
}

GLAPI void APIENTRY impl_glVertex3sv(const GLshort* v)
{
    SPDLOG_DEBUG("glVertex3sv ({}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        1.0f });
}

GLAPI void APIENTRY impl_glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    SPDLOG_DEBUG("glVertex4d ({}, {}, {}, {}) called",
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z),
        static_cast<float>(w));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z),
        static_cast<float>(w) });
}

GLAPI void APIENTRY impl_glVertex4dv(const GLdouble* v)
{
    SPDLOG_DEBUG("glVertex4dv ({}, {}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3]));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3]) });
}

GLAPI void APIENTRY impl_glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    SPDLOG_DEBUG("glVertex4f ({}, {}, {}, {}) called", x, y, z, w);
    RIXGL::getInstance().vertexQueue().addVertex({ x, y, z, w });
}

GLAPI void APIENTRY impl_glVertex4fv(const GLfloat* v)
{
    SPDLOG_DEBUG("glVertex4fv ({}, {}, {}, {}) called", v[0], v[1], v[2], v[3]);
    RIXGL::getInstance().vertexQueue().addVertex({ v[0], v[1], v[2], v[3] });
}

GLAPI void APIENTRY impl_glVertex4i(GLint x, GLint y, GLint z, GLint w)
{
    SPDLOG_DEBUG("glVertex4i ({}, {}, {}, {}) called",
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z),
        static_cast<float>(w));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z),
        static_cast<float>(w) });
}

GLAPI void APIENTRY impl_glVertex4iv(const GLint* v)
{
    SPDLOG_DEBUG("glVertex4iv ({}, {}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3]));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3]) });
}

GLAPI void APIENTRY impl_glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    SPDLOG_DEBUG("glVertex4s ({}, {}, {}, {}) called",
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z),
        static_cast<float>(w));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z),
        static_cast<float>(w) });
}

GLAPI void APIENTRY impl_glVertex4sv(const GLshort* v)
{
    SPDLOG_DEBUG("glVertex4sv ({}, {}, {}, {}) called",
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3]));
    RIXGL::getInstance().vertexQueue().addVertex({ static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3]) });
}

GLAPI void APIENTRY impl_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    SPDLOG_DEBUG("glViewport ({}, {}) width {} heigh {} called", x, y, width, height);
    // TODO: Generate a GL_INVALID_VALUE if width or height is negative
    // TODO: Reversed mapping is not working right now, for instance if zFar < zNear
    // Note: The screen resolution is width and height. But during view port transformation it is clamped between
    // 0 and height which means a effective screen resolution of height + 1. For instance, a resolution of
    // 480 x 272. The view port transformation would go from 0 to 480 which is then 481px. Thats the reason why the
    // resolution is decremented by one.
    RIXGL::getInstance().pipeline().getViewPort().setViewport(x, y, width, height);
}

// -------------------------------------------------------

// Open GL 1.1
// -------------------------------------------------------
GLAPI GLboolean APIENTRY impl_glAreTexturesResident(GLsizei n, const GLuint* textures, GLboolean* residences)
{
    SPDLOG_WARN("glAreTexturesResident not implemented");
    return false;
}

GLAPI void APIENTRY impl_glArrayElement(GLint i)
{
    SPDLOG_WARN("glArrayElement not implemented");
}

GLAPI void APIENTRY impl_glBindTexture(GLenum target, GLuint texture)
{
    SPDLOG_DEBUG("glBindTexture target 0x{:X} texture 0x{:X}", target, texture);
    if (target != GL_TEXTURE_2D)
    {
        SPDLOG_WARN("glBindTexture target not supported");
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    if (!RIXGL::getInstance().pipeline().texture().isTextureValid(texture))
    {
        bool ret { RIXGL::getInstance().pipeline().texture().createTextureWithName(texture) };
        if (!ret)
        {
            // TODO: Free allocated textures to avoid leaks
            RIXGL::getInstance().setError(GL_OUT_OF_MEMORY);
            SPDLOG_ERROR("glBindTexture cannot create texture {}", texture);
            return;
        }
    }

    RIXGL::getInstance().pipeline().texture().setBoundTexture(texture);
    RIXGL::getInstance().pipeline().texture().useTexture();
}

GLAPI void APIENTRY impl_glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    SPDLOG_DEBUG("glColorPointer size {} type 0x{:X} stride {} called", size, type, stride);

    RIXGL::getInstance().vertexArray().setColorSize(size);
    RIXGL::getInstance().vertexArray().setColorType(convertType(type));
    RIXGL::getInstance().vertexArray().setColorStride(stride);
    if (RIXGL::getInstance().vertexBuffer().isBufferActive())
    {
        const auto data = RIXGL::getInstance().vertexBuffer().getBufferData();
        RIXGL::getInstance().vertexArray().setColorPointer(data.first.data(), reinterpret_cast<std::size_t>(pointer), data.second);
    }
    else
    {
        RIXGL::getInstance().vertexArray().setColorPointer(pointer, 0, 0);
    }
}

GLAPI void APIENTRY impl_glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    SPDLOG_WARN("glCopyTexImage1D not implemented");
}

GLAPI void APIENTRY impl_glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    SPDLOG_DEBUG("glCopyTexImage2D target 0x{:X} level 0x{:X} internalformat 0x{:X} x {} y {} width {} height {} border 0x{:X} called",
        target, level, internalformat, x, y, width, height, border);

    InternalPixelFormat internalPixelFormat;
    const GLenum error = ImageConverter::convertInternalPixelFormat(internalPixelFormat, internalformat);
    if (error != GL_NO_ERROR)
    {
        RIXGL::getInstance().setError(error);
        return;
    }

    switch (internalPixelFormat)
    {
    case InternalPixelFormat::ALPHA:
    case InternalPixelFormat::LUMINANCE_ALPHA:
    case InternalPixelFormat::RGBA:
    case InternalPixelFormat::RGBA1:
        RIXGL::getInstance().setError(GL_INVALID_OPERATION);
        SPDLOG_ERROR("glCopyTexImage2D invalid internal format used (framebuffer does not support an alpha channel)");
        return;
    default:
        break;
    }

    const GLsizei maxTexSize { static_cast<GLsizei>(RIXGL::getInstance().getMaxTextureSize()) };
    if ((width > maxTexSize) || (height > maxTexSize))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glCopyTexImage2D texture is too big.");
        return;
    }

    const uint16_t* texBuffer = readFromColorBuffer(x, y, width, height, true);

    SPDLOG_DEBUG("glCopyTexImage2D redirect to glTexImage2D");
    impl_glTexImage2D(target, level, internalformat, width, height, border, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, texBuffer);

    delete texBuffer;
}

GLAPI void APIENTRY impl_glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    SPDLOG_WARN("glCopyTexSubImage1D not implemented");
}

GLAPI void APIENTRY impl_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    SPDLOG_DEBUG("glCopyTexSubImage2D target 0x{:X} level 0x{:X} xoffset {} yoffset {} x {} y {} width {} height {} called",
        target, level, xoffset, yoffset, x, y, width, height);

    const uint16_t* texBuffer = readFromColorBuffer(x, y, width, height, true);

    SPDLOG_DEBUG("glCopyTexSubImage2D redirect to glTexSubImage2D");
    impl_glTexSubImage2D(target, level, xoffset, yoffset, width, height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, texBuffer);

    delete texBuffer;
}

GLAPI void APIENTRY impl_glDeleteTextures(GLsizei n, const GLuint* textures)
{
    SPDLOG_DEBUG("glDeleteTextures 0x{:X} called", n);
    if (n < 0)
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glDeleteTextures n < 0");
        return;
    }

    for (GLsizei i = 0; i < n; i++)
    {
        // From the specification: glDeleteTextures silently ignores 0's and names that do not correspond to existing textures.
        if (textures[i] != 0)
        {
            SPDLOG_DEBUG("glDeleteTextures delete 0x{:X}", textures[i]);
            RIXGL::getInstance().pipeline().texture().deleteTexture(textures[i]);
        }
    }
}

GLAPI void APIENTRY impl_glDisableClientState(GLenum cap)
{
    SPDLOG_DEBUG("glDisableClientState cap 0x{:X} called", cap);
    setClientState(cap, false);
}

GLAPI void APIENTRY impl_glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    SPDLOG_DEBUG("glDrawArrays mode 0x{:X} first {} count {} called", mode, first, count);

    RIXGL::getInstance().vertexArray().reset();

    RIXGL::getInstance().vertexArray().setCount(count);
    RIXGL::getInstance().vertexArray().setArrayOffset(first);
    RIXGL::getInstance().vertexArray().setDrawMode(convertDrawMode(mode));
    RIXGL::getInstance().vertexArray().enableIndices(false);

    RIXGL::getInstance().pipeline().drawObj(RIXGL::getInstance().vertexArray().renderObj());
}

GLAPI void APIENTRY impl_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
    SPDLOG_DEBUG("glDrawElements mode 0x{:X} count {} type 0x{:X} called", mode, count, type);
    bool error = false;
    switch (type)
    {
    case GL_UNSIGNED_BYTE:
        RIXGL::getInstance().vertexArray().setIndicesType(Type::BYTE);
        break;
    case GL_UNSIGNED_SHORT:
        RIXGL::getInstance().vertexArray().setIndicesType(Type::UNSIGNED_SHORT);
        break;
    case GL_UNSIGNED_INT:
        RIXGL::getInstance().vertexArray().setIndicesType(Type::UNSIGNED_INT);
        break;
    default:
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        error = true;
        SPDLOG_WARN("glDrawElements type 0x{:X} not supported", type);
        return;
    }

    RIXGL::getInstance().vertexArray().reset();

    RIXGL::getInstance().vertexArray().setCount(count);
    RIXGL::getInstance().vertexArray().setDrawMode(convertDrawMode(mode));
    RIXGL::getInstance().vertexArray().enableIndices(true);

    if (RIXGL::getInstance().vertexBuffer().isBufferActive())
    {
        const auto data = RIXGL::getInstance().vertexBuffer().getBufferData();
        RIXGL::getInstance().vertexArray().setIndicesPointer(
            data.first.last(data.first.size() - reinterpret_cast<std::size_t>(indices)).data());
    }
    else
    {
        RIXGL::getInstance().vertexArray().setIndicesPointer(indices);
    }

    if (!error)
    {
        RIXGL::getInstance().pipeline().drawObj(RIXGL::getInstance().vertexArray().renderObj());
    }
}

GLAPI void APIENTRY impl_glEdgeFlagPointer(GLsizei stride, const GLvoid* pointer)
{
    SPDLOG_WARN("glEdgeFlagPointer not implemented");
}

GLAPI void APIENTRY impl_glEnableClientState(GLenum cap)
{
    SPDLOG_DEBUG("glEnableClientState cap 0x{:X} called", cap);
    setClientState(cap, true);
}

GLAPI void APIENTRY impl_glGenTextures(GLsizei n, GLuint* textures)
{
    SPDLOG_DEBUG("glGenTextures 0x{:X} called", n);
    if (n < 0)
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glGenTextures n < 0");
        return;
    }

    for (GLsizei i = 0; i < n; i++)
    {
        std::pair<bool, uint16_t> ret { RIXGL::getInstance().pipeline().texture().createTexture() };
        if (ret.first)
        {
            textures[i] = ret.second;
        }
        else
        {
            // TODO: Free allocated textures to avoid leaks
            RIXGL::getInstance().setError(GL_OUT_OF_MEMORY);
            SPDLOG_ERROR("glGenTextures Out Of Memory");
            return;
        }
    }
}

GLAPI void APIENTRY impl_glGetPointerv(GLenum pname, GLvoid** params)
{
    SPDLOG_WARN("glGetPointerv not implemented");
    switch (pname)
    {
    case GL_VERTEX_ARRAY_POINTER:
        if (RIXGL::getInstance().vertexBuffer().isBufferActive())
        {
            *params = reinterpret_cast<GLvoid*>(RIXGL::getInstance().vertexArray().getVertexPointerOffset());
        }
        else
        {
            *params = const_cast<GLvoid*>(RIXGL::getInstance().vertexArray().getVertexPointer());
        }
        *params = const_cast<GLvoid*>(RIXGL::getInstance().vertexArray().getVertexPointer());
        break;
    case GL_NORMAL_ARRAY_POINTER:
        if (RIXGL::getInstance().vertexBuffer().isBufferActive())
        {
            *params = reinterpret_cast<GLvoid*>(RIXGL::getInstance().vertexArray().getNormalPointerOffset());
        }
        else
        {
            *params = const_cast<GLvoid*>(RIXGL::getInstance().vertexArray().getNormalPointer());
        }
        break;
    case GL_COLOR_ARRAY_POINTER:
        if (RIXGL::getInstance().vertexBuffer().isBufferActive())
        {
            *params = reinterpret_cast<GLvoid*>(RIXGL::getInstance().vertexArray().getColorPointerOffset());
        }
        else
        {
            *params = const_cast<GLvoid*>(RIXGL::getInstance().vertexArray().getColorPointer());
        }
        break;
    case GL_TEXTURE_COORD_ARRAY_POINTER:
        if (RIXGL::getInstance().vertexBuffer().isBufferActive())
        {
            *params = reinterpret_cast<GLvoid*>(RIXGL::getInstance().vertexArray().getTexCoordPointerOffset());
        }
        else
        {
            *params = const_cast<GLvoid*>(RIXGL::getInstance().vertexArray().getTexCoordPointer());
        }
        break;
    case GL_POINT_SIZE_ARRAY_POINTER_OES:
        if (RIXGL::getInstance().vertexBuffer().isBufferActive())
        {
            *params = reinterpret_cast<GLvoid*>(RIXGL::getInstance().vertexArray().getPointSizePointerOffset());
        }
        else
        {
            *params = const_cast<GLvoid*>(RIXGL::getInstance().vertexArray().getPointSizePointer());
        }
    default:
        SPDLOG_WARN("glGetPointerv pname 0x{:X} not supported", pname);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI GLboolean APIENTRY impl_glIsTexture(GLuint texture)
{
    SPDLOG_DEBUG("glIsTexture called for texture 0x{:X}", texture);
    return RIXGL::getInstance().pipeline().texture().isTextureValid(texture);
}

GLAPI void APIENTRY impl_glIndexPointer(GLenum type, GLsizei stride, const GLvoid* pointer)
{
    SPDLOG_WARN("glIndexPointer not implemented");
}

GLAPI void APIENTRY impl_glIndexub(GLubyte c)
{
    SPDLOG_WARN("glIndexub not implemented");
}

GLAPI void APIENTRY impl_glIndexubv(const GLubyte* c)
{
    SPDLOG_WARN("glIndexubv not implemented");
}

GLAPI void APIENTRY impl_glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid* pointer)
{
    SPDLOG_WARN("glInterleavedArrays not implemented");
}

GLAPI void APIENTRY impl_glNormalPointer(GLenum type, GLsizei stride, const GLvoid* pointer)
{
    SPDLOG_DEBUG("glNormalPointer type 0x{:X} stride {} called", type, stride);

    RIXGL::getInstance().vertexArray().setNormalType(convertType(type));
    RIXGL::getInstance().vertexArray().setNormalStride(stride);
    if (RIXGL::getInstance().vertexBuffer().isBufferActive())
    {
        const auto data = RIXGL::getInstance().vertexBuffer().getBufferData();
        RIXGL::getInstance().vertexArray().setNormalPointer(data.first.data(), reinterpret_cast<std::size_t>(pointer), data.second);
    }
    else
    {
        RIXGL::getInstance().vertexArray().setNormalPointer(pointer, 0, 0);
    }
}

GLAPI void APIENTRY impl_glPolygonOffset(GLfloat factor, GLfloat units)
{
    SPDLOG_DEBUG("glPolygonOffset factor {} units {} called", factor, units);

    RIXGL::getInstance().pipeline().getPolygonOffset().setFactor(factor);
    RIXGL::getInstance().pipeline().getPolygonOffset().setUnits(units);
}

GLAPI void APIENTRY impl_glPopClientAttrib(void)
{
    SPDLOG_WARN("glPopClientAttrib not implemented");
}

GLAPI void APIENTRY impl_glPrioritizeTextures(GLsizei n, const GLuint* textures, const GLclampf* priorities)
{
    SPDLOG_WARN("glPrioritizeTextures not implemented");
}

GLAPI void APIENTRY impl_glPushClientAttrib(GLbitfield mask)
{
    SPDLOG_WARN("glPushClientAttrib not implemented");
}

GLAPI void APIENTRY impl_glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    SPDLOG_DEBUG("glTexCoordPointer size {} type 0x{:X} stride {} called", size, type, stride);

    RIXGL::getInstance().vertexArray().setTexCoordSize(size);
    RIXGL::getInstance().vertexArray().setTexCoordType(convertType(type));
    RIXGL::getInstance().vertexArray().setTexCoordStride(stride);
    if (RIXGL::getInstance().vertexBuffer().isBufferActive())
    {
        const auto data = RIXGL::getInstance().vertexBuffer().getBufferData();
        RIXGL::getInstance().vertexArray().setTexCoordPointer(data.first.data(), reinterpret_cast<std::size_t>(pointer), data.second);
    }
    else
    {
        RIXGL::getInstance().vertexArray().setTexCoordPointer(pointer, 0, 0);
    }
}

GLAPI void APIENTRY impl_glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid* pixels)
{
    SPDLOG_WARN("glTexSubImage1D not implemented");
}

GLAPI void APIENTRY impl_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
    SPDLOG_DEBUG("glTexSubImage2D target 0x{:X} level 0x{:X} xoffset {} yoffset {} width {} height {} format 0x{:X} type 0x{:X} called", target, level, xoffset, yoffset, width, height, format, type);

    if (static_cast<std::size_t>(level) > RIXGL::getInstance().getMaxLOD())
    {
        SPDLOG_ERROR("glTexSubImage2D invalid lod.");
        return;
    }

    if (!RIXGL::getInstance().isMipmappingAvailable() && (level != 0))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glTexSubImage2D mipmapping on hardware not supported.");
        return;
    }

    TextureObject& texObj { RIXGL::getInstance().pipeline().texture().getTexture() };
    if (((xoffset + width) > static_cast<GLint>(texObj.getWidth(level)))
        || ((yoffset + height) > static_cast<GLint>(texObj.getHeight(level))))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glTexSubImage2D offsets or texture sizes are too big");
        return;
    }

    using PixelType = TextureObject::PixelsType::element_type;

    const std::size_t texMemSize = texObj.getWidth(level) * texObj.getHeight(level) * sizeof(PixelType);
    if (texMemSize == 0)
    {
        SPDLOG_DEBUG("glTexSubImage2D texture with zero dimensions loaded.");
        return;
    }

    std::shared_ptr<PixelType> texMemShared(
        new PixelType[texMemSize / sizeof(PixelType)],
        [](const PixelType* p)
        { delete[] p; });

    if (!texMemShared)
    {
        RIXGL::getInstance().setError(GL_OUT_OF_MEMORY);
        SPDLOG_ERROR("glTexSubImage2D Out Of Memory");
        return;
    }
    if (texObj.getPixels(level))
    {
        std::memcpy(texMemShared.get(), texObj.getPixels(level).get(), std::min(texObj.getSizeInBytes(level), texMemSize));
    }
    else
    {
        std::memset(texMemShared.get(), 0, texMemSize);
    }
    texObj.setPixels(level, texMemShared);

    // Check if pixels is null. If so, just set the empty memory area and don't copy anything.
    if (pixels != nullptr)
    {
        RIXGL::getInstance().imageConverter().convertUnpack(
            texObj.getPixels(level),
            texObj.getInternalPixelFormat(level),
            texObj.getWidth(level),
            xoffset,
            yoffset,
            width,
            height,
            format,
            type,
            reinterpret_cast<const uint8_t*>(pixels));

        RIXGL::getInstance().mipMapGenerator().generateMipMap(RIXGL::getInstance().pipeline().texture().getTexture(), level);
    }
}

GLAPI void APIENTRY impl_glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    SPDLOG_DEBUG("glVertexPointer size {} type 0x{:X} stride {} called", size, type, stride);

    RIXGL::getInstance().vertexArray().setVertexSize(size);
    RIXGL::getInstance().vertexArray().setVertexType(convertType(type));
    RIXGL::getInstance().vertexArray().setVertexStride(stride);
    if (RIXGL::getInstance().vertexBuffer().isBufferActive())
    {
        const auto data = RIXGL::getInstance().vertexBuffer().getBufferData();
        RIXGL::getInstance().vertexArray().setVertexPointer(data.first.data(), reinterpret_cast<std::size_t>(pointer), data.second);
    }
    else
    {
        RIXGL::getInstance().vertexArray().setVertexPointer(pointer, 0, 0);
    }
}

// -------------------------------------------------------

// Open GL 1.2
// -------------------------------------------------------
GLAPI void APIENTRY impl_glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices)
{
    SPDLOG_WARN("glDrawRangeElements not implemented");
}

GLAPI void APIENTRY impl_glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* data)
{
    SPDLOG_WARN("glTexImage3D not implemented");
}

GLAPI void APIENTRY impl_glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels)
{
    SPDLOG_WARN("glTexSubImage3D not implemented");
}

GLAPI void APIENTRY impl_glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    SPDLOG_WARN("glCopyTexSubImage3D not implemented");
}

// -------------------------------------------------------

// Open GL 1.3
// -------------------------------------------------------
GLAPI void APIENTRY impl_glActiveTexture(GLenum texture)
{
    SPDLOG_DEBUG("glActiveTexture texture 0x{:X} called", texture - GL_TEXTURE0);
    // TODO: Check how many TMUs the hardware actually has
    RIXGL::getInstance().pipeline().texture().activateTmu(texture - GL_TEXTURE0);
    RIXGL::getInstance().pipeline().activateTmu(texture - GL_TEXTURE0);
}

GLAPI void APIENTRY impl_glSampleCoverage(GLfloat value, GLboolean invert)
{
    SPDLOG_WARN("glSampleCoverage not implemented");
}

GLAPI void APIENTRY impl_glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data)
{
    SPDLOG_WARN("glCompressedTexImage3D not implemented");
}

GLAPI void APIENTRY impl_glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data)
{
    SPDLOG_WARN("glCompressedTexImage2D not implemented");
}

GLAPI void APIENTRY impl_glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid* data)
{
    SPDLOG_WARN("glCompressedTexImage1D not implemented");
}

GLAPI void APIENTRY impl_glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data)
{
    SPDLOG_WARN("glCompressedTexSubImage3D not implemented");
}

GLAPI void APIENTRY impl_glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data)
{
    SPDLOG_WARN("glCompressedTexSubImage2D not implemented");
}

GLAPI void APIENTRY impl_glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid* data)
{
    SPDLOG_WARN("glCompressedTexSubImage1D not implemented");
}

GLAPI void APIENTRY impl_glGetCompressedTexImage(GLenum target, GLint level, GLvoid* pixels)
{
    SPDLOG_WARN("glGetCompressedTexImage not implemented");
}

GLAPI void APIENTRY impl_glClientActiveTexture(GLenum texture)
{
    SPDLOG_DEBUG("glClientActiveTexture texture 0x{:X} called", texture - GL_TEXTURE0);
    RIXGL::getInstance().vertexQueue().setActiveTexture(texture - GL_TEXTURE0);
    RIXGL::getInstance().vertexArray().setActiveTexture(texture - GL_TEXTURE0);
}

GLAPI void APIENTRY impl_glMultiTexCoord1d(GLenum target, GLdouble s)
{
    SPDLOG_DEBUG("glMultiTexCoord1d 0x{:X} ({}) called",
        target - GL_TEXTURE0,
        static_cast<float>(s));
    const Vec4 tex { static_cast<float>(s), 0.0f, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord1dv(GLenum target, const GLdouble* v)
{
    SPDLOG_DEBUG("glMultiTexCoord1dv 0x{:X} ({}) called",
        target - GL_TEXTURE0,
        static_cast<float>(v[0]));
    const Vec4 tex { static_cast<float>(v[0]), 0.0f, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord1f(GLenum target, GLfloat s)
{
    SPDLOG_DEBUG("glMultiTexCoord1f 0x{:X} ({}) called", target - GL_TEXTURE0, s);
    const Vec4 tex { s, 0.0f, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord1fv(GLenum target, const GLfloat* v)
{
    SPDLOG_DEBUG("glMultiTexCoord1fv 0x{:X} ({}) called", target - GL_TEXTURE0, v[0]);
    const Vec4 tex { v[0], 0.0f, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord1i(GLenum target, GLint s)
{
    SPDLOG_DEBUG("glMultiTexCoord1i 0x{:X} ({}) called",
        target - GL_TEXTURE0,
        static_cast<float>(s));
    const Vec4 tex { static_cast<float>(s), 0.0f, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord1iv(GLenum target, const GLint* v)
{
    SPDLOG_DEBUG("glMultiTexCoord1iv 0x{:X} ({}) called",
        target - GL_TEXTURE0,
        static_cast<float>(v[0]));
    const Vec4 tex { static_cast<float>(v[0]), 0.0f, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord1s(GLenum target, GLshort s)
{
    SPDLOG_DEBUG("glMultiTexCoord1s 0x{:X} ({}) called",
        target - GL_TEXTURE0,
        static_cast<float>(s));
    const Vec4 tex { static_cast<float>(s), 0.0f, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord1sv(GLenum target, const GLshort* v)
{
    SPDLOG_DEBUG("glMultiTexCoord1sv 0x{:X} ({}) called",
        target - GL_TEXTURE0,
        static_cast<float>(v[0]));
    const Vec4 tex { static_cast<float>(v[0]), 0.0f, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t)
{
    SPDLOG_DEBUG("glMultiTexCoord2d 0x{:X} ({}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(s),
        static_cast<float>(t));
    const Vec4 tex { static_cast<float>(s), static_cast<float>(t), 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord2dv(GLenum target, const GLdouble* v)
{
    SPDLOG_DEBUG("glMultiTexCoord2dv 0x{:X} ({}, {}) called",
        target - GL_TEXTURE0, static_cast<float>(v[0]),
        static_cast<float>(v[1]));
    const Vec4 tex { static_cast<float>(v[0]), static_cast<float>(v[1]), 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t)
{
    SPDLOG_DEBUG("glMultiTexCoord2f 0x{:X} ({}, {}) called", target - GL_TEXTURE0, s, t);
    const Vec4 tex { s, t, 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord2fv(GLenum target, const GLfloat* v)
{
    SPDLOG_DEBUG("glMultiTexCoord2fv 0x{:X} ({}, {}) called", target - GL_TEXTURE0, v[0], v[1]);
    const Vec4 tex { v[0], v[1], 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord2i(GLenum target, GLint s, GLint t)
{
    SPDLOG_DEBUG("glMultiTexCoord2i 0x{:X} ({}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(s),
        static_cast<float>(t));
    const Vec4 tex { static_cast<float>(s), static_cast<float>(t), 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord2iv(GLenum target, const GLint* v)
{
    SPDLOG_DEBUG("glMultiTexCoord2iv 0x{:X} ({}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(v[0]),
        static_cast<float>(v[1]));
    const Vec4 tex { static_cast<float>(v[0]), static_cast<float>(v[1]), 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord2s(GLenum target, GLshort s, GLshort t)
{
    SPDLOG_DEBUG("glMultiTexCoord2s 0x{:X} ({}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(s),
        static_cast<float>(t));
    const Vec4 tex { static_cast<float>(s), static_cast<float>(t), 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord2sv(GLenum target, const GLshort* v)
{
    SPDLOG_DEBUG("glMultiTexCoord2sv 0x{:X} ({}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(v[0]),
        static_cast<float>(v[1]));
    const Vec4 tex { static_cast<float>(v[0]), static_cast<float>(v[1]), 0.0f, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
    SPDLOG_DEBUG("glMultiTexCoord3d 0x{:X} ({}, {}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r));
    const Vec4 tex {
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord3dv(GLenum target, const GLdouble* v)
{
    SPDLOG_DEBUG("glMultiTexCoord3dv 0x{:X} ({}, {}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]));
    const Vec4 tex {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
    SPDLOG_DEBUG("glMultiTexCoord3f 0x{:X} ({}, {}, {}) called", target - GL_TEXTURE0, s, t, r);
    const Vec4 tex { s, t, r, 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord3fv(GLenum target, const GLfloat* v)
{
    SPDLOG_DEBUG("glMultiTexCoord3fv 0x{:X} ({}, {}, {}) called", target - GL_TEXTURE0, v[0], v[1], v[2]);
    const Vec4 tex { v[0], v[1], v[2], 1.0f };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r)
{
    SPDLOG_DEBUG("glMultiTexCoord3i 0x{:X} ({}, {}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r));
    const Vec4 tex {
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord3iv(GLenum target, const GLint* v)
{
    SPDLOG_DEBUG("glMultiTexCoord3iv 0x{:X} ({}, {}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]));
    const Vec4 tex {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r)
{
    SPDLOG_DEBUG("glMultiTexCoord3s 0x{:X} ({}, {}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r));
    const Vec4 tex {
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord3sv(GLenum target, const GLshort* v)
{
    SPDLOG_DEBUG("glMultiTexCoord3sv 0x{:X} ({}, {}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]));
    const Vec4 tex {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        1.0f
    };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    SPDLOG_DEBUG("glMultiTexCoord4d 0x{:X} ({}, {}, {}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        static_cast<float>(q));
    const Vec4 tex {
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        static_cast<float>(q)
    };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord4dv(GLenum target, const GLdouble* v)
{
    SPDLOG_DEBUG("glMultiTexCoord4dv 0x{:X} ({}, {}, {}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3]));
    const Vec4 tex {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3])
    };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    SPDLOG_DEBUG("glMultiTexCoord4f 0x{:X} ({}, {}, {}, {}) called",
        target - GL_TEXTURE0, s, t, r, q);
    const Vec4 tex { s, t, r, q };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord4fv(GLenum target, const GLfloat* v)
{
    SPDLOG_DEBUG("glMultiTexCoord4fv 0x{:X} ({}, {}, {}, {}) called",
        target - GL_TEXTURE0, v[0], v[1], v[2], v[3]);
    const Vec4 tex { v[0], v[1], v[2], v[3] };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
    SPDLOG_DEBUG("glMultiTexCoord4i 0x{:X} ({}, {}, {}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        static_cast<float>(q));
    const Vec4 tex {
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        static_cast<float>(q)
    };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord4iv(GLenum target, const GLint* v)
{
    SPDLOG_DEBUG("glMultiTexCoord4iv 0x{:X} ({}, {}, {}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3]));
    const Vec4 tex {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3])
    };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
    SPDLOG_DEBUG("glMultiTexCoord4s 0x{:X} ({}, {}, {}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        static_cast<float>(q));
    const Vec4 tex {
        static_cast<float>(s),
        static_cast<float>(t),
        static_cast<float>(r),
        static_cast<float>(q)
    };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glMultiTexCoord4sv(GLenum target, const GLshort* v)
{
    SPDLOG_DEBUG("glMultiTexCoord4sv 0x{:X} ({}, {}, {}, {}) called",
        target - GL_TEXTURE0,
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3]));
    const Vec4 tex {
        static_cast<float>(v[0]),
        static_cast<float>(v[1]),
        static_cast<float>(v[2]),
        static_cast<float>(v[3])
    };
    RIXGL::getInstance().vertexQueue().setMultiTexCoord(target - GL_TEXTURE0, tex);
    RIXGL::getInstance().vertexArray().setMultiTexCoord(target - GL_TEXTURE0, tex);
}

GLAPI void APIENTRY impl_glLoadTransposeMatrixf(const GLfloat* m)
{
    SPDLOG_WARN("glLoadTransposeMatrixf not implemented");
}

GLAPI void APIENTRY impl_glLoadTransposeMatrixd(const GLdouble* m)
{
    SPDLOG_WARN("glLoadTransposeMatrixd not implemented");
}

GLAPI void APIENTRY impl_glMultTransposeMatrixf(const GLfloat* m)
{
    SPDLOG_WARN("glMultTransposeMatrixf not implemented");
}

GLAPI void APIENTRY impl_glMultTransposeMatrixd(const GLdouble* m)
{
    SPDLOG_WARN("glMultTransposeMatrixd not implemented");
}

// -------------------------------------------------------

// Some extensions
// -------------------------------------------------------
GLAPI void APIENTRY impl_glLockArrays(GLint first, GLsizei count)
{
    SPDLOG_WARN("glLockArrays not implemented");
}

GLAPI void APIENTRY impl_glUnlockArrays()
{
    SPDLOG_WARN("glUnlockArrays not implemented");
}

GLAPI void APIENTRY impl_glActiveStencilFaceEXT(GLenum face)
{
    SPDLOG_DEBUG("impl_glActiveStencilFaceEXT face 0x{:X} called", face);

    if (face == GL_FRONT)
    {
        RIXGL::getInstance().pipeline().getStencil().setStencilFace(StencilFace::FRONT);
    }
    else if (face == GL_BACK)
    {
        RIXGL::getInstance().pipeline().getStencil().setStencilFace(StencilFace::BACK);
    }
    else
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
    }
}

GLAPI void APIENTRY impl_glBlendEquation(GLenum mode)
{
    SPDLOG_WARN("glBlendEquation not implemented");
}

GLAPI void APIENTRY impl_glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    SPDLOG_WARN("glBlendFuncSeparate not implemented");
}

GLAPI void APIENTRY impl_glBindBuffer(GLenum target, GLuint buffer)
{
    SPDLOG_DEBUG("glBindBuffer target 0x{:X} buffer {} called", target, buffer);

    if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER)
    {
        SPDLOG_ERROR("glBindBuffer: invalid target 0x{:X}", target);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    RIXGL::getInstance().vertexBuffer().bindBuffer(buffer);
    if (target == GL_ELEMENT_ARRAY_BUFFER)
    {
        RIXGL::getInstance().vertexArray().bindElementBuffer(buffer);
    }
}

GLAPI void APIENTRY impl_glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
    SPDLOG_DEBUG("glBufferData target 0x{:X} size {} usage 0x{:X} called", target, size, usage);

    if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER)
    {
        SPDLOG_ERROR("glBufferData: invalid target 0x{:X}", target);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    if (usage != GL_STATIC_DRAW && usage != GL_DYNAMIC_DRAW)
    {
        SPDLOG_ERROR("glBufferData: invalid usage 0x{:X}", usage);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    if (size < 0)
    {
        SPDLOG_ERROR("glBufferData: size < 0");
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }

    // Check if buffer 0 is bound (invalid operation)
    if (RIXGL::getInstance().vertexBuffer().getBoundBuffer() == 0)
    {
        SPDLOG_ERROR("glBufferData: buffer 0 is bound (invalid operation)");
        RIXGL::getInstance().setError(GL_INVALID_OPERATION);
        return;
    }

    // Usage is ignored for now

    auto& vb = RIXGL::getInstance().vertexBuffer();
    const bool ret = vb.bufferData(tcb::span<const std::uint8_t>(reinterpret_cast<const std::uint8_t*>(data), size));
    if (!ret)
    {
        SPDLOG_ERROR("glBufferData: out of memory");
        RIXGL::getInstance().setError(GL_OUT_OF_MEMORY);
        return;
    }
}

GLAPI void APIENTRY impl_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
    SPDLOG_DEBUG("glBufferSubData target 0x{:X} offset {} size {} called", target, offset, size);

    if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER)
    {
        SPDLOG_ERROR("glBufferSubData: invalid target 0x{:X}", target);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    if (offset < 0 || size < 0)
    {
        SPDLOG_ERROR("glBufferSubData: offset {} or size {} is negative", offset, size);
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }

    auto& vb = RIXGL::getInstance().vertexBuffer();
    if (vb.getBoundBuffer() == 0)
    {
        SPDLOG_ERROR("glBufferSubData: buffer 0 is bound (invalid operation)");
        RIXGL::getInstance().setError(GL_INVALID_OPERATION);
        return;
    }

    auto bufferData = vb.getBufferData();
    if (static_cast<std::size_t>(offset) + static_cast<std::size_t>(size) > bufferData.first.size())
    {
        SPDLOG_ERROR("glBufferSubData: offset {} + size {} exceeds buffer size {}", offset, size, bufferData.first.size());
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }

    vb.bufferSubData(static_cast<std::size_t>(offset), tcb::span<const std::uint8_t>(reinterpret_cast<const std::uint8_t*>(data), size));
}

GLAPI void APIENTRY impl_glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
    SPDLOG_DEBUG("glDeleteBuffers n {} called", n);

    if (n < 0)
    {
        SPDLOG_ERROR("glDeleteBuffers: n < 0");
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }

    auto& vb = RIXGL::getInstance().vertexBuffer();
    for (GLsizei i = 0; i < n; ++i)
    {
        vb.deleteBuffer(buffers[i]);
    }
}

GLAPI void APIENTRY impl_glGenBuffers(GLsizei n, GLuint* buffers)
{
    SPDLOG_DEBUG("glGenBuffers n {} called", n);

    if (n < 0)
    {
        SPDLOG_ERROR("glGenBuffers: n < 0");
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }

    auto& vb = RIXGL::getInstance().vertexBuffer();
    for (GLsizei i = 0; i < n; ++i)
    {
        buffers[i] = vb.generateBuffer();
        if (buffers[i] == 0)
        {
            SPDLOG_ERROR("glGenBuffers: out of memory");
            RIXGL::getInstance().setError(GL_OUT_OF_MEMORY);
            return;
        }
    }
}

GLAPI void APIENTRY impl_glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    SPDLOG_DEBUG("glGetBufferParameteriv target 0x{:X} pname 0x{:X} called", target, pname);

    auto& vb = RIXGL::getInstance().vertexBuffer();
    if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER)
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        SPDLOG_ERROR("glGetBufferParameteriv: invalid target 0x{:X}", target);
        return;
    }

    if (vb.getBoundBuffer() == 0)
    {
        RIXGL::getInstance().setError(GL_INVALID_OPERATION);
        SPDLOG_ERROR("glGetBufferParameteriv: buffer 0 is bound (invalid operation)");
        return;
    }

    switch (pname)
    {
    case GL_BUFFER_SIZE:
        *params = static_cast<GLint>(vb.getBufferData().first.size());
        break;
    case GL_BUFFER_USAGE:
        SPDLOG_INFO("glGetBufferParameteriv: usage is not tracked, returning GL_STATIC_DRAW");
        *params = GL_STATIC_DRAW;
        break;
    default:
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI GLboolean APIENTRY impl_glIsBuffer(GLuint buffer)
{
    SPDLOG_ERROR("glIsBuffer buffer {} called", buffer);
    return RIXGL::getInstance().vertexBuffer().isBuffer(buffer);
}

GLAPI void APIENTRY impl_glPointSizePointerOES(GLenum type, GLsizei stride, const void* pointer)
{
    SPDLOG_DEBUG("glPointSizeOES type 0x{:X} stride {} pointer {} called", type, stride, reinterpret_cast<std::size_t>(pointer));

    if (type != GL_FLOAT)
    {
        SPDLOG_ERROR("glPointSizeOES: invalid type 0x{:X}", type);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    if (stride < 0)
    {
        SPDLOG_ERROR("glPointSizeOES: stride < 0");
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }

    RIXGL::getInstance().vertexArray().setPointSizeType(convertType(type));
    RIXGL::getInstance().vertexArray().setPointSizeStride(stride);
    if (RIXGL::getInstance().vertexBuffer().isBufferActive())
    {
        const auto data = RIXGL::getInstance().vertexBuffer().getBufferData();
        RIXGL::getInstance().vertexArray().setPointSizePointer(data.first.data(), reinterpret_cast<std::size_t>(pointer), data.second);
    }
    else
    {
        RIXGL::getInstance().vertexArray().setPointSizePointer(pointer, 0, 0);
    }
}
