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

#ifndef GL_MIP_MAP_GENERATOR_HPP_
#define GL_MIP_MAP_GENERATOR_HPP_

#include "Enums.hpp"
#include "ImageConverter.hpp"
#include "RIXGL.hpp"
#include "gl.h"
#include "renderer/TextureObject.hpp"
#include <algorithm>
#include <cstring>
#include <spdlog/spdlog.h>

namespace rr
{

class MipMapGenerator
{
public:
    void generateMipMap(TextureObject& textureObject, const std::size_t baseLevel)
    {
        if (m_enableMipMapGeneration)
        {
            mipMapCreator(textureObject, baseLevel);
        }
    }

    void setEnableMipMapGeneration(const bool enableGeneration) { m_enableMipMapGeneration = enableGeneration; }

private:
    static void mipMapCreator(TextureObject& textureObject, const std::size_t baseLevel)
    {
        using PixelType = TextureObject::PixelsType::element_type;

        const std::size_t levels = textureObject.getLevels();
        for (std::size_t i = baseLevel; i < levels - 1; i++)
        {
            if ((textureObject.getWidth(i) == 1) && (textureObject.getHeight(i) == 1))
            {
                // No more mip map reduction is available. The 1x1 texture is reached
                return;
            }

            std::shared_ptr<PixelType> texMemShared(
                new PixelType[textureObject.getSizeInBytes(i + 1) / sizeof(PixelType)],
                [](const PixelType* p)
                { delete[] p; });

            if (!texMemShared)
            {
                RIXGL::getInstance().setError(GL_OUT_OF_MEMORY);
                SPDLOG_ERROR("mipMapCreator Out Of Memory");
                return;
            }
            textureObject.setPixels(i + 1, texMemShared);

            ImageConverter::computeMipMapLevel(
                textureObject.getPixels(i + 1),
                textureObject.getInternalPixelFormat(i),
                textureObject.getWidth(i),
                textureObject.getHeight(i),
                textureObject.getPixels(i));
        }
    }

    bool m_enableMipMapGeneration { false };
};
} // namespace rr

#endif // GL_MIP_MAP_GENERATOR_HPP_
