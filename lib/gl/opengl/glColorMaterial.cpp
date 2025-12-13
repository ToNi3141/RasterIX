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

GLAPI void APIENTRY impl_glColorMaterial(GLenum face, GLenum mode)
{
    SPDLOG_DEBUG("glColorMaterial face 0x{:X} mode 0x{:X} called", face, mode);

    bool error = false;
    Face faceConverted {};
    switch (face)
    {
    case GL_FRONT:
        faceConverted = Face::FRONT;
        break;
    case GL_BACK:
        faceConverted = Face::BACK;
        break;
    case GL_FRONT_AND_BACK:
        faceConverted = Face::FRONT_AND_BACK;
        break;
    default:
        SPDLOG_WARN("glColorMaterial face 0x{:X} not supported", face);
        faceConverted = Face::FRONT_AND_BACK;
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        error = true;
        break;
    }

    if (!error)
    {
        switch (mode)
        {
        case GL_AMBIENT:
            RIXGL::getInstance().pipeline().getLighting().setColorMaterialTracking(faceConverted, ColorMaterialTracking::AMBIENT);
            break;
        case GL_DIFFUSE:
            RIXGL::getInstance().pipeline().getLighting().setColorMaterialTracking(faceConverted, ColorMaterialTracking::DIFFUSE);
            break;
        case GL_AMBIENT_AND_DIFFUSE:
            RIXGL::getInstance().pipeline().getLighting().setColorMaterialTracking(faceConverted, ColorMaterialTracking::AMBIENT_AND_DIFFUSE);
            break;
        case GL_SPECULAR:
            RIXGL::getInstance().pipeline().getLighting().setColorMaterialTracking(faceConverted, ColorMaterialTracking::SPECULAR);
            break;
        case GL_EMISSION:
            RIXGL::getInstance().pipeline().getLighting().setColorMaterialTracking(faceConverted, ColorMaterialTracking::EMISSION);
            break;
        default:
            SPDLOG_WARN("glColorMaterial mode 0x{:X} not supported", mode);
            RIXGL::getInstance().setError(GL_INVALID_ENUM);
            RIXGL::getInstance().pipeline().getLighting().setColorMaterialTracking(faceConverted, ColorMaterialTracking::AMBIENT_AND_DIFFUSE);
            break;
        }
    }
}