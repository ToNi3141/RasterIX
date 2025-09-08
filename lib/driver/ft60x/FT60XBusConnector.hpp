#ifndef FT60XBUSCONNECTOR_H
#define FT60XBUSCONNECTOR_H

#include <chrono>
#include <thread>

#include "GenericMemoryBusConnector.hpp"
#include <ftd3xx.h>

#include <stdio.h>

namespace rr
{
// Bus connector to use an FT600 chip configured in the FT245 FIFO mode.
// Important: Before you use this class, configure the FT600 in the FT245 FIFO mode with FT60X Chip Configuration Programmer. Otherwise, it will not work.
// This class uses GPIO0 to reset the FPGA
class FT60XBusConnector : public GenericMemoryBusConnector<11, 8 * 1024 * 1024>
{
public:
    virtual ~FT60XBusConnector();

    FT60XBusConnector();

    virtual void writeData(const uint8_t index, const uint32_t size) override;
    virtual void readData(const uint8_t index, const uint32_t size) override;
    virtual void blockUntilTransferIsComplete() override;

private:
    FT_HANDLE fthandle;
};

} // namespace rr
#endif // FT60XBUSCONNECTOR_H
