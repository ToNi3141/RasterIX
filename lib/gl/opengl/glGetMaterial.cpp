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
#include "Helpers.hpp"
#include "RIXGL.hpp"
#include "TypeConverters.hpp"
#include "pixelpipeline/PixelPipeline.hpp"
#include "vertexpipeline/VertexArray.hpp"
#include "vertexpipeline/VertexBuffer.hpp"
#include "vertexpipeline/VertexPipeline.hpp"
#include "vertexpipeline/VertexQueue.hpp"
#include <cmath>
#include <cstring>
#include <spdlog/spdlog.h>

using namespace rr;

GLAPI void APIENTRY impl_glGetMaterialfv(GLenum face, GLenum pname, GLfloat* params)
{
    SPDLOG_WARN("glGetMaterialfv called for face 0x{:X} pname 0x{:X} not implemented", face, pname);

    if (face != GL_FRONT_AND_BACK)
    {
        SPDLOG_WARN("glGetMaterialfv only GL_FRONT_AND_BACK supported");
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    const auto& lighting = RIXGL::getInstance().pipeline().getLighting();

    switch (pname)
    {
    case GL_AMBIENT:
    {
        const Vec4& v = lighting.getAmbientColorMaterial();
        std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
        break;
    }
    case GL_DIFFUSE:
    {
        const Vec4& v = lighting.getDiffuseColorMaterial();
        std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
        break;
    }
    case GL_SPECULAR:
    {
        const Vec4& v = lighting.getSpecularColorMaterial();
        std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
        break;
    }
    case GL_EMISSION:
    {
        const Vec4& v = lighting.getEmissionColorMaterial();
        std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
        break;
    }
    case GL_SHININESS:
        params[0] = lighting.getSpecularExponentMaterial();
        break;

    default:
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        SPDLOG_ERROR("glGetMaterialfv pname 0x{:X} not supported", pname);
        break;
    }
}

GLAPI void APIENTRY impl_glGetMaterialiv(
    [[maybe_unused]] GLenum face,
    [[maybe_unused]] GLenum pname,
    [[maybe_unused]] GLint* params)
{
    SPDLOG_WARN("glGetMaterialiv not implemented");
}
