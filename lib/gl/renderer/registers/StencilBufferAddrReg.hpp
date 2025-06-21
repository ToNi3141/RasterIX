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

#ifndef _STENCIL_BUFFER_ADDR_REG_
#define _STENCIL_BUFFER_ADDR_REG_

#include "RenderConfigs.hpp"
#include "renderer/registers/BaseSingleReg.hpp"

namespace rr
{
class StencilBufferAddrReg : public BaseSingleReg<0xffffffff>
{
public:
    StencilBufferAddrReg() = default;
    StencilBufferAddrReg(const uint32_t addr)
        : BaseSingleReg<0xffffffff> { addr + RenderConfig::GRAM_MEMORY_LOC }
    {
    }

    static constexpr uint32_t getAddr() { return 18; }
};
} // namespace rr

#endif // _STENCIL_BUFFER_ADDR_REG_
