// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2024 ToNi3141

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef RRXGL_HPP
#define RRXGL_HPP

#include "IBusConnector.hpp"
#include "IThreadRunner.hpp"
#include <array>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace rr
{
class RenderDevice;
class VertexPipeline;
class PixelPipeline;
class VertexArray;
class VertexQueue;
class RRXGL
{
public:
    /// @brief Getting the instance of the current render context
    /// @return The instance of the current render context
    static RRXGL& getInstance();

    /// @brief Creates a new render context
    /// @param busConnector Driver to access the RasterIX hardware
    /// @param runner Runner to run some parts of the renderer in an own thread.
    ///     This project contains already two sample runners, one MultiThreadRunner for systems which
    ///     implement std::async and a SingleThreadRunner with no thread logic. If you have an multi core
    ///     system like the rppico, an own runner needs to be implemented to offload work to other cores.
    /// @return true if the creation was successful. This function currently uses heap memory. A false
    ///     can occur when the memory allocation fails.
    static bool createInstance(IBusConnector& busConnector, IThreadRunner& runner);

    /// @brief  Destroys the current context, switches the framebuffer to the system framebuffer and
    ///     and frees all allocated memory.
    static void destroy();

    void setError(const uint32_t error) { m_error = error; }
    uint32_t getError() const { return m_error; }

    VertexPipeline& pipeline();
    VertexQueue& vertexQueue();
    VertexArray& vertexArray();

    void swapDisplayList();
    void uploadDisplayList();

    const char* getLibExtensions() const;
    const void* getLibProcedure(std::string name) const;

    void addLibProcedure(std::string name, const void* address);
    void addLibExtension(std::string extension);

    /// @brief Queries the maximum texture size in pixels
    /// @return The maximum texture size in pixel
    std::size_t getMaxTextureSize() const;

    /// @brief Returns the maximum supported LOD level
    /// @return The maximum supported LOD level
    static std::size_t getMaxLOD();

    /// @brief Queries the maximum number of TMUs available for the hardware
    /// @brief The number of TMUs available
    std::size_t getTmuCount() const;

    /// @brief Queries of mip mapping is available on hardware
    /// @return true when mipmapping is available
    bool isMipmappingAvailable() const;

    /// @brief Sets the resolution of the screen
    /// @param x screen width
    /// @param y screen height
    /// @return true if succeeded
    bool setRenderResolution(const std::size_t x, const std::size_t y);

    /// @brief Enables vsync when swapping the color buffer
    /// @param enable true to enable vsync
    void enableVSync(const bool enable);

private:
    RRXGL(IBusConnector& busConnector, IThreadRunner& runner);
    ~RRXGL();
    RenderDevice* m_renderDevice { nullptr };

    // Errors
    uint32_t m_error { 0 };

    // OpenGL extensions
    std::map<std::string, const void*> m_glProcedures;
    std::string m_glExtensions;
};

} // namespace rr
#endif // RRXGL_HPP
