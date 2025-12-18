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
#include "vertexpipeline/VertexPipeline.hpp"
#include <spdlog/spdlog.h>

using namespace rr;

GLAPI void APIENTRY impl_glBlendFunc(GLenum srcFactor, GLenum dstFactor)
{
    SPDLOG_DEBUG("glBlendFunc srcFactor 0x{:X} dstFactor 0x{:X} called", srcFactor, dstFactor);

    if (srcFactor == GL_SRC_ALPHA_SATURATE)
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
    }
    else
    {
        RIXGL::getInstance().pipeline().fragmentPipeline().setBlendFuncSFactor(convertBlendFunc(srcFactor));
        RIXGL::getInstance().pipeline().fragmentPipeline().setBlendFuncDFactor(convertBlendFunc(dstFactor));
    }
}