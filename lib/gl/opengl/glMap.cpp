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

GLAPI void APIENTRY impl_glMap1d(
    [[maybe_unused]] GLenum target,
    [[maybe_unused]] GLdouble u1,
    [[maybe_unused]] GLdouble u2,
    [[maybe_unused]] GLint stride,
    [[maybe_unused]] GLint order,
    [[maybe_unused]] const GLdouble* points)
{
    SPDLOG_WARN("glMap1d not implemented");
}

GLAPI void APIENTRY impl_glMap1f(
    [[maybe_unused]] GLenum target,
    [[maybe_unused]] GLfloat u1,
    [[maybe_unused]] GLfloat u2,
    [[maybe_unused]] GLint stride,
    [[maybe_unused]] GLint order,
    [[maybe_unused]] const GLfloat* points)
{
    SPDLOG_WARN("glMap1f not implemented");
}

GLAPI void APIENTRY impl_glMap2d(
    [[maybe_unused]] GLenum target,
    [[maybe_unused]] GLdouble u1,
    [[maybe_unused]] GLdouble u2,
    [[maybe_unused]] GLint ustride,
    [[maybe_unused]] GLint uorder,
    [[maybe_unused]] GLdouble v1,
    [[maybe_unused]] GLdouble v2,
    [[maybe_unused]] GLint vstride,
    [[maybe_unused]] GLint vorder,
    [[maybe_unused]] const GLdouble* points)
{
    SPDLOG_WARN("glMap2d not implemented");
}

GLAPI void APIENTRY impl_glMap2f(
    [[maybe_unused]] GLenum target,
    [[maybe_unused]] GLfloat u1,
    [[maybe_unused]] GLfloat u2,
    [[maybe_unused]] GLint ustride,
    [[maybe_unused]] GLint uorder,
    [[maybe_unused]] GLfloat v1,
    [[maybe_unused]] GLfloat v2,
    [[maybe_unused]] GLint vstride,
    [[maybe_unused]] GLint vorder,
    [[maybe_unused]] const GLfloat* points)
{
    SPDLOG_WARN("glMap2f not implemented");
}
