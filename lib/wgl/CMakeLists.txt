add_library(OpenGL32 SHARED
    wgl.cpp
    gl.def
)
SET(CMAKE_LINK_DEF_FILE_FLAG "/DEF:gl.def")
target_link_libraries(OpenGL32 PRIVATE gl spdlog::spdlog ft60x span threadrunner softwarerasterizerbusconnector)
