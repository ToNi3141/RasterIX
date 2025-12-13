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

#ifndef STENCIL_HPP_
#define STENCIL_HPP_

#include "Enums.hpp"
#include "math/Vec.hpp"
#include "renderer/Rasterizer.hpp"
#include "renderer/registers/StencilReg.hpp"
#include <functional>

namespace rr::stencil
{
struct StencilData
{
    bool enableTwoSideStencil { false };
    StencilReg stencilConfFront {};
    StencilReg stencilConfBack {};
};

class StencilCalc
{
public:
    StencilCalc(const StencilData& stencilData)
        : m_data { stencilData }
    {
    }

    StencilReg updateStencilFace(const Vec4& v0, const Vec4& v1, const Vec4& v2) const
    {
        const float edgeVal = Rasterizer::edgeFunctionFloat(v0, v1, v2);
        const StencilFace currentOrientation = (edgeVal <= 0.0f) ? StencilFace::BACK : StencilFace::FRONT;
        if (currentOrientation != StencilFace::FRONT) // The rasterizer expects triangles in CW. OpenGL in CCW. Thats the reason why Front and Back does not match.
        {
            return m_data.stencilConfFront;
        }
        else
        {
            return m_data.stencilConfBack;
        }
    }

private:
    const StencilData& m_data;
};

class StencilSetter
{
public:
    StencilSetter(const std::function<bool(const StencilReg&)>& sender, StencilData& stencilData);

    void setTestFunc(const TestFunc val) { stencilConfig().setTestFunc(val); }
    void setOpZPass(const StencilOp val) { stencilConfig().setOpZPass(val); }
    void setOpZFail(const StencilOp val) { stencilConfig().setOpZFail(val); }
    void setOpFail(const StencilOp val) { stencilConfig().setOpFail(val); }
    void setMask(const uint8_t val) { stencilConfig().setMask(val); }
    void setRef(const uint8_t val) { stencilConfig().setRef(val); }
    void setClearStencil(const uint8_t val) { stencilConfig().setClearStencil(val); }
    void setStencilMask(const uint8_t val) { stencilConfig().setStencilMask(val); }

    TestFunc getTestFunc() const { return stencilConfig().getTestFunc(); }
    StencilOp getOpZPass() const { return stencilConfig().getOpZPass(); }
    StencilOp getOpZFail() const { return stencilConfig().getOpZFail(); }
    StencilOp getOpFail() const { return stencilConfig().getOpFail(); }
    uint8_t getRef() const { return stencilConfig().getRef(); }
    uint8_t getMask() const { return stencilConfig().getMask(); }
    uint8_t getClearStencil() const { return stencilConfig().getClearStencil(); }
    uint8_t getStencilMask() const { return stencilConfig().getStencilMask(); }

    void enableTwoSideStencil(const bool enable) { m_data.enableTwoSideStencil = enable; }
    void setStencilFace(const StencilFace face) { m_stencilFace = face; }

    bool getTwoSideStencilEnabled() const { return m_data.enableTwoSideStencil; }
    StencilFace getStencilFace() const { return m_stencilFace; }

    bool update();

private:
    StencilReg& stencilConfig();
    const StencilReg& stencilConfig() const { return this->stencilConfig(); };

    std::function<bool(const StencilReg&)> m_sender {};
    StencilData& m_data;

    bool m_stencilDirty { true };
    StencilFace m_stencilFace { StencilFace::FRONT };
    StencilReg m_stencilConf {};
};

} // namespace rr::stencil
#endif // STENCIL_HPP_
