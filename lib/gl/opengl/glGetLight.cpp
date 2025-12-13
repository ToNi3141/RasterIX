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
#include "vertexpipeline/VertexPipeline.hpp"
#include <spdlog/spdlog.h>

using namespace rr;

GLAPI void APIENTRY impl_glGetLightfv(GLenum light, GLenum pname, GLfloat* params)
{
    SPDLOG_DEBUG("glGetLightfv light 0x{:X} pname 0x{:X} called", light, pname);

    if (light < GL_LIGHT0 || light > GL_LIGHT7)
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    const auto& lighting = RIXGL::getInstance().pipeline().getLighting();
    const int idx = light - GL_LIGHT0;

    switch (pname)
    {
    case GL_AMBIENT:
    {
        const Vec4& v = lighting.getAmbientColorLight(idx);
        std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
    }
    break;
    case GL_DIFFUSE:
    {
        const Vec4& v = lighting.getDiffuseColorLight(idx);
        std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
    }
    break;
    case GL_SPECULAR:
    {
        const Vec4& v = lighting.getSpecularColorLight(idx);
        std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
    }
    break;
    case GL_POSITION:
    {
        const Vec4& v = lighting.getPosLight(idx);
        std::memcpy(params, v.data(), 4 * sizeof(GLfloat));
    }
    break;
    case GL_SPOT_DIRECTION:
    {
        const Vec3& v = lighting.getSpotlightDirection(idx);
        std::memcpy(params, v.data(), 3 * sizeof(GLfloat));
    }
    break;
    case GL_SPOT_EXPONENT:
        params[0] = lighting.getSpotlightExponent(idx);
        break;
    case GL_SPOT_CUTOFF:
        params[0] = lighting.getSpotlightCutoff(idx);
        break;
    case GL_CONSTANT_ATTENUATION:
        params[0] = lighting.getConstantAttenuationLight(idx);
        break;
    case GL_LINEAR_ATTENUATION:
        params[0] = lighting.getLinearAttenuationLight(idx);
        break;
    case GL_QUADRATIC_ATTENUATION:
        params[0] = lighting.getQuadraticAttenuationLight(idx);
        break;
    default:
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI void APIENTRY impl_glGetLightiv(
    [[maybe_unused]] GLenum light,
    [[maybe_unused]] GLenum pname,
    [[maybe_unused]] GLint* params)
{
    SPDLOG_WARN("glGetLightiv not implemented");
}