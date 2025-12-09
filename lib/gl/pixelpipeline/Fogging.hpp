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

#ifndef FOGGING_HPP_
#define FOGGING_HPP_

#include "Enums.hpp"
#include "math/Vec.hpp"
#include "renderer/Renderer.hpp"
#include <optional>

namespace rr
{
class Fogging
{
public:
    Fogging(Renderer& renderer);

    void setFogMode(const FogMode val);
    void setFogStart(const float val);
    void setFogEnd(const float val);
    void setFogDensity(const float val);
    bool setFogColor(const Vec4& val);

    FogMode getFogMode() const { return m_fogMode; }
    float getFogStart() const { return m_fogStart; }
    float getFogEnd() const { return m_fogEnd; }
    float getFogDensity() const { return m_fogDensity; }
    Vec4 getFogColor() const;

    bool updateFogLut();

private:
    Renderer& m_renderer;
    bool m_fogDirty { false };
    FogMode m_fogMode { FogMode::EXP };
    float m_fogStart { 0.0f };
    float m_fogEnd { 1.0f };
    float m_fogDensity { 1.0f };
};

} // namespace rr
#endif // FOGGING_HPP_
