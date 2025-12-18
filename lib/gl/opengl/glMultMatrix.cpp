// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2025 ToNi3141

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

#include "GLImpl.h"
#include "RIXGL.hpp"
#include "vertexpipeline/VertexPipeline.hpp"
#include <spdlog/spdlog.h>

using namespace rr;

GLAPI void APIENTRY impl_glMultMatrixd(const GLdouble* m)
{
    SPDLOG_DEBUG("glMultMatrixd redirected to glMultMatrixf");
    GLfloat mf[16];
    for (std::size_t i = 0; i < 4 * 4; i++)
    {
        mf[i] = m[i];
    }
    impl_glMultMatrixf(mf);
}

GLAPI void APIENTRY impl_glMultMatrixf(const GLfloat* m)
{
    SPDLOG_DEBUG("glMultMatrixf called");
    const Mat44* m44 = reinterpret_cast<const Mat44*>(m);
    RIXGL::getInstance().pipeline().getMatrixStore().getCurrentMatrix().multiplyLhs(*m44);
}