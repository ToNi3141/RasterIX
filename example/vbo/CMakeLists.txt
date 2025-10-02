add_executable(vbo_example main.cpp VboExample.hpp)
target_link_libraries(vbo_example PRIVATE runner)

if (RIX_BUILD_RPPICO)
    # create map/bin/hex/uf2 file in addition to ELF.
    pico_add_extra_outputs(vbo_example)
endif()
