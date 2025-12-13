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

GLAPI void APIENTRY impl_glNormalPointer(GLenum type, GLsizei stride, const GLvoid* pointer)
{
    SPDLOG_DEBUG("glNormalPointer type 0x{:X} stride {} called", type, stride);

    RIXGL::getInstance().vertexArray().setNormalType(convertType(type));
    RIXGL::getInstance().vertexArray().setNormalStride(stride);
    if (RIXGL::getInstance().vertexBuffer().isBufferActive())
    {
        const auto data = RIXGL::getInstance().vertexBuffer().getBufferData();
        RIXGL::getInstance().vertexArray().setNormalPointer(data.first.data(), reinterpret_cast<std::size_t>(pointer), data.second);
    }
    else
    {
        RIXGL::getInstance().vertexArray().setNormalPointer(pointer, 0, 0);
    }
}