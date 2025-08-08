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

#ifndef FEATURE_ENABLE_HPP_
#define FEATURE_ENABLE_HPP_

#include "Texture.hpp"
#include "renderer/Renderer.hpp"

namespace rr
{
class FeatureEnable
{
public:
    FeatureEnable(Renderer& renderer, Texture& texture)
        : m_renderer { renderer }
        , m_texture { texture }
    {
        m_renderer.setFeatureEnableConfig(m_featureEnableUploaded);
    }

    void setEnableTmu(const bool enable) { m_featureEnable.setEnableTmu(m_texture.getActiveTmu(), enable); }
    void setEnableAlphaTest(const bool enable) { m_featureEnable.setEnableAlphaTest(enable); }
    void setEnableDepthTest(const bool enable) { m_featureEnable.setEnableDepthTest(enable); }
    void setEnableBlending(const bool enable) { m_featureEnable.setEnableBlending(enable); }
    void setEnableFog(const bool enable) { m_featureEnable.setEnableFog(enable); }
    void setEnableScissor(const bool enable) { m_featureEnable.setEnableScissor(enable); }
    void setEnableStencil(const bool enable) { m_featureEnable.setEnableStencilTest(enable); }
    void setEnableLogicOp(const bool enable) { m_featureEnable.setEnableLogicOp(enable); }
    bool getEnableTmu() const { return m_featureEnable.getEnableTmu(m_texture.getActiveTmu()); }
    bool getEnableTmu(const std::size_t tmu) const { return m_featureEnable.getEnableTmu(tmu); }
    bool getEnableAlphaTest() const { return m_featureEnable.getEnableAlphaTest(); }
    bool getEnableDepthTest() const { return m_featureEnable.getEnableDepthTest(); }
    bool getEnableBlending() const { return m_featureEnable.getEnableBlending(); }
    bool getEnableFog() const { return m_featureEnable.getEnableFog(); }
    bool getEnableScissor() const { return m_featureEnable.getEnableScissor(); }
    bool getEnableStencil() const { return m_featureEnable.getEnableStencilTest(); }
    bool getEnableLogicOp() const { return m_featureEnable.getEnableLogicOp(); }

    bool update()
    {
        bool ret { true };

        if (m_featureEnableUploaded.serialize() != m_featureEnable.serialize())
        {
            ret = ret && m_renderer.setFeatureEnableConfig(m_featureEnable);
            m_featureEnableUploaded = m_featureEnable;
        }

        return ret;
    }

private:
    Renderer& m_renderer;
    Texture& m_texture;
    FeatureEnableReg m_featureEnable {};
    FeatureEnableReg m_featureEnableUploaded {};
};

} // namespace rr
#endif // FEATURE_ENABLE_HPP_
