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

GLAPI void APIENTRY impl_glBindBuffer(GLenum target, GLuint buffer)
{
    SPDLOG_DEBUG("glBindBuffer target 0x{:X} buffer {} called", target, buffer);

    if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER)
    {
        SPDLOG_ERROR("glBindBuffer: invalid target 0x{:X}", target);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    RIXGL::getInstance().vertexBuffer().bindBuffer(buffer);
    if (target == GL_ELEMENT_ARRAY_BUFFER)
    {
        RIXGL::getInstance().vertexArray().bindElementBuffer(buffer);
    }
}