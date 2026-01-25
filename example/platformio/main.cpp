// This file is an example how to integrate the rix with an arduino.
// Note: Only platformio is supported due to the lack of c++17 features in the arduino IDE.
#include "renderer/devicedatauploader/DeviceDataUploader.hpp"
#include <Arduino.h>
#include <IBusConnector.hpp>
#include <Minimal.hpp>
#include <RIXGL.hpp>
#include <SPI.h>
#include <StencilShadow.hpp>

// Create a connector for the rix library. This is a wrapper around the arduino SPI interface.
// This currently uses the blocking SPI interface. It seems, that this is always supported from
// the boards. The async one is not always supported.
template <uint32_t DISPLAYLIST_SIZE = 30 * 1024>
class ArduinoBusConnector : public rr::IBusConnector
{
public:
    // Configure here the pinout of the arduino
    // This currently uses the pinout of the RPPico. Might be adapted for other boards.
    static constexpr uint32_t RESET { 21 };
    static constexpr uint32_t CTS { 20 };
    static constexpr uint32_t CSN { 17 };

    // The max chunk size (if the parameter is not changed in the FPGA, then this can be left as it is)
    static constexpr uint32_t MAX_CHUNK_SIZE { 32768 - 2048 };

    virtual ~ArduinoBusConnector() = default;

    virtual void writeData(const uint8_t index, const uint32_t size, const uint32_t) override
    {
        tcb::span<uint8_t> data = requestWriteBuffer(index);
        uint32_t dataToSend = size;
        uint32_t counter = 0;
        while (dataToSend != 0)
        {
            blockUntilTransferIsComplete();
            // SPI has no flow control. Therefore the flow control must be implemented in software.
            // Divide the data into smaller chunks. Check after each chunk, if the fifo has enough space
            // before sending the next chunk.
            const uint32_t chunkSize = (dataToSend < MAX_CHUNK_SIZE) ? dataToSend : MAX_CHUNK_SIZE;
            SPI.transfer(data.data() + counter, chunkSize);
            counter += chunkSize;
            dataToSend -= chunkSize;
        }
    }

    virtual void readData(const uint8_t, const uint32_t) override
    {
    }

    virtual void blockUntilTransferIsComplete() override
    {
        while (!digitalRead(CTS))
            ;
    }

    virtual tcb::span<uint8_t> requestWriteBuffer(const uint8_t index) override
    {
        switch (index)
        {
        case 0:
        case 1:
            return { m_dlMem[index] };
        case 2:
            return { m_displayListTmp };
        default:
            break;
        }
        return {};
    }

    virtual tcb::span<uint8_t> requestReadBuffer(const uint8_t index) override
    {
        return {};
    }

    virtual uint8_t getWriteBufferCount() const override
    {
        return m_dlMem.size() + 1;
    }

    virtual uint8_t getReadBufferCount() const override
    {
        return 0;
    }

    void init()
    {
        // Initialize hardware
        pinMode(RESET, OUTPUT);
        pinMode(CTS, INPUT);
        pinMode(CSN, OUTPUT);
        digitalWrite(CSN, HIGH);
        digitalWrite(RESET, HIGH);
        SPI.begin();
        SPI.beginTransaction(SPISettings(22000000, MSBFIRST, SPI_MODE0));

        // Reset FPGA
        digitalWrite(RESET, LOW);
        delay(50);
        digitalWrite(RESET, HIGH);
        delay(500); // Wait a moment till the FPGA has load its configuration
        digitalWrite(RESET, LOW);
        digitalWrite(CSN, LOW);
        delay(50);
    }

private:
    std::array<std::array<uint8_t, DISPLAYLIST_SIZE>, 2> m_dlMem;
    std::array<uint8_t, 4096 + 64> m_displayListTmp; // The last buffer is used to upload texture chunks and can be smaller
};

static ArduinoBusConnector<> m_busConnector {};
rr::devicedatauploader::DeviceDataUploader m_device { m_busConnector };
static constexpr uint32_t RESOLUTION_H = 240;
static constexpr uint32_t RESOLUTION_W = 320;
static constexpr uint LED_PIN = 25;
bool state { false };

// Select a demo
static Minimal m_scene {};
// static StencilShadow m_scene {};

void setup()
{
    pinMode(LED_PIN, OUTPUT);
    // Initialize the connector
    m_busConnector.init();
    // Create a instance with the current connector. This will also initialize the library.
    rr::RIXGL::createInstance(m_device);
    // Set the display resolution
    rr::RIXGL::getInstance().setRenderResolution(RESOLUTION_W, RESOLUTION_H);
    // Initialize the scene
    m_scene.init(RESOLUTION_W, RESOLUTION_H);
}

void loop()
{
    // Draw the scene
    m_scene.draw();
    // Swap to a new display list
    rr::RIXGL::getInstance().swapDisplayList();
    // Use LED as a heartbeat
    digitalWrite(LED_PIN, state);
    state = !state;
}