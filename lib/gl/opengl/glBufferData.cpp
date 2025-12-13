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

GLAPI void APIENTRY impl_glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
    SPDLOG_DEBUG("glBufferData target 0x{:X} size {} usage 0x{:X} called", target, size, usage);

    if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER)
    {
        SPDLOG_ERROR("glBufferData: invalid target 0x{:X}", target);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    if (usage != GL_STATIC_DRAW && usage != GL_DYNAMIC_DRAW)
    {
        SPDLOG_ERROR("glBufferData: invalid usage 0x{:X}", usage);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    if (size < 0)
    {
        SPDLOG_ERROR("glBufferData: size < 0");
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }

    // Check if buffer 0 is bound (invalid operation)
    if (RIXGL::getInstance().vertexBuffer().getBoundBuffer() == 0)
    {
        SPDLOG_ERROR("glBufferData: buffer 0 is bound (invalid operation)");
        RIXGL::getInstance().setError(GL_INVALID_OPERATION);
        return;
    }

    // Usage is ignored for now

    auto& vb = RIXGL::getInstance().vertexBuffer();
    const bool ret = vb.bufferData(tcb::span<const std::uint8_t>(reinterpret_cast<const std::uint8_t*>(data), size));
    if (!ret)
    {
        SPDLOG_ERROR("glBufferData: out of memory");
        RIXGL::getInstance().setError(GL_OUT_OF_MEMORY);
        return;
    }
}