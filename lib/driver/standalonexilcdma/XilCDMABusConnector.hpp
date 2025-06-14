#include "IBusConnector.hpp"
#include "sleep.h"
#include "xaxidma.h"
#include "xil_printf.h"
#include "xparameters.h"
#include <array>
#include <tcb/span.hpp>
#include <xil_cache.h>

namespace rr
{
class XilCDMABusConnector : public rr::IBusConnector
{
public:
    XilCDMABusConnector()
    {
        XAxiDma_Config* CfgPtr;
        int Status;

        // Initialize the XAxiDma device.
        CfgPtr = XAxiDma_LookupConfig(XPAR_AXI_DMA_0_BASEADDR);
        if (!CfgPtr)
        {
            xil_printf("No config found for %d\r\n", XPAR_AXI_DMA_0_BASEADDR);
        }

        Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
        if (Status != XST_SUCCESS)
        {
            xil_printf("Initialization failed %d\r\n", Status);
        }

        if (XAxiDma_HasSg(&AxiDma))
        {
            xil_printf("Device configured as SG mode \r\n");
        }

        // Disable interrupts, we use polling mode
        XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
            XAXIDMA_DEVICE_TO_DMA);
        XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
            XAXIDMA_DMA_TO_DEVICE);
    }

    ~XilCDMABusConnector()
    {
        waitForDma();
    }

    virtual void writeData(const uint8_t index, const uint32_t size)
    {
        int Status;
        int TimeOut = 10000;

        // Flush the buffers before the DMA transfer, in case the Data Cache
        // is enabled
        Xil_L1DCacheFlush();

        Status = XAxiDma_SimpleTransfer(&AxiDma, (UINTPTR)(m_dlMem[index].data()), size, XAXIDMA_DMA_TO_DEVICE);
        // Wait till transfer is done or 1usec * 10^6 iterations of timeout occurs
        waitForDma();
    }

    virtual void blockUntilWriteComplete()
    {
        waitForDma();
    }

    virtual tcb::span<uint8_t> requestBuffer(const uint8_t index) { return { m_dlMem[index] }; }
    virtual uint8_t getBufferCount() const { return m_dlMem.size(); }

protected:
    void waitForDma()
    {
        while (true)
        {
            if (!(XAxiDma_Busy(&AxiDma, XAXIDMA_DMA_TO_DEVICE)))
            {
                break;
            }
            usleep(1U);
        }
    }

    std::array<std::array<uint8_t, 64 * 1024>, 4> m_dlMem {};
    XAxiDma AxiDma;
};

} // namespace rr