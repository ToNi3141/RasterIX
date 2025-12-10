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

GLAPI void APIENTRY impl_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
    SPDLOG_DEBUG("glReadPixels x {} y {} width {} height {} format 0x{:X} type {:X} called",
        x, y, width, height, format, type);

    if (format != GL_RGBA)
    {
        SPDLOG_WARN("glReadPixels format not supported");
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }
    if (type != GL_UNSIGNED_BYTE)
    {
        SPDLOG_WARN("glReadPixels type not supported");
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }
    if (!((format == GL_RGBA) && (type == GL_UNSIGNED_BYTE)))
    {
        SPDLOG_WARN("glReadPixels only GL_RGBA/GL_UNSIGNED_BYTE is supported");
        RIXGL::getInstance().setError(GL_INVALID_OPERATION);
        return;
    }
    const uint16_t* pixelsDevice = readFromColorBuffer(x, y, width, height, false);
    RIXGL::getInstance().imageConverter().convertRGB565ToPackedRGBA8888(reinterpret_cast<uint8_t*>(pixels), width, height, pixelsDevice);
    delete pixelsDevice;
}