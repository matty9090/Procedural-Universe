#include "AtmosphereComponent.hpp"
#include "Render/Planet/Planet.hpp"
#include "Services/ResourceManager.hpp"
#include "Services/Log.hpp"

CAtmosphereComponent::CAtmosphereComponent(CPlanet* planet, float height)
    : Planet(planet),
      Height(height),
      Buffer(planet->GetDevice())
{
    CommonStates = std::make_unique<DirectX::CommonStates>(Planet->GetDevice());

    PipelineSky.LoadVertex(L"shaders/Volumetric/SkyFromAtmosphere.vsh");
    PipelineSky.LoadPixel(L"shaders/Volumetric/SkyFromAtmosphere.psh");
    PipelineSky.CreateRasteriser(Planet->GetDevice(), ECullMode::Anticlockwise);
    PipelineSky.CreateInputLayout(Planet->GetDevice(), CreateInputLayoutPosition());
    PipelineSky.Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    PipelineSpace.LoadVertex(L"shaders/Volumetric/SkyFromSpace.vsh");
    PipelineSpace.LoadPixel(L"shaders/Volumetric/SkyFromSpace.psh");
    PipelineSpace.CreateRasteriser(Planet->GetDevice(), ECullMode::Anticlockwise);
    PipelineSpace.CreateInputLayout(Planet->GetDevice(), CreateInputLayoutPosition());
    PipelineSpace.Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    Sphere = std::make_unique<CModel>(planet->GetDevice(), RESM.GetMesh("assets/Sphere.obj"));
    Sphere->Scale(Planet->Radius * 1.025f);
}

void CAtmosphereComponent::Update(float dt)
{
    Sphere->SetPosition(Planet->GetPosition());
}

void CAtmosphereComponent::Render(DirectX::SimpleMath::Matrix viewProj)
{
    Buffer.SetData(Planet->GetContext(), GetScatterBuffer());

    Planet->GetContext()->OMSetBlendState(CommonStates->AlphaBlend(), DirectX::Colors::Black, 0xFFFFFFFF);
    Planet->GetContext()->OMSetDepthStencilState(CommonStates->DepthRead(), 0);
    Planet->GetContext()->VSSetConstantBuffers(1, 1, Buffer.GetBuffer());
    Planet->GetContext()->PSSetConstantBuffers(1, 1, Buffer.GetBuffer());
    
    auto camHeight = (Planet->Camera.GetPosition() - Planet->GetPosition()).Length();
    auto atmRadius = Planet->Radius * 1.025f;

    Sphere->Draw(Planet->GetContext(), viewProj, camHeight < atmRadius ? PipelineSky : PipelineSpace);
}

ScatterBuffer CAtmosphereComponent::GetScatterBuffer()
{
    auto camPos = Planet->Camera.GetPosition();

    float radius = Planet->Radius;
    float atmradius = radius * 1.025f;
    float camHeight = (camPos - Planet->GetPosition()).Length();

    float scale = 1.0f / (atmradius - radius);
    float scaleDepth = 0.25f;

    DirectX::SimpleMath::Vector3 wavelength(0.65f, 0.57f, 0.475f);
    wavelength.x = 1.0f / powf(wavelength.x, 4.0f);
    wavelength.y = 1.0f / powf(wavelength.y, 4.0f);
    wavelength.z = 1.0f / powf(wavelength.z, 4.0f);

    ScatterBuffer buffer;
    buffer.fInnerRadius = radius;
    buffer.fOuterRadius = atmradius;
    buffer.fInnerRadius2 = radius * radius;
    buffer.fOuterRadius2 = atmradius * atmradius;
    buffer.fScale = scale;
    buffer.fScaleDepth = scaleDepth;
    buffer.fScaleOverScaleDepth = scale / scaleDepth;
    buffer.v3CameraPos = camPos;
    buffer.v3LightPos = DirectX::SimpleMath::Vector3(0.0f, 0.5f, -0.5f);
    buffer.fCameraHeight = camHeight;
    buffer.fCameraHeight2 = camHeight * camHeight;
    buffer.v3InvWavelength  = wavelength;
    buffer.fKrESun = Constants::Kr * Constants::ESun;
    buffer.fKmESun = Constants::Km * Constants::ESun;
    buffer.fKr4PI = Constants::Kr * 4.0f * DirectX::XM_PI;
    buffer.fKm4PI = Constants::Km * 4.0f * DirectX::XM_PI;
    buffer.g = Constants::Af;
    buffer.g2 = Constants::Af * Constants::Af;

    return buffer;
}
