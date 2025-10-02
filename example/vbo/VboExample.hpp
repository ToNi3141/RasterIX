#pragma once
#include "gl.h"
#include "glu.h"
#include <array>
#include <cstdint>
#include <vector>

class VboExample
{
public:
    void init(const uint32_t resolutionW, const uint32_t resolutionH)
    {
        glViewport(0, 0, resolutionW, resolutionH);
        glDepthRange(0.0, 1.0);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(30.0, static_cast<float>(resolutionW) / static_cast<float>(resolutionH), 1.0, 111.0);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        setupVBOs();
    }

    void draw()
    {
        glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 235.0f / 255.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(12.0f, -2.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        rotation += 1.0f;
        glRotatef(rotation, 0.0f, 0.0f, 1.0f);
        glScalef(1.5f, 1.5f, 1.5f);
        glBindBuffer(GL_ARRAY_BUFFER, vboVerts);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, vboColors);
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(3, GL_FLOAT, 0, nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
        glDrawElements(GL_TRIANGLES, cubeIndex.size(), GL_UNSIGNED_SHORT, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    ~VboExample()
    {
        GLuint buffers[4] = { vboVerts, vboNormals, vboColors, vboIndices };
        glDeleteBuffers(4, buffers);
    }

    void setupVBOs()
    {
        glGenBuffers(1, &vboVerts);
        glBindBuffer(GL_ARRAY_BUFFER, vboVerts);
        glBufferData(GL_ARRAY_BUFFER, cubeVerts.size() * sizeof(float), cubeVerts.data(), GL_STATIC_DRAW);
        glGenBuffers(1, &vboNormals);
        glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
        glBufferData(GL_ARRAY_BUFFER, cubeNormals.size() * sizeof(float), cubeNormals.data(), GL_STATIC_DRAW);
        glGenBuffers(1, &vboColors);
        glBindBuffer(GL_ARRAY_BUFFER, vboColors);
        glBufferData(GL_ARRAY_BUFFER, cubeColors.size() * sizeof(float), cubeColors.data(), GL_STATIC_DRAW);
        glGenBuffers(1, &vboIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeIndex.size() * sizeof(uint16_t), cubeIndex.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

private:
    GLuint vboVerts = 0;
    GLuint vboNormals = 0;
    GLuint vboColors = 0;
    GLuint vboIndices = 0;
    GLint tmuCount = 0;

    float rotation = 0.0f;

    const std::array<uint16_t, 36> cubeIndex = { {
        // clang-format off
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23,
        // clang-format on
    } };

    const std::array<float, 72> cubeVerts = { {
        // clang-format off
         -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        // clang-format on
    } };

    const std::array<float, 72> cubeNormals = { {
        // clang-format off
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        // clang-format on
    } };

    const std::array<float, 72> cubeColors = { {
        // clang-format off
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f,
        1.0f, 0.5f, 0.0f,
        0.5f, 0.0f, 0.5f,
        0.0f, 0.5f, 0.5f,
        0.5f, 1.0f, 0.5f,
        0.5f, 0.5f, 1.0f,
        1.0f, 0.5f, 1.0f,
        0.5f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.5f,
        0.5f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.5f,
        0.0f, 1.0f, 0.5f,
        0.5f, 0.0f, 1.0f,
        0.0f, 0.5f, 1.0f,
        1.0f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.0f,
        0.0f, 0.0f, 0.5f,
        0.5f, 0.0f, 0.0f,
        // clang-format on
    } };
};