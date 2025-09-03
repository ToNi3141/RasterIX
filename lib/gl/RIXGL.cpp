// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

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
#include "RenderConfigs.hpp"
#include "TextureConverter.hpp"
#include "glImpl.h"
#include "pixelpipeline/PixelPipeline.hpp"
#include "renderer/dse/DmaStreamEngine.hpp"
#include "renderer/threadedRasterizer/ThreadedRasterizer.hpp"
#include "vertexpipeline/VertexArray.hpp"
#include "vertexpipeline/VertexPipeline.hpp"
#include "vertexpipeline/VertexQueue.hpp"
#include <assert.h>
#include <math.h>
#include <spdlog/spdlog.h>
#include <stdlib.h>
#include <string.h>

#define ADDRESS_OF(X) reinterpret_cast<const void*>(&X)
namespace rr
{
RIXGL* instance { nullptr };

RIXGL& RIXGL::getInstance()
{
    return *instance;
}

class WithThreadedRasterization
{
public:
    WithThreadedRasterization(IBusConnector& busConnector, IThreadRunner& uploadThread, IThreadRunner& workerThread)
        : dmaStreamEngine { busConnector }
        , device { dmaStreamEngine, uploadThread, workerThread }
    {
    }

    void deinit()
    {
        device.deinit();
        dmaStreamEngine.deinit();
    }

    DSEC::DmaStreamEngine dmaStreamEngine;
    ThreadedRasterizer device;
};

class OnlyDse
{
public:
    OnlyDse(IBusConnector& busConnector, IThreadRunner&, IThreadRunner&)
        : device { busConnector }
    {
    }

    void deinit()
    {
        device.deinit();
    }

    DSEC::DmaStreamEngine device;
};

class RenderDevice
{
public:
    RenderDevice(IBusConnector& busConnector, IThreadRunner& workerThread, IThreadRunner& uploadThread)
        : device { busConnector, uploadThread, workerThread }
        , pixelPipeline { device.device }
        , vertexPipeline { pixelPipeline }
    {
    }

    void deinit()
    {
        vertexPipeline.deinit();
        pixelPipeline.deinit();
        device.deinit();
    }

    using Device = std::conditional<RenderConfig::THREADED_RASTERIZATION, WithThreadedRasterization, OnlyDse>::type;

    Device device;
    PixelPipeline pixelPipeline;
    VertexPipeline vertexPipeline;
    VertexQueue vertexQueue {};
    VertexArray vertexArray {};
    TextureConverter textureConverter {};
};

bool RIXGL::createInstance(IBusConnector& busConnector, IThreadRunner& workerThread, IThreadRunner& uploadThread)
{
    if (instance)
    {
        delete instance;
    }
    instance = new RIXGL { busConnector, workerThread, uploadThread };
    return instance != nullptr;
}

void RIXGL::destroy()
{
    if (instance)
    {
        instance->m_renderDevice->deinit();
        delete instance;
        instance = nullptr;
    }
}

RIXGL::RIXGL(IBusConnector& busConnector, IThreadRunner& workerThread, IThreadRunner& uploadThread)
    : m_renderDevice { new RenderDevice { busConnector, workerThread, uploadThread } }
{
    // Register Open GL 1.0 procedures
    addLibProcedure("glAccum", ADDRESS_OF(impl_glAccum));
    addLibProcedure("glAlphaFunc", ADDRESS_OF(impl_glAlphaFunc));
    addLibProcedure("glBegin", ADDRESS_OF(impl_glBegin));
    addLibProcedure("glBitmap", ADDRESS_OF(impl_glBitmap));
    addLibProcedure("glBlendFunc", ADDRESS_OF(impl_glBlendFunc));
    addLibProcedure("glCallList", ADDRESS_OF(impl_glCallList));
    addLibProcedure("glCallLists", ADDRESS_OF(impl_glCallLists));
    addLibProcedure("glClear", ADDRESS_OF(impl_glClear));
    addLibProcedure("glClearAccum", ADDRESS_OF(impl_glClearAccum));
    addLibProcedure("glClearColor", ADDRESS_OF(impl_glClearColor));
    addLibProcedure("glClearDepth", ADDRESS_OF(impl_glClearDepth));
    addLibProcedure("glClearIndex", ADDRESS_OF(impl_glClearIndex));
    addLibProcedure("glClearStencil", ADDRESS_OF(impl_glClearStencil));
    addLibProcedure("glClipPlane", ADDRESS_OF(impl_glClipPlane));
    addLibProcedure("glColor3b", ADDRESS_OF(impl_glColor3b));
    addLibProcedure("glColor3bv", ADDRESS_OF(impl_glColor3bv));
    addLibProcedure("glColor3d", ADDRESS_OF(impl_glColor3d));
    addLibProcedure("glColor3dv", ADDRESS_OF(impl_glColor3dv));
    addLibProcedure("glColor3f", ADDRESS_OF(impl_glColor3f));
    addLibProcedure("glColor3fv", ADDRESS_OF(impl_glColor3fv));
    addLibProcedure("glColor3i", ADDRESS_OF(impl_glColor3i));
    addLibProcedure("glColor3iv", ADDRESS_OF(impl_glColor3iv));
    addLibProcedure("glColor3s", ADDRESS_OF(impl_glColor3s));
    addLibProcedure("glColor3sv", ADDRESS_OF(impl_glColor3sv));
    addLibProcedure("glColor3ub", ADDRESS_OF(impl_glColor3ub));
    addLibProcedure("glColor3ubv", ADDRESS_OF(impl_glColor3ubv));
    addLibProcedure("glColor3ui", ADDRESS_OF(impl_glColor3ui));
    addLibProcedure("glColor3uiv", ADDRESS_OF(impl_glColor3uiv));
    addLibProcedure("glColor3us", ADDRESS_OF(impl_glColor3us));
    addLibProcedure("glColor3usv", ADDRESS_OF(impl_glColor3usv));
    addLibProcedure("glColor4b", ADDRESS_OF(impl_glColor4b));
    addLibProcedure("glColor4bv", ADDRESS_OF(impl_glColor4bv));
    addLibProcedure("glColor4d", ADDRESS_OF(impl_glColor4d));
    addLibProcedure("glColor4dv", ADDRESS_OF(impl_glColor4dv));
    addLibProcedure("glColor4f", ADDRESS_OF(impl_glColor4f));
    addLibProcedure("glColor4fv", ADDRESS_OF(impl_glColor4fv));
    addLibProcedure("glColor4i", ADDRESS_OF(impl_glColor4i));
    addLibProcedure("glColor4iv", ADDRESS_OF(impl_glColor4iv));
    addLibProcedure("glColor4s", ADDRESS_OF(impl_glColor4s));
    addLibProcedure("glColor4sv", ADDRESS_OF(impl_glColor4sv));
    addLibProcedure("glColor4ub", ADDRESS_OF(impl_glColor4ub));
    addLibProcedure("glColor4ubv", ADDRESS_OF(impl_glColor4ubv));
    addLibProcedure("glColor4ui", ADDRESS_OF(impl_glColor4ui));
    addLibProcedure("glColor4uiv", ADDRESS_OF(impl_glColor4uiv));
    addLibProcedure("glColor4us", ADDRESS_OF(impl_glColor4us));
    addLibProcedure("glColor4usv", ADDRESS_OF(impl_glColor4usv));
    addLibProcedure("glColorMask", ADDRESS_OF(impl_glColorMask));
    addLibProcedure("glColorMaterial", ADDRESS_OF(impl_glColorMaterial));
    addLibProcedure("glCopyPixels", ADDRESS_OF(impl_glCopyPixels));
    addLibProcedure("glCullFace", ADDRESS_OF(impl_glCullFace));
    addLibProcedure("glDeleteLists", ADDRESS_OF(impl_glDeleteLists));
    addLibProcedure("glDepthFunc", ADDRESS_OF(impl_glDepthFunc));
    addLibProcedure("glDepthMask", ADDRESS_OF(impl_glDepthMask));
    addLibProcedure("glDepthRange", ADDRESS_OF(impl_glDepthRange));
    addLibProcedure("glDisable", ADDRESS_OF(impl_glDisable));
    addLibProcedure("glDrawBuffer", ADDRESS_OF(impl_glDrawBuffer));
    addLibProcedure("glDrawPixels", ADDRESS_OF(impl_glDrawPixels));
    addLibProcedure("glEdgeFlag", ADDRESS_OF(impl_glEdgeFlag));
    addLibProcedure("glEdgeFlagv", ADDRESS_OF(impl_glEdgeFlagv));
    addLibProcedure("glEnable", ADDRESS_OF(impl_glEnable));
    addLibProcedure("glEnd", ADDRESS_OF(impl_glEnd));
    addLibProcedure("glEndList", ADDRESS_OF(impl_glEndList));
    addLibProcedure("glEvalCoord1d", ADDRESS_OF(impl_glEvalCoord1d));
    addLibProcedure("glEvalCoord1dv", ADDRESS_OF(impl_glEvalCoord1dv));
    addLibProcedure("glEvalCoord1f", ADDRESS_OF(impl_glEvalCoord1f));
    addLibProcedure("glEvalCoord1fv", ADDRESS_OF(impl_glEvalCoord1fv));
    addLibProcedure("glEvalCoord2d", ADDRESS_OF(impl_glEvalCoord2d));
    addLibProcedure("glEvalCoord2dv", ADDRESS_OF(impl_glEvalCoord2dv));
    addLibProcedure("glEvalCoord2f", ADDRESS_OF(impl_glEvalCoord2f));
    addLibProcedure("glEvalCoord2fv", ADDRESS_OF(impl_glEvalCoord2fv));
    addLibProcedure("glEvalMesh1", ADDRESS_OF(impl_glEvalMesh1));
    addLibProcedure("glEvalMesh2", ADDRESS_OF(impl_glEvalMesh2));
    addLibProcedure("glEvalPoint1", ADDRESS_OF(impl_glEvalPoint1));
    addLibProcedure("glEvalPoint2", ADDRESS_OF(impl_glEvalPoint2));
    addLibProcedure("glFeedbackBuffer", ADDRESS_OF(impl_glFeedbackBuffer));
    addLibProcedure("glFinish", ADDRESS_OF(impl_glFinish));
    addLibProcedure("glFlush", ADDRESS_OF(impl_glFlush));
    addLibProcedure("glFogf", ADDRESS_OF(impl_glFogf));
    addLibProcedure("glFogfv", ADDRESS_OF(impl_glFogfv));
    addLibProcedure("glFogi", ADDRESS_OF(impl_glFogi));
    addLibProcedure("glFogiv", ADDRESS_OF(impl_glFogiv));
    addLibProcedure("glFrontFace", ADDRESS_OF(impl_glFrontFace));
    addLibProcedure("glFrustum", ADDRESS_OF(impl_glFrustum));
    addLibProcedure("glGenLists", ADDRESS_OF(impl_glGenLists));
    addLibProcedure("glGetBooleanv", ADDRESS_OF(impl_glGetBooleanv));
    addLibProcedure("glGetClipPlane", ADDRESS_OF(impl_glGetClipPlane));
    addLibProcedure("glGetDoublev", ADDRESS_OF(impl_glGetDoublev));
    addLibProcedure("glGetError", ADDRESS_OF(impl_glGetError));
    addLibProcedure("glGetFloatv", ADDRESS_OF(impl_glGetFloatv));
    addLibProcedure("glGetIntegerv", ADDRESS_OF(impl_glGetIntegerv));
    addLibProcedure("glGetLightfv", ADDRESS_OF(impl_glGetLightfv));
    addLibProcedure("glGetLightiv", ADDRESS_OF(impl_glGetLightiv));
    addLibProcedure("glGetMapdv", ADDRESS_OF(impl_glGetMapdv));
    addLibProcedure("glGetMapfv", ADDRESS_OF(impl_glGetMapfv));
    addLibProcedure("glGetMapiv", ADDRESS_OF(impl_glGetMapiv));
    addLibProcedure("glGetMaterialfv", ADDRESS_OF(impl_glGetMaterialfv));
    addLibProcedure("glGetMaterialiv", ADDRESS_OF(impl_glGetMaterialiv));
    addLibProcedure("glGetPixelMapfv", ADDRESS_OF(impl_glGetPixelMapfv));
    addLibProcedure("glGetPixelMapuiv", ADDRESS_OF(impl_glGetPixelMapuiv));
    addLibProcedure("glGetPixelMapusv", ADDRESS_OF(impl_glGetPixelMapusv));
    addLibProcedure("glGetPolygonStipple", ADDRESS_OF(impl_glGetPolygonStipple));
    addLibProcedure("glGetString", ADDRESS_OF(impl_glGetString));
    addLibProcedure("glGetTexEnvfv", ADDRESS_OF(impl_glGetTexEnvfv));
    addLibProcedure("glGetTexEnviv", ADDRESS_OF(impl_glGetTexEnviv));
    addLibProcedure("glGetTexGendv", ADDRESS_OF(impl_glGetTexGendv));
    addLibProcedure("glGetTexGenfv", ADDRESS_OF(impl_glGetTexGenfv));
    addLibProcedure("glGetTexGeniv", ADDRESS_OF(impl_glGetTexGeniv));
    addLibProcedure("glGetTexImage", ADDRESS_OF(impl_glGetTexImage));
    addLibProcedure("glGetTexLevelParameterfv", ADDRESS_OF(impl_glGetTexLevelParameterfv));
    addLibProcedure("glGetTexLevelParameteriv", ADDRESS_OF(impl_glGetTexLevelParameteriv));
    addLibProcedure("glGetTexParameterfv", ADDRESS_OF(impl_glGetTexParameterfv));
    addLibProcedure("glGetTexParameteriv", ADDRESS_OF(impl_glGetTexParameteriv));
    addLibProcedure("glHint", ADDRESS_OF(impl_glHint));
    addLibProcedure("glIndexMask", ADDRESS_OF(impl_glIndexMask));
    addLibProcedure("glIndexd", ADDRESS_OF(impl_glIndexd));
    addLibProcedure("glIndexdv", ADDRESS_OF(impl_glIndexdv));
    addLibProcedure("glIndexf", ADDRESS_OF(impl_glIndexf));
    addLibProcedure("glIndexfv", ADDRESS_OF(impl_glIndexfv));
    addLibProcedure("glIndexi", ADDRESS_OF(impl_glIndexi));
    addLibProcedure("glIndexiv", ADDRESS_OF(impl_glIndexiv));
    addLibProcedure("glIndexs", ADDRESS_OF(impl_glIndexs));
    addLibProcedure("glIndexsv", ADDRESS_OF(impl_glIndexsv));
    addLibProcedure("glInitNames", ADDRESS_OF(impl_glInitNames));
    addLibProcedure("glIsEnabled", ADDRESS_OF(impl_glIsEnabled));
    addLibProcedure("glIsList", ADDRESS_OF(impl_glIsList));
    addLibProcedure("glLightModelf", ADDRESS_OF(impl_glLightModelf));
    addLibProcedure("glLightModelfv", ADDRESS_OF(impl_glLightModelfv));
    addLibProcedure("glLightModeli", ADDRESS_OF(impl_glLightModeli));
    addLibProcedure("glLightModeliv", ADDRESS_OF(impl_glLightModeliv));
    addLibProcedure("glLightf", ADDRESS_OF(impl_glLightf));
    addLibProcedure("glLightfv", ADDRESS_OF(impl_glLightfv));
    addLibProcedure("glLighti", ADDRESS_OF(impl_glLighti));
    addLibProcedure("glLightiv", ADDRESS_OF(impl_glLightiv));
    addLibProcedure("glLineStipple", ADDRESS_OF(impl_glLineStipple));
    addLibProcedure("glLineWidth", ADDRESS_OF(impl_glLineWidth));
    addLibProcedure("glListBase", ADDRESS_OF(impl_glListBase));
    addLibProcedure("glLoadIdentity", ADDRESS_OF(impl_glLoadIdentity));
    addLibProcedure("glLoadMatrixd", ADDRESS_OF(impl_glLoadMatrixd));
    addLibProcedure("glLoadMatrixf", ADDRESS_OF(impl_glLoadMatrixf));
    addLibProcedure("glLoadName", ADDRESS_OF(impl_glLoadName));
    addLibProcedure("glLogicOp", ADDRESS_OF(impl_glLogicOp));
    addLibProcedure("glMap1d", ADDRESS_OF(impl_glMap1d));
    addLibProcedure("glMap1f", ADDRESS_OF(impl_glMap1f));
    addLibProcedure("glMap2d", ADDRESS_OF(impl_glMap2d));
    addLibProcedure("glMap2f", ADDRESS_OF(impl_glMap2f));
    addLibProcedure("glMapGrid1d", ADDRESS_OF(impl_glMapGrid1d));
    addLibProcedure("glMapGrid1f", ADDRESS_OF(impl_glMapGrid1f));
    addLibProcedure("glMapGrid2d", ADDRESS_OF(impl_glMapGrid2d));
    addLibProcedure("glMapGrid2f", ADDRESS_OF(impl_glMapGrid2f));
    addLibProcedure("glMaterialf", ADDRESS_OF(impl_glMaterialf));
    addLibProcedure("glMaterialfv", ADDRESS_OF(impl_glMaterialfv));
    addLibProcedure("glMateriali", ADDRESS_OF(impl_glMateriali));
    addLibProcedure("glMaterialiv", ADDRESS_OF(impl_glMaterialiv));
    addLibProcedure("glMatrixMode", ADDRESS_OF(impl_glMatrixMode));
    addLibProcedure("glMultMatrixd", ADDRESS_OF(impl_glMultMatrixd));
    addLibProcedure("glMultMatrixf", ADDRESS_OF(impl_glMultMatrixf));
    addLibProcedure("glNewList", ADDRESS_OF(impl_glNewList));
    addLibProcedure("glNormal3b", ADDRESS_OF(impl_glNormal3b));
    addLibProcedure("glNormal3bv", ADDRESS_OF(impl_glNormal3bv));
    addLibProcedure("glNormal3d", ADDRESS_OF(impl_glNormal3d));
    addLibProcedure("glNormal3dv", ADDRESS_OF(impl_glNormal3dv));
    addLibProcedure("glNormal3f", ADDRESS_OF(impl_glNormal3f));
    addLibProcedure("glNormal3fv", ADDRESS_OF(impl_glNormal3fv));
    addLibProcedure("glNormal3i", ADDRESS_OF(impl_glNormal3i));
    addLibProcedure("glNormal3iv", ADDRESS_OF(impl_glNormal3iv));
    addLibProcedure("glNormal3s", ADDRESS_OF(impl_glNormal3s));
    addLibProcedure("glNormal3sv", ADDRESS_OF(impl_glNormal3sv));
    addLibProcedure("glOrtho", ADDRESS_OF(impl_glOrtho));
    addLibProcedure("glPassThrough", ADDRESS_OF(impl_glPassThrough));
    addLibProcedure("glPixelMapfv", ADDRESS_OF(impl_glPixelMapfv));
    addLibProcedure("glPixelMapuiv", ADDRESS_OF(impl_glPixelMapuiv));
    addLibProcedure("glPixelMapusv", ADDRESS_OF(impl_glPixelMapusv));
    addLibProcedure("glPixelStoref", ADDRESS_OF(impl_glPixelStoref));
    addLibProcedure("glPixelStorei", ADDRESS_OF(impl_glPixelStorei));
    addLibProcedure("glPixelTransferf", ADDRESS_OF(impl_glPixelTransferf));
    addLibProcedure("glPixelTransferi", ADDRESS_OF(impl_glPixelTransferi));
    addLibProcedure("glPixelZoom", ADDRESS_OF(impl_glPixelZoom));
    addLibProcedure("glPointSize", ADDRESS_OF(impl_glPointSize));
    addLibProcedure("glPolygonMode", ADDRESS_OF(impl_glPolygonMode));
    addLibProcedure("glPolygonStipple", ADDRESS_OF(impl_glPolygonStipple));
    addLibProcedure("glPopAttrib", ADDRESS_OF(impl_glPopAttrib));
    addLibProcedure("glPopMatrix", ADDRESS_OF(impl_glPopMatrix));
    addLibProcedure("glPopName", ADDRESS_OF(impl_glPopName));
    addLibProcedure("glPushAttrib", ADDRESS_OF(impl_glPushAttrib));
    addLibProcedure("glPushMatrix", ADDRESS_OF(impl_glPushMatrix));
    addLibProcedure("glPushName", ADDRESS_OF(impl_glPushName));
    addLibProcedure("glRasterPos2d", ADDRESS_OF(impl_glRasterPos2d));
    addLibProcedure("glRasterPos2dv", ADDRESS_OF(impl_glRasterPos2dv));
    addLibProcedure("glRasterPos2f", ADDRESS_OF(impl_glRasterPos2f));
    addLibProcedure("glRasterPos2fv", ADDRESS_OF(impl_glRasterPos2fv));
    addLibProcedure("glRasterPos2i", ADDRESS_OF(impl_glRasterPos2i));
    addLibProcedure("glRasterPos2iv", ADDRESS_OF(impl_glRasterPos2iv));
    addLibProcedure("glRasterPos2s", ADDRESS_OF(impl_glRasterPos2s));
    addLibProcedure("glRasterPos2sv", ADDRESS_OF(impl_glRasterPos2sv));
    addLibProcedure("glRasterPos3d", ADDRESS_OF(impl_glRasterPos3d));
    addLibProcedure("glRasterPos3dv", ADDRESS_OF(impl_glRasterPos3dv));
    addLibProcedure("glRasterPos3f", ADDRESS_OF(impl_glRasterPos3f));
    addLibProcedure("glRasterPos3fv", ADDRESS_OF(impl_glRasterPos3fv));
    addLibProcedure("glRasterPos3i", ADDRESS_OF(impl_glRasterPos3i));
    addLibProcedure("glRasterPos3iv", ADDRESS_OF(impl_glRasterPos3iv));
    addLibProcedure("glRasterPos3s", ADDRESS_OF(impl_glRasterPos3s));
    addLibProcedure("glRasterPos3sv", ADDRESS_OF(impl_glRasterPos3sv));
    addLibProcedure("glRasterPos4d", ADDRESS_OF(impl_glRasterPos4d));
    addLibProcedure("glRasterPos4dv", ADDRESS_OF(impl_glRasterPos4dv));
    addLibProcedure("glRasterPos4f", ADDRESS_OF(impl_glRasterPos4f));
    addLibProcedure("glRasterPos4fv", ADDRESS_OF(impl_glRasterPos4fv));
    addLibProcedure("glRasterPos4i", ADDRESS_OF(impl_glRasterPos4i));
    addLibProcedure("glRasterPos4iv", ADDRESS_OF(impl_glRasterPos4iv));
    addLibProcedure("glRasterPos4s", ADDRESS_OF(impl_glRasterPos4s));
    addLibProcedure("glRasterPos4sv", ADDRESS_OF(impl_glRasterPos4sv));
    addLibProcedure("glReadBuffer", ADDRESS_OF(impl_glReadBuffer));
    addLibProcedure("glReadPixels", ADDRESS_OF(impl_glReadPixels));
    addLibProcedure("glRectd", ADDRESS_OF(impl_glRectd));
    addLibProcedure("glRectdv", ADDRESS_OF(impl_glRectdv));
    addLibProcedure("glRectf", ADDRESS_OF(impl_glRectf));
    addLibProcedure("glRectfv", ADDRESS_OF(impl_glRectfv));
    addLibProcedure("glRecti", ADDRESS_OF(impl_glRecti));
    addLibProcedure("glRectiv", ADDRESS_OF(impl_glRectiv));
    addLibProcedure("glRects", ADDRESS_OF(impl_glRects));
    addLibProcedure("glRectsv", ADDRESS_OF(impl_glRectsv));
    addLibProcedure("glRenderMode", ADDRESS_OF(impl_glRenderMode));
    addLibProcedure("glRotated", ADDRESS_OF(impl_glRotated));
    addLibProcedure("glRotatef", ADDRESS_OF(impl_glRotatef));
    addLibProcedure("glScaled", ADDRESS_OF(impl_glScaled));
    addLibProcedure("glScalef", ADDRESS_OF(impl_glScalef));
    addLibProcedure("glScissor", ADDRESS_OF(impl_glScissor));
    addLibProcedure("glSelectBuffer", ADDRESS_OF(impl_glSelectBuffer));
    addLibProcedure("glShadeModel", ADDRESS_OF(impl_glShadeModel));
    addLibProcedure("glStencilFunc", ADDRESS_OF(impl_glStencilFunc));
    addLibProcedure("glStencilMask", ADDRESS_OF(impl_glStencilMask));
    addLibProcedure("glStencilOp", ADDRESS_OF(impl_glStencilOp));
    addLibProcedure("glTexCoord1d", ADDRESS_OF(impl_glTexCoord1d));
    addLibProcedure("glTexCoord1dv", ADDRESS_OF(impl_glTexCoord1dv));
    addLibProcedure("glTexCoord1f", ADDRESS_OF(impl_glTexCoord1f));
    addLibProcedure("glTexCoord1fv", ADDRESS_OF(impl_glTexCoord1fv));
    addLibProcedure("glTexCoord1i", ADDRESS_OF(impl_glTexCoord1i));
    addLibProcedure("glTexCoord1iv", ADDRESS_OF(impl_glTexCoord1iv));
    addLibProcedure("glTexCoord1s", ADDRESS_OF(impl_glTexCoord1s));
    addLibProcedure("glTexCoord1sv", ADDRESS_OF(impl_glTexCoord1sv));
    addLibProcedure("glTexCoord2d", ADDRESS_OF(impl_glTexCoord2d));
    addLibProcedure("glTexCoord2dv", ADDRESS_OF(impl_glTexCoord2dv));
    addLibProcedure("glTexCoord2f", ADDRESS_OF(impl_glTexCoord2f));
    addLibProcedure("glTexCoord2fv", ADDRESS_OF(impl_glTexCoord2fv));
    addLibProcedure("glTexCoord2i", ADDRESS_OF(impl_glTexCoord2i));
    addLibProcedure("glTexCoord2iv", ADDRESS_OF(impl_glTexCoord2iv));
    addLibProcedure("glTexCoord2s", ADDRESS_OF(impl_glTexCoord2s));
    addLibProcedure("glTexCoord2sv", ADDRESS_OF(impl_glTexCoord2sv));
    addLibProcedure("glTexCoord3d", ADDRESS_OF(impl_glTexCoord3d));
    addLibProcedure("glTexCoord3dv", ADDRESS_OF(impl_glTexCoord3dv));
    addLibProcedure("glTexCoord3f", ADDRESS_OF(impl_glTexCoord3f));
    addLibProcedure("glTexCoord3fv", ADDRESS_OF(impl_glTexCoord3fv));
    addLibProcedure("glTexCoord3i", ADDRESS_OF(impl_glTexCoord3i));
    addLibProcedure("glTexCoord3iv", ADDRESS_OF(impl_glTexCoord3iv));
    addLibProcedure("glTexCoord3s", ADDRESS_OF(impl_glTexCoord3s));
    addLibProcedure("glTexCoord3sv", ADDRESS_OF(impl_glTexCoord3sv));
    addLibProcedure("glTexCoord4d", ADDRESS_OF(impl_glTexCoord4d));
    addLibProcedure("glTexCoord4dv", ADDRESS_OF(impl_glTexCoord4dv));
    addLibProcedure("glTexCoord4f", ADDRESS_OF(impl_glTexCoord4f));
    addLibProcedure("glTexCoord4fv", ADDRESS_OF(impl_glTexCoord4fv));
    addLibProcedure("glTexCoord4i", ADDRESS_OF(impl_glTexCoord4i));
    addLibProcedure("glTexCoord4iv", ADDRESS_OF(impl_glTexCoord4iv));
    addLibProcedure("glTexCoord4s", ADDRESS_OF(impl_glTexCoord4s));
    addLibProcedure("glTexCoord4sv", ADDRESS_OF(impl_glTexCoord4sv));
    addLibProcedure("glTexEnvf", ADDRESS_OF(impl_glTexEnvf));
    addLibProcedure("glTexEnvfv", ADDRESS_OF(impl_glTexEnvfv));
    addLibProcedure("glTexEnvi", ADDRESS_OF(impl_glTexEnvi));
    addLibProcedure("glTexEnviv", ADDRESS_OF(impl_glTexEnviv));
    addLibProcedure("glTexGend", ADDRESS_OF(impl_glTexGend));
    addLibProcedure("glTexGendv", ADDRESS_OF(impl_glTexGendv));
    addLibProcedure("glTexGenf", ADDRESS_OF(impl_glTexGenf));
    addLibProcedure("glTexGenfv", ADDRESS_OF(impl_glTexGenfv));
    addLibProcedure("glTexGeni", ADDRESS_OF(impl_glTexGeni));
    addLibProcedure("glTexGeniv", ADDRESS_OF(impl_glTexGeniv));
    addLibProcedure("glTexImage1D", ADDRESS_OF(impl_glTexImage1D));
    addLibProcedure("glTexImage2D", ADDRESS_OF(impl_glTexImage2D));
    addLibProcedure("glTexParameterf", ADDRESS_OF(impl_glTexParameterf));
    addLibProcedure("glTexParameterfv", ADDRESS_OF(impl_glTexParameterfv));
    addLibProcedure("glTexParameteri", ADDRESS_OF(impl_glTexParameteri));
    addLibProcedure("glTexParameteriv", ADDRESS_OF(impl_glTexParameteriv));
    addLibProcedure("glTranslated", ADDRESS_OF(impl_glTranslated));
    addLibProcedure("glTranslatef", ADDRESS_OF(impl_glTranslatef));
    addLibProcedure("glVertex2d", ADDRESS_OF(impl_glVertex2d));
    addLibProcedure("glVertex2dv", ADDRESS_OF(impl_glVertex2dv));
    addLibProcedure("glVertex2f", ADDRESS_OF(impl_glVertex2f));
    addLibProcedure("glVertex2fv", ADDRESS_OF(impl_glVertex2fv));
    addLibProcedure("glVertex2i", ADDRESS_OF(impl_glVertex2i));
    addLibProcedure("glVertex2iv", ADDRESS_OF(impl_glVertex2iv));
    addLibProcedure("glVertex2s", ADDRESS_OF(impl_glVertex2s));
    addLibProcedure("glVertex2sv", ADDRESS_OF(impl_glVertex2sv));
    addLibProcedure("glVertex3d", ADDRESS_OF(impl_glVertex3d));
    addLibProcedure("glVertex3dv", ADDRESS_OF(impl_glVertex3dv));
    addLibProcedure("glVertex3f", ADDRESS_OF(impl_glVertex3f));
    addLibProcedure("glVertex3fv", ADDRESS_OF(impl_glVertex3fv));
    addLibProcedure("glVertex3i", ADDRESS_OF(impl_glVertex3i));
    addLibProcedure("glVertex3iv", ADDRESS_OF(impl_glVertex3iv));
    addLibProcedure("glVertex3s", ADDRESS_OF(impl_glVertex3s));
    addLibProcedure("glVertex3sv", ADDRESS_OF(impl_glVertex3sv));
    addLibProcedure("glVertex4d", ADDRESS_OF(impl_glVertex4d));
    addLibProcedure("glVertex4dv", ADDRESS_OF(impl_glVertex4dv));
    addLibProcedure("glVertex4f", ADDRESS_OF(impl_glVertex4f));
    addLibProcedure("glVertex4fv", ADDRESS_OF(impl_glVertex4fv));
    addLibProcedure("glVertex4i", ADDRESS_OF(impl_glVertex4i));
    addLibProcedure("glVertex4iv", ADDRESS_OF(impl_glVertex4iv));
    addLibProcedure("glVertex4s", ADDRESS_OF(impl_glVertex4s));
    addLibProcedure("glVertex4sv", ADDRESS_OF(impl_glVertex4sv));
    addLibProcedure("glViewport", ADDRESS_OF(impl_glViewport));

    // Register Open GL 1.1 procedures
    addLibProcedure("glAreTexturesResident", ADDRESS_OF(impl_glAreTexturesResident));
    addLibProcedure("glArrayElement", ADDRESS_OF(impl_glArrayElement));
    addLibProcedure("glBindTexture", ADDRESS_OF(impl_glBindTexture));
    addLibProcedure("glColorPointer", ADDRESS_OF(impl_glColorPointer));
    addLibProcedure("glCopyTexImage1D", ADDRESS_OF(impl_glCopyTexImage1D));
    addLibProcedure("glCopyTexImage2D", ADDRESS_OF(impl_glCopyTexImage2D));
    addLibProcedure("glCopyTexSubImage1D", ADDRESS_OF(impl_glCopyTexSubImage1D));
    addLibProcedure("glCopyTexSubImage2D", ADDRESS_OF(impl_glCopyTexSubImage2D));
    addLibProcedure("glDeleteTextures", ADDRESS_OF(impl_glDeleteTextures));
    addLibProcedure("glDisableClientState", ADDRESS_OF(impl_glDisableClientState));
    addLibProcedure("glDrawArrays", ADDRESS_OF(impl_glDrawArrays));
    addLibProcedure("glDrawElements", ADDRESS_OF(impl_glDrawElements));
    addLibProcedure("glEdgeFlagPointer", ADDRESS_OF(impl_glEdgeFlagPointer));
    addLibProcedure("glEnableClientState", ADDRESS_OF(impl_glEnableClientState));
    addLibProcedure("glGenTextures", ADDRESS_OF(impl_glGenTextures));
    addLibProcedure("glGetPointerv", ADDRESS_OF(impl_glGetPointerv));
    addLibProcedure("glIsTexture", ADDRESS_OF(impl_glIsTexture));
    addLibProcedure("glIndexPointer", ADDRESS_OF(impl_glIndexPointer));
    addLibProcedure("glIndexub", ADDRESS_OF(impl_glIndexub));
    addLibProcedure("glIndexubv", ADDRESS_OF(impl_glIndexubv));
    addLibProcedure("glInterleavedArrays", ADDRESS_OF(impl_glInterleavedArrays));
    addLibProcedure("glNormalPointer", ADDRESS_OF(impl_glNormalPointer));
    addLibProcedure("glPolygonOffset", ADDRESS_OF(impl_glPolygonOffset));
    addLibProcedure("glPopClientAttrib", ADDRESS_OF(impl_glPopClientAttrib));
    addLibProcedure("glPrioritizeTextures", ADDRESS_OF(impl_glPrioritizeTextures));
    addLibProcedure("glPushClientAttrib", ADDRESS_OF(impl_glPushClientAttrib));
    addLibProcedure("glTexCoordPointer", ADDRESS_OF(impl_glTexCoordPointer));
    addLibProcedure("glTexSubImage1D", ADDRESS_OF(impl_glTexSubImage1D));
    addLibProcedure("glTexSubImage2D", ADDRESS_OF(impl_glTexSubImage2D));
    addLibProcedure("glVertexPointer", ADDRESS_OF(impl_glVertexPointer));

    // Register Open GL 1.2 procedures
    addLibProcedure("glDrawRangeElements", ADDRESS_OF(impl_glDrawRangeElements));
    addLibProcedure("glTexImage3D", ADDRESS_OF(impl_glTexImage3D));
    addLibProcedure("glTexSubImage3D", ADDRESS_OF(impl_glTexSubImage3D));
    addLibProcedure("glCopyTexSubImage3D", ADDRESS_OF(impl_glCopyTexSubImage3D));

    // Register Open GL 1.3 procedures
    addLibProcedure("glActiveTexture", ADDRESS_OF(impl_glActiveTexture));
    addLibProcedure("glSampleCoverage", ADDRESS_OF(impl_glSampleCoverage));
    addLibProcedure("glCompressedTexImage3D", ADDRESS_OF(impl_glCompressedTexImage3D));
    addLibProcedure("glCompressedTexImage2D", ADDRESS_OF(impl_glCompressedTexImage2D));
    addLibProcedure("glCompressedTexImage1D", ADDRESS_OF(impl_glCompressedTexImage1D));
    addLibProcedure("glCompressedTexSubImage3D", ADDRESS_OF(impl_glCompressedTexSubImage3D));
    addLibProcedure("glCompressedTexSubImage2D", ADDRESS_OF(impl_glCompressedTexSubImage2D));
    addLibProcedure("glCompressedTexSubImage1D", ADDRESS_OF(impl_glCompressedTexSubImage1D));
    addLibProcedure("glGetCompressedTexImage", ADDRESS_OF(impl_glGetCompressedTexImage));
    addLibProcedure("glClientActiveTexture", ADDRESS_OF(impl_glClientActiveTexture));
    addLibProcedure("glMultiTexCoord1d", ADDRESS_OF(impl_glMultiTexCoord1d));
    addLibProcedure("glMultiTexCoord1dv", ADDRESS_OF(impl_glMultiTexCoord1dv));
    addLibProcedure("glMultiTexCoord1f", ADDRESS_OF(impl_glMultiTexCoord1f));
    addLibProcedure("glMultiTexCoord1fv", ADDRESS_OF(impl_glMultiTexCoord1fv));
    addLibProcedure("glMultiTexCoord1i", ADDRESS_OF(impl_glMultiTexCoord1i));
    addLibProcedure("glMultiTexCoord1iv", ADDRESS_OF(impl_glMultiTexCoord1iv));
    addLibProcedure("glMultiTexCoord1s", ADDRESS_OF(impl_glMultiTexCoord1s));
    addLibProcedure("glMultiTexCoord1sv", ADDRESS_OF(impl_glMultiTexCoord1sv));
    addLibProcedure("glMultiTexCoord2d", ADDRESS_OF(impl_glMultiTexCoord2d));
    addLibProcedure("glMultiTexCoord2dv", ADDRESS_OF(impl_glMultiTexCoord2dv));
    addLibProcedure("glMultiTexCoord2f", ADDRESS_OF(impl_glMultiTexCoord2f));
    addLibProcedure("glMultiTexCoord2fv", ADDRESS_OF(impl_glMultiTexCoord2fv));
    addLibProcedure("glMultiTexCoord2i", ADDRESS_OF(impl_glMultiTexCoord2i));
    addLibProcedure("glMultiTexCoord2iv", ADDRESS_OF(impl_glMultiTexCoord2iv));
    addLibProcedure("glMultiTexCoord2s", ADDRESS_OF(impl_glMultiTexCoord2s));
    addLibProcedure("glMultiTexCoord2sv", ADDRESS_OF(impl_glMultiTexCoord2sv));
    addLibProcedure("glMultiTexCoord3d", ADDRESS_OF(impl_glMultiTexCoord3d));
    addLibProcedure("glMultiTexCoord3dv", ADDRESS_OF(impl_glMultiTexCoord3dv));
    addLibProcedure("glMultiTexCoord3f", ADDRESS_OF(impl_glMultiTexCoord3f));
    addLibProcedure("glMultiTexCoord3fv", ADDRESS_OF(impl_glMultiTexCoord3fv));
    addLibProcedure("glMultiTexCoord3i", ADDRESS_OF(impl_glMultiTexCoord3i));
    addLibProcedure("glMultiTexCoord3iv", ADDRESS_OF(impl_glMultiTexCoord3iv));
    addLibProcedure("glMultiTexCoord3s", ADDRESS_OF(impl_glMultiTexCoord3s));
    addLibProcedure("glMultiTexCoord3sv", ADDRESS_OF(impl_glMultiTexCoord3sv));
    addLibProcedure("glMultiTexCoord4d", ADDRESS_OF(impl_glMultiTexCoord4d));
    addLibProcedure("glMultiTexCoord4dv", ADDRESS_OF(impl_glMultiTexCoord4dv));
    addLibProcedure("glMultiTexCoord4f", ADDRESS_OF(impl_glMultiTexCoord4f));
    addLibProcedure("glMultiTexCoord4fv", ADDRESS_OF(impl_glMultiTexCoord4fv));
    addLibProcedure("glMultiTexCoord4i", ADDRESS_OF(impl_glMultiTexCoord4i));
    addLibProcedure("glMultiTexCoord4iv", ADDRESS_OF(impl_glMultiTexCoord4iv));
    addLibProcedure("glMultiTexCoord4s", ADDRESS_OF(impl_glMultiTexCoord4s));
    addLibProcedure("glMultiTexCoord4sv", ADDRESS_OF(impl_glMultiTexCoord4sv));
    addLibProcedure("glLoadTransposeMatrixf", ADDRESS_OF(impl_glLoadTransposeMatrixf));
    addLibProcedure("glLoadTransposeMatrixd", ADDRESS_OF(impl_glLoadTransposeMatrixd));
    addLibProcedure("glMultTransposeMatrixf", ADDRESS_OF(impl_glMultTransposeMatrixf));
    addLibProcedure("glMultTransposeMatrixd", ADDRESS_OF(impl_glMultTransposeMatrixd));

    // Register the implemented extensions
    addLibExtension("GL_ARB_texture_env_add");
    addLibExtension("GL_ARB_texture_env_combine");
    // addLibExtension("GL_ARB_texture_cube_map");
    addLibExtension("GL_ARB_texture_env_dot3");
    addLibExtension("GL_ARB_multitexture");
    {

        addLibProcedure("glMultiTexCoord1dARB", ADDRESS_OF(impl_glMultiTexCoord1d));
        addLibProcedure("glMultiTexCoord1dvARB", ADDRESS_OF(impl_glMultiTexCoord1dv));
        addLibProcedure("glMultiTexCoord1fARB", ADDRESS_OF(impl_glMultiTexCoord1f));
        addLibProcedure("glMultiTexCoord1fvARB", ADDRESS_OF(impl_glMultiTexCoord1fv));
        addLibProcedure("glMultiTexCoord1iARB", ADDRESS_OF(impl_glMultiTexCoord1i));
        addLibProcedure("glMultiTexCoord1ivARB", ADDRESS_OF(impl_glMultiTexCoord1iv));
        addLibProcedure("glMultiTexCoord1sARB", ADDRESS_OF(impl_glMultiTexCoord1s));
        addLibProcedure("glMultiTexCoord1svARB", ADDRESS_OF(impl_glMultiTexCoord1sv));
        addLibProcedure("glMultiTexCoord2dARB", ADDRESS_OF(impl_glMultiTexCoord2d));
        addLibProcedure("glMultiTexCoord2dvARB", ADDRESS_OF(impl_glMultiTexCoord2dv));
        addLibProcedure("glMultiTexCoord2fARB", ADDRESS_OF(impl_glMultiTexCoord2f));
        addLibProcedure("glMultiTexCoord2fvARB", ADDRESS_OF(impl_glMultiTexCoord2fv));
        addLibProcedure("glMultiTexCoord2iARB", ADDRESS_OF(impl_glMultiTexCoord2i));
        addLibProcedure("glMultiTexCoord2ivARB", ADDRESS_OF(impl_glMultiTexCoord2iv));
        addLibProcedure("glMultiTexCoord2sARB", ADDRESS_OF(impl_glMultiTexCoord2s));
        addLibProcedure("glMultiTexCoord2svARB", ADDRESS_OF(impl_glMultiTexCoord2sv));
        addLibProcedure("glMultiTexCoord3dARB", ADDRESS_OF(impl_glMultiTexCoord3d));
        addLibProcedure("glMultiTexCoord3dvARB", ADDRESS_OF(impl_glMultiTexCoord3dv));
        addLibProcedure("glMultiTexCoord3fARB", ADDRESS_OF(impl_glMultiTexCoord3f));
        addLibProcedure("glMultiTexCoord3fvARB", ADDRESS_OF(impl_glMultiTexCoord3fv));
        addLibProcedure("glMultiTexCoord3iARB", ADDRESS_OF(impl_glMultiTexCoord3i));
        addLibProcedure("glMultiTexCoord3ivARB", ADDRESS_OF(impl_glMultiTexCoord3iv));
        addLibProcedure("glMultiTexCoord3sARB", ADDRESS_OF(impl_glMultiTexCoord3s));
        addLibProcedure("glMultiTexCoord3svARB", ADDRESS_OF(impl_glMultiTexCoord3sv));
        addLibProcedure("glMultiTexCoord4dARB", ADDRESS_OF(impl_glMultiTexCoord4d));
        addLibProcedure("glMultiTexCoord4dvARB", ADDRESS_OF(impl_glMultiTexCoord4dv));
        addLibProcedure("glMultiTexCoord4fARB", ADDRESS_OF(impl_glMultiTexCoord4f));
        addLibProcedure("glMultiTexCoord4fvARB", ADDRESS_OF(impl_glMultiTexCoord4fv));
        addLibProcedure("glMultiTexCoord4iARB", ADDRESS_OF(impl_glMultiTexCoord4i));
        addLibProcedure("glMultiTexCoord4ivARB", ADDRESS_OF(impl_glMultiTexCoord4iv));
        addLibProcedure("glMultiTexCoord4sARB", ADDRESS_OF(impl_glMultiTexCoord4s));
        addLibProcedure("glMultiTexCoord4svARB", ADDRESS_OF(impl_glMultiTexCoord4sv));
        addLibProcedure("glActiveTextureARB", ADDRESS_OF(impl_glActiveTexture));
        addLibProcedure("glClientActiveTextureARB", ADDRESS_OF(impl_glClientActiveTexture));
    }

    addLibProcedure("glLockArrays", ADDRESS_OF(impl_glLockArrays));
    addLibProcedure("glUnlockArrays", ADDRESS_OF(impl_glUnlockArrays));
    addLibProcedure("glActiveStencilFaceEXT", ADDRESS_OF(impl_glActiveStencilFaceEXT));
    addLibProcedure("glBlendEquation", ADDRESS_OF(impl_glBlendEquation));
    addLibProcedure("glBlendFuncSeparate", ADDRESS_OF(impl_glBlendFuncSeparate));
    // addLibExtension("GL_EXT_compiled_vertex_array");
    // {

    //     addLibProcedure("glLockArraysEXT", ADDRESS_OF(impl_glLockArrays));
    //     addLibProcedure("glUnlockArraysEXT", ADDRESS_OF(impl_glUnlockArrays));
    // }
    // addLibExtension("WGL_3DFX_gamma_control");
    // {

    //     addLibProcedure("wglGetDeviceGammaRamp3DFX", ADDRESS_OF(wglGetDeviceGammaRamp));
    //     addLibProcedure("wglSetDeviceGammaRamp3DFX", ADDRESS_OF(wglSetDeviceGammaRamp));
    // }
    // addLibExtension("WGL_ARB_extensions_string");
    // {

    //     addLibProcedure("wglGetExtensionsStringARB", ADDRESS_OF(wglGetExtensionsString));
    // }
}

RIXGL::~RIXGL()
{
    delete m_renderDevice;
}

void RIXGL::swapDisplayList()
{
    SPDLOG_INFO("swapDisplayList called");
    m_renderDevice->pixelPipeline.swapDisplayList();
}

const char* RIXGL::getLibExtensions() const
{
    return m_glExtensions.c_str();
}

const void* RIXGL::getLibProcedure(std::string name) const
{
    auto proc = m_glProcedures.find(name);
    if (proc != m_glProcedures.end())
    {

        return proc->second;
    }

    SPDLOG_WARN("Procedure {} isn't implemented", name.c_str());

    return nullptr;
}

void RIXGL::addLibProcedure(std::string name, const void* address)
{
    m_glProcedures[name] = address;
}

void RIXGL::addLibExtension(std::string extension)
{
    if (!m_glExtensions.empty())
    {

        m_glExtensions.append(" ");
    }
    m_glExtensions.append(extension);
}

VertexPipeline& RIXGL::pipeline()
{
    return m_renderDevice->vertexPipeline;
}

VertexQueue& RIXGL::vertexQueue()
{
    return m_renderDevice->vertexQueue;
}

VertexArray& RIXGL::vertexArray()
{
    return m_renderDevice->vertexArray;
}

TextureConverter& RIXGL::textureConverter()
{
    return m_renderDevice->textureConverter;
}

std::size_t RIXGL::getMaxTextureSize() const
{
    return RenderConfig::MAX_TEXTURE_SIZE;
}

std::size_t RIXGL::getTmuCount() const
{
    return RenderConfig::TMU_COUNT;
}

bool RIXGL::isMipmappingAvailable() const
{
    return RenderConfig::ENABLE_MIPMAPPING;
}

bool RIXGL::setRenderResolution(const std::size_t x, const std::size_t y)
{
    return m_renderDevice->pixelPipeline.setRenderResolution(x, y);
}

std::size_t RIXGL::getMaxLOD()
{
    return TextureObject::MAX_LOD;
}

void RIXGL::enableVSync(const bool enable)
{
    m_renderDevice->pixelPipeline.enableVSync(enable);
}

} // namespace rr
