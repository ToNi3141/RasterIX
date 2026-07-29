// RasterIX
// https://github.com/ToNi3141/RasterIX
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

#include "glu.h"
#include "math/Mat44.hpp"
#include "math/Vec.hpp"
#include <cmath>

#define __glPi 3.14159265358979323846f

GLAPI void APIENTRY gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
{
    rr::Mat44 m;
    float sine, cotangent, deltaZ;
    float radians = fovy / 2.0f * __glPi / 180.0f;

    deltaZ = zFar - zNear;
    sine = sinf(radians);
    if ((deltaZ == 0.0f) || (sine == 0.0f) || (aspect == 0.0f))
    {
        return;
    }
    cotangent = cosf(radians) / sine;

    m.identity();
    m[0][0] = cotangent / aspect;
    m[1][1] = cotangent;
    m[2][2] = -(zFar + zNear) / deltaZ;
    m[2][3] = -1.0f;
    m[3][2] = -2.0f * zNear * zFar / deltaZ;
    m[3][3] = 0.0f;
    glMultMatrixf(&m[0][0]);
}

GLAPI void APIENTRY gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx,
    GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy,
    GLfloat upz)
{
    rr::Vec3 forward { { centerx - eyex,
        centery - eyey,
        centerz - eyez } };
    rr::Vec3 up { { upx, upy, upz } };

    rr::Mat44 m;

    forward = forward.normalize();
    rr::Vec3 side { forward };

    /* Side = forward x up */
    side = side.cross(up);
    side = side.normalize();

    /* Recompute up as: up = side x forward */
    up = side;
    up = up.cross(forward);

    m.identity();
    m[0][0] = side[0];
    m[1][0] = side[1];
    m[2][0] = side[2];

    m[0][1] = up[0];
    m[1][1] = up[1];
    m[2][1] = up[2];

    m[0][2] = -forward[0];
    m[1][2] = -forward[1];
    m[2][2] = -forward[2];

    glMultMatrixf(&m[0][0]);
    glTranslatef(-eyex, -eyey, -eyez);
}
