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

GLAPI void APIENTRY impl_glBindTexture(GLenum target, GLuint texture)
{
    SPDLOG_DEBUG("glBindTexture target 0x{:X} texture 0x{:X}", target, texture);
    if (target != GL_TEXTURE_2D)
    {
        SPDLOG_WARN("glBindTexture target not supported");
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    if (!RIXGL::getInstance().pipeline().texture().isTextureValid(texture))
    {
        bool ret { RIXGL::getInstance().pipeline().texture().createTextureWithName(texture) };
        if (!ret)
        {
            // TODO: Free allocated textures to avoid leaks
            RIXGL::getInstance().setError(GL_OUT_OF_MEMORY);
            SPDLOG_ERROR("glBindTexture cannot create texture {}", texture);
            return;
        }
    }

    RIXGL::getInstance().pipeline().texture().setBoundTexture(texture);
    RIXGL::getInstance().pipeline().texture().useTexture();
}
