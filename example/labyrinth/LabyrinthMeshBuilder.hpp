#pragma once

#include "LabyrinthMaze.hpp"
#include "StaticLighting.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <vector>

namespace labyrinth
{
template <typename T>
using Quad = std::array<T, 4>;

enum class MeshKind
{
    Floor,
    Wall,
    Ceiling,
    LightMarker,
};

struct Face
{
    Quad<Vec3> vertices;
    Quad<Vec3RGB> colors;
    Quad<Vec2> texCoords;
};

using FaceBatch = std::vector<Face>;

struct RenderMesh
{
    std::vector<float> vertices;
    std::vector<float> colors;
    std::vector<float> texCoords;
    std::vector<uint16_t> indices;
};

struct BuiltMesh
{
    MeshKind kind;
    RenderMesh mesh;
};

class MeshBuilder
{
public:
    explicit MeshBuilder(const MazeLayout& mazeLayout)
        : m_mazeLayout(mazeLayout)
        , m_lightPositions(m_mazeLayout.collectLightPositions())
    {
        m_staticLighting.setLightPositions(m_lightPositions);
    }

    std::vector<BuiltMesh> build() const
    {
        FaceBatch floorFaces;
        FaceBatch wallFaces;
        FaceBatch ceilingFaces;
        FaceBatch lightMarkerFaces;

        buildMazeFaces(floorFaces, wallFaces, ceilingFaces);
        buildLightMarkerFaces(lightMarkerFaces);

        return {
            { MeshKind::Floor, buildRenderMesh(floorFaces) },
            { MeshKind::Wall, buildRenderMesh(wallFaces) },
            { MeshKind::Ceiling, buildRenderMesh(ceilingFaces) },
            { MeshKind::LightMarker, buildRenderMesh(lightMarkerFaces) },
        };
    }

private:
    Quad<Vec3RGB> buildVertexColors(const Quad<Vec3>& vertices) const
    {
        Quad<Vec3RGB> colors {};
        for (uint32_t i = 0; i < vertices.size(); ++i)
        {
            colors[i] = m_staticLighting.vertexColor(vertices[i]);
        }
        return colors;
    }

    void addFace(FaceBatch& batch, const Quad<Vec3>& vertices, const Quad<Vec2>& texCoords) const
    {
        batch.push_back({ vertices, buildVertexColors(vertices), texCoords });
    }

    static void addFaceWithColors(
        FaceBatch& batch,
        const Quad<Vec3>& vertices,
        const Quad<Vec3RGB>& colors,
        const Quad<Vec2>& texCoords)
    {
        batch.push_back({ vertices, colors, texCoords });
    }

    void buildMazeFaces(FaceBatch& floorFaces, FaceBatch& wallFaces, FaceBatch& ceilingFaces) const
    {
        static constexpr Quad<Vec2> wallTex = { { { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f } } };
        static constexpr Quad<Vec2> floorTex = { { { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f } } };
        static constexpr Quad<Vec2> ceilingTex = { { { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f } } };

        for (uint32_t row = 0; row < MazeLayout::Height; ++row)
        {
            for (uint32_t col = 0; col < MazeLayout::Width; ++col)
            {
                if (m_mazeLayout.isWall(static_cast<int32_t>(row), static_cast<int32_t>(col)))
                {
                    continue;
                }

                const float x0 = MazeLayout::cellMinX(col);
                const float x1 = MazeLayout::cellMaxX(col);
                const float y0 = MazeLayout::cellMinY(row);
                const float y1 = MazeLayout::cellMaxY(row);

                addFace(floorFaces, { { { x0, y0, 0.0f }, { x1, y0, 0.0f }, { x1, y1, 0.0f }, { x0, y1, 0.0f } } }, floorTex);
                addFace(
                    ceilingFaces,
                    { { { x0, y1, MazeLayout::CeilingHeight },
                        { x1, y1, MazeLayout::CeilingHeight },
                        { x1, y0, MazeLayout::CeilingHeight },
                        { x0, y0, MazeLayout::CeilingHeight } } },
                    ceilingTex);

                if (m_mazeLayout.isWall(static_cast<int32_t>(row) - 1, static_cast<int32_t>(col)))
                {
                    addFace(wallFaces, { { { x0, y1, 0.0f }, { x1, y1, 0.0f }, { x1, y1, MazeLayout::CeilingHeight }, { x0, y1, MazeLayout::CeilingHeight } } }, wallTex);
                }
                if (m_mazeLayout.isWall(static_cast<int32_t>(row) + 1, static_cast<int32_t>(col)))
                {
                    addFace(wallFaces, { { { x1, y0, 0.0f }, { x0, y0, 0.0f }, { x0, y0, MazeLayout::CeilingHeight }, { x1, y0, MazeLayout::CeilingHeight } } }, wallTex);
                }
                if (m_mazeLayout.isWall(static_cast<int32_t>(row), static_cast<int32_t>(col) - 1))
                {
                    addFace(wallFaces, { { { x0, y0, 0.0f }, { x0, y1, 0.0f }, { x0, y1, MazeLayout::CeilingHeight }, { x0, y0, MazeLayout::CeilingHeight } } }, wallTex);
                }
                if (m_mazeLayout.isWall(static_cast<int32_t>(row), static_cast<int32_t>(col) + 1))
                {
                    addFace(wallFaces, { { { x1, y1, 0.0f }, { x1, y0, 0.0f }, { x1, y0, MazeLayout::CeilingHeight }, { x1, y1, MazeLayout::CeilingHeight } } }, wallTex);
                }
            }
        }
    }

    void buildLightMarkerFaces(FaceBatch& lightMarkerFaces) const
    {
        static constexpr Quad<Vec2> markerTex = { { { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f } } };
        static constexpr Quad<Vec3RGB> markerColor = { {
            { 1.0f, 1.0f, 1.0f },
            { 1.0f, 1.0f, 1.0f },
            { 1.0f, 1.0f, 1.0f },
            { 1.0f, 1.0f, 1.0f },
        } };
        static constexpr float markerHalfSize = MazeLayout::CellSize * 0.16f;
        static constexpr float markerZ = MazeLayout::CeilingHeight - 0.02f;

        for (const Vec2 light : m_lightPositions)
        {
            const float x0 = light.x - markerHalfSize;
            const float x1 = light.x + markerHalfSize;
            const float y0 = light.y - markerHalfSize;
            const float y1 = light.y + markerHalfSize;
            addFaceWithColors(
                lightMarkerFaces,
                { { { x0, y1, markerZ }, { x1, y1, markerZ }, { x1, y0, markerZ }, { x0, y0, markerZ } } },
                markerColor,
                markerTex);
        }
    }

    static RenderMesh buildRenderMesh(const FaceBatch& batch)
    {
        static constexpr size_t verticesPerFace = std::tuple_size_v<decltype(std::declval<Face>().vertices)>;
        static constexpr size_t colorsPerFace = std::tuple_size_v<decltype(std::declval<Face>().colors)>;
        static constexpr size_t texCoordsPerFace = std::tuple_size_v<decltype(std::declval<Face>().texCoords)>;
        static constexpr size_t positionComponents = sizeof(Vec3) / sizeof(float);
        static constexpr size_t colorComponents = sizeof(Vec3RGB) / sizeof(float);
        static constexpr size_t texCoordComponents = sizeof(Vec2) / sizeof(float);
        static constexpr size_t indicesPerFace = (verticesPerFace - 2) * 3; // 6 indices for 4 vertices (two triangles)

        RenderMesh mesh;
        mesh.vertices.reserve(batch.size() * verticesPerFace * positionComponents);
        mesh.colors.reserve(batch.size() * colorsPerFace * colorComponents);
        mesh.texCoords.reserve(batch.size() * texCoordsPerFace * texCoordComponents);
        mesh.indices.reserve(batch.size() * indicesPerFace);

        for (const Face& face : batch)
        {
            const uint16_t base = static_cast<uint16_t>(mesh.vertices.size() / positionComponents);
            for (const Vec3& vertex : face.vertices)
            {
                mesh.vertices.push_back(vertex.x);
                mesh.vertices.push_back(vertex.y);
                mesh.vertices.push_back(vertex.z);
            }
            for (const Vec3RGB& color : face.colors)
            {
                mesh.colors.push_back(color.r);
                mesh.colors.push_back(color.g);
                mesh.colors.push_back(color.b);
            }
            for (const Vec2& texCoord : face.texCoords)
            {
                mesh.texCoords.push_back(texCoord.x);
                mesh.texCoords.push_back(texCoord.y);
            }
            mesh.indices.insert(mesh.indices.end(), { base, static_cast<uint16_t>(base + 1), static_cast<uint16_t>(base + 2), base, static_cast<uint16_t>(base + 2), static_cast<uint16_t>(base + 3) });
        }

        return mesh;
    }

    const MazeLayout& m_mazeLayout;
    std::vector<Vec2> m_lightPositions;
    StaticLighting m_staticLighting;
};
} // namespace labyrinth
