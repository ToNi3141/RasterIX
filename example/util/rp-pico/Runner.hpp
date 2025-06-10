#include "NoThreadRunner.hpp"
#include "RIXGL.hpp"
#include "gl.h"
#include "glu.h"

#include <hardware/dma.h>
#include <hardware/spi.h>
#include <pico/binary_info.h>
#include <pico/stdlib.h>

#include <algorithm>

// Simple BusConnector which wraps the SPI
template <uint32_t DISPLAYLIST_SIZE = 30 * 1024>
class BusConnector : public rr::IBusConnector
{
public:
    static constexpr uint32_t RESET { 21 };
    static constexpr uint32_t CTS { 20 };
    static constexpr uint32_t MAX_CHUNK_SIZE { 32768 - 2048 };

    BusConnector() { }

    virtual ~BusConnector() = default;

    virtual void writeData(const uint8_t index, const uint32_t size) override
    {
        tcb::span<const uint8_t> data = requestBuffer(index);
        uint32_t dataToSend = size;
        uint32_t counter = 0;
        while (dataToSend != 0)
        {
            waitTillWriteIsDone();
            // SPI has no flow control. Therefore the flow control must be implemented in software.
            // Divide the data into smaller chunks. Check after each chunk, if the fifo has enough space
            // before sending the next chunk.
            const uint32_t chunkSize = (dataToSend < MAX_CHUNK_SIZE) ? dataToSend : MAX_CHUNK_SIZE;
            dma_channel_config c = dma_channel_get_default_config(dma_tx);
            channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
            channel_config_set_dreq(&c, spi_get_dreq(spi_default, true));
            dma_channel_configure(dma_tx, &c,
                &spi_get_hw(spi_default)->dr, // write address
                data.data() + counter, // read address
                chunkSize, // element count (each element is of size transfer_data_size)
                true); // Start immediately
            counter += chunkSize;
            dataToSend -= chunkSize;
        }
    }

    virtual void waitTillWriteIsDone() override
    {
        while (dma_channel_is_busy(dma_tx) || !gpio_get(CTS))
            ;
    }

    virtual tcb::span<uint8_t> requestBuffer(const uint8_t index) override
    {
        switch (index)
        {
        case 2:
            return { m_displayListTmp };
        default:
            return { m_dlMem[index] };
        }
        return {};
    }
    virtual uint8_t getBufferCount() const override
    {
        return m_dlMem.size() + 1;
    }

    void init()
    {
        gpio_init(RESET);
        gpio_init(CTS);
        gpio_set_dir(RESET, GPIO_OUT);
        gpio_set_dir(CTS, GPIO_IN);
        spi_init(spi_default, 20 * 1000 * 1000);
        gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
        gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
        gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, true);
        gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
        gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

        // Make the SPI pins available to picotool
        bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));
        // Make the CS pin available to picotool
        bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));

        // Grab some unused dma channels
        dma_tx = dma_claim_unused_channel(true);

        gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
        gpio_put(RESET, 0);
        sleep_ms(50);
        gpio_put(RESET, 1);
        sleep_ms(500); // Wait a moment till the FPGA has load its configuration
        gpio_put(RESET, 0);
        gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0);
        sleep_ms(50);
    }

private:
    uint dma_tx;
    std::array<std::array<uint8_t, DISPLAYLIST_SIZE>, 2> m_dlMem;
    std::array<uint8_t, 4096 + 64> m_displayListTmp;
};

template <typename Scene>
class Runner
{
public:
    Runner()
    {
        stdio_init_all();
        gpio_init(LED_PIN);
        gpio_set_dir(LED_PIN, GPIO_OUT);
        m_busConnector.init();
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
            gpio_put(LED_PIN, led);
            led = !led;
            m_scene.draw();
            rr::RIXGL::getInstance().swapDisplayList();
        }
    }

private:
    static constexpr uint32_t RESOLUTION_H = 240;
    static constexpr uint32_t RESOLUTION_W = 320;
    static constexpr uint LED_PIN = 25;
    BusConnector<> m_busConnector {};
    rr::NoThreadRunner m_workerThread {};
    rr::NoThreadRunner m_uploadThread {};
    bool led = false;
    Scene m_scene {};
};
