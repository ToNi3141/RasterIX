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

#ifndef OP_HPP
#define OP_HPP

namespace rr::op
{
// In Hardware supported commands
static constexpr uint32_t NOP { 0x0000'0000 };
static constexpr uint32_t RENDER_CONFIG { 0x1000'0000 };
static constexpr uint32_t FRAMEBUFFER { 0x2000'0000 };
static constexpr uint32_t TRIANGLE_STREAM { 0x3000'0000 };
static constexpr uint32_t FOG_LUT_STREAM { 0x4000'0000 };
static constexpr uint32_t TEXTURE_STREAM { 0x5000'0000 };
// Virtual commands only supported by the ThreadedRasterizer
static constexpr uint32_t SET_ELEMENT_GLOBAL_CTX { 0xB000'0000 };
static constexpr uint32_t SET_LIGHTING_CTX { 0xC000'0000 };
static constexpr uint32_t PUSH_VERTEX { 0xD000'0000 };
static constexpr uint32_t SET_ELEMENT_LOCAL_CTX { 0xE000'0000 };
static constexpr uint32_t DRAW_NEW_ELEMENT { 0xF000'0000 };

static constexpr uint32_t MASK { 0xF000'0000 };
} // namespace rr::op

#endif // OP_HPP