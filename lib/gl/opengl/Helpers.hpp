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

#ifndef GL_HELPERS_HPP_
#define GL_HELPERS_HPP_

#include "RIXGL.hpp"
#include "gl.h"
#include "vertexpipeline/VertexPipeline.hpp"

namespace rr
{

[[maybe_unused]] static const uint16_t* readFromColorBuffer(const GLint x, const GLint y, const GLint width, const GLint height, const bool readFromBackBuffer)
{
    const GLint cbw = RIXGL::getInstance().pipeline().getFramebufferWidth();
    const GLint cbh = RIXGL::getInstance().pipeline().getFramebufferHeight();
    const GLint colorBufferSize = cbw * cbh;

    uint16_t* colorBuffer = new uint16_t[colorBufferSize];
    if (readFromBackBuffer)
    {
        RIXGL::getInstance().pipeline().readBackColorBuffer({ reinterpret_cast<uint8_t*>(colorBuffer), static_cast<std::size_t>(colorBufferSize * 2) });
    }
    else
    {
        RIXGL::getInstance().pipeline().readFrontColorBuffer({ reinterpret_cast<uint8_t*>(colorBuffer), static_cast<std::size_t>(colorBufferSize * 2) });
    }

    uint16_t* texBuffer = new uint16_t[width * height];

    for (GLint ih = 0; ih < height; ih++)
    {
        for (GLint iw = 0; iw < width; iw++)
        {
            GLint cba = ((cbh - ih - y - 1) * cbw) + iw + x;
            cba = std::min(cba, colorBufferSize - 1);
            cba = std::max(0, cba);
            texBuffer[(ih * width) + iw] = colorBuffer[cba];
        }
    }

    delete colorBuffer;

    return texBuffer;
}

} // namespace rr

#endif // GL_HELPERS_HPP_
