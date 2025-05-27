#include "FT60XBusConnector.hpp"
#include "MultiThreadRunner.hpp"
#include "RIXGL.hpp"
#include "gl.h"
#include "glu.h"
#include <spdlog/spdlog.h>
#include <stdio.h>

template <typename Scene>
class Runner
{
public:
    Runner()
    {
        spdlog::set_level(spdlog::level::trace);
        rr::RIXGL::createInstance(m_busConnector, m_workerThread, m_uploadThread);
        rr::RIXGL::getInstance().setRenderResolution(RESOLUTION_W, RESOLUTION_H);
    }

    ~Runner()
    {
        rr::RIXGL::getInstance().destroy();
    }

    void execute()
    {
        m_scene.init(RESOLUTION_W, RESOLUTION_H);
        while (1)
        {
            m_scene.draw();
            rr::RIXGL::getInstance().swapDisplayList();
            rr::RIXGL::getInstance().uploadDisplayList();
        }
    }

private:
    static constexpr uint32_t RESOLUTION_H = 600;
    static constexpr uint32_t RESOLUTION_W = 1024;
    rr::FT60XBusConnector m_busConnector {};
    rr::MultiThreadRunner m_workerThread {};
    rr::MultiThreadRunner m_uploadThread {};
    Scene m_scene {};
};
