add_library(OpenGL32 SHARED
    wgl.cpp
    gl.def
)
SET(CMAKE_LINK_DEF_FILE_FLAG "/DEF:gl.def")

set(OpenGL32_PRIVATE_LIBS gl spdlog::spdlog span threadrunner softwarerasterizerbusconnector utils)
if (RIX_DRIVER_FT60X)
    list(APPEND OpenGL32_PRIVATE_LIBS ft60x)
endif()

target_link_libraries(OpenGL32 PRIVATE ${OpenGL32_PRIVATE_LIBS})
