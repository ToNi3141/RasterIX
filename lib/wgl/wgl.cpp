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

#include "wgl.h"
#include "FT60XBusConnector.hpp"
#include "MultiThreadRunner.hpp"
#include "NoThreadRunner.hpp"
#include "RIXGL.hpp"
#include "renderer/dse/DmaStreamEngine.hpp"
#include "renderer/threadedrasterizer/ThreadedRasterizer.hpp"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

using namespace rr;

static const uint32_t RESOLUTION_H = 600;
static const uint32_t RESOLUTION_W = 1024;

class GLInitGuard
{
public:
    GLInitGuard()
    {
        rr::RIXGL::createInstance(m_threadedRasterizer);
#define ADDRESS_OF(X) reinterpret_cast<const void*>(&X)
        rr::RIXGL::getInstance().addLibExtension("WGL_ARB_extensions_string");
        rr::RIXGL::getInstance().addLibExtension("WGL_ARB_render_texture");
        rr::RIXGL::getInstance().addLibExtension("WGL_ARB_pixel_format");
        rr::RIXGL::getInstance().addLibExtension("WGL_ARB_pbuffer");
        rr::RIXGL::getInstance().addLibProcedure("wglGetExtensionsStringARB", ADDRESS_OF(impl_wglGetExtensionsString));
        rr::RIXGL::getInstance().addLibProcedure("wglCreatePbufferARB", ADDRESS_OF(impl_wglCreatePbufferARB));
        rr::RIXGL::getInstance().addLibProcedure("wglGetPbufferDCARB", ADDRESS_OF(impl_wglGetPbufferDCARB));
        rr::RIXGL::getInstance().addLibProcedure("wglReleasePbufferDCARB", ADDRESS_OF(impl_wglReleasePbufferDCARB));
        rr::RIXGL::getInstance().addLibProcedure("wglDestroyPbufferARB", ADDRESS_OF(impl_wglDestroyPbufferARB));
        rr::RIXGL::getInstance().addLibProcedure("wglQueryPbufferARB", ADDRESS_OF(impl_wglQueryPbufferARB));
        rr::RIXGL::getInstance().addLibProcedure("wglGetPixelFormatAttribivARB", ADDRESS_OF(impl_wglGetPixelFormatAttribivARB));
        rr::RIXGL::getInstance().addLibProcedure("wglGetPixelFormatAttribfvARB", ADDRESS_OF(impl_wglGetPixelFormatAttribfvARB));
        rr::RIXGL::getInstance().addLibProcedure("wglChoosePixelFormatARB", ADDRESS_OF(impl_wglChoosePixelFormatARB));
        rr::RIXGL::getInstance().addLibProcedure("wglBindTexImageARB", ADDRESS_OF(impl_wglBindTexImageARB));
        rr::RIXGL::getInstance().addLibProcedure("wglReleaseTexImageARB", ADDRESS_OF(impl_wglReleaseTexImageARB));
        rr::RIXGL::getInstance().addLibProcedure("wglSetPbufferAttribARB", ADDRESS_OF(impl_wglSetPbufferAttribARB));
#undef ADDRESS_OF
    }
    ~GLInitGuard()
    {
    }

    void deinit()
    {
        rr::RIXGL::destroy();
    }

    void render()
    {
        rr::RIXGL::getInstance().swapDisplayList();
    }

private:
    rr::MultiThreadRunner m_workerThread {};
    rr::MultiThreadRunner m_uploadThread {};
    FT60XBusConnector m_busConnector {};
    rr::dsec::DmaStreamEngine m_dseDevice { m_busConnector };
    rr::ThreadedRasterizer m_threadedRasterizer { m_dseDevice, m_workerThread, m_uploadThread };
} guard;

// Wiggle API
// -------------------------------------------------------
GLAPI int APIENTRY impl_wglChoosePixelFormat(HDC hdc, CONST PIXELFORMATDESCRIPTOR* ppfd)
{
    SPDLOG_DEBUG("wglChoosePixelFormat Flags: {}, Stencil Bits: {}, Accum Bits: {}, Color Bits: {}, Alpha Bits: {}, Red Bits: {}, Green Bits: {}, Blue Bits: {}, Depth Bits: {} called", ppfd->dwFlags, ppfd->cStencilBits, ppfd->cAccumBits, ppfd->cColorBits, ppfd->cAlphaBits, ppfd->cRedBits, ppfd->cGreenBits, ppfd->cBlueBits, ppfd->cDepthBits);
    return 1;
}

GLAPI BOOL APIENTRY impl_wglCopyContext(HGLRC hglrc, HGLRC hglrc2, UINT i)
{
    SPDLOG_WARN("wglCopyContext not implemented");
    return false;
}

GLAPI HGLRC APIENTRY impl_wglCreateContext(HDC hdc)
{
    SPDLOG_DEBUG("wglCreateContext called");
    static auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("RIXGL.log", "basic-log.txt");
    file_sink->set_level(static_cast<spdlog::level::level_enum>(SPDLOG_ACTIVE_LEVEL));
    auto logger = std::make_shared<spdlog::logger>("RIXGL", file_sink);
    logger->set_level(static_cast<spdlog::level::level_enum>(SPDLOG_ACTIVE_LEVEL));
    logger->info("RIXGL started");

    // or you can even set multi_sink logger as default logger
    spdlog::set_default_logger(logger);
    RIXGL::getInstance().setRenderResolution(RESOLUTION_W, RESOLUTION_H);

    return reinterpret_cast<HGLRC>(&RIXGL::getInstance());
}

GLAPI HGLRC APIENTRY impl_wglCreateLayerContext(HDC hdc, int iLayerPlane)
{
    SPDLOG_WARN("wglCreateLayerContext not implemented");
    return reinterpret_cast<HGLRC>(&RIXGL::getInstance());
}

GLAPI BOOL APIENTRY impl_wglDeleteContext(HGLRC hglrc)
{
    SPDLOG_DEBUG("wglDeleteContext called");
    guard.deinit();
    return TRUE;
}

GLAPI BOOL APIENTRY impl_wglDescribeLayerPlane(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nBytes, LPLAYERPLANEDESCRIPTOR plpd)
{
    SPDLOG_WARN("wglDescribeLayerPlane not implemented");
    return false;
}

GLAPI int APIENTRY impl_wglDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd)
{
    SPDLOG_DEBUG("wglDescribePixelFormat Pixelformat: {}, Bytes: {} called", iPixelFormat, nBytes);

    if (ppfd != nullptr)
    {

        memset(ppfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

        ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
        ppfd->nVersion = 1;
        ppfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL;
        ppfd->iPixelType = PFD_TYPE_RGBA;
        ppfd->cColorBits = 32;
        ppfd->cAlphaBits = 8;
        ppfd->cAlphaShift = 24;
        ppfd->cRedBits = 8;
        ppfd->cRedShift = 16;
        ppfd->cGreenBits = 8;
        ppfd->cGreenShift = 8;
        ppfd->cBlueBits = 8;
        ppfd->cBlueShift = 0;
        ppfd->cDepthBits = 24;
    }

    return 1;
}

GLAPI HGLRC APIENTRY impl_wglGetCurrentContext()
{
    SPDLOG_DEBUG("wglGetCurrentContext called");
    return reinterpret_cast<HGLRC>(&RIXGL::getInstance());
}

GLAPI HDC APIENTRY impl_wglGetCurrentDC()
{
    SPDLOG_WARN("wglGetCurrentDC not implemented");
    return reinterpret_cast<HDC>(1);
}

GLAPI int APIENTRY impl_wglGetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, COLORREF* pcr)
{
    SPDLOG_WARN("wglGetLayerPaletteEntries not implemented");
    return 0;
}

GLAPI int APIENTRY impl_wglGetPixelFormat(HDC hdc)
{
    SPDLOG_DEBUG("wglGetPixelFormat called");
    return 1;
}

GLAPI PROC APIENTRY impl_wglGetProcAddress(LPCSTR s)
{
    SPDLOG_INFO("wglGetProcAddress {} called", s);

    return reinterpret_cast<PROC>(RIXGL::getInstance().getLibProcedure(s));
}

GLAPI BOOL APIENTRY impl_wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
    SPDLOG_WARN("wglMakeCurrent not implemented");
    return TRUE;
}

GLAPI BOOL APIENTRY impl_wglRealizeLayerPalette(HDC hdc, int iLayerPlane, BOOL b)
{
    SPDLOG_WARN("wglRealizeLayerPalette not implemented");
    return FALSE;
}

GLAPI int APIENTRY impl_wglSetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, CONST COLORREF* pcr)
{
    SPDLOG_WARN("wglSetLayerPaletteEntries not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglSetPixelFormat(HDC hdc, int format, CONST PIXELFORMATDESCRIPTOR* ppfd)
{
    SPDLOG_DEBUG("wglSetPixelFormat Format: {}", format);

    return format == 1;
}

GLAPI BOOL APIENTRY impl_wglShareLists(HGLRC hglrc, HGLRC hglrc2)
{
    SPDLOG_WARN("wglShareLists not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglSwapBuffers(HDC hdc)
{
    SPDLOG_DEBUG("wglSwapBuffers called");
    guard.render();
    return TRUE;
}

GLAPI BOOL APIENTRY impl_wglSwapLayerBuffers(HDC hdc, UINT planes)
{
    SPDLOG_DEBUG("wglSwapLayerBuffers planes {} called", planes);

    if ((planes & WGL_SWAP_MAIN_PLANE) != 0U)
    {

        guard.render();
    }

    return TRUE;
}

GLAPI BOOL APIENTRY impl_wglUseFontBitmapsA(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3)
{
    SPDLOG_WARN("wglUseFontBitmapsA not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglUseFontBitmapsW(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3)
{
    SPDLOG_WARN("wglUseFontBitmapsW not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglUseFontOutlinesA(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3, FLOAT f1, FLOAT f2, int i, LPGLYPHMETRICSFLOAT pgmf)
{
    SPDLOG_WARN("wglUseFontOutlinesA not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglUseFontOutlinesW(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3, FLOAT f1, FLOAT f2, int i, LPGLYPHMETRICSFLOAT pgmf)
{
    SPDLOG_WARN("wglUseFontOutlinesW not implemented");
    return FALSE;
}

// -------------------------------------------------------

// Some extensions
// -------------------------------------------------------
GLAPI BOOL APIENTRY impl_wglSetDeviceGammaRamp(HDC hdc, LPVOID lpRamp)
{
    SPDLOG_WARN("wglSetDeviceGammaRamp not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglGetDeviceGammaRamp(HDC hdc, LPVOID lpRamp)
{
    SPDLOG_WARN("wglGetDeviceGammaRamp not implemented");
    return FALSE;
}

GLAPI const char* APIENTRY impl_wglGetExtensionsString(HDC hdc)
{
    SPDLOG_DEBUG("wglGetExtensionsString called");
    return rr::RIXGL::getInstance().getLibExtensions();
}

GLAPI HPBUFFERARB APIENTRY impl_wglCreatePbufferARB(HDC hDC,
    int iPixelFormat,
    int iWidth,
    int iHeight,
    const int* piAttribList)
{
    SPDLOG_WARN("wglCreatePbufferARB not implemented");
    return reinterpret_cast<HPBUFFERARB>(3);
}

GLAPI HDC APIENTRY impl_wglGetPbufferDCARB(HPBUFFERARB hPbuffer)
{
    SPDLOG_WARN("wglGetPbufferDCARB not implemented");
    return reinterpret_cast<HDC>(1);
}

GLAPI int APIENTRY impl_wglReleasePbufferDCARB(HPBUFFERARB hPbuffer, HDC hDC)
{
    SPDLOG_WARN("wglReleasePbufferDCARB not implemented");
    return 0;
}

GLAPI BOOL APIENTRY impl_wglDestroyPbufferARB(HPBUFFERARB hPbuffer)
{
    SPDLOG_WARN("wglDestroyPbufferARB not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglQueryPbufferARB(HPBUFFERARB hPbuffer,
    int iAttribute,
    int* piValue)
{
    SPDLOG_WARN("wglQueryPbufferARB not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglGetPixelFormatAttribivARB(HDC hdc,
    int iPixelFormat,
    int iLayerPlane,
    UINT nAttributes,
    const int* piAttributes,
    int* piValues)
{
    SPDLOG_WARN("wglGetPixelFormatAttribivARB not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglGetPixelFormatAttribfvARB(HDC hdc,
    int iPixelFormat,
    int iLayerPlane,
    UINT nAttributes,
    const int* piAttributes,
    FLOAT* pfValues)
{
    SPDLOG_WARN("wglGetPixelFormatAttribfvARB not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglChoosePixelFormatARB(HDC hdc,
    const int* piAttribIList,
    const FLOAT* pfAttribFList,
    UINT nMaxFormats,
    int* piFormats,
    UINT* nNumFormats)
{
    SPDLOG_WARN("wglChoosePixelFormatARB not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglBindTexImageARB(HPBUFFERARB hPbuffer, int iBuffer)
{
    SPDLOG_WARN("wglBindTexImageARB not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglReleaseTexImageARB(HPBUFFERARB hPbuffer, int iBuffer)
{
    SPDLOG_WARN("wglReleaseTexImageARB not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglSetPbufferAttribARB(HPBUFFERARB hPbuffer,
    const int* piAttribList)
{
    SPDLOG_WARN("wglSetPbufferAttribARB not implemented");
    return FALSE;
}

// -------------------------------------------------------