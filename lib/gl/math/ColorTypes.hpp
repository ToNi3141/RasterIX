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

#ifndef _COLOR_TYPES_HPP_
#define _COLOR_TYPES_HPP_

#include "math/Veci.hpp"

namespace rr
{

using Vec4iColorRGBA = Veci<int_fast16_t, 4, 8>;
using Vec3iColorRGB = Veci<int_fast16_t, 3, 8>;
using Vec1iColorR = Veci<int_fast16_t, 1, 8>;

} // namespace rr

#endif // _COLOR_TYPES_HPP_
