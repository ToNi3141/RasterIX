#pragma once

#include "LabyrinthTypes.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace labyrinth
{
class MazeLayout
{
public:
    static constexpr uint32_t Width = 11;
    static constexpr uint32_t Height = 11;
    static constexpr float CellSize = 1.65f;
    static constexpr float CeilingHeight = 1.85f;
    static constexpr float EyeHeight = 0.82f;
    using GridType = std::array<const char*, Height>;

    template<size_t... RowLengths>
    static constexpr GridType makeGrid(const char (&...rows)[RowLengths])
    {
        static_assert(sizeof...(rows) == Height, "Maze grid must provide exactly Height rows.");
        static_assert(((RowLengths == (Width + 1)) && ...), "Each maze row must contain exactly Width characters.");
        return { { rows... } };
    }

    explicit MazeLayout(const GridType& grid)
        : m_grid(grid)
    {
    }

    bool isWall(const int32_t row, const int32_t col) const
    {
        if (row < 0 || col < 0 || row >= static_cast<int32_t>(Height) || col >= static_cast<int32_t>(Width))
        {
            return true;
        }
        return m_grid[static_cast<uint32_t>(row)][static_cast<uint32_t>(col)] == '#';
    }

    static bool isLightMarker(const char cell)
    {
        return (cell == 'l') || (cell == 'L');
    }

    static float cellMinX(const uint32_t col)
    {
        return (static_cast<float>(col) - static_cast<float>(Width) * 0.5f) * CellSize;
    }

    static float cellMaxX(const uint32_t col)
    {
        return (static_cast<float>(col + 1) - static_cast<float>(Width) * 0.5f) * CellSize;
    }

    static float cellMinY(const uint32_t row)
    {
        return (static_cast<float>(Height) * 0.5f - static_cast<float>(row + 1)) * CellSize;
    }

    static float cellMaxY(const uint32_t row)
    {
        return (static_cast<float>(Height) * 0.5f - static_cast<float>(row)) * CellSize;
    }

    static Vec2 cellCenter(const Cell cell)
    {
        return {
            (static_cast<float>(cell.col) - (static_cast<float>(Width) - 1.0f) * 0.5f) * CellSize,
            ((static_cast<float>(Height) - 1.0f) * 0.5f - static_cast<float>(cell.row)) * CellSize,
        };
    }

    std::vector<Vec2> collectLightPositions() const
    {
        std::vector<Vec2> lights;
        for (uint32_t row = 0; row < Height; ++row)
        {
            for (uint32_t col = 0; col < Width; ++col)
            {
                if (isLightMarker(m_grid[row][col]))
                {
                    lights.push_back(cellCenter({ static_cast<uint8_t>(row), static_cast<uint8_t>(col) }));
                }
            }
        }
        return lights;
    }

    const GridType& grid() const
    {
        return m_grid;
    }

private:
    const GridType& m_grid;
};
} // namespace labyrinth
