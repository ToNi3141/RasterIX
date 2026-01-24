#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QImage>
#include <QPixmap>
#include <math.h>
#include <QDebug>
#include "QTime"
#include <QThread>
#include <spdlog/spdlog.h>
#include "glu.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_image(RESOLUTION_W, RESOLUTION_H, QImage::Format_RGB888)
{
    ui->setupUi(this);
    rr::RIXGL::createInstance(m_threadedRasterizer);
    rr::RIXGL::getInstance().setRenderResolution(RESOLUTION_W, RESOLUTION_H);

    connect(&m_timer, &QTimer::timeout, this, &MainWindow::newFrame);
    ui->label->setPixmap(QPixmap::fromImage(m_image));
    m_timer.setInterval(1);
    m_timer.setSingleShot(false);
    m_timer.start();

    m_testScene.init(RESOLUTION_W, RESOLUTION_H);
}


void MainWindow::newFrame()
{
     m_testScene.draw();

    rr::RIXGL::getInstance().swapDisplayList();

#if USE_SIMULATION || USE_SOFTWARE
    for (uint32_t i = 0; i < RESOLUTION_H; i++)
    {
        for (uint32_t j = 0; j < RESOLUTION_W; j++)
        {
            const uint8_t r = m_framebuffer[((i*RESOLUTION_W)+j)*3+2];
            const uint8_t g = m_framebuffer[((i*RESOLUTION_W)+j)*3+1];
            const uint8_t b = m_framebuffer[((i*RESOLUTION_W)+j)*3+0];
            m_image.setPixelColor(QPoint(j, i), QColor(r, g, b));
        }
    }
    ui->label->setPixmap(QPixmap::fromImage(m_image.scaled(m_image.width()*PREVIEW_WINDOW_SCALING, m_image.height()*PREVIEW_WINDOW_SCALING)));
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}
