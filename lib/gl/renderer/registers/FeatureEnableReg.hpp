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

#ifndef _FEATURE_ENABLE_REG_
#define _FEATURE_ENABLE_REG_

namespace rr
{

class FeatureEnableReg
{
public:
    FeatureEnableReg() = default;
    void setEnableFog(const bool val) { m_regVal.fields.fog = val; }
    void setEnableBlending(const bool val) { m_regVal.fields.blending = val; }
    void setEnableDepthTest(const bool val) { m_regVal.fields.depthTest = val; }
    void setEnableAlphaTest(const bool val) { m_regVal.fields.alphaTest = val; }
    void setEnableTmu(const std::size_t tmu, const bool val)
    {
        if (tmu == 0)
            m_regVal.fields.tmu0 = val;
        else
            m_regVal.fields.tmu1 = val;
    }
    void setEnableScissor(const bool val) { m_regVal.fields.scissor = val; }
    void setEnableStencilTest(const bool val) { m_regVal.fields.stencilTest = val; }
    void setEnableLogicOp(const bool val) { m_regVal.fields.logicOp = val; }

    bool getEnableFog() const { return m_regVal.fields.fog; }
    bool getEnableBlending() const { return m_regVal.fields.blending; }
    bool getEnableDepthTest() const { return m_regVal.fields.depthTest; }
    bool getEnableAlphaTest() const { return m_regVal.fields.alphaTest; }
    bool getEnableTmu(const std::size_t tmu) const { return (tmu == 0) ? m_regVal.fields.tmu0 : m_regVal.fields.tmu1; }
    bool getEnableScissor() const { return m_regVal.fields.scissor; }
    bool getEnableStencilTest() const { return m_regVal.fields.stencilTest; }
    bool getEnableLogicOp() const { return m_regVal.fields.logicOp; }

    uint32_t serialize() const { return m_regVal.data; }
    void deserialize(const uint32_t data) { m_regVal.data = data; }
    static constexpr uint32_t getAddr() { return 0x0; }

private:
    union RegVal
    {
#pragma pack(push, 1)
        struct RegContent
        {
            RegContent()
                : fog { false }
                , blending { false }
                , depthTest { false }
                , alphaTest { false }
                , stencilTest { false }
                , scissor { false }
                , tmu0 { false }
                , tmu1 { false }
                , logicOp { false }
            {
            }

            uint32_t fog : 1;
            uint32_t blending : 1;
            uint32_t depthTest : 1;
            uint32_t alphaTest : 1;
            uint32_t stencilTest : 1;
            uint32_t scissor : 1;
            uint32_t tmu0 : 1;
            uint32_t tmu1 : 1;
            uint32_t logicOp : 1;
        } fields {};
        uint32_t data;
#pragma pack(pop)
    } m_regVal;
};

} // namespace rr

#endif // _FEATURE_ENABLE_REG_
