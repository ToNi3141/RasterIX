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

#include "glx.h"
#include "DMAProxyBusConnector.hpp"
#include "MultiThreadRunner.hpp"
#include "RIXGL.hpp"
#include "renderer/devicedatauploader/DeviceDataUploader.hpp"
#include "renderer/threadedvertextransformer/ThreadedVertexTransformer.hpp"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

static constexpr uint32_t RESOLUTION_H = 600;
static constexpr uint32_t RESOLUTION_W = 1024;

class GLInitGuard
{
public:
    GLInitGuard()
    {
        rr::RIXGL::createInstance(m_threadedRasterizer);
#define ADDRESS_OF(X) reinterpret_cast<const void*>(&X)
        rr::RIXGL::getInstance().addLibProcedure("glXChooseVisual", ADDRESS_OF(glXChooseVisual));
        rr::RIXGL::getInstance().addLibProcedure("glXCreateContext", ADDRESS_OF(glXCreateContext));
        rr::RIXGL::getInstance().addLibProcedure("glXDestroyContext", ADDRESS_OF(glXDestroyContext));
        rr::RIXGL::getInstance().addLibProcedure("glXMakeCurrent", ADDRESS_OF(glXMakeCurrent));
        rr::RIXGL::getInstance().addLibProcedure("glXSwapBuffers", ADDRESS_OF(glXSwapBuffers));
        rr::RIXGL::getInstance().addLibProcedure("glXQueryDrawable", ADDRESS_OF(glXQueryDrawable));
        rr::RIXGL::getInstance().addLibProcedure("glXGetCurrentContext", ADDRESS_OF(glXGetCurrentContext));
        rr::RIXGL::getInstance().addLibProcedure("glXGetCurrentDrawable", ADDRESS_OF(glXGetCurrentDrawable));
#undef ADDRESS_OF
    }
    ~GLInitGuard()
    {
        deinit();
    }

    void deinit()
    {
        rr::RIXGL::getInstance().destroy();
    }

    void render()
    {
        rr::RIXGL::getInstance().swapDisplayList();
    }

    rr::RIXGL& getInst()
    {
        return rr::RIXGL::getInstance();
    }

private:
    rr::DMAProxyBusConnector m_busConnector {};
    rr::MultiThreadRunner m_workerThread {};
    rr::MultiThreadRunner m_uploadThread {};
    rr::devicedatauploader::DeviceDataUploader m_dduDevice { m_busConnector };
    rr::threadedvertextransformer::ThreadedVertexTransformer m_threadedRasterizer { m_dduDevice, m_workerThread, m_uploadThread };
} guard;

GLAPI XVisualInfo* APIENTRY glXChooseVisual(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] int screen,
    [[maybe_unused]] int* attribList)
{
    SPDLOG_DEBUG("glXChooseVisual called");
    XVisualInfo* vi = new XVisualInfo;
    vi->visual = new Visual;
    vi->visualid = 0x21;
    vi->screen = 0;
    vi->depth = 16;
    vi->red_mask = 0x1f;
    vi->green_mask = 0x3f;
    vi->blue_mask = 0x1f;
    vi->colormap_size = 16;
    vi->bits_per_rgb = log2(16);
    vi->visual->visualid = 0x21;
    vi->visual->bits_per_rgb = log2(16);
    vi->visual->red_mask = 0x1f << 11;
    vi->visual->green_mask = 0x3f << 5;
    vi->visual->blue_mask = 0x1f;
    vi->visual->c_class = TrueColor;
    vi->visual->map_entries = 0x20;
    return vi;
}

GLAPI GLXContext APIENTRY glXCreateContext(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] XVisualInfo* vis,
    [[maybe_unused]] GLXContext shareList,
    [[maybe_unused]] Bool direct)
{
    SPDLOG_DEBUG("glXCreateContext called");

#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_TRACE
    spdlog::set_level(spdlog::level::trace);
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
    spdlog::set_level(spdlog::level::debug);
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_INFO
    spdlog::set_level(spdlog::level::info);
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_WARN
    spdlog::set_level(spdlog::level::warn);
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_ERROR
    spdlog::set_level(spdlog::level::err);
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_CRITICAL
    spdlog::set_level(spdlog::level::critical);
#endif

    guard.getInst().setRenderResolution(RESOLUTION_W, RESOLUTION_H);
    return reinterpret_cast<GLXContext>(&guard.getInst());
}

GLAPI void APIENTRY glXDestroyContext([[maybe_unused]] Display* dpy, [[maybe_unused]] GLXContext ctx)
{
    SPDLOG_WARN("glXDestroyContext not implemented");
}

GLAPI Bool APIENTRY glXMakeCurrent(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXDrawable drawable,
    [[maybe_unused]] GLXContext ctx)
{
    SPDLOG_DEBUG("glXMakeCurrent called");
    // Nothing todo. Only one context exists
    return 1;
}

GLAPI void APIENTRY glXCopyContext(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXContext src,
    [[maybe_unused]] GLXContext dst,
    [[maybe_unused]] unsigned long mask)
{
    SPDLOG_WARN("glXCopyContext not implemented");
}

GLAPI void APIENTRY glXSwapBuffers([[maybe_unused]] Display* dpy, [[maybe_unused]] GLXDrawable drawable)
{
    SPDLOG_DEBUG("glXSwapBuffers called");
    guard.render();
}

GLAPI GLXPixmap APIENTRY glXCreateGLXPixmap(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] XVisualInfo* visual,
    [[maybe_unused]] Pixmap pixmap)
{
    SPDLOG_WARN("glXCreateGLXPixmap not implemented");
    return 0;
}

GLAPI void APIENTRY glXDestroyGLXPixmap([[maybe_unused]] Display* dpy, [[maybe_unused]] GLXPixmap pixmap)
{
    SPDLOG_WARN("glXDestroyGLXPixmap not implemented");
}

GLAPI Bool APIENTRY glXQueryExtension([[maybe_unused]] Display* dpy, int* errorb, int* event)
{
    SPDLOG_DEBUG("glXQueryExtension called");
    *errorb = 0;
    *event = 0;
    return 1;
}

GLAPI Bool APIENTRY glXQueryVersion([[maybe_unused]] Display* dpy, [[maybe_unused]] int* maj, [[maybe_unused]] int* min)
{
    SPDLOG_WARN("glXQueryVersion not implemented");
    return 0;
}

GLAPI Bool APIENTRY glXIsDirect([[maybe_unused]] Display* dpy, [[maybe_unused]] GLXContext ctx)
{
    SPDLOG_WARN("glXIsDirect not implemented");
    return 0;
}

GLAPI int APIENTRY glXGetConfig(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] XVisualInfo* visual,
    [[maybe_unused]] int attrib,
    [[maybe_unused]] int* value)
{
    SPDLOG_WARN("glXGetConfig not implemented");
    return 0;
}

GLAPI GLXContext APIENTRY glXGetCurrentContext(void)
{
    SPDLOG_DEBUG("glXGetCurrentContext called");
    return reinterpret_cast<GLXContext>(&guard.getInst());
}

GLAPI GLXDrawable APIENTRY glXGetCurrentDrawable(void)
{
    SPDLOG_WARN("glXGetCurrentDrawable not implemented");
    return 1;
}

GLAPI void APIENTRY glXWaitGL(void)
{
    SPDLOG_WARN("glXWaitGL not implemented");
}

GLAPI void APIENTRY glXWaitX(void)
{
    SPDLOG_WARN("glXWaitX not implemented");
}

GLAPI void APIENTRY glXUseXFont(
    [[maybe_unused]] Font font,
    [[maybe_unused]] int first,
    [[maybe_unused]] int count,
    [[maybe_unused]] int list)
{
    SPDLOG_WARN("glXUseXFont not implemented");
}

/* GLX 1.1 and later */
GLAPI const char* APIENTRY glXQueryExtensionsString([[maybe_unused]] Display* dpy, [[maybe_unused]] int screen)
{
    SPDLOG_WARN("glXQueryExtensionsString not implemented");
    return nullptr;
}

GLAPI const char* APIENTRY glXQueryServerString([[maybe_unused]] Display* dpy, [[maybe_unused]] int screen, [[maybe_unused]] int name)
{
    SPDLOG_WARN("glXQueryServerString not implemented");
    return nullptr;
}

GLAPI const char* APIENTRY glXGetClientString([[maybe_unused]] Display* dpy, [[maybe_unused]] int name)
{
    SPDLOG_WARN("glXGetClientString not implemented");
    return nullptr;
}

/* GLX 1.2 and later */
GLAPI Display* APIENTRY glXGetCurrentDisplay(void)
{
    SPDLOG_WARN("glXGetCurrentDisplay not implemented");
    return nullptr;
}

/* GLX 1.3 and later */
GLAPI GLXFBConfig* APIENTRY glXChooseFBConfig(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] int screen,
    [[maybe_unused]] const int* attribList,
    [[maybe_unused]] int* nitems)
{
    SPDLOG_WARN("glXChooseFBConfig not implemented");
    return nullptr;
}

GLAPI int APIENTRY glXGetFBConfigAttrib(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXFBConfig config,
    [[maybe_unused]] int attribute,
    [[maybe_unused]] int* value)
{
    SPDLOG_WARN("glXGetFBConfigAttrib not implemented");
    return 0;
}

GLAPI GLXFBConfig* APIENTRY glXGetFBConfigs(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] int screen,
    [[maybe_unused]] int* nelements)
{
    SPDLOG_WARN("glXGetFBConfigs not implemented");
    return nullptr;
}

GLAPI XVisualInfo* APIENTRY glXGetVisualFromFBConfig(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXFBConfig config)
{
    SPDLOG_WARN("glXGetVisualFromFBConfig not implemented");
    return nullptr;
}

GLAPI GLXWindow APIENTRY glXCreateWindow(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXFBConfig config,
    [[maybe_unused]] Window win,
    [[maybe_unused]] const int* attribList)
{
    SPDLOG_WARN("glXCreateWindow not implemented");
    return 0;
}

GLAPI void APIENTRY glXDestroyWindow([[maybe_unused]] Display* dpy, [[maybe_unused]] GLXWindow window)
{
    SPDLOG_WARN("glXDestroyWindow not implemented");
}

GLAPI GLXPixmap APIENTRY glXCreatePixmap(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXFBConfig config,
    [[maybe_unused]] Pixmap pixmap,
    [[maybe_unused]] const int* attribList)
{
    SPDLOG_WARN("glXCreatePixmap not implemented");
    return 0;
}

GLAPI void APIENTRY glXDestroyPixmap([[maybe_unused]] Display* dpy, [[maybe_unused]] GLXPixmap pixmap)
{
    SPDLOG_WARN("glXDestroyPixmap not implemented");
}

GLAPI GLXPbuffer APIENTRY glXCreatePbuffer(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXFBConfig config,
    [[maybe_unused]] const int* attribList)
{
    SPDLOG_WARN("glXCreatePbuffer not implemented");
    return 0;
}

GLAPI void APIENTRY glXDestroyPbuffer([[maybe_unused]] Display* dpy, [[maybe_unused]] GLXPbuffer pbuf)
{
    SPDLOG_WARN("glXDestroyPbuffer not implemented");
}

GLAPI void APIENTRY glXQueryDrawable(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXDrawable draw,
    [[maybe_unused]] int attribute,
    [[maybe_unused]] unsigned int* value)
{
    SPDLOG_WARN("glXQueryDrawable not implemented");
}

GLAPI GLXContext APIENTRY glXCreateNewContext(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXFBConfig config,
    [[maybe_unused]] int renderType,
    [[maybe_unused]] GLXContext shareList,
    [[maybe_unused]] Bool direct)
{
    SPDLOG_WARN("glXCreateNewContext not implemented");
    return nullptr;
}

GLAPI Bool APIENTRY glXMakeContextCurrent(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXDrawable draw,
    [[maybe_unused]] GLXDrawable read,
    [[maybe_unused]] GLXContext ctx)
{
    SPDLOG_WARN("glXMakeContextCurrent not implemented");
    return 0;
}

GLAPI GLXDrawable APIENTRY glXGetCurrentReadDrawable(void)
{
    SPDLOG_WARN("glXGetCurrentReadDrawable not implemented");
    return 0;
}

GLAPI int APIENTRY glXQueryContext(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXContext ctx,
    [[maybe_unused]] int attribute,
    [[maybe_unused]] int* value)
{
    SPDLOG_WARN("glXQueryContext not implemented");
    return 0;
}

GLAPI void APIENTRY glXSelectEvent(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXDrawable drawable,
    [[maybe_unused]] unsigned long mask)
{
    SPDLOG_WARN("glXSelectEvent not implemented");
}

GLAPI void APIENTRY glXGetSelectedEvent(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXDrawable drawable,
    [[maybe_unused]] unsigned long* mask)
{
    SPDLOG_WARN("glXGetSelectedEvent not implemented");
}

GLAPI __GLXextFuncPtr APIENTRY glXGetProcAddressARB([[maybe_unused]] const GLubyte* s)
{
    SPDLOG_DEBUG("glXGetProcAddressARB {} called", s);
    return reinterpret_cast<__GLXextFuncPtr>(guard.getInst().getLibProcedure(reinterpret_cast<const char*>(s)));
}
