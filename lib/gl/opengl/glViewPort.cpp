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