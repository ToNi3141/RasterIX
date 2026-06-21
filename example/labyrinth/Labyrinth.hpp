#pragma once

#include "CameraPathController.hpp"
#include "LabyrinthMaze.hpp"
#include "LabyrinthMeshBuilder.hpp"
#include "LabyrinthTextures.hpp"
#include "gl.h"
#include "glu.h"
#include <array>
#include <cstdint>
#include <vector>

class Labyrinth
{
public:
    inline static constexpr labyrinth::MazeLayout::GridType Grid = labyrinth::MazeLayout::makeGrid(
        "###########",
        "#l..l..l.l#",
        "#.#########",
        "#l..l..l#.#",
        "#.#####.#.#",
        "#l#.#l.l#.#",
        "#.#.#.#.#.#",
        "#l.l..#..l#",
        "#.#########",
        "#l..l..l.l#",
        "###########");

    void init(const uint32_t resolutionW, const uint32_t resolutionH)
    {
        m_wallTexture = uploadTexture(labyrinth::WallTexture::create());
        m_floorTexture = uploadTexture(labyrinth::FloorTexture::create());
        m_ceilingTexture = uploadTexture(labyrinth::CeilingTexture::create());
        m_lightMarkerTexture = uploadTexture(labyrinth::LightMarkerTexture::create());

        uploadBuiltMeshes(labyrinth::MeshBuilder { m_mazeLayout }.build());

        m_cameraPath.resetAndBuild();

        glViewport(0, 0, resolutionW, resolutionH);
        glDepthRange(0.0, 1.0);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(55.0, static_cast<float>(resolutionW) / static_cast<float>(resolutionH), 0.1, 80.0);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);

        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        static constexpr GLfloat fogColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        glFogf(GL_FOG_START, 2.0f);
        glFogf(GL_FOG_END, 15.0f);
        glFogi(GL_FOG_MODE, GL_LINEAR);
        glFogfv(GL_FOG_COLOR, fogColor);
        glEnable(GL_FOG);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    void draw()
    {
        glDisable(GL_SCISSOR_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const labyrinth::CameraPathController::CameraView cameraView = m_cameraPath.currentView();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(
            cameraView.position.x,
            cameraView.position.y,
            labyrinth::MazeLayout::EyeHeight,
            cameraView.lookTarget.x,
            cameraView.lookTarget.y,
            labyrinth::MazeLayout::EyeHeight,
            0.0f,
            0.0f,
            1.0f);

        drawMesh(m_floorMesh, m_floorTexture);
        drawMesh(m_wallMesh, m_wallTexture);
        drawMesh(m_ceilingMesh, m_ceilingTexture);
        drawMesh(m_lightMarkerMesh, m_lightMarkerTexture);

        m_cameraPath.advanceByFrameTime();
    }

    ~Labyrinth()
    {
        const GLuint textures[] = { m_wallTexture, m_floorTexture, m_ceilingTexture, m_lightMarkerTexture };
        glDeleteTextures(sizeof(textures) / sizeof(textures[0]), textures);
        deleteMesh(m_floorMesh);
        deleteMesh(m_wallMesh);
        deleteMesh(m_ceilingMesh);
        deleteMesh(m_lightMarkerMesh);
    }

private:
    struct MeshVboSet
    {
        GLuint vertices = 0;
        GLuint colors = 0;
        GLuint texCoords = 0;
        GLuint indices = 0;
        GLsizei indexCount = 0;
    };

    static constexpr float LOOK_AHEAD_CELLS = 0.8f;
    static constexpr float CAMERA_SPEED_CELLS_PER_SECOND = 1.5f;
    static constexpr float CAMERA_TURN_SPEED_RADIANS_PER_FRAME = 0.085f;

    GLuint m_wallTexture = 0;
    GLuint m_floorTexture = 0;
    GLuint m_ceilingTexture = 0;
    GLuint m_lightMarkerTexture = 0;

    MeshVboSet m_wallMesh;
    MeshVboSet m_floorMesh;
    MeshVboSet m_ceilingMesh;
    MeshVboSet m_lightMarkerMesh;

    labyrinth::MazeLayout m_mazeLayout { Grid };

    labyrinth::CameraPathController m_cameraPath {
        m_mazeLayout,
        LOOK_AHEAD_CELLS,
        CAMERA_SPEED_CELLS_PER_SECOND,
        CAMERA_TURN_SPEED_RADIANS_PER_FRAME,
    };

    void uploadBuiltMeshes(const std::vector<labyrinth::BuiltMesh>& builtMeshes)
    {
        for (const labyrinth::BuiltMesh& builtMesh : builtMeshes)
        {
            switch (builtMesh.kind)
            {
            case labyrinth::MeshKind::Floor:
                uploadMesh(m_floorMesh, builtMesh.mesh);
                break;
            case labyrinth::MeshKind::Wall:
                uploadMesh(m_wallMesh, builtMesh.mesh);
                break;
            case labyrinth::MeshKind::Ceiling:
                uploadMesh(m_ceilingMesh, builtMesh.mesh);
                break;
            case labyrinth::MeshKind::LightMarker:
                uploadMesh(m_lightMarkerMesh, builtMesh.mesh);
                break;
            }
        }
    }

    static GLuint uploadTexture(const labyrinth::TextureImage& textureImage)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB,
            static_cast<GLint>(textureImage.size),
            static_cast<GLint>(textureImage.size),
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            textureImage.pixels.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        return texture;
    }

    static void uploadMesh(MeshVboSet& meshVboSet, const labyrinth::RenderMesh& renderMesh)
    {
        meshVboSet.indexCount = static_cast<GLsizei>(renderMesh.indices.size());

        glGenBuffers(1, &meshVboSet.vertices);
        glBindBuffer(GL_ARRAY_BUFFER, meshVboSet.vertices);
        glBufferData(GL_ARRAY_BUFFER, renderMesh.vertices.size() * sizeof(float), renderMesh.vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &meshVboSet.colors);
        glBindBuffer(GL_ARRAY_BUFFER, meshVboSet.colors);
        glBufferData(GL_ARRAY_BUFFER, renderMesh.colors.size() * sizeof(float), renderMesh.colors.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &meshVboSet.texCoords);
        glBindBuffer(GL_ARRAY_BUFFER, meshVboSet.texCoords);
        glBufferData(GL_ARRAY_BUFFER, renderMesh.texCoords.size() * sizeof(float), renderMesh.texCoords.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &meshVboSet.indices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshVboSet.indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderMesh.indices.size() * sizeof(uint16_t), renderMesh.indices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    static void deleteMesh(const MeshVboSet& mesh)
    {
        const GLuint buffers[] = { mesh.vertices, mesh.colors, mesh.texCoords, mesh.indices };
        glDeleteBuffers(sizeof(buffers) / sizeof(buffers[0]), buffers);
    }

    static void drawMesh(const MeshVboSet& mesh, const GLuint texture)
    {
        if (mesh.indexCount == 0)
        {
            return;
        }

        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
        glClientActiveTexture(GL_TEXTURE0);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.texCoords);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.colors);
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(3, GL_FLOAT, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertices);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, nullptr);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indices);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_SHORT, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
};
