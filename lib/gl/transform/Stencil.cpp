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

#include "Stencil.hpp"
#include "renderer/Rasterizer.hpp"
#include <spdlog/spdlog.h>

namespace rr::stencil
{
StencilSetter::StencilSetter(const std::function<bool(const StencilReg&)>& sender, StencilData& stencilData)
    : m_sender { sender }
    , m_data { stencilData }
{
    m_sender(m_stencilConf);
}

StencilReg& StencilSetter::stencilConfig()
{
    if (m_data.enableTwoSideStencil)
    {
        if (m_stencilFace == StencilFace::FRONT)
        {
            return m_data.stencilConfFront;
        }
        return m_data.stencilConfBack;
    }
    m_stencilDirty = true;
    return m_stencilConf;
}

const StencilReg& StencilSetter::stencilConfig() const
{
    if (m_data.enableTwoSideStencil)
    {
        if (m_stencilFace == StencilFace::FRONT)
        {
            return m_data.stencilConfFront;
        }
        return m_data.stencilConfBack;
    }
    return m_stencilConf;
}

bool StencilSetter::update()
{
    bool ret { true };

    if (!m_data.enableTwoSideStencil && m_stencilDirty)
    {
        ret = m_sender(m_stencilConf);
        m_stencilDirty = !ret;
    }

    return ret;
}

} // namespace rr::stencil
