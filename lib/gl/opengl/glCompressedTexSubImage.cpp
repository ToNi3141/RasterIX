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
#include <spdlog/spdlog.h>

GLAPI void APIENTRY impl_glCompressedTexSubImage3D(
    [[maybe_unused]] GLenum target,
    [[maybe_unused]] GLint level,
    [[maybe_unused]] GLint xoffset,
    [[maybe_unused]] GLint yoffset,
    [[maybe_unused]] GLint zoffset,
    [[maybe_unused]] GLsizei width,
    [[maybe_unused]] GLsizei height,
    [[maybe_unused]] GLsizei depth,
    [[maybe_unused]] GLenum format,
    [[maybe_unused]] GLsizei imageSize,
    [[maybe_unused]] const GLvoid* data)
{
    SPDLOG_WARN("glCompressedTexSubImage3D not implemented");
}

GLAPI void APIENTRY impl_glCompressedTexSubImage2D(
    [[maybe_unused]] GLenum target,
    [[maybe_unused]] GLint level,
    [[maybe_unused]] GLint xoffset,
    [[maybe_unused]] GLint yoffset,
    [[maybe_unused]] GLsizei width,
    [[maybe_unused]] GLsizei height,
    [[maybe_unused]] GLenum format,
    [[maybe_unused]] GLsizei imageSize,
    [[maybe_unused]] const GLvoid* data)
{
    SPDLOG_WARN("glCompressedTexSubImage2D not implemented");
}

GLAPI void APIENTRY impl_glCompressedTexSubImage1D(
    [[maybe_unused]] GLenum target,
    [[maybe_unused]] GLint level,
    [[maybe_unused]] GLint xoffset,
    [[maybe_unused]] GLsizei width,
    [[maybe_unused]] GLenum format,
    [[maybe_unused]] GLsizei imageSize,
    [[maybe_unused]] const GLvoid* data)
{
    SPDLOG_WARN("glCompressedTexSubImage1D not implemented");
}