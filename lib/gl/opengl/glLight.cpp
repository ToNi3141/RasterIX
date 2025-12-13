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

GLAPI void APIENTRY impl_glLightf(GLenum light, GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glLightf light 0x{:X} pname 0x{:X} param {} called", light - GL_LIGHT0, pname, param);

    if (light > GL_LIGHT7)
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
    }

    switch (pname)
    {
    case GL_SPOT_EXPONENT:
        RIXGL::getInstance().pipeline().getLighting().setSpotlightExponent(light - GL_LIGHT0, param);
        break;
    case GL_SPOT_CUTOFF:
        RIXGL::getInstance().pipeline().getLighting().setSpotlightCutoff(light - GL_LIGHT0, param);
        break;
    case GL_CONSTANT_ATTENUATION:
        RIXGL::getInstance().pipeline().getLighting().setConstantAttenuationLight(light - GL_LIGHT0, param);
        break;
    case GL_LINEAR_ATTENUATION:
        RIXGL::getInstance().pipeline().getLighting().setLinearAttenuationLight(light - GL_LIGHT0, param);
        break;
    case GL_QUADRATIC_ATTENUATION:
        RIXGL::getInstance().pipeline().getLighting().setQuadraticAttenuationLight(light - GL_LIGHT0, param);
        break;
    default:
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI void APIENTRY impl_glLightfv(GLenum light, GLenum pname, const GLfloat* params)
{
    SPDLOG_DEBUG("glLightfv light 0x{:X} pname 0x{:X}", light - GL_LIGHT0, pname);

    if (light > GL_LIGHT7)
    {
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    switch (pname)
    {
    case GL_AMBIENT:
        RIXGL::getInstance().pipeline().getLighting().setAmbientColorLight(light - GL_LIGHT0, { params });
        break;
    case GL_DIFFUSE:
        RIXGL::getInstance().pipeline().getLighting().setDiffuseColorLight(light - GL_LIGHT0, { params });
        break;
    case GL_SPECULAR:
        RIXGL::getInstance().pipeline().getLighting().setSpecularColorLight(light - GL_LIGHT0, { params });
        break;
    case GL_POSITION:
    {
        Vec4 lightPos { params };
        Vec4 lightPosTransformed {};
        RIXGL::getInstance().pipeline().getMatrixStore().getModelView().transform(lightPosTransformed, lightPos);
        RIXGL::getInstance().pipeline().getLighting().setPosLight(light - GL_LIGHT0, lightPosTransformed);
        break;
    }
    case GL_SPOT_DIRECTION:
        RIXGL::getInstance().pipeline().getLighting().setSpotlightDirection(light - GL_LIGHT0, { params });
        break;
    default:
        SPDLOG_DEBUG("glLightfv redirected to glLightf");
        impl_glLightf(light, pname, params[0]);
        break;
    }
}

GLAPI void APIENTRY impl_glLighti(GLenum light, GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glLighti redirected to glLightf");
    impl_glLightf(light, pname, static_cast<float>(param));
}

GLAPI void APIENTRY impl_glLightiv(GLenum light, GLenum pname, const GLint* params)
{
    SPDLOG_DEBUG("glLightiv redirected to glLightfv");
    Vec4 color = Vec4::createFromArray<GLint, 4>(params);
    color.div(255);
    impl_glLightfv(light, pname, color.data());
}
