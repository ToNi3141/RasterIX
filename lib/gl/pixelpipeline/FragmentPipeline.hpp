// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

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

#ifndef FRAGMENT_PIPELINE_HPP_
#define FRAGMENT_PIPELINE_HPP_

#include "math/Vec.hpp"
#include "renderer/Renderer.hpp"
#include <optional>

namespace rr
{
class FragmentPipeline
{
public:
    FragmentPipeline(Renderer& renderer)
        : m_renderer { renderer }
    {
        m_renderer.setFragmentPipelineConfig(m_fragmentPipelineConf);
    }

    void setRefAlphaValue(const uint8_t val) { config().setRefAlphaValue(val); }
    void setDepthMask(const bool val) { config().setDepthMask(val); }
    void setColorMaskA(const bool val) { config().setColorMaskA(val); }
    void setColorMaskR(const bool val) { config().setColorMaskR(val); }
    void setColorMaskG(const bool val) { config().setColorMaskG(val); }
    void setColorMaskB(const bool val) { config().setColorMaskB(val); }
    void setDepthFunc(const TestFunc val) { config().setDepthFunc(val); }
    void setAlphaFunc(const TestFunc val) { config().setAlphaFunc(val); }
    void setBlendFuncSFactor(const BlendFunc val) { config().setBlendFuncSFactor(val); }
    void setBlendFuncDFactor(const BlendFunc val) { config().setBlendFuncDFactor(val); }
    void setLogicOp(const LogicOp val) { config().setLogicOp(val); }

    uint8_t getRefAlphaValue() const { return config().getRefAlphaValue(); }
    bool getDepthMask() const { return config().getDepthMask(); }
    bool getColorMaskA() const { return config().getColorMaskA(); }
    bool getColorMaskR() const { return config().getColorMaskR(); }
    bool getColorMaskG() const { return config().getColorMaskG(); }
    bool getColorMaskB() const { return config().getColorMaskB(); }
    TestFunc getDepthFunc() const { return config().getDepthFunc(); }
    TestFunc getAlphaFunc() const { return config().getAlphaFunc(); }
    BlendFunc getBlendFuncSFactor() const { return config().getBlendFuncSFactor(); }
    BlendFunc getBlendFuncDFactor() const { return config().getBlendFuncDFactor(); }
    LogicOp getLogicOp() const { return config().getLogicOp(); }

    bool update()
    {
        bool ret { true };

        if (m_fragmentPipelineConfUploaded.serialize() != m_fragmentPipelineConf.serialize())
        {
            ret = ret && m_renderer.setFragmentPipelineConfig(m_fragmentPipelineConf);
            m_fragmentPipelineConfUploaded = m_fragmentPipelineConf;
        }

        return ret;
    }

private:
    FragmentPipelineReg& config() { return m_fragmentPipelineConf; }
    const FragmentPipelineReg& config() const { return m_fragmentPipelineConf; }

    Renderer& m_renderer;
    FragmentPipelineReg m_fragmentPipelineConf {};
    FragmentPipelineReg m_fragmentPipelineConfUploaded {};
};

} // namespace rr
#endif // FRAGMENT_PIPELINE_HPP_
