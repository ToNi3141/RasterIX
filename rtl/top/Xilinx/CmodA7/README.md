
# CMOD A7 Build
The build target is a CMOD A7 board with an `XC7A35` FPGA. An SPI interface is used to connect the FPGA with an MCU. An additional CTS pin for flow control (in software) is used.

This builds the `RasterIX_EF` and uses one TMU with a maximum texture resolution of 128x128px.

There is one variant available:

`rixef`:  
  - 1 TMU (max res: 128x128)
  - Mip mapping
  - Fixpoint
  - 10MPixel maximum (because of slow SRAM)

Note: glReadPixels, glCopyTexImage2d and glCopyTexSubImage2d are not available. The SPI MISO channel is not used and needs to be implemented.

To build the binaries, use the following commands.
```sh
cd rtl/top/Xilinx/CmodA7
/Xilinx/Vivado/2022.2/bin/vivado -mode batch -source build.tcl
```
You will find `rasterix.bin` and `rasterix.bit` in the synth directory. Use Vivado to program the FPGA or to flash the binary into the flash.

## Hardware Setup
The following hardware setup shows an Raspberry Pi Pico connected to an CMOD A7 and an 320x240 pixel display with an `ILI9341` chipset.

<img src="../../../../screenshots/cmod7.jpg" width="50%"> 

To connect the Pico to the FPGA, use the following table:
| Port Name | Pico | CMOD A7 |
|-----------|------|---------|
| MOSI      | GP19 | 45      |
| SCK       | GP18 | 47      |
| MISO      | GP16 | 48      |
| CSN       | GP17 | 46      |
| CTS       | GP20 |  2      |
| RSTN      | GP21 |  1      |

Supported SPI clock speed: Max 25MHz.

The display is directly connected to the FPGA via the 8080-I parallel interface. The FPGA automatically configures the display when the `RSTN` pin is asserted. To connect the display, use the following table:

| Port Name | CMOD A7 | ILI9341 |
|-----------|---------|---------|
| CS        | 27      | GND     |
| C/D       | 28      | C/D     |
| WR        | 29      | WR      |
| RD        | 30      | RD      |
| RST       | 31      | 3.3V    |
| DATA[7:0] | [38:45] | D[7:0]  |

The intention of the choosen pinout is to simplify the buildup with an breadboard. The display (Adafruit 2.4" TFT LCD Breakout Board) has a one to one mapping to the FPGA pins when connected. Only one jumper wire to the displays `Vin` is used. __Please note that the `Vin` of the display goes to an FPGA IO. That is not an issue as long as `Vin` does not exceed 3.3V and when the FPGA IO is not configured as an output!__ When the xdc is not modified, you should be safe.
 
# RPPICO Build
Uses the [CMOD A7 Build](#cmod-a7-build) and the pico-sdk. By default, this build will download the pico-sdk automatically.

Open a terminal. Use the following commands to build a rppico binary:
```sh
cd <rasterix_directory>
cmake --preset rppico -DPICO_BOARD=pico
cmake --build build/rppico --config Release --parallel 
```
You will find a `minimal.uf2` file in the `build/rppico/example/rp-pico` directory.

Note: The rix library heavily relies on floating point arithmetics. A MCU, like the `rp2350`, with FPU is recommended. It can speed up the overall performance of around 10 times.

# PlatformIO
If you are using [PlatformIO](https://platformio.org/), you can add this repo directly to your `platformio.ini` like this:

```ini
[env:teensy40]
platform = teensy
board = teensy40
framework = arduino
lib_deps = toni3141-RasterIX=https://github.com/ToNi3141/RasterIX.git
build_flags = ${rixef.build_flags}

[rixef]
build_flags = 
    -Ofast 
    -std=c++17
    -DRIX_CORE_TMU_COUNT=1
    -DRIX_CORE_MAX_TEXTURE_SIZE=128
    -DRIX_CORE_ENABLE_MIPMAPPING=true
    -DRIX_CORE_MAX_DISPLAY_WIDTH=320
    -DRIX_CORE_MAX_DISPLAY_HEIGHT=240
    -DRIX_CORE_FRAMEBUFFER_SIZE_IN_PIXEL_LG=20
    -DRIX_CORE_USE_FLOAT_INTERPOLATION=false
    -DRIX_CORE_NUMBER_OF_TEXTURE_PAGES=68
    -DRIX_CORE_NUMBER_OF_TEXTURES=68
    -DRIX_CORE_TEXTURE_PAGE_SIZE=2048
    -DRIX_CORE_GRAM_MEMORY_LOC=0x0
    -DRIX_CORE_COLOR_BUFFER_LOC_0=0x35000
    -DRIX_CORE_COLOR_BUFFER_LOC_1=0x35000
    -DRIX_CORE_COLOR_BUFFER_LOC_2=0x35000
    -DRIX_CORE_DEPTH_BUFFER_LOC=0x5A800
    -DRIX_CORE_STENCIL_BUFFER_LOC=0x22400
    -DRIX_CORE_THREADED_RASTERIZATION=false
    -DRIX_CORE_THREADED_RASTERIZATION_DISPLAY_LIST_SIZE=0
    -DRIX_CORE_ENABLE_VSYNC=false
```

An example for the Arduino framework is available under examples.