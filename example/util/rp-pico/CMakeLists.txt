add_library(runner INTERFACE)

target_link_libraries(runner INTERFACE pico_stdlib pico_multicore gl spdlog::spdlog hardware_spi hardware_dma span threadrunner)

target_include_directories(runner INTERFACE .)
