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

using namespace rr;

GLAPI void APIENTRY impl_glIndexd([[maybe_unused]] GLdouble c)
{
    SPDLOG_WARN("glIndexd not implemented");
}

GLAPI void APIENTRY impl_glIndexdv([[maybe_unused]] const GLdouble* c)
{
    SPDLOG_WARN("glIndexdv not implemented");
}

GLAPI void APIENTRY impl_glIndexf([[maybe_unused]] GLfloat c)
{
    SPDLOG_WARN("glIndexf not implemented");
}

GLAPI void APIENTRY impl_glIndexfv([[maybe_unused]] const GLfloat* c)
{
    SPDLOG_WARN("glIndexfv not implemented");
}

GLAPI void APIENTRY impl_glIndexi([[maybe_unused]] GLint c)
{
    SPDLOG_WARN("glIndexi not implemented");
}

GLAPI void APIENTRY impl_glIndexiv([[maybe_unused]] const GLint* c)
{
    SPDLOG_WARN("glIndexiv not implemented");
}

GLAPI void APIENTRY impl_glIndexs([[maybe_unused]] GLshort c)
{
    SPDLOG_WARN("glIndexs not implemented");
}

GLAPI void APIENTRY impl_glIndexsv([[maybe_unused]] const GLshort* c)
{
    SPDLOG_WARN("glIndexsv not implemented");
}

GLAPI void APIENTRY impl_glIndexub([[maybe_unused]] GLubyte c)
{
    SPDLOG_WARN("glIndexub not implemented");
}

GLAPI void APIENTRY impl_glIndexubv([[maybe_unused]] const GLubyte* c)
{
    SPDLOG_WARN("glIndexubv not implemented");
}
