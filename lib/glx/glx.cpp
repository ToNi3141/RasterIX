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
#include <cstdlib>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

static constexpr uint32_t RESOLUTION_H = 600;
static constexpr uint32_t RESOLUTION_W = 1024;

// The hardware supports exactly one fixed framebuffer format: RGB565 with a
// 16 bit depth buffer. This single config is exposed to all FBConfig queries.
struct __GLXFBConfigRec
{
    int visualId;
    int fbConfigId;
    int screen;
    int redSize;
    int greenSize;
    int blueSize;
    int alphaSize;
    int depthSize;
    int stencilSize;
    int bufferSize;
    int level;
    Bool doubleBuffer;
    int renderType;
    int drawableType;
};

static __GLXFBConfigRec g_fbConfig {
    /* visualId */ 0x21,
    /* fbConfigId */ 0x21,
    /* screen */ 0,
    /* redSize */ 5,
    /* greenSize */ 6,
    /* blueSize */ 5,
    /* alphaSize */ 0,
    /* depthSize */ 16,
    /* stencilSize */ 8,
    /* bufferSize */ 16,
    /* level */ 0,
    /* doubleBuffer */ True,
    /* renderType */ GLX_RGBA_BIT,
    /* drawableType */ GLX_WINDOW_BIT,
};

// The implementation advertises GLX 1.4, matching the highest GLX_VERSION_1_x
// token defined in the header.
static constexpr int GLX_IMPL_VERSION_MAJOR = 1;
static constexpr int GLX_IMPL_VERSION_MINOR = 4;
static constexpr char GLX_IMPL_VENDOR[] = "RasterIX";
static constexpr char GLX_IMPL_VERSION_STRING[] = "1.4";
static constexpr char GLX_IMPL_EXTENSIONS[] = "";

// glXGetCurrentDisplay returns the Display* of the most recently made-current
// context. It is captured whenever a context is made current.
static Display* g_currentDisplay = nullptr;

static XVisualInfo* createVisualInfo(Display* display, int screen)
{
    XVisualInfo templateInfo {};
    templateInfo.screen = screen;
    templateInfo.depth = 16;
    templateInfo.c_class = TrueColor;

    int count = 0;
    XVisualInfo* visuals = XGetVisualInfo(
        display,
        VisualScreenMask | VisualDepthMask | VisualClassMask,
        &templateInfo,
        &count
    );

    for (int i = 0; i < count; i++)
    {
        if (visuals[i].red_mask == 0xf800 &&
            visuals[i].green_mask == 0x07e0 &&
            visuals[i].blue_mask == 0x001f)
        {
            XVisualInfo* result = new XVisualInfo(visuals[i]);
            XFree(visuals);
            return result;
        }
    }

    XFree(visuals);
    return nullptr;
}

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
        rr::RIXGL::getInstance().addLibProcedure("glXChooseFBConfig", ADDRESS_OF(glXChooseFBConfig));
        rr::RIXGL::getInstance().addLibProcedure("glXGetFBConfigs", ADDRESS_OF(glXGetFBConfigs));
        rr::RIXGL::getInstance().addLibProcedure("glXGetFBConfigAttrib", ADDRESS_OF(glXGetFBConfigAttrib));
        rr::RIXGL::getInstance().addLibProcedure("glXGetVisualFromFBConfig", ADDRESS_OF(glXGetVisualFromFBConfig));
        rr::RIXGL::getInstance().addLibProcedure("glXCreateNewContext", ADDRESS_OF(glXCreateNewContext));
        rr::RIXGL::getInstance().addLibProcedure("glXCreateWindow", ADDRESS_OF(glXCreateWindow));
        rr::RIXGL::getInstance().addLibProcedure("glXMakeContextCurrent", ADDRESS_OF(glXMakeContextCurrent));
        rr::RIXGL::getInstance().addLibProcedure("glXQueryVersion", ADDRESS_OF(glXQueryVersion));
        rr::RIXGL::getInstance().addLibProcedure("glXIsDirect", ADDRESS_OF(glXIsDirect));
        rr::RIXGL::getInstance().addLibProcedure("glXGetConfig", ADDRESS_OF(glXGetConfig));
        rr::RIXGL::getInstance().addLibProcedure("glXQueryExtensionsString", ADDRESS_OF(glXQueryExtensionsString));
        rr::RIXGL::getInstance().addLibProcedure("glXQueryServerString", ADDRESS_OF(glXQueryServerString));
        rr::RIXGL::getInstance().addLibProcedure("glXGetClientString", ADDRESS_OF(glXGetClientString));
        rr::RIXGL::getInstance().addLibProcedure("glXGetCurrentDisplay", ADDRESS_OF(glXGetCurrentDisplay));
        rr::RIXGL::getInstance().addLibProcedure("glXSwapIntervalEXT", ADDRESS_OF(glXSwapIntervalEXT));
        rr::RIXGL::getInstance().addLibProcedure("glXCreateContextAttribsARB", ADDRESS_OF(glXCreateContextAttribsARB));
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
    Display* dpy,
    int screen,
    [[maybe_unused]] int* attribList)
{
    SPDLOG_DEBUG("glXChooseVisual called");
    return createVisualInfo(dpy, screen);
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
    Display* dpy,
    [[maybe_unused]] GLXDrawable drawable,
    [[maybe_unused]] GLXContext ctx)
{
    SPDLOG_DEBUG("glXMakeCurrent called");
    // Nothing todo. Only one context exists
    g_currentDisplay = dpy;
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

GLAPI Bool APIENTRY glXQueryVersion([[maybe_unused]] Display* dpy, int* maj, int* min)
{
    SPDLOG_DEBUG("glXQueryVersion called");
    if (maj != nullptr)
    {
        *maj = GLX_IMPL_VERSION_MAJOR;
    }
    if (min != nullptr)
    {
        *min = GLX_IMPL_VERSION_MINOR;
    }
    return True;
}

GLAPI Bool APIENTRY glXIsDirect([[maybe_unused]] Display* dpy, [[maybe_unused]] GLXContext ctx)
{
    SPDLOG_DEBUG("glXIsDirect called");
    // Rendering happens locally through the hardware, so the context is direct.
    return True;
}

GLAPI int APIENTRY glXGetConfig(
    [[maybe_unused]] Display* dpy,
    XVisualInfo* visual,
    int attrib,
    int* value)
{
    SPDLOG_DEBUG("glXGetConfig called");
    if (visual == nullptr || value == nullptr)
    {
        return GLX_BAD_VALUE;
    }

    switch (attrib)
    {
    case GLX_USE_GL:
        *value = True;
        break;
    case GLX_RGBA:
        *value = True;
        break;
    case GLX_BUFFER_SIZE:
        *value = g_fbConfig.bufferSize;
        break;
    case GLX_LEVEL:
        *value = g_fbConfig.level;
        break;
    case GLX_DOUBLEBUFFER:
        *value = g_fbConfig.doubleBuffer;
        break;
    case GLX_STEREO:
        *value = False;
        break;
    case GLX_AUX_BUFFERS:
        *value = 0;
        break;
    case GLX_RED_SIZE:
        *value = g_fbConfig.redSize;
        break;
    case GLX_GREEN_SIZE:
        *value = g_fbConfig.greenSize;
        break;
    case GLX_BLUE_SIZE:
        *value = g_fbConfig.blueSize;
        break;
    case GLX_ALPHA_SIZE:
        *value = g_fbConfig.alphaSize;
        break;
    case GLX_DEPTH_SIZE:
        *value = g_fbConfig.depthSize;
        break;
    case GLX_STENCIL_SIZE:
        *value = g_fbConfig.stencilSize;
        break;
    case GLX_ACCUM_RED_SIZE:
    case GLX_ACCUM_GREEN_SIZE:
    case GLX_ACCUM_BLUE_SIZE:
    case GLX_ACCUM_ALPHA_SIZE:
        *value = 0;
        break;
    default:
        SPDLOG_WARN("glXGetConfig unknown attribute {}", attrib);
        return GLX_BAD_ATTRIBUTE;
    }
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
    SPDLOG_DEBUG("glXQueryExtensionsString called");
    return GLX_IMPL_EXTENSIONS;
}

GLAPI const char* APIENTRY glXQueryServerString([[maybe_unused]] Display* dpy, [[maybe_unused]] int screen, int name)
{
    SPDLOG_DEBUG("glXQueryServerString called");
    switch (name)
    {
    case GLX_VENDOR:
        return GLX_IMPL_VENDOR;
    case GLX_VERSION:
        return GLX_IMPL_VERSION_STRING;
    case GLX_EXTENSIONS:
        return GLX_IMPL_EXTENSIONS;
    default:
        SPDLOG_WARN("glXQueryServerString unknown name {}", name);
        return nullptr;
    }
}

GLAPI const char* APIENTRY glXGetClientString([[maybe_unused]] Display* dpy, int name)
{
    SPDLOG_DEBUG("glXGetClientString called");
    switch (name)
    {
    case GLX_VENDOR:
        return GLX_IMPL_VENDOR;
    case GLX_VERSION:
        return GLX_IMPL_VERSION_STRING;
    case GLX_EXTENSIONS:
        return GLX_IMPL_EXTENSIONS;
    default:
        SPDLOG_WARN("glXGetClientString unknown name {}", name);
        return nullptr;
    }
}

/* GLX 1.2 and later */
GLAPI Display* APIENTRY glXGetCurrentDisplay(void)
{
    SPDLOG_DEBUG("glXGetCurrentDisplay called");
    return g_currentDisplay;
}

/* GLX 1.3 and later */
GLAPI GLXFBConfig* APIENTRY glXChooseFBConfig(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] int screen,
    [[maybe_unused]] const int* attribList,
    int* nitems)
{
    SPDLOG_DEBUG("glXChooseFBConfig called");
    // The hardware exposes a single fixed config, so any requested attributes are
    // ignored and the one available config is always returned.
    GLXFBConfig* configs = static_cast<GLXFBConfig*>(malloc(sizeof(GLXFBConfig)));
    configs[0] = &g_fbConfig;
    if (nitems != nullptr)
    {
        *nitems = 1;
    }
    return configs;
}

GLAPI int APIENTRY glXGetFBConfigAttrib(
    [[maybe_unused]] Display* dpy,
    GLXFBConfig config,
    int attribute,
    int* value)
{
    SPDLOG_DEBUG("glXGetFBConfigAttrib called");
    if (config == nullptr || value == nullptr)
    {
        return GLX_BAD_VALUE;
    }

    switch (attribute)
    {
    case GLX_FBCONFIG_ID:
        *value = config->fbConfigId;
        break;
    case GLX_VISUAL_ID:
        *value = config->visualId;
        break;
    case GLX_SCREEN:
        *value = config->screen;
        break;
    case GLX_BUFFER_SIZE:
        *value = config->bufferSize;
        break;
    case GLX_LEVEL:
        *value = config->level;
        break;
    case GLX_DOUBLEBUFFER:
        *value = config->doubleBuffer;
        break;
    case GLX_STEREO:
        *value = False;
        break;
    case GLX_AUX_BUFFERS:
        *value = 0;
        break;
    case GLX_RED_SIZE:
        *value = config->redSize;
        break;
    case GLX_GREEN_SIZE:
        *value = config->greenSize;
        break;
    case GLX_BLUE_SIZE:
        *value = config->blueSize;
        break;
    case GLX_ALPHA_SIZE:
        *value = config->alphaSize;
        break;
    case GLX_DEPTH_SIZE:
        *value = config->depthSize;
        break;
    case GLX_STENCIL_SIZE:
        *value = config->stencilSize;
        break;
    case GLX_ACCUM_RED_SIZE:
    case GLX_ACCUM_GREEN_SIZE:
    case GLX_ACCUM_BLUE_SIZE:
    case GLX_ACCUM_ALPHA_SIZE:
        *value = 0;
        break;
    case GLX_RENDER_TYPE:
        *value = config->renderType;
        break;
    case GLX_DRAWABLE_TYPE:
        *value = config->drawableType;
        break;
    case GLX_X_RENDERABLE:
        *value = True;
        break;
    case GLX_X_VISUAL_TYPE:
        *value = GLX_TRUE_COLOR;
        break;
    case GLX_CONFIG_CAVEAT:
        *value = GLX_NONE;
        break;
    case GLX_TRANSPARENT_TYPE:
        *value = GLX_NONE;
        break;
    case GLX_SAMPLE_BUFFERS:
    case GLX_SAMPLES:
        *value = 0;
        break;
    case GLX_MAX_PBUFFER_WIDTH:
    case GLX_MAX_PBUFFER_HEIGHT:
    case GLX_MAX_PBUFFER_PIXELS:
        *value = 0;
        break;
    default:
        SPDLOG_WARN("glXGetFBConfigAttrib unknown attribute {}", attribute);
        return GLX_BAD_ATTRIBUTE;
    }
    return 0;
}

GLAPI GLXFBConfig* APIENTRY glXGetFBConfigs(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] int screen,
    int* nelements)
{
    SPDLOG_DEBUG("glXGetFBConfigs called");
    GLXFBConfig* configs = static_cast<GLXFBConfig*>(malloc(sizeof(GLXFBConfig)));
    configs[0] = &g_fbConfig;
    if (nelements != nullptr)
    {
        *nelements = 1;
    }
    return configs;
}

GLAPI XVisualInfo* APIENTRY glXGetVisualFromFBConfig(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXFBConfig config)
{
    SPDLOG_DEBUG("glXGetVisualFromFBConfig called");
    if (!config)
        return nullptr;

    int count = 0;
    XVisualInfo templateInfo {};

    templateInfo.screen = config->screen;
    templateInfo.depth = 16;
    templateInfo.c_class = TrueColor;

    XVisualInfo* visuals = XGetVisualInfo(
        dpy,
        VisualScreenMask | VisualDepthMask | VisualClassMask,
        &templateInfo,
        &count
    );
    return visuals;
}

GLAPI GLXWindow APIENTRY glXCreateWindow(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXFBConfig config,
    Window win,
    [[maybe_unused]] const int* attribList)
{
    SPDLOG_DEBUG("glXCreateWindow called");
    // Only a single drawable exists; map the GLX window onto the X window.
    return win;
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
    SPDLOG_DEBUG("glXCreateNewContext called");
    guard.getInst().setRenderResolution(RESOLUTION_W, RESOLUTION_H);
    return reinterpret_cast<GLXContext>(&guard.getInst());
}

GLAPI Bool APIENTRY glXMakeContextCurrent(
    Display* dpy,
    [[maybe_unused]] GLXDrawable draw,
    [[maybe_unused]] GLXDrawable read,
    [[maybe_unused]] GLXContext ctx)
{
    SPDLOG_DEBUG("glXMakeContextCurrent called");
    // Nothing todo. Only one context exists
    g_currentDisplay = dpy;
    return 1;
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

GLAPI void APIENTRY glXSwapIntervalEXT(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXDrawable drawable,
    [[maybe_unused]] int interval)
{
    SPDLOG_DEBUG("glXSwapIntervalEXT called");
    // The hardware presents every swap unconditionally; the swap interval is
    // not configurable, so this is intentionally a no-op.
}

GLAPI GLXContext APIENTRY glXCreateContextAttribsARB(
    [[maybe_unused]] Display* dpy,
    [[maybe_unused]] GLXFBConfig config,
    [[maybe_unused]] GLXContext share_context,
    [[maybe_unused]] Bool direct,
    [[maybe_unused]] const int* attrib_list)
{
    SPDLOG_DEBUG("glXCreateContextAttribsARB called");
    return glXCreateContext(dpy, nullptr, share_context, direct);
}
