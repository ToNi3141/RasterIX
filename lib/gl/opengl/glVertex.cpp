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

#pragma GCC diagnostic ignored "-Wunused-parameter"

using namespace rr;

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
