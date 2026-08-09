/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef GLX_H
#define GLX_H

#include "gl.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define GLX_VERSION_1_1 1
#define GLX_VERSION_1_2 1
#define GLX_VERSION_1_3 1
#define GLX_VERSION_1_4 1

#define GLX_EXTENSION_NAME "GLX"

/*
 * Tokens for glXChooseVisual and glXGetConfig:
 */
#define GLX_USE_GL 1
#define GLX_BUFFER_SIZE 2
#define GLX_LEVEL 3
#define GLX_RGBA 4
#define GLX_DOUBLEBUFFER 5
#define GLX_STEREO 6
#define GLX_AUX_BUFFERS 7
#define GLX_RED_SIZE 8
#define GLX_GREEN_SIZE 9
#define GLX_BLUE_SIZE 10
#define GLX_ALPHA_SIZE 11
#define GLX_DEPTH_SIZE 12
#define GLX_STENCIL_SIZE 13
#define GLX_ACCUM_RED_SIZE 14
#define GLX_ACCUM_GREEN_SIZE 15
#define GLX_ACCUM_BLUE_SIZE 16
#define GLX_ACCUM_ALPHA_SIZE 17

/*
 * Error codes returned by glXGetConfig:
 */
#define GLX_BAD_SCREEN 1
#define GLX_BAD_ATTRIBUTE 2
#define GLX_NO_EXTENSION 3
#define GLX_BAD_VISUAL 4
#define GLX_BAD_CONTEXT 5
#define GLX_BAD_VALUE 6
#define GLX_BAD_ENUM 7

/*
 * GLX 1.1 and later:
 */
#define GLX_VENDOR 1
#define GLX_VERSION 2
#define GLX_EXTENSIONS 3

/*
 * GLX 1.3 and later:
 */
#define GLX_CONFIG_CAVEAT 0x20
#define GLX_DONT_CARE 0xFFFFFFFF
#define GLX_X_VISUAL_TYPE 0x22
#define GLX_TRANSPARENT_TYPE 0x23
#define GLX_TRANSPARENT_INDEX_VALUE 0x24
#define GLX_TRANSPARENT_RED_VALUE 0x25
#define GLX_TRANSPARENT_GREEN_VALUE 0x26
#define GLX_TRANSPARENT_BLUE_VALUE 0x27
#define GLX_TRANSPARENT_ALPHA_VALUE 0x28
#define GLX_WINDOW_BIT 0x00000001
#define GLX_PIXMAP_BIT 0x00000002
#define GLX_PBUFFER_BIT 0x00000004
#define GLX_AUX_BUFFERS_BIT 0x00000010
#define GLX_FRONT_LEFT_BUFFER_BIT 0x00000001
#define GLX_FRONT_RIGHT_BUFFER_BIT 0x00000002
#define GLX_BACK_LEFT_BUFFER_BIT 0x00000004
#define GLX_BACK_RIGHT_BUFFER_BIT 0x00000008
#define GLX_DEPTH_BUFFER_BIT 0x00000020
#define GLX_STENCIL_BUFFER_BIT 0x00000040
#define GLX_ACCUM_BUFFER_BIT 0x00000080
#define GLX_NONE 0x8000
#define GLX_SLOW_CONFIG 0x8001
#define GLX_TRUE_COLOR 0x8002
#define GLX_DIRECT_COLOR 0x8003
#define GLX_PSEUDO_COLOR 0x8004
#define GLX_STATIC_COLOR 0x8005
#define GLX_GRAY_SCALE 0x8006
#define GLX_STATIC_GRAY 0x8007
#define GLX_TRANSPARENT_RGB 0x8008
#define GLX_TRANSPARENT_INDEX 0x8009
#define GLX_VISUAL_ID 0x800B
#define GLX_SCREEN 0x800C
#define GLX_NON_CONFORMANT_CONFIG 0x800D
#define GLX_DRAWABLE_TYPE 0x8010
#define GLX_RENDER_TYPE 0x8011
#define GLX_X_RENDERABLE 0x8012
#define GLX_FBCONFIG_ID 0x8013
#define GLX_RGBA_TYPE 0x8014
#define GLX_COLOR_INDEX_TYPE 0x8015
#define GLX_MAX_PBUFFER_WIDTH 0x8016
#define GLX_MAX_PBUFFER_HEIGHT 0x8017
#define GLX_MAX_PBUFFER_PIXELS 0x8018
#define GLX_PRESERVED_CONTENTS 0x801B
#define GLX_LARGEST_PBUFFER 0x801C
#define GLX_WIDTH 0x801D
#define GLX_HEIGHT 0x801E
#define GLX_EVENT_MASK 0x801F
#define GLX_DAMAGED 0x8020
#define GLX_SAVED 0x8021
#define GLX_WINDOW 0x8022
#define GLX_PBUFFER 0x8023
#define GLX_PBUFFER_HEIGHT 0x8040
#define GLX_PBUFFER_WIDTH 0x8041
#define GLX_RGBA_BIT 0x00000001
#define GLX_COLOR_INDEX_BIT 0x00000002
#define GLX_PBUFFER_CLOBBER_MASK 0x08000000

/*
 * GLX 1.4 and later:
 */
#define GLX_SAMPLE_BUFFERS 0x186a0 /*100000*/
#define GLX_SAMPLES 0x186a1 /*100001*/

    typedef struct __GLXcontextRec* GLXContext;
    typedef XID GLXPixmap;
    typedef XID GLXDrawable;
    /* GLX 1.3 and later */
    typedef struct __GLXFBConfigRec* GLXFBConfig;
    typedef XID GLXFBConfigID;
    typedef XID GLXContextID;
    typedef XID GLXWindow;
    typedef XID GLXPbuffer;

/*
** Events.
** __GLX_NUMBER_EVENTS is set to 17 to account for the BufferClobberSGIX
**  event - this helps initialization if the server supports the pbuffer
**  extension and the client doesn't.
*/
#define GLX_PbufferClobber 0
#define GLX_BufferSwapComplete 1

#define __GLX_NUMBER_EVENTS 17

    GLAPI XVisualInfo* APIENTRY glXChooseVisual(Display* dpy, int screen,
        int* attribList);

    GLAPI GLXContext APIENTRY glXCreateContext(Display* dpy, XVisualInfo* vis,
        GLXContext shareList, Bool direct);

    GLAPI void APIENTRY glXDestroyContext(Display* dpy, GLXContext ctx);

    GLAPI Bool APIENTRY glXMakeCurrent(Display* dpy, GLXDrawable drawable,
        GLXContext ctx);

    GLAPI void APIENTRY glXCopyContext(Display* dpy, GLXContext src, GLXContext dst,
        unsigned long mask);

    GLAPI void APIENTRY glXSwapBuffers(Display* dpy, GLXDrawable drawable);

    GLAPI GLXPixmap APIENTRY glXCreateGLXPixmap(Display* dpy, XVisualInfo* visual,
        Pixmap pixmap);

    GLAPI void APIENTRY glXDestroyGLXPixmap(Display* dpy, GLXPixmap pixmap);

    GLAPI Bool APIENTRY glXQueryExtension(Display* dpy, int* errorb, int* event);

    GLAPI Bool APIENTRY glXQueryVersion(Display* dpy, int* maj, int* min);

    GLAPI Bool APIENTRY glXIsDirect(Display* dpy, GLXContext ctx);

    GLAPI int APIENTRY glXGetConfig(Display* dpy, XVisualInfo* visual,
        int attrib, int* value);

    GLAPI GLXContext APIENTRY glXGetCurrentContext(void);

    GLAPI GLXDrawable APIENTRY glXGetCurrentDrawable(void);

    GLAPI void APIENTRY glXWaitGL(void);

    GLAPI void APIENTRY glXWaitX(void);

    GLAPI void APIENTRY glXUseXFont(Font font, int first, int count, int list);

    /* GLX 1.1 and later */
    GLAPI const char* APIENTRY glXQueryExtensionsString(Display* dpy, int screen);

    GLAPI const char* APIENTRY glXQueryServerString(Display* dpy, int screen, int name);

    GLAPI const char* APIENTRY glXGetClientString(Display* dpy, int name);

    /* GLX 1.2 and later */
    GLAPI Display* APIENTRY glXGetCurrentDisplay(void);

    /* GLX 1.3 and later */
    GLAPI GLXFBConfig* APIENTRY glXChooseFBConfig(Display* dpy, int screen,
        const int* attribList, int* nitems);

    GLAPI int APIENTRY glXGetFBConfigAttrib(Display* dpy, GLXFBConfig config,
        int attribute, int* value);

    GLAPI GLXFBConfig* APIENTRY glXGetFBConfigs(Display* dpy, int screen,
        int* nelements);

    GLAPI XVisualInfo* APIENTRY glXGetVisualFromFBConfig(Display* dpy,
        GLXFBConfig config);

    GLAPI GLXWindow APIENTRY glXCreateWindow(Display* dpy, GLXFBConfig config,
        Window win, const int* attribList);

    GLAPI void APIENTRY glXDestroyWindow(Display* dpy, GLXWindow window);

    GLAPI GLXPixmap APIENTRY glXCreatePixmap(Display* dpy, GLXFBConfig config,
        Pixmap pixmap, const int* attribList);

    GLAPI void APIENTRY glXDestroyPixmap(Display* dpy, GLXPixmap pixmap);

    GLAPI GLXPbuffer APIENTRY glXCreatePbuffer(Display* dpy, GLXFBConfig config,
        const int* attribList);

    GLAPI void APIENTRY glXDestroyPbuffer(Display* dpy, GLXPbuffer pbuf);

    GLAPI void APIENTRY glXQueryDrawable(Display* dpy, GLXDrawable draw, int attribute,
        unsigned int* value);

    GLAPI GLXContext APIENTRY glXCreateNewContext(Display* dpy, GLXFBConfig config,
        int renderType, GLXContext shareList,
        Bool direct);

    GLAPI Bool APIENTRY glXMakeContextCurrent(Display* dpy, GLXDrawable draw,
        GLXDrawable read, GLXContext ctx);

    GLAPI GLXDrawable APIENTRY glXGetCurrentReadDrawable(void);

    GLAPI int APIENTRY glXQueryContext(Display* dpy, GLXContext ctx, int attribute,
        int* value);

    GLAPI void APIENTRY glXSelectEvent(Display* dpy, GLXDrawable drawable,
        unsigned long mask);

    GLAPI void APIENTRY glXGetSelectedEvent(Display* dpy, GLXDrawable drawable,
        unsigned long* mask);

    /* GLX 1.3 function pointer typedefs */
    typedef GLXFBConfig* (*PFNGLXGETFBCONFIGSPROC)(Display* dpy, int screen, int* nelements);
    typedef GLXFBConfig* (*PFNGLXCHOOSEFBCONFIGPROC)(Display* dpy, int screen, const int* attrib_list, int* nelements);
    typedef int (*PFNGLXGETFBCONFIGATTRIBPROC)(Display* dpy, GLXFBConfig config, int attribute, int* value);
    typedef XVisualInfo* (*PFNGLXGETVISUALFROMFBCONFIGPROC)(Display* dpy, GLXFBConfig config);
    typedef GLXWindow (*PFNGLXCREATEWINDOWPROC)(Display* dpy, GLXFBConfig config, Window win, const int* attrib_list);
    typedef void (*PFNGLXDESTROYWINDOWPROC)(Display* dpy, GLXWindow win);
    typedef GLXPixmap (*PFNGLXCREATEPIXMAPPROC)(Display* dpy, GLXFBConfig config, Pixmap pixmap, const int* attrib_list);
    typedef void (*PFNGLXDESTROYPIXMAPPROC)(Display* dpy, GLXPixmap pixmap);
    typedef GLXPbuffer (*PFNGLXCREATEPBUFFERPROC)(Display* dpy, GLXFBConfig config, const int* attrib_list);
    typedef void (*PFNGLXDESTROYPBUFFERPROC)(Display* dpy, GLXPbuffer pbuf);
    typedef void (*PFNGLXQUERYDRAWABLEPROC)(Display* dpy, GLXDrawable draw, int attribute, unsigned int* value);
    typedef GLXContext (*PFNGLXCREATENEWCONTEXTPROC)(Display* dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct);
    typedef Bool (*PFNGLXMAKECONTEXTCURRENTPROC)(Display* dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx);
    typedef GLXDrawable (*PFNGLXGETCURRENTREADDRAWABLEPROC)(void);
    typedef Display* (*PFNGLXGETCURRENTDISPLAYPROC)(void);
    typedef int (*PFNGLXQUERYCONTEXTPROC)(Display* dpy, GLXContext ctx, int attribute, int* value);
    typedef void (*PFNGLXSELECTEVENTPROC)(Display* dpy, GLXDrawable draw, unsigned long event_mask);
    typedef void (*PFNGLXGETSELECTEDEVENTPROC)(Display* dpy, GLXDrawable draw, unsigned long* event_mask);

/*
 * ARB 2. GLX_ARB_get_proc_address
 */
#ifndef GLX_ARB_get_proc_address
#define GLX_ARB_get_proc_address 1

    typedef void (*__GLXextFuncPtr)(void);
    GLAPI __GLXextFuncPtr APIENTRY glXGetProcAddressARB(const GLubyte*);

#endif /* GLX_ARB_get_proc_address */

/*
 * GLX_EXT_swap_control
 */
#ifndef GLX_EXT_swap_control
#define GLX_EXT_swap_control 1

    GLAPI void APIENTRY glXSwapIntervalEXT(Display* dpy, GLXDrawable drawable, int interval);
    typedef void (*PFNGLXSWAPINTERVALEXTPROC)(Display* dpy, GLXDrawable drawable, int interval);

#endif /* GLX_EXT_swap_control */

/*
 * GLX_ARB_create_context
 */
#ifndef GLX_ARB_create_context
#define GLX_ARB_create_context 1

#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
#define GLX_CONTEXT_FLAGS_ARB 0x2094
#define GLX_CONTEXT_DEBUG_BIT_ARB 0x00000001
#define GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002

    GLAPI GLXContext APIENTRY glXCreateContextAttribsARB(Display* dpy, GLXFBConfig config,
        GLXContext share_context, Bool direct,
        const int* attrib_list);
    typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSARBPROC)(Display* dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int* attrib_list);

#endif /* GLX_ARB_create_context */

#ifdef __cplusplus
}
#endif

#endif
