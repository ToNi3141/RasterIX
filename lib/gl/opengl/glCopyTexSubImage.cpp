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

GLAPI void APIENTRY impl_glCopyTexSubImage1D(
    [[maybe_unused]] GLenum target,
    [[maybe_unused]] GLint level,
    [[maybe_unused]] GLint xoffset,
    [[maybe_unused]] GLint x,
    [[maybe_unused]] GLint y,
    [[maybe_unused]] GLsizei width)
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

GLAPI void APIENTRY impl_glCopyTexSubImage3D(
    [[maybe_unused]] GLenum target,
    [[maybe_unused]] GLint level,
    [[maybe_unused]] GLint xoffset,
    [[maybe_unused]] GLint yoffset,
    [[maybe_unused]] GLint zoffset,
    [[maybe_unused]] GLint x,
    [[maybe_unused]] GLint y,
    [[maybe_unused]] GLsizei width,
    [[maybe_unused]] GLsizei height)
{
    SPDLOG_WARN("glCopyTexSubImage3D not implemented");
}