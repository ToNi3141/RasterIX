#ifndef _RENDER_CONFIG_HPP_
#define _RENDER_CONFIG_HPP_

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace rr
{

struct RenderConfig
{
    // TMU settings
    static constexpr std::size_t TMU_COUNT { RIX_CORE_TMU_COUNT };
    static constexpr std::size_t MAX_TEXTURE_SIZE { RIX_CORE_MAX_TEXTURE_SIZE };
    static constexpr bool ENABLE_MIPMAPPING { RIX_CORE_ENABLE_MIPMAPPING };

    // Display Settings
    static constexpr std::size_t MAX_DISPLAY_WIDTH { RIX_CORE_MAX_DISPLAY_WIDTH };
    static constexpr std::size_t MAX_DISPLAY_HEIGHT { RIX_CORE_MAX_DISPLAY_HEIGHT };
    static constexpr std::size_t FRAMEBUFFER_SIZE_IN_PIXEL_LG { RIX_CORE_FRAMEBUFFER_SIZE_IN_PIXEL_LG };
    static constexpr std::size_t FRAMEBUFFER_SIZE_IN_PIXEL { 1 << FRAMEBUFFER_SIZE_IN_PIXEL_LG };

    // Rasterizer settings
    static constexpr bool USE_FLOAT_INTERPOLATION { RIX_CORE_USE_FLOAT_INTERPOLATION };

    // Texture Memory Settings
    static constexpr std::size_t NUMBER_OF_TEXTURE_PAGES { RIX_CORE_NUMBER_OF_TEXTURE_PAGES };
    static constexpr std::size_t NUMBER_OF_TEXTURES { RIX_CORE_NUMBER_OF_TEXTURES };
    static constexpr std::size_t TEXTURE_PAGE_SIZE { RIX_CORE_TEXTURE_PAGE_SIZE };

    // Memory RAM location. This is used as memory offset for all device memory
    // address calculations. Mostly useful for architectures with shared memory
    static constexpr uint32_t GRAM_MEMORY_LOC { RIX_CORE_GRAM_MEMORY_LOC }; // Shares memory with linux

    // Framebuffer Memory Location
    static constexpr uint32_t COLOR_BUFFER_LOC_0 { RIX_CORE_COLOR_BUFFER_LOC_0 };
    static constexpr uint32_t COLOR_BUFFER_LOC_1 { RIX_CORE_COLOR_BUFFER_LOC_1 };
    static constexpr uint32_t COLOR_BUFFER_LOC_2 { RIX_CORE_COLOR_BUFFER_LOC_2 };
    static constexpr uint32_t DEPTH_BUFFER_LOC { RIX_CORE_DEPTH_BUFFER_LOC };
    static constexpr uint32_t STENCIL_BUFFER_LOC { RIX_CORE_STENCIL_BUFFER_LOC };

    // Misc
    static constexpr bool THREADED_RASTERIZATION { RIX_CORE_THREADED_RASTERIZATION };
    static constexpr std::size_t THREADED_RASTERIZATION_DISPLAY_LIST_BUFFER_SIZE { RIX_CORE_THREADED_RASTERIZATION_DISPLAY_LIST_SIZE };
    static constexpr bool ENABLE_VSYNC { RIX_CORE_ENABLE_VSYNC };
    static constexpr std::size_t MAX_VBO_COUNT { RIX_CORE_MAX_VBO_COUNT };
    static constexpr bool PERFORMANCE_MODE { RIX_CORE_PERFORMANCE_MODE };

    static constexpr std::size_t getDisplayLines()
    {
        constexpr std::size_t MAX_FRAMEBUFFER_SIZE = MAX_DISPLAY_WIDTH * MAX_DISPLAY_HEIGHT;
        if constexpr (MAX_FRAMEBUFFER_SIZE == FRAMEBUFFER_SIZE_IN_PIXEL)
        {
            return 1;
        }
        return (MAX_FRAMEBUFFER_SIZE / FRAMEBUFFER_SIZE_IN_PIXEL) + 1;
    }

    static constexpr uint32_t getAlignedSize(const uint32_t size)
    {
        constexpr uint32_t MASK = TEXTURE_PAGE_SIZE - 1;
        if (size & MASK)
        {
            return (size | MASK) + 1;
        }
        return size;
    }
};

} // namespace rr

#endif // _RENDER_CONFIG_HPP_
