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

GLAPI void APIENTRY impl_glLogicOp(GLenum opcode)
{
    SPDLOG_DEBUG("glLogicOp 0x{:X} called", opcode);

    LogicOp logicOp;
    const GLenum error = convertLogicOp(logicOp, opcode);
    if (error == GL_NO_ERROR)
    {
        RIXGL::getInstance().pipeline().fragmentPipeline().setLogicOp(logicOp);
    }
    else
    {
        RIXGL::getInstance().setError(error);
    }
}
