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
#include "ImageConverter.hpp"
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

GLAPI void APIENTRY impl_glPixelStoref(GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glPixelStoref redirected to glPixelStorei");
    impl_glPixelStorei(pname, static_cast<GLint>(param));
}

GLAPI void APIENTRY impl_glPixelStorei(GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glPixelStorei pname 0x{:X} param 0x{:X} called", pname, param);
    switch (param)
    {
    case 1:
    case 2:
    case 4:
    case 8:
        if (pname == GL_UNPACK_ALIGNMENT)
        {
            RIXGL::getInstance().imageConverter().setUnpackAlignment(param);
        }
        else if (pname == GL_PACK_ALIGNMENT)
        {
            RIXGL::getInstance().imageConverter().setPackAlignment(param);
        }
        else
        {
            SPDLOG_WARN("glPixelStorei pname 0x{:X} and param 0x{:X} not supported", pname, param);
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
        }
        break;
    default:
        SPDLOG_ERROR("glPixelStorei pname GL_PACK_ALIGNMENT and param 0x{:X} not supported", param);
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        break;
    }
}