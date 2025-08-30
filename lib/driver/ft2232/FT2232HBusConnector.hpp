#ifndef FT2232HBUSCONNECTOR_H
#define FT2232HBUSCONNECTOR_H

#include <chrono>
#include <thread>

#include "IBusConnector.hpp"
#include "ftd2xx.h"
#include <stdio.h>

namespace rr
{
// Bus connector to use an FT2232H chip configured in the FT245 FIFO mode.
// Important: Before you use this class, configure the FT2232H in the FT245 FIFO mode with FT_PROG. Otherwise, it will not work.
// This class utilizes the async FIFO mode.
// Channel A is used for the data port
// Channel B is used as GPIO to reset the FPGA
class FT2232HBusConnector : public IBusConnector
{
public:
    virtual ~FT2232HBusConnector()
    {
        FT_Close(fthandle);
    }

    FT2232HBusConnector()
    {
        // Configure data port
        FT_STATUS status;
        status = FT_Open(0, &fthandle);
        if (status != FT_OK)
            printf("open status not ok %d\n", status);

        status = FT_ResetDevice(fthandle);

        if (status != FT_OK)
            printf("reset status not ok %d\n", status);

        if (status != FT_OK)
            printf("mode status not ok %d\n", status);

        // Configure GPIO to control the reset line
        status = FT_Open(1, &fthandlegpio);
        if (status != FT_OK)
            printf("open gpio status not ok %d\n", status);

        status = FT_ResetDevice(fthandlegpio);
        if (status != FT_OK)
            printf("reset gpio status not ok %d\n", status);

        status = FT_SetBitMode(fthandlegpio, 0, 0);
        status = FT_SetBitMode(fthandlegpio, 0xFF, 0x02); // All pins outputs, MPSSE

        // Trigger reset cycle
        // Write initial value (high)
        uint8_t gpio_command[] = { 0x80, 0xFF, 0xFF };
        DWORD bytesWritten = 0;
        status = FT_Write(fthandlegpio, gpio_command, sizeof(gpio_command), &bytesWritten);
        if (status != FT_OK)
            printf("can't write gpio value %d\n", status);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // After 100ms pull the reset (low)
        gpio_command[1] = 0x00;
        status = FT_Write(fthandlegpio, gpio_command, sizeof(gpio_command), &bytesWritten);
        if (status != FT_OK)
            printf("can't write gpio value %d\n", status);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // After 100ms, release the reset (high)
        gpio_command[1] = 0xFF;
        status = FT_Write(fthandlegpio, gpio_command, sizeof(gpio_command), &bytesWritten);
        if (status != FT_OK)
            printf("can't write gpio value %d\n", status);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    virtual void writeData(const tcb::span<const uint8_t>& data) override
    {
        DWORD data_written;
        FT_Write(fthandle, (LPVOID*)(data.data()), data.size(), &data_written);
    }

    virtual void blockUntilTransferIsComplete() override
    {
    }

private:
    FT_HANDLE fthandle;
    FT_HANDLE fthandlegpio;
};

} // namespace rr
#endif // FT2232HBUSCONNECTOR_H
