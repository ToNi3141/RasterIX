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

#ifndef _SET_LIGHTING_CTX_CMD_HPP_
#define _SET_LIGHTING_CTX_CMD_HPP_

#include "DataTransferCmdBase.hpp"
#include "Op.hpp"
#include "RenderConfigs.hpp"
#include "transform/Lighting.hpp"

namespace rr
{

using SetLightingCtxCmd = DataTransferCmdBase<lighting::LightingData, op::SET_LIGHTING_CTX>;

} // namespace rr

#endif // _SET_LIGHTING_CTX_CMD_HPP_
