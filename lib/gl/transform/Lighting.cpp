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

#include "Lighting.hpp"

namespace rr::lighting
{
LightingSetter::LightingSetter(LightingData& lightingData)
    : m_data { lightingData }
{
    setEmissiveColorMaterial({ { 0.0f, 0.0f, 0.0f, 1.0f } });
    setAmbientColorMaterial({ { 0.2f, 0.2f, 0.2f, 1.0 } });
    setAmbientColorScene({ { 0.2f, 0.2f, 0.2f, 1.0f } });
    setDiffuseColorMaterial({ { 0.8f, 0.8f, 0.8f, 1.0 } });
    setSpecularColorMaterial({ { 0.0f, 0.0f, 0.0f, 1.0 } });
    setSpecularExponentMaterial(0.0f);

    for (std::size_t i = 0; i < m_data.lights.size(); i++)
    {
        enableLight(i, false);
        setAmbientColorLight(i, { { 0.0f, 0.0f, 0.0f, 1.0f } });
        setDiffuseColorLight(i, { { 0.0f, 0.0f, 0.0f, 1.0f } });
        setSpecularColorLight(i, { { 0.0f, 0.0f, 0.0f, 1.0f } });
        setPosLight(i, { { 0.0f, 0.0f, 1.0f, 0.0f } });
        setConstantAttenuationLight(i, 1.0f);
        setLinearAttenuationLight(i, 0.0f);
        setQuadraticAttenuationLight(i, 0.0f);
    }
    setDiffuseColorLight(0, { { 1.0f, 1.0f, 1.0f, 1.0f } }); // Light Zero has a slightly different configuration here
    setSpecularColorLight(0, { { 1.0f, 1.0f, 1.0f, 1.0f } }); // Light Zero has a slightly different configuration here
}

void LightingCalc::calculateLights(
    Vec4& __restrict color,
    const Vec4& triangleColor,
    const Vec4& vertex,
    const Vec3& normal) const
{
    const float alphaOld = triangleColor[3];
    if (m_data.lightingEnabled)
    {
        const Vec4& emissiveColor = (m_data.enableColorMaterialEmission) ? triangleColor : m_data.material.emissiveColor;
        const Vec4& ambientColor = (m_data.enableColorMaterialAmbient) ? triangleColor : m_data.material.ambientColor;
        const Vec4& diffuseColor = (m_data.enableColorMaterialDiffuse) ? triangleColor : m_data.material.diffuseColor;
        const Vec4& specularColor = (m_data.enableColorMaterialSpecular) ? triangleColor : m_data.material.specularColor;

        Vec4 colorTmp;
        calculateSceneLight(colorTmp, emissiveColor, ambientColor, m_data.material.ambientColorScene);

        const Vec4 n { normal[0], normal[1], normal[2], 0 };
        for (std::size_t i = 0; i < LightingData::MAX_LIGHTS; i++)
        {
            if (!m_data.lightEnable[i])
                continue;
            calculateLight(colorTmp,
                m_data.lights[i],
                m_data.material.specularExponent,
                ambientColor,
                diffuseColor,
                specularColor,
                vertex,
                n);
        }

        color = colorTmp;
        color[3] = alphaOld;
    }
}

void LightingCalc::calculateLight(Vec4& __restrict color,
    const LightingData::LightConfig& lightConfig,
    const float materialSpecularExponent,
    const Vec4& materialAmbientColor,
    const Vec4& materialDiffuseColor,
    const Vec4& materialSpecularColor,
    const Vec4& v0,
    const Vec4& n0) const
{
    // Calculate light from lights
    Vec4 dir;
    float att = 1.0f;

    // Calculate Diffuse Light
    // w unequal zero means: Point light
    // w equal to zero means: Directional light
    // It seems that mac os is interpolating between 0.0 and 1.0 so that the different
    // light sources can lerp to each other. We will not do this here.
    if (lightConfig.position[3] != 0.0f)
    {
        // Point light, is the normalized direction vector
        dir = lightConfig.position - v0;
        dir.normalize();

        const float dist = v0.dist(lightConfig.position);
        att = 1.0f / (lightConfig.constantAttenuation + (lightConfig.linearAttenuation * dist) + lightConfig.quadraticAttenuation * (dist * dist));
    }
    else
    {
        // Directional light, direction is the unit vector
        dir = lightConfig.preCalcDirectionalLightDir;
    }
    float dotDirDiffuse = n0.dot(dir);
    dotDirDiffuse = (dotDirDiffuse < 0.01f) ? 0.0f : dotDirDiffuse;

    // Calculate specular light
    float f = 0.0f;
    if (dotDirDiffuse != 0.0f)
    {
        f = 1.0f;
    }

    // Convert now the direction in dir to the half way vector
    if (lightConfig.localViewer)
    {
        Vec4 dirEye { 0.0f, 0.0f, 0.0f, 1.0f };
        dirEye -= v0;
        dirEye.normalize();
        dir += dirEye;
        dir.unit();
    }
    else
    {
        // Optimization: When position.w is equal to zero, then dirDiffuse is constant and
        // we can precompute with this constant value the half way vector.
        // Otherwise dirDiffuse depends on the vertex and no pre computation is possible
        if (lightConfig.position[3] != 0.0f)
        {
            const Vec4 pointEye { 0.0f, 0.0f, 1.0f, 1.0f };
            dir += pointEye;
            dir.unit();
        }
        else
        {
            dir = lightConfig.preCalcHalfWayVectorInfinite;
        }
    }

    float dotDirSpecular = n0.dot(dir);

    // Optimization: pows are expensive
    if (materialSpecularExponent == 0.0f) // x^0 == 1.0
    {
        dotDirSpecular = 1.0f;
    }
    else if (materialSpecularExponent != 1.0f) // x^1 == x
    {
        dotDirSpecular = powf(dotDirSpecular, materialSpecularExponent);
    }

    const Vec4 colorLightSpecular = lightConfig.specularColor * materialSpecularColor * (f * dotDirSpecular);
    const Vec4 ambientColor = lightConfig.ambientColor * materialAmbientColor;
    const Vec4 colorLight = ((lightConfig.diffuseColor * materialDiffuseColor * dotDirDiffuse) + ambientColor + colorLightSpecular) * att;

    // TODO: Spotlight has to be implemented. Please see in the OpenGL 1.5 spec equation 2.5.
    // Basically it is a val = dot(normalize(v0 - lightConfig.position), unit(lightConfig.spotlightDirectionLight))
    // spot = pow(val, lightConfig.spotlightExponentLight); when lightConfig.spotlightCutoffLight != 180 and val >= cos(lightConfig.spotlightCutoffLight)
    // spot = 0; when lightConfig.spotlightCutoffLight != 180 and val < cos(lightConfig.spotlightCutoffLight)
    // spot = 1; when lightConfig.spotlightCutoffLight == 180
    // colorLight0 *= spot;

    // Add light sums to final color
    color += colorLight;
}

void LightingCalc::calculateSceneLight(Vec4& __restrict sceneLight,
    const Vec4& emissiveColor,
    const Vec4& ambientColor,
    const Vec4& ambientColorScene) const
{
    // Ambient Color Material and ambient scene color
    sceneLight = ambientColor;
    sceneLight *= ambientColorScene;
    // Emission color of material
    sceneLight += emissiveColor;
}

void LightingSetter::enableLighting(bool enable)
{
    m_data.lightingEnabled = enable;
}

void LightingSetter::enableLight(const std::size_t light, const bool enable)
{
    m_data.lightEnable[light] = enable;
}

void LightingSetter::setAmbientColorLight(const std::size_t light, const Vec4& color)
{
    m_data.lights[light].ambientColor = color;
}

void LightingSetter::setDiffuseColorLight(const std::size_t light, const Vec4& color)
{
    m_data.lights[light].diffuseColor = color;
}

void LightingSetter::setSpecularColorLight(const std::size_t light, const Vec4& color)
{
    m_data.lights[light].specularColor = color;
}

void LightingSetter::setPosLight(const std::size_t light, const Vec4& pos)
{
    m_data.lights[light].position = pos;
    m_data.lights[light].preCalcVectors();
}

void LightingSetter::setConstantAttenuationLight(const std::size_t light, const float val)
{
    m_data.lights[light].constantAttenuation = val;
}

void LightingSetter::setLinearAttenuationLight(const std::size_t light, const float val)
{
    m_data.lights[light].linearAttenuation = val;
}

void LightingSetter::setQuadraticAttenuationLight(const std::size_t light, const float val)
{
    m_data.lights[light].quadraticAttenuation = val;
}

void LightingSetter::enableColorMaterial(bool emission, bool ambient, bool diffuse, bool specular)
{
    m_data.enableColorMaterialEmission = emission;
    m_data.enableColorMaterialAmbient = ambient;
    m_data.enableColorMaterialDiffuse = diffuse;
    m_data.enableColorMaterialSpecular = specular;
}

void LightingSetter::setEmissiveColorMaterial(const Vec4& color)
{
    m_data.material.emissiveColor = color;
}

void LightingSetter::setAmbientColorMaterial(const Vec4& color)
{
    m_data.material.ambientColor = color;
}

void LightingSetter::setAmbientColorScene(const Vec4& color)
{
    m_data.material.ambientColorScene = color;
}

void LightingSetter::setDiffuseColorMaterial(const Vec4& color)
{
    m_data.material.diffuseColor = color;
}

void LightingSetter::setSpecularColorMaterial(const Vec4& color)
{
    m_data.material.specularColor = color;
}

void LightingSetter::setSpecularExponentMaterial(const float val)
{
    m_data.material.specularExponent = val;
}

void LightingSetter::setColorMaterialTracking(const Face face, const ColorMaterialTracking material)
{
    switch (material)
    {
    case ColorMaterialTracking::AMBIENT:
        if (m_enableColorMaterial)
            enableColorMaterial(false, true, false, false);
        break;
    case ColorMaterialTracking::DIFFUSE:
        if (m_enableColorMaterial)
            enableColorMaterial(false, false, true, false);
        break;
    case ColorMaterialTracking::AMBIENT_AND_DIFFUSE:
        if (m_enableColorMaterial)
            enableColorMaterial(false, true, true, false);
        break;
    case ColorMaterialTracking::SPECULAR:
        if (m_enableColorMaterial)
            enableColorMaterial(false, false, false, true);
        break;
    case ColorMaterialTracking::EMISSION:
        if (m_enableColorMaterial)
            enableColorMaterial(true, false, false, false);
        break;
    default:
        if (m_enableColorMaterial)
            enableColorMaterial(false, true, true, false);
        break;
    }
    m_colorMaterialTracking = material;
    m_colorMaterialFace = face;
}

void LightingSetter::enableColorMaterial(const bool enable)
{
    m_enableColorMaterial = enable;
    if (enable)
    {
        setColorMaterialTracking(m_colorMaterialFace, m_colorMaterialTracking);
    }
    else
    {
        enableColorMaterial(false, false, false, false);
    }
}

} // namespace rr::lighting