add_library(gl STATIC
    RIXGL.cpp
    glImpl.cpp
    gl.cpp
    glu.cpp
    vertexpipeline/VertexPipeline.cpp
    vertexpipeline/RenderObj.cpp
    transform/Lighting.cpp
    transform/Clipper.cpp
    transform/TexGen.cpp
    transform/ViewPort.cpp
    transform/MatrixStore.cpp
    transform/Culling.cpp
    transform/PrimitiveAssembler.cpp
    transform/PlaneClipper.cpp
    transform/PolygonOffset.cpp
    transform/Stencil.cpp
    transform/LineAssembly.cpp
    pixelpipeline/PixelPipeline.cpp
    pixelpipeline/Fogging.cpp
    pixelpipeline/Texture.cpp
    renderer/Rasterizer.cpp
    renderer/Renderer.cpp
)

target_link_libraries(gl PRIVATE spdlog::spdlog span)
target_include_directories(gl PUBLIC .)

add_compile_definitions(
    RIX_CORE_TMU_COUNT=${RIX_CORE_TMU_COUNT}
    RIX_CORE_MAX_TEXTURE_SIZE=${RIX_CORE_MAX_TEXTURE_SIZE}
    RIX_CORE_ENABLE_MIPMAPPING=${RIX_CORE_ENABLE_MIPMAPPING}
    RIX_CORE_MAX_DISPLAY_WIDTH=${RIX_CORE_MAX_DISPLAY_WIDTH}
    RIX_CORE_MAX_DISPLAY_HEIGHT=${RIX_CORE_MAX_DISPLAY_HEIGHT}
    RIX_CORE_FRAMEBUFFER_SIZE_IN_PIXEL_LG=${RIX_CORE_FRAMEBUFFER_SIZE_IN_PIXEL_LG}
    RIX_CORE_USE_FLOAT_INTERPOLATION=${RIX_CORE_USE_FLOAT_INTERPOLATION}
    RIX_CORE_NUMBER_OF_TEXTURE_PAGES=${RIX_CORE_NUMBER_OF_TEXTURE_PAGES}
    RIX_CORE_NUMBER_OF_TEXTURES=${RIX_CORE_NUMBER_OF_TEXTURES}
    RIX_CORE_TEXTURE_PAGE_SIZE=${RIX_CORE_TEXTURE_PAGE_SIZE}
    RIX_CORE_GRAM_MEMORY_LOC=${RIX_CORE_GRAM_MEMORY_LOC}
    RIX_CORE_COLOR_BUFFER_LOC_0=${RIX_CORE_COLOR_BUFFER_LOC_0}
    RIX_CORE_COLOR_BUFFER_LOC_1=${RIX_CORE_COLOR_BUFFER_LOC_1}
    RIX_CORE_COLOR_BUFFER_LOC_2=${RIX_CORE_COLOR_BUFFER_LOC_2}
    RIX_CORE_DEPTH_BUFFER_LOC=${RIX_CORE_DEPTH_BUFFER_LOC}
    RIX_CORE_STENCIL_BUFFER_LOC=${RIX_CORE_STENCIL_BUFFER_LOC}
    RIX_CORE_THREADED_RASTERIZATION=${RIX_CORE_THREADED_RASTERIZATION}
    RIX_CORE_THREADED_RASTERIZATION_DISPLAY_LIST_SIZE=${RIX_CORE_THREADED_RASTERIZATION_DISPLAY_LIST_SIZE}
    RIX_CORE_ENABLE_VSYNC=${RIX_CORE_ENABLE_VSYNC}
)
