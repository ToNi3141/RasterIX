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

#pragma GCC diagnostic ignored "-Wunused-parameter"

using namespace rr;

GLAPI void APIENTRY impl_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    SPDLOG_DEBUG("glStencilOp fail 0x{:X} zfail 0x{:X} zpass 0x{:X} called", fail, zfail, zpass);

    StencilOp failOp;
    StencilOp zfailOp;
    StencilOp zpassOp;

    const GLenum error0 = convertStencilOp(failOp, fail);
    const GLenum error1 = convertStencilOp(zfailOp, zfail);
    const GLenum error2 = convertStencilOp(zpassOp, zpass);

    if ((error0 == GL_NO_ERROR) && (error1 == GL_NO_ERROR) == (error2 == GL_NO_ERROR))
    {
        RIXGL::getInstance().pipeline().getStencil().setOpFail(failOp);
        RIXGL::getInstance().pipeline().getStencil().setOpZFail(zfailOp);
        RIXGL::getInstance().pipeline().getStencil().setOpZPass(zpassOp);
    }
    else
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
    }
}