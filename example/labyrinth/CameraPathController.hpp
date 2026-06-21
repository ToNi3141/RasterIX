#pragma once

#include "LabyrinthMaze.hpp"
#include <array>
#include <chrono>
#include <cmath>
#include <vector>

namespace labyrinth
{
class CameraPathController
{
public:
    struct CameraView
    {
        Vec2 position;
        Vec2 lookTarget;
    };

    CameraPathController(
        const MazeLayout& mazeLayout,
        const float lookAheadCells,
        const float speedCellsPerSecond,
        const float turnSpeedRadiansPerFrame)
        : m_mazeLayout(mazeLayout)
        , m_lookAheadCells(lookAheadCells)
        , m_speedCellsPerSecond(speedCellsPerSecond)
        , m_turnSpeedRadiansPerFrame(turnSpeedRadiansPerFrame)
    {
    }

    void resetAndBuild()
    {
        std::array<bool, MazeLayout::Width * MazeLayout::Height> visited {};
        m_path.clear();
        m_pathLengthCells = 0.0f;
        m_pathDistance = 0.0f;
        m_hasLastFrameTime = false;

        const Cell startCell = findStartCell();
        if (m_mazeLayout.isWall(startCell.row, startCell.col))
        {
            return;
        }

        addPathCell(startCell);
        buildDepthFirst(startCell, visited);
        initializeHeading();
    }

    CameraView currentView()
    {
        const Vec2 camera = getPathPosition(m_pathDistance);
        const Vec2 desiredTarget = getPathPosition(m_pathDistance + m_lookAheadCells);
        updateHeading(camera, desiredTarget);

        return {
            camera,
            {
                camera.x + std::cos(m_heading) * MazeLayout::CellSize,
                camera.y + std::sin(m_heading) * MazeLayout::CellSize,
            },
        };
    }

    void advanceByFrameTime()
    {
        const auto now = std::chrono::steady_clock::now();
        float deltaSeconds = 0.0f;
        if (m_hasLastFrameTime)
        {
            deltaSeconds = std::chrono::duration<float>(now - m_lastFrameTime).count();
            if (deltaSeconds > 0.1f)
            {
                deltaSeconds = 0.1f;
            }
        }
        m_lastFrameTime = now;
        m_hasLastFrameTime = true;

        m_pathDistance += m_speedCellsPerSecond * deltaSeconds;
        if (m_pathLengthCells > 0.0f && m_pathDistance >= m_pathLengthCells)
        {
            m_pathDistance -= m_pathLengthCells;
        }
    }

private:
    void updateHeading(const Vec2 camera, const Vec2 desiredTarget)
    {
        const float directionX = desiredTarget.x - camera.x;
        const float directionY = desiredTarget.y - camera.y;
        if ((directionX * directionX + directionY * directionY) <= 0.001f)
        {
            return;
        }

        const float desiredHeading = std::atan2(directionY, directionX);
        if (!m_headingInitialized)
        {
            m_heading = desiredHeading;
            m_headingInitialized = true;
            return;
        }

        const float headingDelta = wrapAngle(desiredHeading - m_heading);
        if (headingDelta > m_turnSpeedRadiansPerFrame)
        {
            m_heading += m_turnSpeedRadiansPerFrame;
        }
        else if (headingDelta < -m_turnSpeedRadiansPerFrame)
        {
            m_heading -= m_turnSpeedRadiansPerFrame;
        }
        else
        {
            m_heading = desiredHeading;
        }

        m_heading = wrapAngle(m_heading);
    }

    static constexpr float PI = 3.14159265359f;
    static constexpr float TWO_PI = PI * 2.0f;

    static uint32_t segmentLength(const Cell a, const Cell b)
    {
        return static_cast<uint32_t>(std::abs(static_cast<int32_t>(a.row) - static_cast<int32_t>(b.row))
            + std::abs(static_cast<int32_t>(a.col) - static_cast<int32_t>(b.col)));
    }

    Cell findStartCell() const
    {
        for (uint32_t row = 0; row < MazeLayout::Height; ++row)
        {
            for (uint32_t col = 0; col < MazeLayout::Width; ++col)
            {
                if (!m_mazeLayout.isWall(static_cast<int32_t>(row), static_cast<int32_t>(col)))
                {
                    return { static_cast<uint8_t>(row), static_cast<uint8_t>(col) };
                }
            }
        }
        return { 0, 0 };
    }

    void initializeHeading()
    {
        m_headingInitialized = false;
        if (m_path.size() < 2)
        {
            m_heading = 0.0f;
            return;
        }

        const Vec2 start = MazeLayout::cellCenter(m_path[0]);
        for (uint32_t i = 1; i < m_path.size(); ++i)
        {
            const Vec2 target = MazeLayout::cellCenter(m_path[i]);
            const float directionX = target.x - start.x;
            const float directionY = target.y - start.y;
            if ((directionX * directionX + directionY * directionY) > 0.001f)
            {
                m_heading = std::atan2(directionY, directionX);
                m_headingInitialized = true;
                return;
            }
        }
    }

    void buildDepthFirst(const Cell cell, std::array<bool, MazeLayout::Width * MazeLayout::Height>& visited)
    {
        visited[cell.row * MazeLayout::Width + cell.col] = true;

        static constexpr std::array<CellOffset, 4> neighborOffsets = { {
            { 0, 1 },
            { 1, 0 },
            { 0, -1 },
            { -1, 0 },
        } };

        for (const CellOffset offset : neighborOffsets)
        {
            const int32_t nextRow = static_cast<int32_t>(cell.row) + offset.row;
            const int32_t nextCol = static_cast<int32_t>(cell.col) + offset.col;
            if (m_mazeLayout.isWall(nextRow, nextCol))
            {
                continue;
            }

            const Cell nextCell = { static_cast<uint8_t>(nextRow), static_cast<uint8_t>(nextCol) };
            if (visited[nextCell.row * MazeLayout::Width + nextCell.col])
            {
                continue;
            }

            addPathCell(nextCell);
            buildDepthFirst(nextCell, visited);
            addPathCell(cell);
        }
    }

    void addPathCell(const Cell cell)
    {
        if (!m_path.empty())
        {
            m_pathLengthCells += static_cast<float>(segmentLength(m_path.back(), cell));
        }
        m_path.push_back(cell);
    }

    Vec2 getPathPosition(float distance) const
    {
        if (m_path.empty())
        {
            return { 0.0f, 0.0f };
        }

        if (m_pathLengthCells <= 0.0f)
        {
            return MazeLayout::cellCenter(m_path.front());
        }

        while (distance >= m_pathLengthCells)
        {
            distance -= m_pathLengthCells;
        }

        for (uint32_t i = 0; i < m_path.size(); ++i)
        {
            const Cell startCell = m_path[i];
            const Cell endCell = m_path[(i + 1) % m_path.size()];
            const float length = static_cast<float>(segmentLength(startCell, endCell));
            if (distance <= length)
            {
                const Vec2 start = MazeLayout::cellCenter(startCell);
                const Vec2 end = MazeLayout::cellCenter(endCell);
                const float t = length > 0.0f ? distance / length : 0.0f;
                return {
                    start.x + (end.x - start.x) * t,
                    start.y + (end.y - start.y) * t,
                };
            }
            distance -= length;
        }

        return MazeLayout::cellCenter(m_path.front());
    }

    static float wrapAngle(float angle)
    {
        while (angle > PI)
        {
            angle -= TWO_PI;
        }
        while (angle < -PI)
        {
            angle += TWO_PI;
        }
        return angle;
    }

    struct CellOffset
    {
        int8_t row;
        int8_t col;
    };

    const MazeLayout& m_mazeLayout;
    std::vector<Cell> m_path;
    float m_pathLengthCells = 0.0f;
    float m_pathDistance = 0.0f;
    std::chrono::steady_clock::time_point m_lastFrameTime {};
    bool m_hasLastFrameTime = false;
    float m_heading = 0.0f;
    bool m_headingInitialized = false;

    float m_lookAheadCells = 0.8f;
    float m_speedCellsPerSecond = 1.5f;
    float m_turnSpeedRadiansPerFrame = 0.085f;
};
} // namespace labyrinth
