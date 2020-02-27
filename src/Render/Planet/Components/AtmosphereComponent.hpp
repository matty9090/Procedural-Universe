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

struct ScatterBuffer
{
    DirectX::SimpleMath::Vector3 v3CameraPos;
    float fCameraHeight;
    DirectX::SimpleMath::Vector3 v3LightPos;
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
    CAtmosphereComponent(CPlanet* planet, float height = 1.025f);

    void Init() final;
    void Update(float dt) final;
    void Render(DirectX::SimpleMath::Matrix viewProj) final;
    void RenderUI() final;

    float GetRadius() const;

private:
    ScatterBuffer GetScatterBuffer();

    float Height = 1.025f;
    float ScaleDepth = 0.25f;
    DirectX::SimpleMath::Vector3 Colour = { 0.65f, 0.57f, 0.475f };

    float Kr = 0.0025f; // Rayleigh scattering constant
    float Km = 0.0010f; // Mie scattering constant
    float Af = -0.990f; // Atmospheric scattering constant
    float ESun = 15.0f; // Mie phase asymmetry factor

    CPlanet* Planet;
    ConstantBuffer<ScatterBuffer> Buffer;
    
    RenderPipeline PipelineSky;
    RenderPipeline PipelineSpace;

    std::unique_ptr<CModel> Sphere;
    std::unique_ptr<DirectX::CommonStates> CommonStates;
};