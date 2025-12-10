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