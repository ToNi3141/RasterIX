#ifndef VERILATORBUSCONNECTOR_H
#define VERILATORBUSCONNECTOR_H

#include "GenericMemoryBusConnector.hpp"
#include "Vtop.h"
#include "verilated.h"
#include <tcb/span.hpp>
#include <verilated_vcd_c.h>

namespace rr
{
template <uint32_t NUMBER_OF_DISPLAY_LISTS = 33, uint32_t DISPLAY_LIST_SIZE = 128 * 1024>
class VerilatorBusConnector : public GenericMemoryBusConnector<NUMBER_OF_DISPLAY_LISTS, DISPLAY_LIST_SIZE>
{
public:
    virtual ~VerilatorBusConnector() = default;

    VerilatorBusConnector(tcb::span<uint8_t> framebuffer, const uint16_t resolutionW = 128, const uint16_t resolutionH = 128)
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

    virtual void writeData(const uint8_t index, const uint32_t size, const uint32_t offset) override
    {
        // Convert data to 32 bit variables to ease the access
        const uint32_t* data32 = reinterpret_cast<const uint32_t*>(this->m_dlMemTx[index].data() + offset);
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

    virtual void readData(const uint8_t index, const uint32_t size) override
    {
        // Convert data to 32 bit variables to ease the access
        uint32_t* data32 = reinterpret_cast<uint32_t*>(this->m_dlMemRx[index].data());
        const uint32_t bytes32 = size / sizeof(*data32);
        m_top.m_cmd_resp_axis_tready = 1;
        for (uint32_t i = 0; i < bytes32;)
        {
            if (m_top.m_cmd_resp_axis_tvalid)
            {
                data32[i] = m_top.m_cmd_resp_axis_tdata;
                i++;
            }
            clk();
        }
        m_top.m_cmd_resp_axis_tready = 0;
    }

    virtual void blockUntilTransferIsComplete() override
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

        const std::size_t framebufferSize = (m_resolutionW * m_resolutionH * 3); // 3 for 24 bit color
        if (m_top.m_framebuffer_axis_tvalid && (m_streamAddr < framebufferSize) && (!m_framebuffer.empty()))
        {
            const uint16_t f0 = m_top.m_framebuffer_axis_tdata & 0xFFFF;
            const uint16_t f1 = (m_top.m_framebuffer_axis_tdata >> 16) & 0xFFFF;

            toBgr888(m_framebuffer.subspan(m_streamAddr, 3), f0);
            m_streamAddr += 3;
            toBgr888(m_framebuffer.subspan(m_streamAddr, 3), f1);
            m_streamAddr += 3;
        }

        if ((m_streamAddr >= framebufferSize) && m_top.m_framebuffer_axis_tlast)
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
    void toBgr888(tcb::span<uint8_t> dst, const uint16_t pixelData)
    {
        const uint32_t r = (pixelData >> 11) & 0x1F;
        const uint32_t g = (pixelData >> 5) & 0x3F;
        const uint32_t b = (pixelData >> 0) & 0x1F;
        dst[2] = (r << 3) | (r >> 2);
        dst[1] = (g << 2) | (g >> 4);
        dst[0] = (b << 3) | (b >> 2);
    }

    const uint16_t m_resolutionW = 128;
    const uint16_t m_resolutionH = 128;
    tcb::span<uint8_t> m_framebuffer;
    uint32_t m_streamAddr = 0;
    Vtop m_top;
};

} // namespace rr
#endif // VERILATORBUSCONNECTOR_H
