add_subdirectory(util)
add_subdirectory(stencilShadow)
add_subdirectory(minimal)
add_subdirectory(vbo)

if (NOT RIX_BUILD_RPPICO) 
    # exclude it from the RPPico build, because the memory is too small
    add_subdirectory(mipmap)
endif()
