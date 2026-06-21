add_executable(labyrinth main.cpp Labyrinth.hpp)

target_link_libraries(labyrinth PRIVATE runner)

if (RIX_BUILD_RPPICO)
    # create map/bin/hex/uf2 file in addition to ELF.
    pico_add_extra_outputs(labyrinth)
endif()
