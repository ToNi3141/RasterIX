TARGET_BUILD = simulation
#TARGET_BUILD = hardware
VARIANT = RasterIX_IF
#VARIANT = RasterIX_EF

PATH_PREFIX = ../../..

RIXGL_PATH = $${PATH_PREFIX}/lib/gl

QT       += core gui
CONFIG += c++17
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += HARDWARE_RENDERER

TEMPLATE = app
TARGET = qtRasterizerFpga

SOURCES += main.cpp\
        mainwindow.cpp \
    $${RIXGL_PATH}/RIXGL.cpp \
    $${RIXGL_PATH}/transform/Clipper.cpp \
    $${RIXGL_PATH}/transform/Lighting.cpp \
    $${RIXGL_PATH}/transform/TexGen.cpp \
    $${RIXGL_PATH}/transform/ViewPort.cpp \
    $${RIXGL_PATH}/transform/MatrixStore.cpp \
    $${RIXGL_PATH}/transform/Culling.cpp \
    $${RIXGL_PATH}/transform/PrimitiveAssembler.cpp \
    $${RIXGL_PATH}/transform/PlaneClipper.cpp \
    $${RIXGL_PATH}/transform/PolygonOffset.cpp \
    $${RIXGL_PATH}/transform/Stencil.cpp \
    $${RIXGL_PATH}/transform/LineAssembly.cpp \
    $${RIXGL_PATH}/transform/PointAssembly.cpp \
    $${RIXGL_PATH}/vertexpipeline/VertexPipeline.cpp \
    $${RIXGL_PATH}/vertexpipeline/RenderObj.cpp \
    $${RIXGL_PATH}/pixelpipeline/PixelPipeline.cpp \
    $${RIXGL_PATH}/glImpl.cpp \
    $${RIXGL_PATH}/renderer/Rasterizer.cpp \
    $${RIXGL_PATH}/renderer/Renderer.cpp \
    $${RIXGL_PATH}/pixelpipeline/Fogging.cpp \
    $${RIXGL_PATH}/pixelpipeline/Texture.cpp \
    $${RIXGL_PATH}/gl.cpp \
    $${RIXGL_PATH}/glu.cpp

HEADERS  += mainwindow.h \
    ../testscene.hpp \
    $${RIXGL_PATH}/*.hpp \
    $${RIXGL_PATH}/vertexpipeline/*.hpp \
    $${RIXGL_PATH}/pixelpipeline/*.hpp \
    $${RIXGL_PATH}/math/*.hpp \
    $${RIXGL_PATH}/renderer/*.hpp \
    $${RIXGL_PATH}/transform/*.hpp \
    $${RIXGL_PATH}/*.h \
    $${RIXGL_PATH}/renderer/registers/* \
    $${RIXGL_PATH}/renderer/dse/* \
    $${RIXGL_PATH}/renderer/threadedRasterizer/* \
    $${RIXGL_PATH}/renderer/commands/* \
    $${RIXGL_PATH}/renderer/displaylist/*.hpp \
    $${RIXGL_PATH}/../threadrunner/*.hpp


# spdlog
DEFINES += SPDLOG_ACTIVE_LEVEL=3
DEFINES += GL_SILENCE_DEPRICATION
QMAKE_CXXFLAGS += -I$${PATH_PREFIX}/lib/3rdParty/spdlog-1.10.0/include/
QMAKE_CFLAGS += -I$${PATH_PREFIX}/lib/3rdParty/spdlog-1.10.0/include/

# tcb span
QMAKE_CXXFLAGS += -I$${PATH_PREFIX}/lib/3rdParty/span/include/
QMAKE_CFLAGS += -I$${PATH_PREFIX}/lib/3rdParty/span/include/

# TMU settings
DEFINES += RIX_CORE_TMU_COUNT=2
DEFINES += RIX_CORE_MAX_TEXTURE_SIZE=256
DEFINES += RIX_CORE_ENABLE_MIPMAPPING=true
# Display Settings
DEFINES += RIX_CORE_MAX_DISPLAY_WIDTH=640
DEFINES += RIX_CORE_MAX_DISPLAY_HEIGHT=480
# Rasterizer settings
DEFINES += RIX_CORE_USE_FLOAT_INTERPOLATION=false
# Texture Memory Settings
DEFINES += RIX_CORE_NUMBER_OF_TEXTURE_PAGES=6912
DEFINES += RIX_CORE_NUMBER_OF_TEXTURES=6912
DEFINES += RIX_CORE_TEXTURE_PAGE_SIZE=4096
# Memory RAM location. This is used as memory offset for all device memory  
# address calculations. Mostly useful for architectures with shared memory
DEFINES += RIX_CORE_GRAM_MEMORY_LOC=0x0 # No shared memory
# Framebuffer Memory Location
DEFINES += RIX_CORE_COLOR_BUFFER_LOC_0=0x01E00000
DEFINES += RIX_CORE_COLOR_BUFFER_LOC_1=0x01E00000
DEFINES += RIX_CORE_COLOR_BUFFER_LOC_2=0x01C00000
DEFINES += RIX_CORE_DEPTH_BUFFER_LOC=0x01A00000
DEFINES += RIX_CORE_STENCIL_BUFFER_LOC=0x01900000
DEFINES += RIX_CORE_THREADED_RASTERIZATION=true
DEFINES += RIX_CORE_THREADED_RASTERIZATION_DISPLAY_LIST_SIZE=1048576
DEFINES += RIX_CORE_ENABLE_VSYNC=false
DEFINES += RIX_CORE_MAX_VBO_COUNT=256
equals(VARIANT, "RasterIX_IF") {
    DEFINES += RIX_CORE_FRAMEBUFFER_SIZE_IN_PIXEL_LG=15
}
equals(VARIANT, "RasterIX_EF") {
    DEFINES += RIX_CORE_FRAMEBUFFER_SIZE_IN_PIXEL_LG=20
}

equals(TARGET_BUILD, "hardware") {
    DEFINES += USE_HARDWARE

    FT60X_BUS_CONNECTOR_PATH = $${PATH_PREFIX}/lib/driver/ft60x
    FT60X_LIB_PATH = $${PATH_PREFIX}/lib/driver/ft60x/ftd3xx/osx

    LIBS += /usr/local/homebrew/Cellar/libusb/1.0.26/lib/libusb-1.0.dylib
    LIBS += $${FT60X_LIB_PATH}/libftd3xx-static.a

    QMAKE_CXXFLAGS += -I$${FT60X_BUS_CONNECTOR_PATH}/ \
        -I$${FT60X_LIB_PATH}/

    QMAKE_CFLAGS += -I$${FT60X_BUS_CONNECTOR_PATH}/\
        -I$${FT60X_LIB_PATH}/

    HEADERS += $${FT60X_BUS_CONNECTOR_PATH}/FT60XBusConnector.hpp
    SOURCES += $${FT60X_BUS_CONNECTOR_PATH}/FT60XBusConnector.cpp
}
equals(TARGET_BUILD, "simulation") {
    # Set here the path to your local verilator installation
    unix:!macx {
        VERILATOR_PATH = /usr/local/share/verilator
        SOURCES += $${VERILATOR_PATH}/include/verilated_threads.cpp
    }
    macx: {
        VERILATOR_PATH = /opt/homebrew/Cellar/verilator/4.220/share/verilator
    }

    VERILATOR_BUS_CONNECTOR_PATH = $${PATH_PREFIX}/lib/driver/verilator
    VERILATOR_CODE_GEN_PATH = $${PATH_PREFIX}/rtl/top/Verilator/obj_dir

    DEFINES += USE_SIMULATION

    HEADERS += $${VERILATOR_BUS_CONNECTOR_PATH}/VerilatorBusConnector.hpp
    HEADERS += $${PATH_PREFIX}/lib/utils/GenericMemoryBusConnector.hpp

    SOURCES += $${VERILATOR_PATH}/include/verilated.cpp
    SOURCES += $${VERILATOR_PATH}/include/verilated_vcd_c.cpp

    LIBS += $${VERILATOR_CODE_GEN_PATH}/Vtop__ALL.a
}

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -I$${VERILATOR_CODE_GEN_PATH}/ \
    -I$${VERILATOR_BUS_CONNECTOR_PATH}/ \
    -I$${VERILATOR_BUS_CONNECTOR_PATH}/../ \
    -I$${VERILATOR_PATH}/include/ \
    -I$${RIXGL_PATH}/ \
    -I$${PATH_PREFIX}/lib/utils \
    -I$${PATH_PREFIX}/lib/threadrunner
