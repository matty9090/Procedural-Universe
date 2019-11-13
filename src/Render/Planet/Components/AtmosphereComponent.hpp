#pragma once

#include <map>
#include <array>
#include <vector>
#include <memory>
#include <CommonStates.h>

#include "Render/DX/RenderCommon.hpp"
#include "Render/DX/ConstantBuffer.hpp"
#include "Render/Model/Model.hpp"
#include "PlanetComponent.hpp"

class CPlanet;

namespace Constants
{
    // Rayleigh scattering constant
    const float Kr = 0.0025f;

    // Mie scattering constant
    const float Km = 0.0010f;

    // Atmospheric scattering constant
    const float Af = -0.990f;

    // Mie phase asymmetry factor
    const float ESun = 15.0f;
}

struct ScatterBuffer
{
    DirectX::SimpleMath::Vector3 v3CameraPos;
    float fCameraHeight;
    DirectX::SimpleMath::Vector3 v3LightDir;
    float fCameraHeight2;
    DirectX::SimpleMath::Vector3 v3InvWavelength;
    float fOuterRadius;
    float fOuterRadius2;
    float fInnerRadius;
    float fInnerRadius2;
    float fKrESun;
    float fKmESun;
    float fKr4PI;
    float fKm4PI;
    float fScale;
    float fScaleDepth;
    float fScaleOverScaleDepth;
    float g;
    float g2;
};

class CAtmosphereComponent : public IPlanetComponent
{
public:
    CAtmosphereComponent(CPlanet* planet, float height);

    void Update(float dt) final;
    void Render(DirectX::SimpleMath::Matrix viewProj) final;

private:
    ScatterBuffer GetScatterBuffer();

    float Height;
    CPlanet* Planet;
    RenderPipeline Pipeline;
    ConstantBuffer<ScatterBuffer> Buffer;

    std::unique_ptr<CModel> Sphere;
    std::unique_ptr<DirectX::CommonStates> CommonStates;
};