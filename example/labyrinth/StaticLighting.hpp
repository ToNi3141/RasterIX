#pragma once

#include "LabyrinthMaze.hpp"
#include <array>
#include <cmath>
#include <vector>

namespace labyrinth
{
class StaticLighting
{
public:
    void setLightPositions(const std::vector<Vec2>& lightPositions)
    {
        m_lightPositions = lightPositions;
    }

    Vec3RGB vertexColor(const Vec3& vertex) const
    {
        const Vec2 position { vertex.x, vertex.y };
        const float intensity = intensityAt(position);
        return { intensity, intensity, intensity };
    }

private:
    float intensityAt(const Vec2 position) const
    {
        static constexpr float ambient = 0.38f;
        static constexpr float lightRadius = MazeLayout::CellSize * 1.15f;
        static constexpr float lightRadiusSquared = lightRadius * lightRadius;
        float intensity = ambient;

        for (const Vec2 light : m_lightPositions)
        {
            const float dx = position.x - light.x;
            const float dy = position.y - light.y;
            const float distanceSquared = dx * dx + dy * dy;
            if (distanceSquared > lightRadiusSquared)
            {
                continue;
            }

            const float falloff = 1.0f - (distanceSquared / lightRadiusSquared);
            intensity += falloff * 0.85f;
        }

        return intensity > 1.0f ? 1.0f : intensity;
    }

    std::vector<Vec2> m_lightPositions;
};
} // namespace labyrinth
