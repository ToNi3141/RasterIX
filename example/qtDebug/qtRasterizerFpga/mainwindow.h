#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QTimer>
#include "RIXGL.hpp"
#include "renderer/Renderer.hpp"
#include "gl.h"
#include "RenderConfigs.hpp"
#if USE_SIMULATION
#include "VerilatorBusConnector.hpp"
#endif
#if USE_HARDWARE
#undef VOID // Undef void because it is defined in the tcl.h and there is a typedef in WinTypes.h (which is used for the FT2232 library)
#include "FT60XBusConnector.hpp"
#endif
#if USE_SOFTWARE
#include "SoftwareRasterizerBusConnector.hpp"
#include "renderer/softwarerasterizer/SoftwareRasterizer.hpp"
#endif

#include "NoThreadRunner.hpp"
#include "RenderConfigs.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/dse/DmaStreamEngine.hpp"
#include "renderer/threadedvertextransformer/ThreadedVertexTransformer.hpp"
#include "../../stencilShadow/StencilShadow.hpp"
#include "../../minimal/Minimal.hpp"
#include "../../mipmap/Mipmap.hpp"
#include "../../vbo/VboExample.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void newFrame();

private:

#if USE_SIMULATION
public:
    static const uint32_t PREVIEW_WINDOW_SCALING = 1;
    static const uint32_t RESOLUTION_W = 640;
    static const uint32_t RESOLUTION_H = 480;
private:
    uint8_t m_framebuffer[RESOLUTION_W * RESOLUTION_H * 4];

    rr::VerilatorBusConnector<> m_busConnector{m_framebuffer, RESOLUTION_W, RESOLUTION_H};
    rr::dsec::DmaStreamEngine m_device{m_busConnector};
#endif

#if USE_SOFTWARE
public:
    static const uint32_t PREVIEW_WINDOW_SCALING = 1;
    static const uint32_t RESOLUTION_W = 640;
    static const uint32_t RESOLUTION_H = 480;
private:
    uint8_t m_framebuffer[RESOLUTION_W * RESOLUTION_H * 4];
    rr::SoftwareRasterizerBusConnector<> m_busConnector{m_framebuffer};
    rr::softwarerasterizer::SoftwareRasterizer m_device{m_busConnector};
#endif

#if USE_HARDWARE
public:
    static const uint32_t RESOLUTION_H = 600;
    static const uint32_t RESOLUTION_W = 1024;
private:
    rr::FT60XBusConnector m_busConnector;
    rr::dsec::DmaStreamEngine m_device{m_busConnector};
#endif

    rr::NoThreadRunner m_workerThread{};
    rr::NoThreadRunner m_uploadThread{};
    rr::threadedvertextransformer::ThreadedVertexTransformer m_threadedRasterizer{m_device, m_workerThread, m_uploadThread};

    Ui::MainWindow *ui;

    QTimer m_timer;
    QImage m_image;
    Minimal m_testScene;
};

#endif // MAINWINDOW_H
