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

GLAPI void APIENTRY impl_glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    SPDLOG_WARN("glCopyTexImage1D not implemented");
}

GLAPI void APIENTRY impl_glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    SPDLOG_DEBUG("glCopyTexImage2D target 0x{:X} level 0x{:X} internalformat 0x{:X} x {} y {} width {} height {} border 0x{:X} called",
        target, level, internalformat, x, y, width, height, border);

    InternalPixelFormat internalPixelFormat;
    const GLenum error = ImageConverter::convertInternalPixelFormat(internalPixelFormat, internalformat);
    if (error != GL_NO_ERROR)
    {
        RIXGL::getInstance().setError(error);
        return;
    }

    switch (internalPixelFormat)
    {
    case InternalPixelFormat::ALPHA:
    case InternalPixelFormat::LUMINANCE_ALPHA:
    case InternalPixelFormat::RGBA:
    case InternalPixelFormat::RGBA1:
        RIXGL::getInstance().setError(GL_INVALID_OPERATION);
        SPDLOG_ERROR("glCopyTexImage2D invalid internal format used (framebuffer does not support an alpha channel)");
        return;
    default:
        break;
    }

    const GLsizei maxTexSize { static_cast<GLsizei>(RIXGL::getInstance().getMaxTextureSize()) };
    if ((width > maxTexSize) || (height > maxTexSize))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glCopyTexImage2D texture is too big.");
        return;
    }

    const uint16_t* texBuffer = readFromColorBuffer(x, y, width, height, true);

    SPDLOG_DEBUG("glCopyTexImage2D redirect to glTexImage2D");
    impl_glTexImage2D(target, level, internalformat, width, height, border, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, texBuffer);

    delete texBuffer;
}
