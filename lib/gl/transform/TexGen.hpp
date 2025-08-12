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

#ifndef TEXGEN_HPP
#define TEXGEN_HPP

#include "Enums.hpp"
#include "math/Mat44.hpp"
#include "math/Vec.hpp"

namespace rr::texgen
{

struct TexGenData
{
    Vec4 texGenVecObjS { { 1.0f, 0.0f, 0.0f, 0.0f } };
    Vec4 texGenVecObjT { { 0.0f, 1.0f, 0.0f, 0.0f } };
    Vec4 texGenVecObjR { { 0.0f, 0.0f, 1.0f, 0.0f } };
    Vec4 texGenVecEyeS { { 1.0f, 0.0f, 0.0f, 0.0f } };
    Vec4 texGenVecEyeT { { 0.0f, 1.0f, 0.0f, 0.0f } };
    Vec4 texGenVecEyeR { { 0.0f, 0.0f, 1.0f, 0.0f } };
    TexGenMode texGenModeS { TexGenMode::EYE_LINEAR };
    TexGenMode texGenModeT { TexGenMode::EYE_LINEAR };
    TexGenMode texGenModeR { TexGenMode::EYE_LINEAR };
    bool texGenEnableS { false };
    bool texGenEnableT { false };
    bool texGenEnableR { false };
};

class TexGenCalc
{
public:
    TexGenCalc(const TexGenData& texGenData)
        : m_data { texGenData }
    {
    }

    void calculateTexGenCoords(
        Vec4& st0,
        const Mat44& modelViewMatrix,
        const Mat44& normalMatrix,
        const Vec4& v0,
        const Vec3& n0) const;

private:
    void calculateObjectLinear(Vec4& st0, const Vec4& v0) const;
    void calculateEyeLinear(Vec4& st0, const Vec4& eyeVertex) const;
    void calculateSphereMap(Vec4& st0, const Vec4& eyeVertex, const Vec3& eyeNormal) const;
    void calculateReflectionMap(Vec4& st0, const Vec4& eyeVertex, const Vec3& eyeNormal) const;
    Vec3 calculateSphereVector(Vec4 eyeVertex, const Vec3& eyeNormal) const;
    Vec3 calculateReflectionVector(Vec4 eyeVertex, const Vec3& eyeNormal) const;

    const TexGenData& m_data;
};

class TexGenSetter
{
public:
    void enableTexGenS(bool enable);
    void enableTexGenT(bool enable);
    void enableTexGenR(bool enable);
    void setTexGenModeS(TexGenMode mode);
    void setTexGenModeT(TexGenMode mode);
    void setTexGenModeR(TexGenMode mode);
    void setTexGenVecObjS(const Vec4& val);
    void setTexGenVecObjT(const Vec4& val);
    void setTexGenVecObjR(const Vec4& val);
    void setTexGenVecEyeS(const Vec4& val);
    void setTexGenVecEyeT(const Vec4& val);
    void setTexGenVecEyeR(const Vec4& val);

    void setTexGenData(TexGenData& texGenCalc, const Mat44& modelViewMatrix);

private:
    Mat44 getInvertedModelViewMatrix() const;

    TexGenData* m_data { nullptr };
    const Mat44* m_modelViewMatrix { nullptr };
};

} // namespace rr::texgen
#endif // TEXGEN_HPP
