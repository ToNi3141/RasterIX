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

#include "ImageConverter.hpp"
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

#pragma GCC diagnostic ignored "-Wunused-parameter"

using namespace rr;

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