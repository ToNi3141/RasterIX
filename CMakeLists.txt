cmake_minimum_required(VERSION 3.14...3.25)

# Selects the platform to build for
option(RIX_BUILD_NATIVE "Sets up a native build for the current platform" OFF)
option(RIX_BUILD_ZYNQ_EMBEDDED_LINUX "Sets up a cross build environment for embedded linux with zynq drivers" OFF)
option(RIX_BUILD_RPPICO "Sets up a cross compilation build for the RPPico" OFF)
# Enables the examples
option(RIX_BUILD_EXAMPLES "Builds the examples" OFF)
# Enables the unit tests
option(RIX_BUILD_TESTS_VERILATOR "Builds the Verilator RTL unit tests" OFF)
option(RIX_BUILD_TESTS_SOFTWARE "Builds the software unit tests" OFF)
# Builds a dynamic library
option(RIX_BUILD_SHARED_LIBRARY "Builds a dynamic loadable library" OFF)
# Selects the bus connector
option(RIX_DRIVER_FT60X "Includes a driver for a FT60X bus connector" OFF)
option(RIX_DRIVER_VERILATOR "Includes a driver for the Verilator simulation" OFF)
option(RIX_DRIVER_DMA_PROXY "Includes a driver for the DMA Proxy bus connector" OFF)
option(RIX_DRIVER_STANDALONE_XIL_CDMA "Includes a driver for the Xilinx CDMA bus connector" OFF)
option(RIX_DRIVER_SOFTWARE_RASTERIZER "Includes a driver for the Software Rasterizer" OFF)
# Enables logging
option(RIX_ENABLE_SPDLOG "Enables logging via the spdlog library" OFF)

# Hardware defines
set(RIX_CORE_TMU_COUNT "1" CACHE STRING "The number of TMUs in the hardware")
# Texture settings
set(RIX_CORE_MAX_TEXTURE_SIZE "256" CACHE STRING "The maximum width of the texture")
set(RIX_CORE_ENABLE_MIPMAPPING "true" CACHE STRING "Enables mipmapping")
# Display settings
set(RIX_CORE_MAX_DISPLAY_WIDTH "1024" CACHE STRING "The maximum width of the display")
set(RIX_CORE_MAX_DISPLAY_HEIGHT "600" CACHE STRING "The maximum height of the display")
set(RIX_CORE_FRAMEBUFFER_SIZE_IN_PIXEL_LG "16" CACHE STRING "The log2(size) of the framebuffer in 16 bit words / in pixel")
# Rasterizer settings
set(RIX_CORE_USE_FLOAT_INTERPOLATION "false" CACHE STRING "Enables float interpolation")
# Texture memory settings
set(RIX_CORE_NUMBER_OF_TEXTURE_PAGES "7280" CACHE STRING "The number of texture pages")
set(RIX_CORE_NUMBER_OF_TEXTURES "7280" CACHE STRING "The number of textures (typically less or the same as RIX_CORE_NUMBER_OF_TEXTURE_PAGES)")
set(RIX_CORE_TEXTURE_PAGE_SIZE "4096" CACHE STRING "The size of a texture page")
# Memory RAM location. This is used as memory offset for all device memory  
# address calculations. Mostly useful for architectures with shared memory
set(RIX_CORE_GRAM_MEMORY_LOC "0x0E000000" CACHE STRING "The location of the GRAM memory")
# Framebuffer memory location
set(RIX_CORE_COLOR_BUFFER_LOC_0 "0x01E00000" CACHE STRING "The location of the color buffer when the RasterIX is disabled")
set(RIX_CORE_COLOR_BUFFER_LOC_1 "0x01E00000" CACHE STRING "The location of the first color buffer")
set(RIX_CORE_COLOR_BUFFER_LOC_2 "0x01C00000" CACHE STRING "The location of the second color buffer")
set(RIX_CORE_DEPTH_BUFFER_LOC "0" CACHE STRING "The location of the depth buffer")
set(RIX_CORE_STENCIL_BUFFER_LOC "0" CACHE STRING "The location of the stencil buffer")
# Misc
set(RIX_CORE_THREADED_RASTERIZATION "false" CACHE STRING "Enables the threaded rasterization. Can improve the performance on multi core linux systems.")
set(RIX_CORE_ENABLE_VSYNC "false" CACHE STRING "Enables vsync. Requires two framebuffers and a display hardware, which supports the vsync signals.")
set(RIX_CORE_THREADED_RASTERIZATION_DISPLAY_LIST_SIZE "1024 * 1024 * 4" CACHE STRING "Sets the size of the display list. Bigger lists are required for the IF config. The EF config allows smaller lists because of intermediate uploads.")
set(RIX_CORE_MAX_VBO_COUNT "256" CACHE STRING "The maximum number of VBOs")
set(RIX_CORE_PERFORMANCE_MODE "false" CACHE STRING "Can increase the rendering performance by sacrificing compatibility.")

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(
    RIX_CORE_TMU_COUNT=${RIX_CORE_TMU_COUNT}
    RIX_CORE_MAX_TEXTURE_SIZE=${RIX_CORE_MAX_TEXTURE_SIZE}
    RIX_CORE_ENABLE_MIPMAPPING=${RIX_CORE_ENABLE_MIPMAPPING}
    RIX_CORE_MAX_DISPLAY_WIDTH=${RIX_CORE_MAX_DISPLAY_WIDTH}
    RIX_CORE_MAX_DISPLAY_HEIGHT=${RIX_CORE_MAX_DISPLAY_HEIGHT}
    RIX_CORE_FRAMEBUFFER_SIZE_IN_PIXEL_LG=${RIX_CORE_FRAMEBUFFER_SIZE_IN_PIXEL_LG}
    RIX_CORE_USE_FLOAT_INTERPOLATION=${RIX_CORE_USE_FLOAT_INTERPOLATION}
    RIX_CORE_NUMBER_OF_TEXTURE_PAGES=${RIX_CORE_NUMBER_OF_TEXTURE_PAGES}
    RIX_CORE_NUMBER_OF_TEXTURES=${RIX_CORE_NUMBER_OF_TEXTURES}
    RIX_CORE_TEXTURE_PAGE_SIZE=${RIX_CORE_TEXTURE_PAGE_SIZE}
    RIX_CORE_GRAM_MEMORY_LOC=${RIX_CORE_GRAM_MEMORY_LOC}
    RIX_CORE_COLOR_BUFFER_LOC_0=${RIX_CORE_COLOR_BUFFER_LOC_0}
    RIX_CORE_COLOR_BUFFER_LOC_1=${RIX_CORE_COLOR_BUFFER_LOC_1}
    RIX_CORE_COLOR_BUFFER_LOC_2=${RIX_CORE_COLOR_BUFFER_LOC_2}
    RIX_CORE_DEPTH_BUFFER_LOC=${RIX_CORE_DEPTH_BUFFER_LOC}
    RIX_CORE_STENCIL_BUFFER_LOC=${RIX_CORE_STENCIL_BUFFER_LOC}
    RIX_CORE_THREADED_RASTERIZATION=${RIX_CORE_THREADED_RASTERIZATION}
    RIX_CORE_THREADED_RASTERIZATION_DISPLAY_LIST_SIZE=${RIX_CORE_THREADED_RASTERIZATION_DISPLAY_LIST_SIZE}
    RIX_CORE_ENABLE_VSYNC=${RIX_CORE_ENABLE_VSYNC}
    RIX_CORE_MAX_VBO_COUNT=${RIX_CORE_MAX_VBO_COUNT}
    RIX_CORE_PERFORMANCE_MODE=${RIX_CORE_PERFORMANCE_MODE}
)

if (NOT RIX_BUILD_RPPICO)
    project(
        RasterIX
        VERSION 0.1
    )
endif()

if (RIX_BUILD_RPPICO)
        add_link_options(-Wl,--print-memory-usage)
        # initialize the SDK based on PICO_SDK_PATH
        # note: this must happen before project()
        include(pico_sdk_import.cmake)
        project(
            RasterIXRPPico
            VERSION 0.1
        )
        # initialize the Raspberry Pi Pico SDK
        pico_sdk_init()
endif()

if (CMAKE_HOST_WIN32)
    add_compile_definitions(NOMINMAX)
endif()

if (CMAKE_HOST_APPLE AND RIX_BUILD_NATIVE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64") # The FTDI library is only available for X86 (at least for mac os)
endif()

if (RIX_BUILD_ZYNQ_EMBEDDED_LINUX AND RIX_BUILD_SHARED_LIBRARY)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=3)

if (RIX_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if (RIX_BUILD_TESTS_VERILATOR OR RIX_BUILD_TESTS_SOFTWARE)
    enable_testing()
    add_subdirectory(unittest)
endif()

if (NOT RIX_BUILD_TESTS_VERILATOR OR RIX_BUILD_TESTS_SOFTWARE)
    add_subdirectory(lib)
endif()
