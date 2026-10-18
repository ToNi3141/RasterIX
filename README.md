# RasterIX

<p align="center">
  <img src="pictures/logo-soft-wordmark-glow.svg" alt="RasterIX" width="560">
</p>

**An open-source, fixed-function 3D rasterizer for FPGA-based systems.** RasterIX pairs a C++ graphics driver and software vertex pipeline with a Verilog pixel-rendering core. It is designed for embedded systems where a dedicated, configurable graphics pipeline is useful.

| | |
|---|---|
| Graphics API | OpenGL ES 1.1-oriented fixed-function rendering, with selected OpenGL 1.3 additions; this is not a complete conformance implementation. |
| Hardware | FPGA rasterization, up to two texture mapping units (TMUs), external or internal framebuffer variants. |
| Software | C++17 driver, vertex processing, device upload/streaming, simulation connectors, and a software rasterizer. |
| License | [GNU GPL v3 or later](LICENSE). |

## About the project

RasterIX began as an experiment in implementing a 3D graphics renderer in FPGA logic. The goal is an open, reusable fixed-function renderer for embedded Linux and microcontroller-class systems—not a programmable shader GPU. The work has achieved a reusable FPGA pixel pipeline with IF/EF framebuffer variants, a C++ OpenGL-oriented driver and software vertex pipeline, up to two TMUs, device streaming, simulation, platform connectors, and example integrations.

The hardware accelerates pixel work; the C++ driver performs the OpenGL state management and vertex pipeline, then streams rendering commands and data to the core. The project has been used with FPGA development boards, Linux systems, and MCU-based integrations. Exact API coverage, features, and game compatibility depend on the selected build and hardware configuration.

The name **RasterIX** abbreviates “Rasterizer 9”; the 9 refers to the late 1990s, when fixed-function 3D renderers were at their peak.

## Architecture

The following diagram is a conceptual overview of the FPGA rendering path. The RTL hierarchy and current dataflow are described below; the diagram is not a cycle-accurate or exact module schematic.

<p align="center">
  <img src="pictures/fpgaFlow.drawio.png" alt="Conceptual RasterIX FPGA rendering pipeline" width="900">
</p>

### Pixel pipeline

| Stage | Work |
|---|---|
| Command parser | Decodes the command stream, state-register writes, and triangle data produced by the software driver. |
| Rasterizer | Walks triangle coverage and emits covered pixel positions and interpolation inputs. |
| Attribute interpolation | Computes per-fragment color, depth, texture coordinates, and other interpolated values. |
| Texture mapping | TMU0 samples and combines texture data with the incoming color. TMU1 is optional and is instantiated only when `TMU_COUNT` is 2. Fog follows the active TMU stages. |
| Framebuffer read/join | Color, depth, and stencil reads are associated with the corresponding fragment. Disabled buffers are omitted from the join. |
| Per-fragment processing | Applies stencil/depth/alpha tests and blending/logic operations, then emits framebuffer writes. |

Framebuffer memory traffic is decoupled from the fragment pipeline using valid/ready streams, FIFOs, and outstanding-request limits. These finite queues let rendering continue while memory responses are in flight and hide some memory latency. They do not make a slow memory system infinitely fast, and “asynchronous” here means decoupled pipeline transactions—not necessarily a crossing between clock domains. External framebuffer performance depends on memory bandwidth, arbitration, and response throughput.

### Texture storage and address translation

The core supports two texture-storage approaches:

- **Texture cache:** `ENABLE_TEXTURE_CACHE` routes texture reads through a direct-mapped read-only cache. It uses external/device memory for texture data and typically consumes less FPGA block RAM than storing whole textures on-chip. Cache size is set by `TEXTURE_CACHE_SIZE`.
- **Full texture buffer:** with the cache disabled, page data is loaded into an on-chip `TextureBuffer` for each active TMU. This stores a complete texture rather than just recently used cache lines. Both approaches are designed for fast texture sampling; the full buffer has appeared somewhat faster in observed configurations, but that is not a universal performance guarantee. The trade-off is FPGA RAM usage, which grows with texture dimensions and mipmapping.

The texture address space is paged. `TextureMMU` uses an internal page table to translate virtual texture addresses into physical memory addresses; page bases are loaded through a stream interface. The software page size and the RTL `TEXTURE_PAGE_SIZE` must agree.

### Command and memory streaming

[`RegisterAndDescriptorDefines.vh`](rtl/RasterIX/RegisterAndDescriptorDefines.vh) defines the RTL command/register/descriptor format: operation words, register indices, framebuffer commands, and triangle payload layout. The corresponding C++ command and triangle payload types live under [`lib/gl/renderer/commands/`](lib/gl/renderer/commands/). Together these define the binary interface between the driver and RTL. Changes to field order, widths, encoding, or TMU-dependent payloads must be kept consistent on both sides.

`FrameStreamingCore` routes transfer commands between stream channels and memory. The driver-side `DeviceDataUploader` uses this path to upload display-list data and transfer device-memory data, including texture pages and framebuffer-related data. It is a general data-transfer engine, not a display framebuffer output stream.

### Software pipeline

The FPGA core is fed by a C++ software stack. This diagram shows the major components and the typical path from application calls to the device transport; the table clarifies a few component boundaries that the diagram simplifies.

<p align="center">
  <img src="pictures/softwareFlow.drawio.png" alt="RasterIX software rendering and device data flow" width="700">
</p>

| Component | Role in the software pipeline |
|---|---|
| Application and `gl` | The application calls the OpenGL-style C API. The API entry points forward calls and state changes into the RasterIX library. |
| `RIXGL` | Main library entry point. It creates and owns the render-device composition, including the vertex and pixel pipelines. |
| `VertexPipeline` | Tracks vertex-related OpenGL state, fetches submitted vertex attributes, and forwards draw data to the renderer. |
| `PixelPipeline` | Tracks pixel-related state—such as texture, fog, and fragment-test settings—and asks the renderer to emit the corresponding hardware configuration commands. Despite its name, this is not a second software per-pixel rasterizer. |
| `Renderer` and `DisplayList` | The renderer coordinates command generation, buffers commands in display lists, manages framebuffer state, and submits triangle data. Display lists are streamed to the device when submitted or when intermediate uploads are needed. |
| CPU transform and `Rasterizer` | Vertex transformation, lighting, clipping, and primitive assembly run in software. The CPU `Rasterizer` prepares triangle bounds and edge/interpolation increments for the FPGA; it does **not** walk every covered pixel. Per-pixel rasterization is performed by the FPGA pipeline above. |
| `TextureMemoryManager` | Tracks texture objects and their device-memory pages. Texture data is uploaded through the device interface, while texture-selection/configuration commands are placed in the display list. |
| Optional `ThreadedVertexTransformer` | When enabled, wraps the device path and moves vertex-command decoding, transformation, and triangle setup to worker work. `IThreadRunner` schedules this work and upload operations. Without it, the renderer performs the CPU transform/setup inline before sending the display list. |
| `IDevice` / `DeviceDataUploader` | `IDevice` is the renderer-facing abstraction for display-list buffers, device-memory reads/writes, and completion. `DeviceDataUploader` is the common implementation: it adds the transfer protocol understood by RTL `FrameStreamingCore` and delegates physical transfers to an `IBusConnector`. |
| `IBusConnector` and transport | `IBusConnector` abstracts indexed transfer buffers and transfer completion. Concrete connectors implement transports such as DMA Proxy, FT60X, Verilator, or SPI. |

In short, the usual device path is `Renderer` → optional `ThreadedVertexTransformer` → `IDevice` (commonly implemented by `DeviceDataUploader`) → `IBusConnector` → transport. The thread runner is used by the optional threaded transformer, rather than being a renderer-owned transport interface. The diagram is an overview; exact object composition varies by platform and build configuration.

## Working games

The project has been documented as working with the following games. Compatibility is configuration-, platform-, and driver-dependent; consider these reported results rather than a guarantee for every current build. This list is retained for review and may include historical testing.

- [Tux Racer](https://github.com/ToNi3141/tuxracer.git), statically linked.
- [Quake 3 Arena / Quake3e](https://github.com/ToNi3141/Quake3e), using SDL2 and glX.
- Warcraft 3, using WGL.
- Other titles and applications.

![Tux Racer running on RasterIX](screenshots/tuxracerRaceStart.png)

The Arty Z7-20 board guide documents 1024×600 HDMI output. The current Zynq EF software preset, however, caps the configured render dimensions at 480×320; these are distinct settings and should not be assumed to match. See the [Arty Z7-20 guide](rtl/top/Xilinx/ArtyZ7-20/README.md) for the board workflow.

<video src="https://github.com/user-attachments/assets/909e6167-abf3-4c31-8bfd-f250dc2a8f0c" width="1024" height="500" controls></video>

## FPGA utilization

The following are the latest checked-in **post-route whole-design** reports located with the Arty Z7-20 and TE0890 builds. They report the complete `design_1_wrapper`, not an isolated RasterIX core. Both were generated with Vivado 2022.2. Resource percentages are relative to the target FPGA shown.

### Board configurations represented by the reports

| Board | Configured RasterIX features |
|---|---|
| Arty Z7-20 (`xc7z020clg400-1`) | EF; 1 TMU; 64-bit AXI; 18-bit fixed-point interpolation precision; 256×256 maximum texture dimension; 8 KiB texture cache enabled; 2048-byte pages; mipmapping, filtering, fog, depth, stencil, and memory coalescing enabled. The generated IP configuration is authoritative; the board guide’s texture-storage row is outdated. |
| TE0890 (`xc7s25ftgb196-1`) | EF; 1 TMU; 16-bit AXI; 18-bit fixed-point precision; 6-bit sub-pixel precision; 128×128 maximum texture dimension; 8 KiB texture cache enabled; 2048-byte pages; mipmapping, filtering, fog, depth, stencil, and memory coalescing enabled. HyperRAM runs at 75 MHz per the board guide. |

### Post-route results

| Board/report date | LUTs | Registers | BRAM tiles | DSPs |
|---|---:|---:|---:|---:|
| Arty Z7-20 — 2026-10-15 | 15,446 / 53,200 (**29.03%**) | 15,899 / 106,400 (**14.94%**) | 17.5 / 140 (**12.50%**) | 19 / 220 (**8.64%**) |
| TE0890 — 2026-10-18 | 10,708 / 14,600 (**73.34%**) | 10,940 / 29,200 (**37.47%**) | 12.5 / 45 (**27.78%**) | 19 / 80 (**23.75%**) |

Reports: [Arty Z7-20 post-route utilization](rtl/top/Xilinx/ArtyZ7-20/synth/reports/post_route_util.rpt) · [TE0890 post-route utilization](rtl/top/Xilinx/te0890/synth-/reports/post_route_util.rpt). These are board-build snapshots, not portable core-area estimates; changing the FPGA, memory interface, enabled features, tool version, or constraints changes the results.

## Checkout and build

Clone the repository with its RTL and library submodules:

```sh
git clone --recurse-submodules https://github.com/ToNi3141/RasterIX.git
cd RasterIX
```

The checked-in CMake presets require **CMake 3.23 or newer**. Configure/build presets for host and embedded environments include:

| Preset | Purpose / notes |
|---|---|
| `native_rixif`, `native_rixef` | Native desktop build with examples and the FT60X driver selected; install the FTDI/FT60X dependencies required by the build. |
| `zynq_embedded_linux_rixef` | Zynq embedded Linux build; requires the matching cross toolchain and PetaLinux SDK/sysroots. See the [Arty guide](rtl/top/Xilinx/ArtyZ7-20/README.md). |
| `rppico` | Raspberry Pi Pico SDK base configuration is marked hidden in CMake and is not a directly selectable preset. Pico SDK and PlatformIO integration are documented in the [TE0890 guide](rtl/top/Xilinx/te0890/README.md); its PlatformIO example is available for direct integration. |
| `win32_sw` | 32-bit Windows WGL software-rasterizer build using Visual Studio. |
| `unittest_software`, `unittest_verilator` | Software unit tests and RTL tests driven by Verilator, respectively. |

Example native build (requires the FT60X/FTDI dependencies enabled by the preset):

```sh
cmake --preset native_rixef
cmake --build build/native --parallel
```

Example software test build:

```sh
cmake --preset unittest_software
cmake --build build/unittest-software --parallel
ctest --test-dir build/unittest-software --output-on-failure
```

For RTL simulation, install Verilator, then use the `unittest_verilator` preset and run CTest in `build/unittest-verilator`. The workspace’s historical test logs may not reflect the current source state.

For Zynq, initialize the PetaLinux SDK and set `SYSROOTS` to its sysroots directory before configuring. For Pico, select the board as required by your SDK (for example, `-DPICO_BOARD=pico2`). Consult each platform guide for tool installation, FPGA programming, and board-specific setup.

## Platforms

### FPGA and simulation targets

- [Verilator simulation](rtl/top/Verilator/README.md)
- [Digilent Nexys Video](rtl/top/Xilinx/NexysVideo/README.md)
- [Trenz Electronic TE0890](rtl/top/Xilinx/te0890/README.md)
- [Digilent Arty Z7-20](rtl/top/Xilinx/ArtyZ7-20/README.md)

### Software build and integration environments

- Native desktop builds and Windows WGL are defined by the CMake presets.
- Zynq embedded Linux uses the DMA Proxy driver and a PetaLinux cross-build environment.
- Raspberry Pi Pico SDK and PlatformIO integration examples are documented with the TE0890 project.
- Verilator supports RTL simulation; the CMake test presets cover software and RTL unit tests.

## Use the driver in a new software environment

RasterIX is built with CMake and C++17. The library target is `gl`; the repository’s CMake configuration selects platform drivers and can also add `threadrunner`. To integrate it into an existing product, use the repository’s targets or mirror the required source, dependency, compile-definition, and configuration settings in your build. The `example/util/` directory contains platform composition examples.

CMake also has top-level switches for selecting the host build and optional components:

| CMake option | Purpose |
|---|---|
| `RIX_BUILD_NATIVE`, `RIX_BUILD_ZYNQ_EMBEDDED_LINUX`, `RIX_BUILD_RPPICO` | Select the desktop, Zynq embedded Linux, or Pico SDK build path. |
| `RIX_DRIVER_FT60X`, `RIX_DRIVER_DMA_PROXY`, `RIX_DRIVER_STANDALONE_XIL_CDMA`, `RIX_DRIVER_VERILATOR`, `RIX_DRIVER_SOFTWARE_RASTERIZER` | Include the selected hardware/simulation/software bus connector. |
| `RIX_BUILD_EXAMPLES` | Build example applications. |
| `RIX_BUILD_TESTS_SOFTWARE`, `RIX_BUILD_TESTS_VERILATOR` | Enable software or Verilator test targets. |
| `RIX_BUILD_SHARED_LIBRARY` | Build a shared library where supported by the platform configuration. |
| `RIX_ENABLE_SPDLOG` | Enable spdlog logging. |

### Porting the device transport

Implement `rr::IBusConnector` for the target transport (for example DMA, SPI, USB, or a memory-mapped interface). The indexed buffers must meet the transport’s alignment/capability needs. `writeData()` and `readData()` transfer the requested byte count; the interface documents these operations as blocking when the prior transfer or requested read must complete. `blockUntilTransferIsComplete()` must make it safe to reuse the active buffer.

```cpp
class IBusConnector
{
public:
    virtual ~IBusConnector() = default;
    virtual void writeData(uint8_t index, uint32_t size, uint32_t offset = 0) = 0;
    virtual void readData(uint8_t index, uint32_t size) = 0;
    virtual void blockUntilTransferIsComplete() = 0;
    virtual tcb::span<uint8_t> requestWriteBuffer(uint8_t index) = 0;
    virtual tcb::span<uint8_t> requestReadBuffer(uint8_t index) = 0;
    virtual uint8_t getWriteBufferCount() const = 0;
    virtual uint8_t getReadBufferCount() const = 0;
};
```

The interface lives in [`lib/gl/IBusConnector.hpp`](lib/gl/IBusConnector.hpp). Existing connectors are platform-specific, not all interchangeable: the Xilinx DMA Proxy connector expects the Linux DMA Proxy kernel driver/device nodes, uses fixed DMA buffers, and warns that nonzero write offsets are ignored. The Xilinx standalone CDMA connector has no implemented read path, and the FT60X implementation’s read behavior is incomplete. Check the implementation before selecting a connector for bidirectional transfers.

### Thread execution

Implement `rr::IThreadRunner` when the platform needs a custom worker scheduler:

```cpp
class IThreadRunner
{
public:
    virtual ~IThreadRunner() = default;
    virtual void wait() = 0;
    virtual void run(const std::function<void()>& operation) = 0;
    virtual bool isBusy() const = 0;
};
```

`StdThreadRunner` uses `std::thread` with a single worker; a call to `run()` waits until its prior operation is complete before scheduling the next. Use it only where the standard threading runtime is available and useful. `NoThreadRunner` executes the operation inline and reports itself as not busy. The Pico example provides a platform-specific runner; other multicore MCUs may need their own implementation to use additional cores. See [`lib/threadrunner/`](lib/threadrunner/) and [`example/util/`](example/util/).

### Software-side driver configuration

The CMake variables below generate the `RIX_CORE_*` definitions consumed by [`RenderConfig`](lib/gl/RenderConfigs.hpp). Values affecting the command/data layout or hardware capability must match the RTL configuration.

| CMake variable | Purpose | Must match RTL? |
|---|---|---|
| `RIX_CORE_TMU_COUNT` | Number of TMUs and corresponding texture stages/triangle payload. | **Yes — `TMU_COUNT`.** |
| `RIX_CORE_ENABLE_MIPMAPPING` | Mipmap support used by the driver. | **Yes — `ENABLE_MIPMAPPING`.** |
| `RIX_CORE_TEXTURE_PAGE_SIZE` | Texture page size used for uploads and virtual addressing. | **Yes — `TEXTURE_PAGE_SIZE`.** |
| `RIX_CORE_TEXTURE_BUFFER_MAX_TEXTURE_SIZE` | Maximum texture dimension expected by the driver. | **Yes when using the full texture-buffer mode — `TEXTURE_BUFFER_MAX_TEXTURE_SIZE`.** |
| `RIX_CORE_IF_FRAMEBUFFER_SIZE_IN_PIXEL_LG` | IF framebuffer capacity in log2 pixels; also influences software frame/display-list splitting. | **Yes for IF — `IF_FRAMEBUFFER_SIZE_IN_PIXEL_LG`.** For EF it is a software splitting setting, not the EF framebuffer capacity. |
| `RIX_CORE_MAX_DISPLAY_WIDTH`, `RIX_CORE_MAX_DISPLAY_HEIGHT` | Driver’s maximum render dimensions. | No direct RTL parameter; configure consistently with your application and buffers. |
| `RIX_CORE_NUMBER_OF_TEXTURE_PAGES`, `RIX_CORE_NUMBER_OF_TEXTURES` | Driver-visible texture page and texture-object limits. | No; size the available device memory appropriately. |
| `RIX_CORE_GRAM_MEMORY_LOC` | Base/offset for device memory shared with other system users. | Must agree with the platform memory map, not an RTL feature parameter. |
| `RIX_CORE_COLOR_BUFFER_LOC_0`, `_1`, `_2`, `RIX_CORE_DEPTH_BUFFER_LOC`, `RIX_CORE_STENCIL_BUFFER_LOC` | Framebuffer memory locations used by the driver/platform. | Must agree with the memory map and display integration. |
| `RIX_CORE_ENABLE_ATTRIBUTE_SCALING` | Software-side attribute scaling for fixed-point interpolation cases. | No direct parameter counterpart. |
| `RIX_CORE_THREADED_RASTERIZATION`, `RIX_CORE_THREADED_RASTERIZATION_DISPLAY_LIST_SIZE` | Enables threaded vertex transformation and sets its display-list buffer size. | No direct RTL feature match; choose with the runner and bus-buffer capacities. |
| `RIX_CORE_ENABLE_VSYNC` | Enables driver-side framebuffer swap synchronization. | Coordinate with swap/display hardware. |
| `RIX_CORE_MAX_VBO_COUNT` | Maximum number of VBOs tracked by the driver. | No. |
| `RIX_CORE_AUTOLOAD_INTERNAL_FRAMEBUFFER` | IF compatibility behavior for loading internal framebuffer contents. | IF-only behavior; no EF counterpart. |
| `RIX_CORE_SOFTWARE_RENDERING` | Selects the software renderer where supported. | No FPGA required in software mode. |

For PlatformIO or other non-CMake builds, provide the same `RIX_CORE_*` compile definitions. See the [TE0890 PlatformIO example](rtl/top/Xilinx/te0890/README.md#platformio).

## Use the RTL core in a new project

Add the RTL under `rtl/RasterIX/` and the required dependencies under `rtl/3rdParty/` and `rtl/Float/rtl/float/` to your FPGA project. Use [`RasterIX.v`](rtl/RasterIX/RasterIX.v) as the public wrapper; it selects `RasterIX_IF` or `RasterIX_EF` with `VARIANT`.

At minimum, integrate:

1. `aclk` and active-low `resetn`.
2. The 32-bit AXI-Stream command input (`s_cmd_axis_*`) and command-response output (`m_cmd_resp_axis_*`). The host connector/device uploader constructs this stream; it is not raw OpenGL calls.
3. The AXI memory master interface (`m_axi_*`) to memory capable of serving framebuffer and texture/device-memory requests.
4. The framebuffer swap handshake (`swap_fb`, `swap_fb_enable_vsync`, `fb_addr`, `fb_size`, and `fb_swapped`) with the display or memory-management logic. The wrapper has no `m_framebuffer_axis` display-output port.
5. Optionally, performance outputs such as `perfBusy`, `perfTriangleRendering`, and `perfRasterizerStall` for instrumentation.

`FrameStreamingCore` handles transfers between the command stream, internal stream channels, and memory. The external memory system must honor AXI handshakes and supply sufficient throughput for your chosen framebuffer, texture, and resolution configuration.

### RTL parameters

`RasterIX` exposes these parameter groups. Exact names are in the [wrapper declaration](rtl/RasterIX/RasterIX.v).

| Parameter(s) | Meaning / integration note |
|---|---|
| `VARIANT` | `"ef"` (external framebuffer, default) or `"if"`. |
| `TMU_COUNT` | 1 or 2; **must equal** `RIX_CORE_TMU_COUNT`. |
| `ENABLE_MIPMAPPING` | Mipmap hardware; **must equal** `RIX_CORE_ENABLE_MIPMAPPING`. |
| `TEXTURE_PAGE_SIZE` | Texture MMU/page size; **must equal** `RIX_CORE_TEXTURE_PAGE_SIZE`. |
| `TEXTURE_BUFFER_MAX_TEXTURE_SIZE` | Full on-chip texture-buffer maximum; **must equal** the software maximum when full texture-buffer mode is used. |
| `IF_FRAMEBUFFER_SIZE_IN_PIXEL_LG` | IF internal framebuffer capacity; **must equal** the software setting when `VARIANT="if"`. It does not size the EF framebuffer. |
| `ENABLE_TEXTURE_CACHE`, `TEXTURE_CACHE_SIZE` | Choose external-memory texture reads through the cache, or disable the cache to use full texture buffers. |
| `ENABLE_TEXTURE_FILTERING`, `ENABLE_FOG`, `ENABLE_DEPTH_BUFFER`, `ENABLE_STENCIL_BUFFER` | Enable corresponding hardware features. These have no direct CMake `RIX_CORE_*` feature toggle; configure the driver/application to use only supported features. |
| `ENABLE_MEMORY_COALESCING` | Coalesces framebuffer memory transactions where supported by the selected memory path. |
| `ADDR_WIDTH`, `ID_WIDTH`, `DATA_WIDTH`, `STRB_WIDTH` | AXI address, ID, data, and byte-strobe widths; must match the attached interconnect. |
| `IF_FRAMEBUFFER_SUB_PIXEL_WIDTH`, `IF_FRAMEBUFFER_ENABLE_ALPHA_CHANNEL` | IF framebuffer storage format options. |
| `SUB_PIXEL_CALC_PRECISION`, `RASTERIZER_FLOAT_PRECISION`, `RASTERIZER_FIXPOINT_PRECISION`, `ENABLE_RASTERIZER_FLOAT_INTERPOLATION` | Interpolation and sub-pixel arithmetic precision options. |

The software/RTL equality constraints are summarized in the driver configuration table above. Additional runtime state—such as buffer addresses and enabled GL features—must also be valid for the selected hardware design.

## Core variants: prefer EF

The RTL has two framebuffer variants. **Use EF (`VARIANT="ef"`) when possible; IF is not recommended for new designs unless its internal-framebuffer trade-off is specifically needed.**

- **EF — External Framebuffer:** color, depth, and stencil buffers reside in external memory. This saves FPGA block RAM compared with keeping the framebuffers on-chip, at the cost of more AXI traffic and dependence on external-memory bandwidth and arbitration. The current EF color path uses RGB565; framebuffer mask/strobe behavior is constrained by byte-granular AXI strobes.
- **IF — Internal Framebuffer:** framebuffer storage resides in FPGA RAM. This consumes substantial BRAM and can require the driver to split rendering into display-list/frame chunks that fit the configured capacity. It can be useful where on-chip RAM and the associated host-side buffering model are acceptable.

The variants share the rendering pipeline but differ in framebuffer storage and behavior. Do not assume every framebuffer format or masking operation behaves identically between them.

## Software rasterizer

The repository also includes a software implementation of the rasterization/device path. The `win32_sw` preset builds it into a 32-bit Windows WGL `OpenGL32.dll`; it requires no FPGA. The current WGL setup uses 16 worker runners and a 32 MiB software GRAM buffer. This is a desktop compatibility path, not evidence of an MCU-optimized renderer.

The software rasterizer uses integer/fixed-point-style pixel data paths, including RGB565 color and integer depth handling. It is not fully feature-equivalent to the FPGA implementation: for example, lighting-context and some element/draw commands are explicitly unimplemented. Review the target application’s command and API usage before relying on software mode.

Build on a Windows host with the Visual Studio generator required by the preset:

```sh
cmake --preset win32_sw
cmake --build build/win32 --config Release --parallel
```

The resulting `OpenGL32.dll` is 32-bit; application-specific installation and compatibility steps vary by game. See the [WGL implementation](lib/wgl/wgl.cpp) and [software rasterizer](lib/gl/renderer/softwarerasterizer/SoftwareRasterizationInstance.cpp).

## Further reading

- [CMake presets](CMakePresets.json)
- [Examples](example/)
- [Arty Z7-20 platform guide](rtl/top/Xilinx/ArtyZ7-20/README.md)
- [Nexys Video platform guide](rtl/top/Xilinx/NexysVideo/README.md)
- [TE0890 platform guide](rtl/top/Xilinx/te0890/README.md)
- [Verilator simulation guide](rtl/top/Verilator/README.md)
- [Architecture and design notes](design.md)