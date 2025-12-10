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
