add_executable(stencilShadow main.cpp)

target_include_directories(stencilShadow PRIVATE ${CMAKE_SOURCE_DIR}/lib/gl/math)

target_link_libraries(stencilShadow PRIVATE runner)

if (RIX_BUILD_RPPICO)
    # create map/bin/hex/uf2 file in addition to ELF.
    pico_add_extra_outputs(stencilShadow)
endif()
