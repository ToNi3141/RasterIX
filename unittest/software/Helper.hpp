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

#ifndef RR_UT_HELPER_HPP
#define RR_UT_HELPER_HPP

#include "math/Vec.hpp"
#include <cmath>

namespace rr::ut
{

// Helper to compare Vec4 with tolerance
inline bool vec4Approx(const rr::Vec4& a, const rr::Vec4& b, float epsilon = 0.001f)
{
    return std::abs(a[0] - b[0]) < epsilon && std::abs(a[1] - b[1]) < epsilon && std::abs(a[2] - b[2]) < epsilon && std::abs(a[3] - b[3]) < epsilon;
}

} // namespace rr::ut

#endif // RR_UT_HELPER_HPP
