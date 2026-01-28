if (RIX_DRIVER_FT60X)
    add_subdirectory(ft60x)
endif()

if (RIX_DRIVER_DMA_PROXY)
    add_subdirectory(dmaproxy)
endif()

if (RIX_DRIVER_VERILATOR)
    add_subdirectory(verilator)
endif()

if (RIX_DRIVER_STANDALONE_XIL_CDMA)
    add_subdirectory(standalonexilcdma)
endif()

if (RIX_DRIVER_SOFTWARE_RASTERIZER)
    add_subdirectory(softwarerasterizerbusconnector)
endif()