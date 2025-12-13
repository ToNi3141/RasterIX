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
#include "vertexpipeline/VertexArray.hpp"
#include "vertexpipeline/VertexPipeline.hpp"
#include "vertexpipeline/VertexQueue.hpp"
#include <spdlog/spdlog.h>

using namespace rr;

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
