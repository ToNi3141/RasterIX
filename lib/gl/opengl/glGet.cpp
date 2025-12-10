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

#include "ImageConverter.hpp"
#include "MipMapGenerator.hpp"
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

GLAPI void APIENTRY impl_glGetBooleanv(GLenum pname, GLboolean* params)
{
    SPDLOG_DEBUG("glGetBooleanv pname 0x{:X} called", pname);

    GLfloat floatVals[16] = { 0.0f }; // Large enough for all cases
    SPDLOG_DEBUG("glGetIntegerv redirected to glGetFloatv", pname);
    impl_glGetFloatv(pname, floatVals);
    std::size_t count = 1;
    switch (pname)
    {
    case GL_COLOR_CLEAR_VALUE:
    case GL_CURRENT_COLOR:
    case GL_FOG_COLOR:
    case GL_LIGHT_MODEL_AMBIENT:
    case GL_TEXTURE_ENV_COLOR:
    case GL_MAX_VIEWPORT_DIMS:
    case GL_SCISSOR_BOX:
    case GL_COLOR_WRITEMASK:
        count = 4;
        break;
    case GL_MODELVIEW_MATRIX:
    case GL_PROJECTION_MATRIX:
    case GL_TEXTURE_MATRIX:
        count = 16;
        break;
    case GL_POINT_DISTANCE_ATTENUATION:
    case GL_CURRENT_NORMAL:
        count = 3;
        break;
    case GL_DEPTH_RANGE:
    case GL_SMOOTH_LINE_WIDTH_RANGE:
    case GL_SMOOTH_POINT_SIZE_RANGE:
        count = 2;
        break;
    default:
        count = 1;
        break;
    }
    for (std::size_t i = 0; i < count; i++)
        params[i] = convertBoolToGLboolean(floatVals[i] > 0.0f);
}

GLAPI void APIENTRY impl_glGetDoublev(GLenum pname, GLdouble* params)
{
    SPDLOG_WARN("glGetDoublev not implemented");
}

GLAPI void APIENTRY impl_glGetFloatv(GLenum pname, GLfloat* params)
{
    SPDLOG_DEBUG("glGetFloatv pname 0x{:X} called", pname);

    switch (pname)
    {
    case GL_ACTIVE_TEXTURE:
        params[0] = static_cast<GLfloat>(GL_TEXTURE0 + RIXGL::getInstance().pipeline().texture().getActiveTmu());
        break;
    case GL_ALIASED_POINT_SIZE_RANGE:
        params[0] = 1.0f;
        params[1] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getPointAssembly().getMaxPointSize());
        break;
    case GL_ALIASED_LINE_WIDTH_RANGE:
        params[0] = 1.0f;
        params[1] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getLineAssembly().getMaxLineWidth());
        break;
    case GL_ALPHA_BITS:
        params[0] = 4.0f;
        break;
    case GL_ALPHA_TEST:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableAlphaTest()));
        break;
    case GL_ALPHA_TEST_FUNC:
        params[0] = static_cast<GLfloat>(convertTestFuncToOpenGL(RIXGL::getInstance().pipeline().fragmentPipeline().getAlphaFunc()));
        break;
    case GL_ALPHA_TEST_REF:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().fragmentPipeline().getRefAlphaValue()) / 255.0f;
        break;
    case GL_ARRAY_BUFFER_BINDING:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getVertexBufferId());
        break;
    case GL_BLEND:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableBlending()));
        break;
    case GL_BLEND_DST:
        params[0] = static_cast<GLfloat>(convertBlendFuncToOpenGL(RIXGL::getInstance().pipeline().fragmentPipeline().getBlendFuncDFactor()));
        break;
    case GL_BLEND_SRC:
        params[0] = static_cast<GLfloat>(convertBlendFuncToOpenGL(RIXGL::getInstance().pipeline().fragmentPipeline().getBlendFuncSFactor()));
        break;
    case GL_BLUE_BITS:
        params[0] = 4.0f;
        break;
    case GL_CLIENT_ACTIVE_TEXTURE:
        params[0] = static_cast<GLfloat>(GL_TEXTURE0 + RIXGL::getInstance().vertexQueue().getActiveTexture());
        break;
    case GL_CLIP_PLANE0:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getPlaneClipper().getEnable()));
        break;
    case GL_COLOR_ARRAY:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().vertexArray().colorArrayEnabled()));
        break;
    case GL_COLOR_ARRAY_BUFFER_BINDING:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getColorBufferId());
        break;
    case GL_COLOR_ARRAY_SIZE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getColorSize());
        break;
    case GL_COLOR_ARRAY_STRIDE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getColorStride());
        break;
    case GL_COLOR_ARRAY_TYPE:
        params[0] = static_cast<GLfloat>(convertTypeToOpenGL(RIXGL::getInstance().vertexArray().getColorType()));
        break;
    case GL_COLOR_CLEAR_VALUE:
    {
        const auto& c = RIXGL::getInstance().pipeline().getClearColor();
        params[0] = c[0];
        params[1] = c[1];
        params[2] = c[2];
        params[3] = c[3];
        break;
    }
    case GL_COLOR_LOGIC_OP:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableLogicOp()));
        break;
    case GL_COLOR_MATERIAL:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().getColorMaterialEnabled()));
        break;
    case GL_COLOR_WRITEMASK:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().fragmentPipeline().getColorMaskR()));
        params[1] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().fragmentPipeline().getColorMaskG()));
        params[2] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().fragmentPipeline().getColorMaskB()));
        params[3] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().fragmentPipeline().getColorMaskA()));
        break;
    case GL_COMPRESSED_TEXTURE_FORMATS:
        params[0] = 0.0f;
        break;
    case GL_CULL_FACE:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getCulling().isCullingEnabled()));
        break;
    case GL_CULL_FACE_MODE:
        params[0] = static_cast<GLfloat>(convertFaceToOpenGL(RIXGL::getInstance().pipeline().getCulling().getCullMode()));
        break;
    case GL_CURRENT_COLOR:
    {
        const auto& c = RIXGL::getInstance().vertexQueue().getColor();
        params[0] = c[0];
        params[1] = c[1];
        params[2] = c[2];
        params[3] = c[3];
        break;
    }
    case GL_CURRENT_NORMAL:
    {
        const auto& n = RIXGL::getInstance().vertexQueue().getNormal();
        params[0] = n[0];
        params[1] = n[1];
        params[2] = n[2];
        break;
    }
    case GL_CURRENT_TEXTURE_COORDS:
    {
        const auto& t = RIXGL::getInstance().vertexQueue().getTexCoord();
        params[0] = t[0];
        params[1] = t[1];
        params[2] = t[2];
        params[3] = t[3];
        break;
    }
    case GL_DEPTH_BITS:
        params[0] = 16.0f;
        break;
    case GL_DEPTH_CLEAR_VALUE:
        params[0] = RIXGL::getInstance().pipeline().getClearDepth();
        break;
    case GL_DEPTH_FUNC:
        params[0] = static_cast<GLfloat>(convertTestFuncToOpenGL(RIXGL::getInstance().pipeline().fragmentPipeline().getDepthFunc()));
        break;
    case GL_DEPTH_RANGE:
        params[0] = RIXGL::getInstance().pipeline().getViewPort().getDepthRangeNear();
        params[1] = RIXGL::getInstance().pipeline().getViewPort().getDepthRangeFar();
        break;
    case GL_DEPTH_TEST:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableDepthTest()));
        break;
    case GL_DEPTH_WRITEMASK:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().fragmentPipeline().getDepthMask()));
        break;
    case GL_ELEMENT_ARRAY_BUFFER_BINDING:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getElementBufferId());
        break;
    case GL_FOG:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableFog()));
        break;
    case GL_FOG_COLOR:
    {
        const auto& c = RIXGL::getInstance().pipeline().fog().getFogColor();
        params[0] = c[0];
        params[1] = c[1];
        params[2] = c[2];
        params[3] = c[3];
        break;
    }
    case GL_FOG_DENSITY:
        params[0] = RIXGL::getInstance().pipeline().fog().getFogDensity();
        break;
    case GL_FOG_END:
        params[0] = RIXGL::getInstance().pipeline().fog().getFogEnd();
        break;
    case GL_FOG_HINT:
        params[0] = static_cast<GLfloat>(GL_DONT_CARE);
        break;
    case GL_FOG_MODE:
        params[0] = static_cast<GLfloat>(convertFogModeToOpenGL(RIXGL::getInstance().pipeline().fog().getFogMode()));
        break;
    case GL_FOG_START:
        params[0] = RIXGL::getInstance().pipeline().fog().getFogStart();
        break;
    case GL_FRONT_FACE:
        params[0] = static_cast<GLfloat>(convertFrontFaceToOpenGL(RIXGL::getInstance().pipeline().getCulling().getFrontFace()));
        break;
    case GL_GREEN_BITS:
        params[0] = 4.0f;
        break;
    case GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES:
        params[0] = static_cast<GLfloat>(GL_RGBA);
        break;
    case GL_IMPLEMENTATION_COLOR_READ_TYPE_OES:
        params[0] = static_cast<GLfloat>(GL_UNSIGNED_BYTE);
        break;
    case GL_LIGHT_MODEL_AMBIENT:
    {
        const auto& c = RIXGL::getInstance().pipeline().getLighting().getAmbientColorScene();
        params[0] = c[0];
        params[1] = c[1];
        params[2] = c[2];
        params[3] = c[3];
        break;
    }
    case GL_LIGHT_MODEL_TWO_SIDE:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().getTwoSideModelEnabled()));
        break;
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().lightEnabled(pname - GL_LIGHT0)));
        break;
    case GL_LIGHTING:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().lightingEnabled()));
        break;
    case GL_LINE_SMOOTH:
        params[0] = static_cast<GLfloat>(GL_FALSE);
        break;
    case GL_LINE_SMOOTH_HINT:
        params[0] = static_cast<GLfloat>(GL_DONT_CARE);
        break;
    case GL_LINE_WIDTH:
        params[0] = RIXGL::getInstance().pipeline().getLineAssembly().getLineWidth();
        break;
    case GL_LOGIC_OP_MODE:
        params[0] = static_cast<GLfloat>(convertLogicOpToOpenGL(RIXGL::getInstance().pipeline().fragmentPipeline().getLogicOp()));
        break;
    case GL_MATRIX_MODE:
        params[0] = static_cast<GLfloat>(convertMatrixModeToOpenGL(RIXGL::getInstance().pipeline().getMatrixStore().getMatrixMode()));
        break;
    case GL_MAX_CLIP_PLANES:
        params[0] = static_cast<GLfloat>(planeclipper::PlaneClipperSetter::getMaxClipPlanes());
        break;
    case GL_MAX_LIGHTS:
        params[0] = static_cast<GLfloat>(lighting::LightingSetter::getMaxLights());
        break;
    case GL_MAX_MODELVIEW_STACK_DEPTH:
        params[0] = static_cast<GLfloat>(matrixstore::MatrixStore::getMaxModelMatrixStackDepth());
        break;
    case GL_MAX_PROJECTION_STACK_DEPTH:
        params[0] = static_cast<GLfloat>(matrixstore::MatrixStore::getMaxProjectionMatrixStackDepth());
        break;
    case GL_MAX_TEXTURE_SIZE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().getMaxTextureSize());
        break;
    case GL_MAX_TEXTURE_STACK_DEPTH:
        params[0] = static_cast<GLfloat>(matrixstore::MatrixStore::getMaxTextureMatrixStackDepth());
        break;
    case GL_MAX_TEXTURE_UNITS:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().getMaxTmuCount());
        break;
    case GL_MAX_VIEWPORT_DIMS:
        params[0] = static_cast<GLfloat>(viewport::ViewPortSetter::getMaxViewportDimsWidth());
        params[1] = static_cast<GLfloat>(viewport::ViewPortSetter::getMaxViewportDimsHeight());
        break;
    case GL_MODELVIEW_MATRIX:
    {
        const auto& m = RIXGL::getInstance().pipeline().getMatrixStore().getModelView();
        std::memcpy(params, m.data(), 16 * sizeof(GLfloat));
        break;
    }
    case GL_MODELVIEW_STACK_DEPTH:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getMatrixStore().getModelMatrixStackDepth());
        break;
    case GL_MULTISAMPLE:
        params[0] = static_cast<GLfloat>(GL_FALSE);
        break;
    case GL_NORMAL_ARRAY:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().vertexArray().normalArrayEnabled()));
        break;
    case GL_NORMAL_ARRAY_BUFFER_BINDING:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getNormalBufferId());
        break;
    case GL_NORMAL_ARRAY_STRIDE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getNormalStride());
        break;
    case GL_NORMAL_ARRAY_TYPE:
        params[0] = static_cast<GLfloat>(convertTypeToOpenGL(RIXGL::getInstance().vertexArray().getNormalType()));
        break;
    case GL_NORMALIZE:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().getNormalNormalizationEnabled()));
        break;
    case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
        params[0] = 0.0f;
        break;
    case GL_PACK_ALIGNMENT:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().imageConverter().getPackAlignment());
        break;
    case GL_PERSPECTIVE_CORRECTION_HINT:
        params[0] = static_cast<GLfloat>(GL_DONT_CARE);
        break;
    case GL_POINT_DISTANCE_ATTENUATION:
    {
        const auto& v = RIXGL::getInstance().pipeline().getPointAssembly().getPointDistanceAttenuation();
        params[0] = v[0];
        params[1] = v[1];
        params[2] = v[2];
        break;
    }
    case GL_POINT_FADE_THRESHOLD_SIZE:
        params[0] = RIXGL::getInstance().pipeline().getPointAssembly().getPointFadeThresholdSize();
        break;
    case GL_POINT_SIZE:
        params[0] = RIXGL::getInstance().vertexArray().getPointSize();
        break;
    case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getPointSizeArrayBufferId());
        break;
    case GL_POINT_SIZE_ARRAY_OES:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().vertexArray().pointSizeArrayEnabled()));
        break;
    case GL_POINT_SIZE_ARRAY_STRIDE_OES:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getPointSizeStride());
        break;
    case GL_POINT_SIZE_ARRAY_TYPE_OES:
        params[0] = static_cast<GLfloat>(convertTypeToOpenGL(RIXGL::getInstance().vertexArray().getPointSizeType()));
        break;
    case GL_POINT_SIZE_MAX:
        params[0] = RIXGL::getInstance().pipeline().getPointAssembly().getPointSizeMax();
        break;
    case GL_POINT_SIZE_MIN:
        params[0] = RIXGL::getInstance().pipeline().getPointAssembly().getPointSizeMin();
        break;
    case GL_POINT_SMOOTH:
        params[0] = static_cast<GLfloat>(GL_FALSE);
        break;
    case GL_POINT_SMOOTH_HINT:
        params[0] = static_cast<GLfloat>(GL_DONT_CARE);
        break;
    case GL_POINT_SPRITE_OES:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getPointAssembly().getEnablePointSprite()));
        break;
    case GL_POLYGON_OFFSET_FACTOR:
        params[0] = RIXGL::getInstance().pipeline().getPolygonOffset().getFactor();
        break;
    case GL_POLYGON_OFFSET_FILL:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getPolygonOffset().getEnableFill()));
        break;
    case GL_POLYGON_OFFSET_UNITS:
        params[0] = RIXGL::getInstance().pipeline().getPolygonOffset().getUnits();
        break;
    case GL_PROJECTION_MATRIX:
    {
        const auto& m = RIXGL::getInstance().pipeline().getMatrixStore().getProjection();
        std::memcpy(params, m.data(), 16 * sizeof(GLfloat));
        break;
    }
    case GL_PROJECTION_STACK_DEPTH:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getMatrixStore().getProjectionMatrixStackDepth());
        break;
    case GL_RED_BITS:
        params[0] = 4.0f;
        break;
    case GL_RESCALE_NORMAL:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().getLighting().getNormalRescaleEnabled()));
        break;
    case GL_SAMPLE_ALPHA_TO_COVERAGE:
    case GL_SAMPLE_ALPHA_TO_ONE:
    case GL_SAMPLE_COVERAGE:
    case GL_SAMPLE_COVERAGE_INVERT:
        params[0] = static_cast<GLfloat>(GL_FALSE);
        break;
    case GL_SAMPLE_BUFFERS:
    case GL_SAMPLES:
        params[0] = 0.0f;
        break;
    case GL_SAMPLE_COVERAGE_VALUE:
        params[0] = 1.0f;
        break;
    case GL_SCISSOR_BOX:
    {
        const auto& box = RIXGL::getInstance().pipeline().getScissorBox();
        params[0] = static_cast<GLfloat>(std::get<0>(box));
        params[1] = static_cast<GLfloat>(std::get<1>(box));
        params[2] = static_cast<GLfloat>(std::get<2>(box));
        params[3] = static_cast<GLfloat>(std::get<3>(box));
        break;
    }
    case GL_SCISSOR_TEST:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableScissor()));
        break;
    case GL_SHADE_MODEL:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getShadeModel().getShadeModelFlat() ? GL_FLAT : GL_SMOOTH);
        break;
    case GL_SMOOTH_LINE_WIDTH_RANGE:
        params[0] = 1.0f;
        params[1] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getLineAssembly().getMaxLineWidth());
        break;
    case GL_SMOOTH_POINT_SIZE_RANGE:
        params[0] = 1.0f;
        params[1] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getPointAssembly().getMaxPointSize());
        break;
    case GL_STENCIL_BITS:
        params[0] = static_cast<GLfloat>(rr::StencilReg::MAX_STENCIL_VAL);
        break;
    case GL_STENCIL_CLEAR_VALUE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getStencil().getClearStencil());
        break;
    case GL_STENCIL_FAIL:
        params[0] = static_cast<GLfloat>(convertStencilOpToOpenGL(RIXGL::getInstance().pipeline().getStencil().getOpFail()));
        break;
    case GL_STENCIL_FUNC:
        params[0] = static_cast<GLfloat>(convertTestFuncToOpenGL(RIXGL::getInstance().pipeline().getStencil().getTestFunc()));
        break;
    case GL_STENCIL_PASS_DEPTH_FAIL:
        params[0] = static_cast<GLfloat>(convertStencilOpToOpenGL(RIXGL::getInstance().pipeline().getStencil().getOpZFail()));
        break;
    case GL_STENCIL_PASS_DEPTH_PASS:
        params[0] = static_cast<GLfloat>(convertStencilOpToOpenGL(RIXGL::getInstance().pipeline().getStencil().getOpZPass()));
        break;
    case GL_STENCIL_REF:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getStencil().getRef());
        break;
    case GL_STENCIL_TEST:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableStencil()));
        break;
    case GL_STENCIL_VALUE_MASK:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getStencil().getMask());
        break;
    case GL_STENCIL_WRITEMASK:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getStencil().getStencilMask());
        break;
    case GL_SUBPIXEL_BITS:
        params[0] = 4.0f;
        break;
    case GL_TEXTURE_2D:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().pipeline().featureEnable().getEnableTmu()));
        break;
    case GL_TEXTURE_BINDING_2D:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().texture().getBoundTexture());
        break;
    case GL_TEXTURE_COORD_ARRAY:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().vertexArray().texCoordArrayEnabled()));
        break;
    case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getTexCoordBufferId());
        break;
    case GL_TEXTURE_COORD_ARRAY_SIZE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getTexCoordSize());
        break;
    case GL_TEXTURE_COORD_ARRAY_STRIDE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getTexCoordStride());
        break;
    case GL_TEXTURE_COORD_ARRAY_TYPE:
        params[0] = static_cast<GLfloat>(convertTypeToOpenGL(RIXGL::getInstance().vertexArray().getTexCoordType()));
        break;
    case GL_TEXTURE_MATRIX:
    {
        const auto& m = RIXGL::getInstance().pipeline().getMatrixStore().getTexture();
        std::memcpy(params, m.data(), 16 * sizeof(GLfloat));
        break;
    }
    case GL_TEXTURE_STACK_DEPTH:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().pipeline().getMatrixStore().getTextureMatrixStackDepth());
        break;
    case GL_UNPACK_ALIGNMENT:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().imageConverter().getUnpackAlignment());
        break;
    case GL_VIEWPORT:
    {
        const auto& vp = RIXGL::getInstance().pipeline().getViewPort();
        params[0] = static_cast<GLfloat>(vp.getViewportX());
        params[1] = static_cast<GLfloat>(vp.getViewportY());
        params[2] = static_cast<GLfloat>(vp.getViewportWidth());
        params[3] = static_cast<GLfloat>(vp.getViewportHeight());
        break;
    }
    case GL_VERTEX_ARRAY:
        params[0] = static_cast<GLfloat>(convertBoolToGLboolean(RIXGL::getInstance().vertexArray().vertexArrayEnabled()));
        break;
    case GL_VERTEX_ARRAY_BUFFER_BINDING:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getVertexBufferId());
        break;
    case GL_VERTEX_ARRAY_SIZE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getVertexSize());
        break;
    case GL_VERTEX_ARRAY_STRIDE:
        params[0] = static_cast<GLfloat>(RIXGL::getInstance().vertexArray().getVertexStride());
        break;
    case GL_VERTEX_ARRAY_TYPE:
        params[0] = static_cast<GLfloat>(convertTypeToOpenGL(RIXGL::getInstance().vertexArray().getVertexType()));
        break;
    default:
        params[0] = 0.0f;
        break;
    }
}

GLAPI void APIENTRY impl_glGetIntegerv(GLenum pname, GLint* params)
{
    SPDLOG_DEBUG("glGetIntegerv pname 0x{:X} called", pname);

    GLfloat floatVals[4] = { 0.0f };
    SPDLOG_DEBUG("glGetIntegerv redirected to glGetFloatv", pname);
    impl_glGetFloatv(pname, floatVals);

    switch (pname)
    {
    // RGBA color components
    case GL_CURRENT_COLOR:
    case GL_COLOR_CLEAR_VALUE:
    case GL_FOG_COLOR:
    case GL_LIGHT_MODEL_AMBIENT:
    case GL_TEXTURE_ENV_COLOR:
        for (std::size_t i = 0; i < 4; i++)
            params[i] = static_cast<GLint>(floatVals[i] * std::numeric_limits<GLint>::max());
        break;

    // DepthRange value
    case GL_DEPTH_RANGE:
        params[0] = static_cast<GLint>(floatVals[0] * std::numeric_limits<GLint>::max());
        params[1] = static_cast<GLint>(floatVals[1] * std::numeric_limits<GLint>::max());
        break;

    // Depth buffer clear value
    case GL_DEPTH_CLEAR_VALUE:
        params[0] = static_cast<GLint>(floatVals[0] * std::numeric_limits<GLint>::max());
        break;

    // Normal coordinate
    case GL_CURRENT_NORMAL:
    case GL_POINT_DISTANCE_ATTENUATION:
        for (std::size_t i = 0; i < 3; i++)
            params[i] = static_cast<GLint>(floatVals[i] * std::numeric_limits<GLint>::max());
        break;

    // Boolean values
    case GL_ALPHA_TEST:
    case GL_BLEND:
    case GL_COLOR_LOGIC_OP:
    case GL_COLOR_MATERIAL:
    case GL_CULL_FACE:
    case GL_DEPTH_TEST:
    case GL_DITHER:
    case GL_FOG:
    case GL_LIGHTING:
    case GL_LINE_SMOOTH:
    case GL_MULTISAMPLE:
    case GL_NORMALIZE:
    case GL_RESCALE_NORMAL:
    case GL_SCISSOR_TEST:
    case GL_STENCIL_TEST:
    case GL_TEXTURE_2D:
        params[0] = convertBoolToGLboolean(floatVals[0] > 0.0f);
        break;

    default:
        // Default: round to nearest integer
        params[0] = static_cast<GLint>(std::lround(floatVals[0]));
        break;
    }
}
