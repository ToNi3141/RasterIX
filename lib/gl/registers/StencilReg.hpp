// Rasterix
// https://github.com/ToNi3141/Rasterix
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


#ifndef _STENCIL_REG_
#define _STENCIL_REG_

#include <cstdint>
#include "TestFuncDef.hpp"

namespace rr
{

class StencilReg
{
public:
    static constexpr uint8_t MAX_STENCIL_VAL { 0xf };
    enum class StencilOp
    {
        KEEP,
        ZERO,
        REPLACE,
        INCR,
        INCR_WRAP,
        DECR,
        DECR_WRAP,
        INVERT
    };

    StencilReg() = default;

    void setTestFunc(const TestFunc val) { m_regVal.fields.testFunc = static_cast<uint32_t>(val); }
    void setMask(const uint8_t val) { m_regVal.fields.mask = std::min(val, MAX_STENCIL_VAL); }
    void setRef(const uint8_t val) { m_regVal.fields.ref = std::min(val, MAX_STENCIL_VAL); }
    void setOpZPass(const StencilOp val) { m_regVal.fields.opZPass = static_cast<uint32_t>(val); }
    void setOpZFail(const StencilOp val) { m_regVal.fields.opZFail = static_cast<uint32_t>(val); }
    void setOpFail(const StencilOp val) { m_regVal.fields.opFail = static_cast<uint32_t>(val); }
    void setClearStencil(const uint8_t val) { m_regVal.fields.clearStencil = std::min(val, MAX_STENCIL_VAL); }
    void setStencilMask(const uint8_t val) { m_regVal.fields.stencilMask = std::min(val, MAX_STENCIL_VAL); }

    TestFunc getTestFunc() const { return static_cast<TestFunc>(m_regVal.fields.testFunc); }
    uint8_t getMask() const { return m_regVal.fields.mask; }
    uint8_t getRef() const { return m_regVal.fields.ref; }
    StencilOp getOpZPass() const { return static_cast<StencilOp>(m_regVal.fields.opZPass); }
    StencilOp getOpZFail() const { return static_cast<StencilOp>(m_regVal.fields.opZFail); }
    StencilOp getOpFail() const { return static_cast<StencilOp>(m_regVal.fields.opFail); }
    uint8_t getClearStencil() const { return m_regVal.fields.clearStencil; }
    uint8_t getStencilMask() const { return m_regVal.fields.stencilMask; }

    uint32_t serialize() const { return m_regVal.data; }
    uint32_t getAddr() const { return 0x4; }
private:
    union RegVal
    {
        #pragma pack(push, 1)
        struct RegContent
        {
            RegContent() :
                testFunc(static_cast<uint32_t>(TestFunc::ALWAYS)),
                mask(~0),
                ref(0),
                opZPass(static_cast<uint32_t>(StencilOp::KEEP)),
                opZFail(static_cast<uint32_t>(StencilOp::KEEP)),
                opFail(static_cast<uint32_t>(StencilOp::KEEP)),
                clearStencil(0),
                stencilMask(~0)
            { }

            uint32_t testFunc : 3;
            uint32_t mask : 4;
            uint32_t ref : 4;
            uint32_t opZPass : 3;
            uint32_t opZFail : 3;
            uint32_t opFail : 3;
            uint32_t clearStencil : 4;
            uint32_t stencilMask : 4;

        } fields {};
        uint32_t data;
        #pragma pack(pop)
    } m_regVal;
};

} // namespace rr

#endif // _STENCIL_REG_
