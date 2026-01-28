# Software unit tests

# Function to add a software unit test
# Usage: add_software_unittest(<name>)
function(add_software_unittest NAME)
    set(TARGET_NAME "test_${NAME}")
    set(TEST_FILE "test_${NAME}.cpp")
    
    add_executable(${TARGET_NAME} ${TEST_FILE})
    target_include_directories(${TARGET_NAME} PRIVATE ${GL_SRC_DIR})
    target_link_libraries(${TARGET_NAME} PRIVATE gl span)
    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)

    # Convert name to camelCase for test name (e.g., BlendFunc -> blendFunc)
    string(SUBSTRING ${NAME} 0 1 FIRST_CHAR)
    string(TOLOWER ${FIRST_CHAR} FIRST_CHAR_LOWER)
    string(SUBSTRING ${NAME} 1 -1 REST)
    set(TEST_NAME "${FIRST_CHAR_LOWER}${REST}")
    
    add_test(NAME ${TEST_NAME} COMMAND ${TARGET_NAME})
endfunction()

# Add unit tests
add_software_unittest(AttributeInterpolator)
add_software_unittest(BlendFunc)
add_software_unittest(Fog)
add_software_unittest(LogicOp)
add_software_unittest(Rasterizer)
add_software_unittest(StencilOp)
add_software_unittest(TestFunc)
add_software_unittest(TexEnv)
add_software_unittest(TextureMap)
