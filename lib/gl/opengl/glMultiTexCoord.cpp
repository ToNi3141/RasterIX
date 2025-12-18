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