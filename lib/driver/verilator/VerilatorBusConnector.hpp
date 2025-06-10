#ifndef VERILATORBUSCONNECTOR_H
#define VERILATORBUSCONNECTOR_H

#include "GenericMemoryBusConnector.hpp"
#include "Vtop.h"
#include "verilated.h"
#include <tcb/span.hpp>
#include <verilated_vcd_c.h>

namespace rr
{
template <typename FBType, uint32_t NUMBER_OF_DISPLAY_LISTS = 33, uint32_t DISPLAY_LIST_SIZE = 1024 * 1024>
class VerilatorBusConnector : public GenericMemoryBusConnector<NUMBER_OF_DISPLAY_LISTS, DISPLAY_LIST_SIZE>
{
public:
    virtual ~VerilatorBusConnector() = default;

    VerilatorBusConnector(FBType* framebuffer, const uint16_t resolutionW = 128, const uint16_t resolutionH = 128)
        : m_resolutionW(resolutionW)
        , m_resolutionH(resolutionH)
        , m_framebuffer(framebuffer)
    {
        Verilated::traceEverOn(true);

        m_top.m_framebuffer_axis_tready = 1;
        m_top.s_cmd_axis_tvalid = 0;

        m_top.resetn = 0;
        clk();
        clk(); // It needs an additional clock cycle, otherwise it will not work. Currently i dont know why
        m_top.resetn = 1;
        clk();
    }

    virtual void writeData(const uint8_t index, const uint32_t size) override
    {
        // Convert data to 32 bit variables to ease the access
        const uint32_t* data32 = reinterpret_cast<const uint32_t*>(this->m_dlMem[index].data());
        const uint32_t bytes32 = size / sizeof(*data32);
        for (uint32_t i = 0; i < bytes32;)
        {
            if (m_top.s_cmd_axis_tready)
            {
                m_top.s_cmd_axis_tdata = data32[i];
                m_top.s_cmd_axis_tvalid = 1;
                i++;
            }
            clk();
        }
        m_top.s_cmd_axis_tvalid = 0;
    }

    virtual void waitTillWriteIsDone() override
    {
    }

    void clk()
    {
        m_top.aclk = 1;
        m_top.eval();
        m_top.aclk = 0;
        m_top.eval();

        if (m_top.resetn == 0)
            return;

        if (m_top.m_framebuffer_axis_tvalid && (m_framebuffer != nullptr))
        {
            m_framebuffer[m_streamAddr] = m_top.m_framebuffer_axis_tdata;
            m_streamAddr++;
        }

        if (m_streamAddr == (m_resolutionW * m_resolutionH / (sizeof(FBType) / 2))) // 4 for 32 bit color
        {
            m_streamAddr = 0;
        }
    }

    void waitForLastFramebufferChunk()
    {
        while (m_streamAddr != 0)
        {
            clk();
        }
    }

private:
    const uint16_t m_resolutionW = 128;
    const uint16_t m_resolutionH = 128;
    FBType* m_framebuffer = nullptr;
    uint32_t m_streamAddr = 0;
    Vtop m_top;
};

} // namespace rr
#endif // VERILATORBUSCONNECTOR_H
