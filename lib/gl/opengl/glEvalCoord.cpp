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

GLAPI void APIENTRY impl_glEvalCoord1d([[maybe_unused]] GLdouble u)
{
    SPDLOG_WARN("glEvalCoord1d not implemented");
}

GLAPI void APIENTRY impl_glEvalCoord1dv([[maybe_unused]] const GLdouble* u)
{
    SPDLOG_WARN("glEvalCoord1dv not implemented");
}

GLAPI void APIENTRY impl_glEvalCoord1f([[maybe_unused]] GLfloat u)
{
    SPDLOG_WARN("glEvalCoord1f not implemented");
}

GLAPI void APIENTRY impl_glEvalCoord1fv([[maybe_unused]] const GLfloat* u)
{
    SPDLOG_WARN("glEvalCoord1fv not implemented");
}

GLAPI void APIENTRY impl_glEvalCoord2d([[maybe_unused]] GLdouble u, [[maybe_unused]] GLdouble v)
{
    SPDLOG_WARN("glEvalCoord2d not implemented");
}

GLAPI void APIENTRY impl_glEvalCoord2dv([[maybe_unused]] const GLdouble* u)
{
    SPDLOG_WARN("glEvalCoord2dv not implemented");
}

GLAPI void APIENTRY impl_glEvalCoord2f([[maybe_unused]] GLfloat u, [[maybe_unused]] GLfloat v)
{
    SPDLOG_WARN("glEvalCoord2f not implemented");
}

GLAPI void APIENTRY impl_glEvalCoord2fv([[maybe_unused]] const GLfloat* u)
{
    SPDLOG_WARN("glEvalCoord2fv not implemented");
}