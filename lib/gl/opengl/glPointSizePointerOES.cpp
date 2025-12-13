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

GLAPI void APIENTRY impl_glPointSizePointerOES(GLenum type, GLsizei stride, const void* pointer)
{
    SPDLOG_DEBUG("glPointSizeOES type 0x{:X} stride {} pointer {} called", type, stride, reinterpret_cast<std::size_t>(pointer));

    if (type != GL_FLOAT)
    {
        SPDLOG_ERROR("glPointSizeOES: invalid type 0x{:X}", type);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    if (stride < 0)
    {
        SPDLOG_ERROR("glPointSizeOES: stride < 0");
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }

    RIXGL::getInstance().vertexArray().setPointSizeType(convertType(type));
    RIXGL::getInstance().vertexArray().setPointSizeStride(stride);
    if (RIXGL::getInstance().vertexBuffer().isBufferActive())
    {
        const auto data = RIXGL::getInstance().vertexBuffer().getBufferData();
        RIXGL::getInstance().vertexArray().setPointSizePointer(data.first.data(), reinterpret_cast<std::size_t>(pointer), data.second);
    }
    else
    {
        RIXGL::getInstance().vertexArray().setPointSizePointer(pointer, 0, 0);
    }
}