// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2023 ToNi3141

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

#ifndef _RENDER_RESOLUTION_REG_
#define _RENDER_RESOLUTION_REG_

#include "renderer/registers/BaseXYReg.hpp"

namespace rr
{
class RenderResolutionReg : public BaseXYReg<0x7ff, 0x7ff>
{
public:
    RenderResolutionReg(const uint16_t w, const uint16_t h)
        : BaseXYReg<0x7ff, 0x7ff> { w, h }
    {
    }
    RenderResolutionReg()
        : BaseXYReg<0x7ff, 0x7ff> { 320, 240 }
    {
    }
    static constexpr uint32_t getAddr() { return 0x9; }
};
} // namespace rr

#endif // _RENDER_RESOLUTION_REG_
