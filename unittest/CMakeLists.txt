if (RIX_BUILD_TESTS_VERILATOR)
    add_subdirectory(verilator)
endif()

if (RIX_BUILD_TESTS_SOFTWARE)
    add_subdirectory(software)
endif()
