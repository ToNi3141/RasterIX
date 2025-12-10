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

GLAPI void APIENTRY impl_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
    SPDLOG_DEBUG("glBufferSubData target 0x{:X} offset {} size {} called", target, offset, size);

    if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER)
    {
        SPDLOG_ERROR("glBufferSubData: invalid target 0x{:X}", target);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    if (offset < 0 || size < 0)
    {
        SPDLOG_ERROR("glBufferSubData: offset {} or size {} is negative", offset, size);
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }

    auto& vb = RIXGL::getInstance().vertexBuffer();
    if (vb.getBoundBuffer() == 0)
    {
        SPDLOG_ERROR("glBufferSubData: buffer 0 is bound (invalid operation)");
        RIXGL::getInstance().setError(GL_INVALID_OPERATION);
        return;
    }

    auto bufferData = vb.getBufferData();
    if (static_cast<std::size_t>(offset) + static_cast<std::size_t>(size) > bufferData.first.size())
    {
        SPDLOG_ERROR("glBufferSubData: offset {} + size {} exceeds buffer size {}", offset, size, bufferData.first.size());
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }

    vb.bufferSubData(static_cast<std::size_t>(offset), tcb::span<const std::uint8_t>(reinterpret_cast<const std::uint8_t*>(data), size));
}