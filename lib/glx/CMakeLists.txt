add_library(GL SHARED
    glx.cpp
)
target_link_libraries(GL PRIVATE spdlog::spdlog dmaproxy span threadrunner X11)

target_link_libraries(GL PUBLIC "-Wl,--whole-archive ../gl/libgl.a -Wl,--no-whole-archive" gl)

set_target_properties(GL PROPERTIES COMPILE_FLAGS "")
