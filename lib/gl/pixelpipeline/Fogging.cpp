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

#include "Fogging.hpp"

namespace rr
{

Fogging::Fogging(Renderer& renderer)
    : m_renderer(renderer)
{
}

void Fogging::setFogMode(const FogMode val)
{
    if (m_fogMode != val)
    {
        m_fogMode = val;
        m_fogDirty = true;
    }
}

void Fogging::setFogStart(const float val)
{
    if (m_fogStart != val)
    {
        m_fogStart = val;
        m_fogDirty = true;
    }
}

void Fogging::setFogEnd(const float val)
{
    if (m_fogEnd != val)
    {
        m_fogEnd = val;
        m_fogDirty = true;
    }
}

void Fogging::setFogDensity(const float val)
{
    if (m_fogDensity != val)
    {
        m_fogDensity = val;
        m_fogDirty = true;
    }
}

bool Fogging::setFogColor(const Vec4& val)
{
    Vec4i color = Vec4i::createFromVec<Vec4, 8>(val);
    color.clamp(0, 255);
    return m_renderer.setFogColor({ color });
}

Vec4 Fogging::getFogColor() const
{
    FogColorReg fogColorReg = m_renderer.getFogColor();
    return Vec4 {
        static_cast<float>(fogColorReg.getColor()[0]) / 255.0f,
        static_cast<float>(fogColorReg.getColor()[1]) / 255.0f,
        static_cast<float>(fogColorReg.getColor()[2]) / 255.0f,
        static_cast<float>(fogColorReg.getColor()[3]) / 255.0f,
    };
}

bool Fogging::updateFogLut()
{
    if (!m_fogDirty)
    {
        return true;
    }
    m_fogDirty = false;

    std::function<float(float)> fogFunction;

    // Set fog function
    switch (m_fogMode)
    {
    case FogMode::LINEAR:
        fogFunction = [&](float z)
        {
            float f = (m_fogEnd - z) / (m_fogEnd - m_fogStart);
            return f;
        };
        break;
    case FogMode::EXP:
        fogFunction = [&](float z)
        {
            float f = expf(-(m_fogDensity * z));
            return f;
        };
        break;
    case FogMode::EXP2:
        fogFunction = [&](float z)
        {
            float f = expf(powf(-(m_fogDensity * z), 2));
            return f;
        };
        break;
    default:
        fogFunction = [](float)
        {
            return 1.0f;
        };
        break;
    }

    // Calculate fog LUT
    std::array<float, 33> lut;
    for (std::size_t i = 0; i < lut.size(); i++)
    {
        float f = fogFunction(powf(2, i));
        lut[i] = f;
    }

    // Set fog LUT
    return m_renderer.setFogLut(lut, m_fogStart, m_fogEnd);
}

} // namespace rr
