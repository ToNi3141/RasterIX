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
#include "vertexpipeline/VertexArray.hpp"
#include "vertexpipeline/VertexPipeline.hpp"
#include <spdlog/spdlog.h>

using namespace rr;

GLAPI GLboolean APIENTRY impl_glIsEnabled(GLenum cap)
{
    switch (cap)
    {
    case GL_TEXTURE_2D:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableTmu());
    case GL_ALPHA_TEST:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableAlphaTest());
    case GL_DEPTH_TEST:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableDepthTest());
    case GL_BLEND:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableBlending());
    case GL_LIGHTING:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().lightingEnabled());
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().lightEnabled(cap - GL_LIGHT0));
    case GL_TEXTURE_GEN_S:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getTexGen().getTexGenEnableS());
    case GL_TEXTURE_GEN_T:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getTexGen().getTexGenEnableT());
    case GL_TEXTURE_GEN_R:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getTexGen().getTexGenEnableR());
    case GL_CULL_FACE:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getCulling().isCullingEnabled());
    case GL_COLOR_MATERIAL:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().getColorMaterialEnabled());
    case GL_FOG:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableFog());
    case GL_SCISSOR_TEST:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableScissor());
    case GL_NORMALIZE:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().getNormalNormalizationEnabled());
    case GL_STENCIL_TEST:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableStencil());
    case GL_STENCIL_TEST_TWO_SIDE_EXT:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getStencil().getTwoSideStencilEnabled());
    case GL_COLOR_LOGIC_OP:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableLogicOp());
    case GL_CLIP_PLANE0:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getPlaneClipper().getEnable());
    case GL_CLIP_PLANE1:
    case GL_CLIP_PLANE2:
    case GL_CLIP_PLANE3:
    case GL_CLIP_PLANE4:
    case GL_CLIP_PLANE5:
        return GL_FALSE; // Not supported
    case GL_POLYGON_OFFSET_FILL:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getPolygonOffset().getEnableFill());
    case GL_POINT_SPRITE_OES:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getPointAssembly().getEnablePointSprite());
    case GL_DITHER:
        // Dither is always enabled in RIXGL, but has no effect
        return GL_TRUE;
    case GL_LINE_SMOOTH:
    case GL_MULTISAMPLE:
    case GL_POINT_SMOOTH:
    case GL_SAMPLE_ALPHA_TO_COVERAGE:
    case GL_SAMPLE_ALPHA_TO_ONE:
    case GL_SAMPLE_COVERAGE:
        // These are not implemented, always return GL_FALSE
        return GL_FALSE;
    case GL_RESCALE_NORMAL:
        return convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().getNormalRescaleEnabled());
    case GL_COLOR_ARRAY:
        return convertBoolToGLboolean(RIXGL::getInstance().vertexArray().colorArrayEnabled());
    case GL_NORMAL_ARRAY:
        return convertBoolToGLboolean(RIXGL::getInstance().vertexArray().normalArrayEnabled());
    case GL_TEXTURE_COORD_ARRAY:
        return convertBoolToGLboolean(RIXGL::getInstance().vertexArray().texCoordArrayEnabled());
    case GL_VERTEX_ARRAY:
        return convertBoolToGLboolean(RIXGL::getInstance().vertexArray().vertexArrayEnabled());
    case GL_POINT_SIZE_ARRAY_OES:
        return convertBoolToGLboolean(RIXGL::getInstance().vertexArray().pointSizeArrayEnabled());
    default:
        RIXGL::getInstance().setError(GL_INVALID_ENUM);
        return GL_FALSE;
    }
}
