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
#include "RIXGL.hpp"
#include "TypeConverters.hpp"
#include "vertexpipeline/VertexArray.hpp"
#include "vertexpipeline/VertexBuffer.hpp"
#include "vertexpipeline/VertexPipeline.hpp"
#include <spdlog/spdlog.h>

using namespace rr;

GLAPI void APIENTRY impl_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
    SPDLOG_DEBUG("glDrawElements mode 0x{:X} count {} type 0x{:X} called", mode, count, type);
    bool error = false;
    switch (type)
    {
    case GL_UNSIGNED_BYTE:
        RIXGL::getInstance().vertexArray().setIndicesType(Type::BYTE);
        break;
    case GL_UNSIGNED_SHORT:
        RIXGL::getInstance().vertexArray().setIndicesType(Type::UNSIGNED_SHORT);
        break;
    case GL_UNSIGNED_INT:
        RIXGL::getInstance().vertexArray().setIndicesType(Type::UNSIGNED_INT);
        break;
    default:
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        error = true;
        SPDLOG_WARN("glDrawElements type 0x{:X} not supported", type);
        return;
    }

    RIXGL::getInstance().vertexArray().reset();

    RIXGL::getInstance().vertexArray().setCount(count);
    RIXGL::getInstance().vertexArray().setDrawMode(convertDrawMode(mode));
    RIXGL::getInstance().vertexArray().enableIndices(true);

    if (RIXGL::getInstance().vertexBuffer().isBufferActive())
    {
        const auto data = RIXGL::getInstance().vertexBuffer().getBufferData();
        RIXGL::getInstance().vertexArray().setIndicesPointer(
            data.first.last(data.first.size() - reinterpret_cast<std::size_t>(indices)).data());
    }
    else
    {
        RIXGL::getInstance().vertexArray().setIndicesPointer(indices);
    }

    if (!error)
    {
        RIXGL::getInstance().pipeline().drawObj(RIXGL::getInstance().vertexArray().renderObj());
    }
}