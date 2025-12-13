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

GLAPI void APIENTRY impl_glDisable(GLenum cap)
{
    switch (cap)
    {
    case GL_TEXTURE_2D:
    {
        SPDLOG_DEBUG("glDisable GL_TEXTURE_2D called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableTmu(false);
        break;
    }
    case GL_ALPHA_TEST:
    {
        SPDLOG_DEBUG("glDisable GL_ALPHA_TEST called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableAlphaTest(false);
        break;
    }
    case GL_DEPTH_TEST:
    {
        SPDLOG_DEBUG("glDisable GL_DEPTH_TEST called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableDepthTest(false);
        break;
    }
    case GL_BLEND:
    {
        SPDLOG_DEBUG("glDisable GL_BLEND called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableBlending(false);
        break;
    }
    case GL_LIGHTING:
        SPDLOG_DEBUG("glDisable GL_LIGHTING called");
        RIXGL::getInstance().pipeline().getLighting().enableLighting(false);
        break;
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
        SPDLOG_DEBUG("glDisable GL_LIGHT{} called", cap - GL_LIGHT0);
        RIXGL::getInstance().pipeline().getLighting().enableLight(cap - GL_LIGHT0, false);
        break;
    case GL_TEXTURE_GEN_S:
        SPDLOG_DEBUG("glDisable GL_TEXTURE_GEN_S called");
        RIXGL::getInstance().pipeline().getTexGen().enableTexGenS(false);
        break;
    case GL_TEXTURE_GEN_T:
        SPDLOG_DEBUG("glDisable GL_TEXTURE_GEN_T called");
        RIXGL::getInstance().pipeline().getTexGen().enableTexGenT(false);
        break;
    case GL_TEXTURE_GEN_R:
        SPDLOG_DEBUG("glDisable GL_TEXTURE_GEN_R called");
        RIXGL::getInstance().pipeline().getTexGen().enableTexGenR(false);
        break;
    case GL_CULL_FACE:
        SPDLOG_DEBUG("glDisable GL_CULL_FACE called");
        RIXGL::getInstance().pipeline().getCulling().enableCulling(false);
        break;
    case GL_COLOR_MATERIAL:
        SPDLOG_DEBUG("glDisable GL_COLOR_MATERIAL called");
        RIXGL::getInstance().pipeline().getLighting().enableColorMaterial(false);
        break;
    case GL_FOG:
        SPDLOG_DEBUG("glDisable GL_FOG called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableFog(false);
        break;
    case GL_SCISSOR_TEST:
        SPDLOG_DEBUG("glDisable GL_SCISSOR_TEST called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableScissor(false);
        break;
    case GL_NORMALIZE:
        SPDLOG_DEBUG("glDisable GL_NORMALIZE called");
        RIXGL::getInstance().pipeline().getLighting().setEnableNormalNormalization(false);
        break;
    case GL_STENCIL_TEST:
        SPDLOG_DEBUG("glDisable GL_STENCIL_TEST called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableStencil(false);
        break;
    case GL_STENCIL_TEST_TWO_SIDE_EXT:
        SPDLOG_DEBUG("glDisable GL_STENCIL_TEST_TWO_SIDE_EXT called");
        RIXGL::getInstance().pipeline().getStencil().enableTwoSideStencil(false);
        break;
    case GL_COLOR_LOGIC_OP:
        SPDLOG_DEBUG("glDisable GL_COLOR_LOGIC_OP called");
        RIXGL::getInstance().pipeline().featureEnable().setEnableLogicOp(false);
        break;
    case GL_CLIP_PLANE0:
        SPDLOG_DEBUG("glDisable GL_CLIP_PLANE0 called");
        RIXGL::getInstance().pipeline().getPlaneClipper().setEnable(false);
        break;
    case GL_POLYGON_OFFSET_FILL:
        SPDLOG_DEBUG("glDisable GL_POLYGON_OFFSET_FILL called");
        RIXGL::getInstance().pipeline().getPolygonOffset().setEnableFill(false);
        break;
    case GL_POINT_SPRITE_OES:
        SPDLOG_DEBUG("glDisable GL_POINT_SPRITE_OES called");
        RIXGL::getInstance().pipeline().getPointAssembly().setEnablePointSprite(false);
        break;
    case GL_DITHER:
        SPDLOG_DEBUG("glDisable GL_DITHER called");
        SPDLOG_INFO("glDisable GL_DITHER has no effect in RIXGL");
        break;
    case GL_LINE_SMOOTH:
        SPDLOG_DEBUG("glDisable GL_LINE_SMOOTH called");
        SPDLOG_INFO("glDisable GL_LINE_SMOOTH has no effect in RIXGL");
        break;
    case GL_MULTISAMPLE:
        SPDLOG_DEBUG("glDisable GL_MULTISAMPLE called");
        SPDLOG_INFO("glDisable GL_MULTISAMPLE has no effect in RIXGL");
        break;
    case GL_POINT_SMOOTH:
        SPDLOG_DEBUG("glDisable GL_POINT_SMOOTH called");
        SPDLOG_INFO("glDisable GL_POINT_SMOOTH has no effect in RIXGL");
        break;
    case GL_RESCALE_NORMAL:
        SPDLOG_DEBUG("glDisable GL_RESCALE_NORMAL called");
        RIXGL::getInstance().pipeline().getLighting().setEnableNormalRescale(false);
        break;
    case GL_SAMPLE_ALPHA_TO_COVERAGE:
        SPDLOG_DEBUG("glDisable GL_SAMPLE_ALPHA_TO_COVERAGE called");
        SPDLOG_INFO("glDisable GL_SAMPLE_ALPHA_TO_COVERAGE has no effect in RIXGL");
        break;
    case GL_SAMPLE_COVERAGE:
        SPDLOG_DEBUG("glDisable GL_SAMPLE_COVERAGE called");
        SPDLOG_INFO("glDisable GL_SAMPLE_COVERAGE has no effect in RIXGL");
        break;
    default:
        SPDLOG_WARN("glDisable cap 0x{:X} not supported", cap);
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}