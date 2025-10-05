// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2025 ToNi3141

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef POINT_ASSEMBLY_HPP
#define POINT_ASSEMBLY_HPP

#include "math/Vec.hpp"
#include "transform/TransformingVertexParameter.hpp"
#include <array>
#include <tcb/span.hpp>

namespace rr::pointassembly
{

struct PointAssemblyData
{
    float pointSize { 1.0f };
    float pointSizeMin { 1.0f };
    float pointSizeMax { 64.0f };
    float pointFadeThresholdSize { 1.0f };
    Vec3 pointDistanceAttenuation { 1.0f, 0.0f, 0.0f };
    bool texCoordReplace { false };
    bool enablePointSprite { false };
};

class PointAssemblyCalc
{
public:
    using Triangles = std::array<TransformingVertexParameter, 6>;

    PointAssemblyCalc(const PointAssemblyData& data, const float& viewPortWidth, const float& viewPortHeight)
        : m_data { data }
        , m_viewPortWidth { viewPortWidth }
        , m_viewPortHeight { viewPortHeight }
    {
    }

    // Creates a square for a point in NDC
    Triangles createPoint(const TransformingVertexParameter& vp) const;

private:
    const PointAssemblyData& m_data;
    const float& m_viewPortWidth;
    const float& m_viewPortHeight;
};

class PointAssemblySetter
{
public:
    PointAssemblySetter(PointAssemblyData& data)
        : m_data { data }
    {
    }

    void setPointSize(const float pointSize) { m_data.pointSize = pointSize; }
    void setPointSizeMin(const float pointSizeMin) { m_data.pointSizeMin = pointSizeMin; }
    void setPointSizeMax(const float pointSizeMax) { m_data.pointSizeMax = pointSizeMax; }
    void setPointFadeThresholdSize(const float pointFadeThresholdSize) { m_data.pointFadeThresholdSize = pointFadeThresholdSize; }
    void setPointDistanceAttenuation(const Vec3& attenuation) { m_data.pointDistanceAttenuation = attenuation; }
    void setTexCoordReplace(bool replace) { m_data.texCoordReplace = replace; }
    void setEnablePointSprite(bool enable) { m_data.enablePointSprite = enable; }

    float getPointSize() const { return m_data.pointSize; }
    float getPointSizeMin() const { return m_data.pointSizeMin; }
    float getPointSizeMax() const { return m_data.pointSizeMax; }
    float getPointFadeThresholdSize() const { return m_data.pointFadeThresholdSize; }
    const Vec3& getPointDistanceAttenuation() const { return m_data.pointDistanceAttenuation; }
    bool getTexCoordReplace() const { return m_data.texCoordReplace; }
    bool getEnablePointSprite() const { return m_data.enablePointSprite; }

private:
    PointAssemblyData& m_data;
};

} // namespace rr::pointassembly

#endif // POINT_ASSEMBLY_HPP
