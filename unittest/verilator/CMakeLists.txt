cmake_minimum_required(VERSION 3.14)

find_program(VERILATOR_EXECUTABLE verilator REQUIRED)

set(RTL_DIR ${CMAKE_SOURCE_DIR}/rtl)
set(RTL_RASTERIX ${RTL_DIR}/RasterIX)
set(RTL_FLOAT ${RTL_DIR}/Float/rtl/float)
set(RTL_DISPLAY ${RTL_DIR}/Display)
set(RTL_3RDPARTY ${RTL_DIR}/3rdParty)

set(COMMON_VERILATOR_INCLUDES
    -I${RTL_RASTERIX}
    -I${RTL_FLOAT}
    -I${RTL_3RDPARTY}
)

# Function to create a Verilator test
# Verilator is executed during build, not during cmake
# Arguments: TEST_NAME TOP_MODULE VERILOG_FILE [EXTRA_INCLUDES...]
function(add_verilator_test TEST_NAME TOP_MODULE VERILOG_FILE)
    set(EXTRA_INCLUDES ${ARGN})
    
    # Convert extra includes to -I flags
    set(EXTRA_INCLUDE_FLAGS "")
    foreach(INC ${EXTRA_INCLUDES})
        list(APPEND EXTRA_INCLUDE_FLAGS -I${INC})
    endforeach()
    
    set(OBJ_DIR ${CMAKE_CURRENT_BINARY_DIR}/obj_dir_${TEST_NAME})
    set(VERILATED_EXECUTABLE ${OBJ_DIR}/V${TOP_MODULE})
    
    # Custom command: Call Verilator with --exe (complete executable)
    add_custom_command(
        OUTPUT ${VERILATED_EXECUTABLE}
        COMMAND ${VERILATOR_EXECUTABLE}
            -DUNITTEST
            -CFLAGS -std=c++20
            --cc
            --exe
            --Mdir ${OBJ_DIR}
            ${COMMON_VERILATOR_INCLUDES}
            ${EXTRA_INCLUDE_FLAGS}
            --top-module ${TOP_MODULE}
            ${VERILOG_FILE}
            ${CMAKE_CURRENT_SOURCE_DIR}/cpp/sim_${TOP_MODULE}.cpp
        COMMAND make -C ${OBJ_DIR} -f V${TOP_MODULE}.mk
        DEPENDS ${VERILOG_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/cpp/sim_${TOP_MODULE}.cpp
        COMMENT "Verilating and building ${TOP_MODULE}..."
        VERBATIM
    )
    
    # Custom target for the executable
    add_custom_target(${TEST_NAME} ALL DEPENDS ${VERILATED_EXECUTABLE})
    
    add_test(NAME ${TEST_NAME} COMMAND ${VERILATED_EXECUTABLE})
endfunction()

# Register tests
add_verilator_test(frameStreamingCore FrameStreamingCore ${RTL_RASTERIX}/FrameStreamingCore.v)
add_verilator_test(simulationILI9486 DisplayController8BitILI9486 ${RTL_DISPLAY}/DisplayController8BitILI9486.v)
add_verilator_test(simulationILI9341 DisplayController8BitILI9341 ${RTL_DISPLAY}/DisplayController8BitILI9341.v)
add_verilator_test(attributeInterpolator AttributeInterpolator ${RTL_RASTERIX}/AttributeInterpolator.v)
add_verilator_test(functionInterpolator FunctionInterpolator ${RTL_RASTERIX}/FunctionInterpolator.v)
add_verilator_test(textureSampler TextureSamplerTestModule ${CMAKE_CURRENT_SOURCE_DIR}/v/TextureSamplerTestModule.v)
add_verilator_test(colorInterpolator ColorInterpolator ${RTL_RASTERIX}/ColorInterpolator.v)
add_verilator_test(texEnv TexEnv ${RTL_RASTERIX}/TexEnv.v)
add_verilator_test(stencilOp StencilOp ${RTL_RASTERIX}/StencilOp.v)
add_verilator_test(memoryReadRequestGenerator MemoryReadRequestGenerator ${RTL_RASTERIX}/MemoryReadRequestGenerator.v)
add_verilator_test(framebufferSerializer FramebufferSerializer ${RTL_RASTERIX}/FramebufferSerializer.v)
add_verilator_test(streamConcatFifo StreamConcatFifo ${RTL_RASTERIX}/StreamConcatFifo.v)
add_verilator_test(framebufferWriter FramebufferWriter ${RTL_RASTERIX}/FramebufferWriter.v)
add_verilator_test(framebufferWriterStrobeGen FramebufferWriterStrobeGen ${RTL_RASTERIX}/FramebufferWriterStrobeGen.v)
add_verilator_test(framebufferWriterClear FramebufferWriterClear ${RTL_RASTERIX}/FramebufferWriterClear.v)
add_verilator_test(framebufferReader FramebufferReader ${RTL_RASTERIX}/FramebufferReader.v 
    ${RTL_3RDPARTY}/verilog-axis)
add_verilator_test(lodCalculator LodCalculator ${RTL_RASTERIX}/LodCalculator.v)
add_verilator_test(attributeInterpolationX AttributeInterpolationX ${RTL_RASTERIX}/AttributeInterpolationX.v)
add_verilator_test(attributePerspectiveCorrectionX AttributePerspectiveCorrectionX ${RTL_RASTERIX}/AttributePerspectiveCorrectionX.v)
add_verilator_test(triangleStreamF2XConverter TriangleStreamF2XConverter ${RTL_RASTERIX}/TriangleStreamF2XConverter.v)
add_verilator_test(pagedMemoryReader PagedMemoryReader ${RTL_RASTERIX}/PagedMemoryReader.v)
add_verilator_test(internalFramebufferCommandHandler InternalFramebufferCommandHandler ${RTL_RASTERIX}/InternalFramebufferCommandHandler.v)
add_verilator_test(axisToAxiAdapter AxisToAxiAdapter ${RTL_RASTERIX}/AxisToAxiAdapter.v)
add_verilator_test(axisToAxiCrossbar AxisToAxiCrossbar ${RTL_RASTERIX}/AxisToAxiCrossbar.v)
add_verilator_test(logicOp LogicOp ${RTL_RASTERIX}/LogicOp.v)
