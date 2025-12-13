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
#include "Helpers.hpp"
#include "RIXGL.hpp"
#include "TypeConverters.hpp"
#include "pixelpipeline/PixelPipeline.hpp"
#include "vertexpipeline/VertexArray.hpp"
#include "vertexpipeline/VertexBuffer.hpp"
#include "vertexpipeline/VertexPipeline.hpp"
#include "vertexpipeline/VertexQueue.hpp"
#include <cmath>
#include <cstring>
#include <spdlog/spdlog.h>

using namespace rr;

GLAPI void APIENTRY impl_glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    SPDLOG_DEBUG("glGetBufferParameteriv target 0x{:X} pname 0x{:X} called", target, pname);

    auto& vb = RIXGL::getInstance().vertexBuffer();
    if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER)
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        SPDLOG_ERROR("glGetBufferParameteriv: invalid target 0x{:X}", target);
        return;
    }

    if (vb.getBoundBuffer() == 0)
    {
        RIXGL::getInstance().setError(GL_INVALID_OPERATION);
        SPDLOG_ERROR("glGetBufferParameteriv: buffer 0 is bound (invalid operation)");
        return;
    }

    switch (pname)
    {
    case GL_BUFFER_SIZE:
        *params = static_cast<GLint>(vb.getBufferData().first.size());
        break;
    case GL_BUFFER_USAGE:
        SPDLOG_INFO("glGetBufferParameteriv: usage is not tracked, returning GL_STATIC_DRAW");
        *params = GL_STATIC_DRAW;
        break;
    default:
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}