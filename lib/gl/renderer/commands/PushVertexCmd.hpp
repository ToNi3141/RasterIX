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

#ifndef _PUSH_VERTEX_CMD_HPP_
#define _PUSH_VERTEX_CMD_HPP_

#include "DataTransferCmdBase.hpp"
#include "Op.hpp"
#include "RenderConfigs.hpp"
#include "VertexParameter.hpp"

namespace rr
{

using PushVertexCmd = DataTransferCmdBase<VertexParameter, op::PUSH_VERTEX>;

} // namespace rr

#endif // _PUSH_VERTEX_CMD_HPP_
