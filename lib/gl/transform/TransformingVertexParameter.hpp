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

#ifndef TRANSFORMINGVERTEXPARAMETER_HPP_
#define TRANSFORMINGVERTEXPARAMETER_HPP_

#include "RenderConfigs.hpp"
#include "VertexParameter.hpp"
#include "math/Vec.hpp"
#include <array>

namespace rr
{

struct TransformingVertexParameter
{
    Vec4 vertex;
    Vec4 colorFront;
    Vec4 colorBack;
    std::array<Vec4, RenderConfig::TMU_COUNT> tex;
    float pointSize;
};

} // namespace rr
#endif // TRANSFORMINGVERTEXPARAMETER_HPP_
