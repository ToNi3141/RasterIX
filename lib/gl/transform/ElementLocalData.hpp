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

#ifndef _ELEMENT_LOCAL_DATA_HPP_
#define _ELEMENT_LOCAL_DATA_HPP_

#include "RenderConfigs.hpp"
#include "transform/MatrixStore.hpp"
#include "transform/PrimitiveAssembler.hpp"
#include <bitset>

namespace rr::transform
{
struct ElementLocalData
{
    matrixstore::TransformMatricesData transformMatrices {};
    primitiveassembler::PrimitiveAssemblerData primitiveAssembler {};
    std::bitset<RenderConfig::TMU_COUNT> tmuEnabled {};
};

} // namespace rr::transform

#endif // _ELEMENT_LOCAL_DATA_HPP_
