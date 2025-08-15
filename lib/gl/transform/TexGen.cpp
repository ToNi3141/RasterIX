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

#include "TexGen.hpp"

namespace rr::texgen
{

void TexGenCalc::calculateTexGenCoords(
    Vec4& st0,
    const Mat44& modelViewMatrix,
    const Mat44& normalMatrix,
    const Vec4& v0,
    const Vec3& n0) const
{
    if (m_data.texGenEnableS || m_data.texGenEnableT || m_data.texGenEnableR)
    {
        if ((m_data.texGenModeS == TexGenMode::OBJECT_LINEAR)
            || (m_data.texGenModeT == TexGenMode::OBJECT_LINEAR)
            || (m_data.texGenModeR == TexGenMode::OBJECT_LINEAR))
        {
            calculateObjectLinear(st0, v0);
        }
        if ((m_data.texGenModeS == TexGenMode::EYE_LINEAR)
            || (m_data.texGenModeT == TexGenMode::EYE_LINEAR)
            || (m_data.texGenModeR == TexGenMode::EYE_LINEAR))
        {
            calculateEyeLinear(st0, modelViewMatrix.transform(v0));
        }
        if ((m_data.texGenModeS == TexGenMode::SPHERE_MAP)
            || (m_data.texGenModeT == TexGenMode::SPHERE_MAP)
            || (m_data.texGenModeR == TexGenMode::SPHERE_MAP))
        {
            calculateSphereMap(st0, modelViewMatrix.transform(v0), normalMatrix.transform(n0));
        }
        if ((m_data.texGenModeS == TexGenMode::REFLECTION_MAP)
            || (m_data.texGenModeT == TexGenMode::REFLECTION_MAP)
            || (m_data.texGenModeR == TexGenMode::REFLECTION_MAP))
        {
            calculateReflectionMap(st0, modelViewMatrix.transform(v0), normalMatrix.transform(n0));
        }
    }
}

void TexGenCalc::calculateObjectLinear(Vec4& st0, const Vec4& v0) const
{
    if (m_data.texGenEnableS && (m_data.texGenModeS == TexGenMode::OBJECT_LINEAR))
    {
        st0[0] = m_data.texGenVecObjS.dot(v0);
    }
    if (m_data.texGenEnableT && (m_data.texGenModeT == TexGenMode::OBJECT_LINEAR))
    {
        st0[1] = m_data.texGenVecObjT.dot(v0);
    }
    if (m_data.texGenEnableR && (m_data.texGenModeR == TexGenMode::OBJECT_LINEAR))
    {
        st0[2] = m_data.texGenVecObjR.dot(v0);
    }
}

void TexGenCalc::calculateEyeLinear(Vec4& st0, const Vec4& eyeVertex) const
{
    if (m_data.texGenEnableS && (m_data.texGenModeS == TexGenMode::EYE_LINEAR))
    {
        st0[0] = m_data.texGenVecEyeS.dot(eyeVertex);
    }
    if (m_data.texGenEnableT && (m_data.texGenModeT == TexGenMode::EYE_LINEAR))
    {
        st0[1] = m_data.texGenVecEyeT.dot(eyeVertex);
    }
    if (m_data.texGenEnableR && (m_data.texGenModeR == TexGenMode::EYE_LINEAR))
    {
        st0[2] = m_data.texGenVecEyeR.dot(eyeVertex);
    }
}

void TexGenCalc::calculateSphereMap(Vec4& st0, const Vec4& eyeVertex, const Vec3& eyeNormal) const
{
    const Vec3 sphereVector = calculateSphereVector(eyeVertex, eyeNormal);
    if (m_data.texGenEnableS && (m_data.texGenModeS == TexGenMode::SPHERE_MAP))
    {
        st0[0] = sphereVector[0];
    }
    if (m_data.texGenEnableT && (m_data.texGenModeT == TexGenMode::SPHERE_MAP))
    {
        st0[1] = sphereVector[1];
    }
    if (m_data.texGenEnableR && (m_data.texGenModeR == TexGenMode::SPHERE_MAP))
    {
        st0[2] = sphereVector[2];
    }
}

void TexGenCalc::calculateReflectionMap(Vec4& st0, const Vec4& eyeVertex, const Vec3& eyeNormal) const
{
    const Vec3 reflectionVector = calculateReflectionVector(eyeVertex, eyeNormal);
    if (m_data.texGenEnableS && (m_data.texGenModeS == TexGenMode::REFLECTION_MAP))
    {
        st0[0] = reflectionVector[0];
    }
    if (m_data.texGenEnableT && (m_data.texGenModeT == TexGenMode::REFLECTION_MAP))
    {
        st0[1] = reflectionVector[1];
    }
    if (m_data.texGenEnableR && (m_data.texGenModeR == TexGenMode::REFLECTION_MAP))
    {
        st0[2] = reflectionVector[2];
    }
}

Vec3 TexGenCalc::calculateSphereVector(Vec4 eyeVertex, const Vec3& eyeNormal) const
{
    eyeVertex.normalize();
    const Vec3 eyeVertex3 { eyeVertex.data() };
    Vec3 reflectionVector = eyeVertex3 - eyeNormal * 2.0f * eyeNormal.dot(eyeVertex3);
    reflectionVector[2] += 1.0f;
    const float m = 1.0f / (2.0f * sqrt(reflectionVector.dot(reflectionVector)));
    return (reflectionVector * m) + Vec3 { 0.5f, 0.5f, 0.5f };
}

Vec3 TexGenCalc::calculateReflectionVector(Vec4 eyeVertex, const Vec3& eyeNormal) const
{
    eyeVertex.normalize();
    const Vec3 eyeVertex3 { eyeVertex.data() };
    const float dotResult = 2.0f * eyeVertex3.dot(eyeNormal);
    return eyeVertex3 - (eyeNormal * dotResult);
}

void TexGenSetter::setTexGenData(TexGenData& texGenCalc, const Mat44& modelViewMatrix)
{
    m_data = &texGenCalc;
    m_modelViewMatrix = &modelViewMatrix;
}

void TexGenSetter::enableTexGenS(bool enable)
{
    m_data->texGenEnableS = enable;
}

void TexGenSetter::enableTexGenT(bool enable)
{
    m_data->texGenEnableT = enable;
}

void TexGenSetter::enableTexGenR(bool enable)
{
    m_data->texGenEnableR = enable;
}

void TexGenSetter::setTexGenModeS(TexGenMode mode)
{
    m_data->texGenModeS = mode;
}

void TexGenSetter::setTexGenModeT(TexGenMode mode)
{
    m_data->texGenModeT = mode;
}

void TexGenSetter::setTexGenModeR(TexGenMode mode)
{
    m_data->texGenModeR = mode;
}

void TexGenSetter::setTexGenVecObjS(const Vec4& val)
{
    m_data->texGenVecObjS = val;
}

void TexGenSetter::setTexGenVecObjT(const Vec4& val)
{
    m_data->texGenVecObjT = val;
}

void TexGenSetter::setTexGenVecObjR(const Vec4& val)
{
    m_data->texGenVecObjR = val;
}

void TexGenSetter::setTexGenVecEyeS(const Vec4& val)
{
    m_data->texGenVecEyeS = getInvertedModelViewMatrix().transform(val);
}

void TexGenSetter::setTexGenVecEyeT(const Vec4& val)
{
    m_data->texGenVecEyeT = getInvertedModelViewMatrix().transform(val);
}

void TexGenSetter::setTexGenVecEyeR(const Vec4& val)
{
    m_data->texGenVecEyeR = getInvertedModelViewMatrix().transform(val);
}

Mat44 TexGenSetter::getInvertedModelViewMatrix() const
{
    Mat44 inverseModelViewMatrix = *m_modelViewMatrix;
    inverseModelViewMatrix.invert();
    return inverseModelViewMatrix;
}

} // namespace rr::texgen