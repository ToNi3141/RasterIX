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

using namespace rr;

GLAPI void APIENTRY impl_glTexSubImage1D(
    [[maybe_unused]] GLenum target,
    [[maybe_unused]] GLint level,
    [[maybe_unused]] GLint xoffset,
    [[maybe_unused]] GLsizei width,
    [[maybe_unused]] GLenum format,
    [[maybe_unused]] GLenum type,
    [[maybe_unused]] const GLvoid* pixels)
{
    SPDLOG_WARN("glTexSubImage1D not implemented");
}

GLAPI void APIENTRY impl_glTexSubImage2D(
    [[maybe_unused]] GLenum target,
    GLint level,
    GLint xoffset,
    GLint yoffset,
    GLsizei width,
    GLsizei height,
    GLenum format,
    GLenum type,
    const GLvoid* pixels)
{
    SPDLOG_DEBUG("glTexSubImage2D target 0x{:X} level 0x{:X} xoffset {} yoffset {} width {} height {} format 0x{:X} type 0x{:X} called", target, level, xoffset, yoffset, width, height, format, type);

    if (static_cast<std::size_t>(level) > RIXGL::getInstance().getMaxLOD())
    {
        SPDLOG_ERROR("glTexSubImage2D invalid lod.");
        return;
    }

    if (!RIXGL::getInstance().isMipmappingAvailable() && (level != 0))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glTexSubImage2D mipmapping on hardware not supported.");
        return;
    }

    TextureObject& texObj { RIXGL::getInstance().pipeline().texture().getTexture() };
    if (((xoffset + width) > static_cast<GLint>(texObj.getWidth(level)))
        || ((yoffset + height) > static_cast<GLint>(texObj.getHeight(level))))
    {
        RIXGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_ERROR("glTexSubImage2D offsets or texture sizes are too big");
        return;
    }

    using PixelType = TextureObject::PixelsType::element_type;

    const std::size_t texMemSize = texObj.getWidth(level) * texObj.getHeight(level) * sizeof(PixelType);
    if (texMemSize == 0)
    {
        SPDLOG_DEBUG("glTexSubImage2D texture with zero dimensions loaded.");
        return;
    }

    std::shared_ptr<PixelType> texMemShared(
        new PixelType[texMemSize / sizeof(PixelType)],
        [](const PixelType* p)
        { delete[] p; });

    if (!texMemShared)
    {
        RIXGL::getInstance().setError(GL_OUT_OF_MEMORY);
        SPDLOG_ERROR("glTexSubImage2D Out Of Memory");
        return;
    }
    if (texObj.getPixels(level))
    {
        std::memcpy(texMemShared.get(), texObj.getPixels(level).get(), std::min(texObj.getSizeInBytes(level), texMemSize));
    }
    else
    {
        std::memset(texMemShared.get(), 0, texMemSize);
    }
    texObj.setPixels(level, texMemShared);

    // Check if pixels is null. If so, just set the empty memory area and don't copy anything.
    if (pixels != nullptr)
    {
        RIXGL::getInstance().imageConverter().convertUnpack(
            texObj.getPixels(level),
            texObj.getInternalPixelFormat(level),
            texObj.getWidth(level),
            xoffset,
            yoffset,
            width,
            height,
            format,
            type,
            reinterpret_cast<const uint8_t*>(pixels));

        RIXGL::getInstance().mipMapGenerator().generateMipMap(RIXGL::getInstance().pipeline().texture().getTexture(), level);
    }
}

GLAPI void APIENTRY impl_glTexSubImage3D(
    [[maybe_unused]] GLenum target,
    [[maybe_unused]] GLint level,
    [[maybe_unused]] GLint xoffset,
    [[maybe_unused]] GLint yoffset,
    [[maybe_unused]] GLint zoffset,
    [[maybe_unused]] GLsizei width,
    [[maybe_unused]] GLsizei height,
    [[maybe_unused]] GLsizei depth,
    [[maybe_unused]] GLenum format,
    [[maybe_unused]] GLenum type,
    [[maybe_unused]] const GLvoid* pixels)
{
    SPDLOG_WARN("glTexSubImage3D not implemented");
}