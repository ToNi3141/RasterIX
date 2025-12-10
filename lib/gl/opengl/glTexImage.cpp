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

GLAPI void APIENTRY impl_glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    SPDLOG_WARN("glTexImage1D not implemented");
}

GLAPI void APIENTRY impl_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    SPDLOG_DEBUG("glTexImage2D target 0x{:X} level 0x{:X} internalformat 0x{:X} width {} height {} border 0x{:X} format 0x{:X} type 0x{:X} called", target, level, internalformat, width, height, border, format, type);

    // Border is not supported in OpenGL ES and is ignored. What does border mean: https://stackoverflow.com/questions/913801/what-does-border-mean-in-the-glteximage2d-function
    if ((border != 0) || (level < 0))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glTexImage2D border or level invalid");
        return;
    }

    const std::size_t maxTexSize { RIXGL::getInstance().getMaxTextureSize() };

    if (static_cast<std::size_t>(level) > RIXGL::getInstance().getMaxLOD())
    {
        SPDLOG_ERROR("glTexImage2D invalid lod.");
        return;
    }

    if ((static_cast<std::size_t>(width) > maxTexSize) || (static_cast<std::size_t>(height) > maxTexSize))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glTexImage2D texture is too big.");
        return;
    }

    if (!RIXGL::getInstance().isMipmappingAvailable() && (level != 0))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glTexImage2D mipmapping on hardware not supported.");
        return;
    }

    // It can happen, that a not power of two texture is used. This little hack allows that the texture can sill be used
    // without crashing the software. But it is likely that it will produce graphical errors.
    const std::size_t widthRounded = powf(2.0f, ceilf(logf(width) / logf(2.0f)));
    const std::size_t heightRounded = powf(2.0f, ceilf(logf(height) / logf(2.0f)));

    if ((widthRounded == 0) || (heightRounded == 0))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glTexImage2D texture with invalid size detected ({} (rounded to {}), {} (rounded to {}))", width, widthRounded, height, heightRounded);
        return;
    }

    InternalPixelFormat internalPixelFormat;
    const GLenum error = ImageConverter::convertInternalPixelFormat(internalPixelFormat, internalformat);
    if (error != GL_NO_ERROR)
    {
        RIXGL::getInstance().setError(error);
        SPDLOG_ERROR("glTexImage2D internal Pixel Format 0x{:X} not supported", internalformat);
        return;
    }

    TextureObject& texObj { RIXGL::getInstance().pipeline().texture().getTexture() };

    texObj.setWidth(level, widthRounded);
    texObj.setHeight(level, heightRounded);
    texObj.setInternalPixelFormat(level, internalPixelFormat);

    SPDLOG_DEBUG("glTexImage2D redirect to glTexSubImage2D");
    impl_glTexSubImage2D(target, level, 0, 0, width, height, format, type, pixels);
}

GLAPI void APIENTRY impl_glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* data)
{
    SPDLOG_WARN("glTexImage3D not implemented");
}