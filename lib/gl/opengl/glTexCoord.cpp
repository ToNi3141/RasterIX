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