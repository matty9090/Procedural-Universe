#include "AtmosphereComponent.hpp"
#include "Render/Planet/Planet.hpp"
#include "Services/ResourceManager.hpp"

CAtmosphereComponent::CAtmosphereComponent(CPlanet* planet, float height)
    : Planet(planet),
      Height(height),
      Buffer(planet->GetDevice())
{
    CommonStates = std::make_unique<DirectX::CommonStates>(Planet->GetDevice());

    Pipeline.LoadVertex(L"shaders/Volumetric/Atmosphere.vsh");
    Pipeline.LoadPixel(L"shaders/Volumetric/Atmosphere.psh");
    Pipeline.CreateRasteriser(Planet->GetDevice(), ECullMode::Anticlockwise);
    Pipeline.CreateInputLayout(Planet->GetDevice(), CreateInputLayoutPosition());
    Pipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    Sphere = std::make_unique<CModel>(planet->GetDevice(), RESM.GetMesh("assets/Atmosphere.obj"));
    Sphere->Scale(Planet->Radius * 1.1f);
}

void CAtmosphereComponent::Update(float dt)
{
    Sphere->SetPosition(Planet->GetPosition());
    Sphere->SetScale(Planet->GetScale());
}

void CAtmosphereComponent::Render(DirectX::SimpleMath::Matrix viewProj)
{
    Buffer.SetData(Planet->GetContext(), GetScatterBuffer());

    Planet->GetContext()->OMSetBlendState(CommonStates->NonPremultiplied(), DirectX::Colors::Black, 0xFFFFFFFF);
    Planet->GetContext()->VSSetConstantBuffers(1, 1, Buffer.GetBuffer());
    Planet->GetContext()->PSSetConstantBuffers(1, 1, Buffer.GetBuffer());

    Sphere->Draw(Planet->GetContext(), viewProj, Pipeline);
}

ScatterBuffer CAtmosphereComponent::GetScatterBuffer()
{
    float radius = Planet->Radius;
    float atmradius = radius + Height;
    float camHeight = (Planet->Camera->GetPosition() - Planet->GetPosition()).Length();

    float scale = 1.0f / (atmradius - Planet->Radius);
    float scaleDepth = 0.25f;

    DirectX::SimpleMath::Vector3 wavelength(0.65f, 0.57f, 0.475f);
    wavelength.x = 1.0f / powf(wavelength.x, 4.0f);
    wavelength.y = 1.0f / powf(wavelength.y, 4.0f);
    wavelength.z = 1.0f / powf(wavelength.z, 4.0f);

    ScatterBuffer buffer = {
        Planet->Camera->GetPosition() - Planet->GetPosition(),
        camHeight,
        DirectX::SimpleMath::Vector3(0.0f, 0.5f, -0.5f),
        camHeight * camHeight,
        wavelength,
        atmradius,
        atmradius * atmradius,
        radius,
        radius * radius,
        Constants::Kr * Constants::ESun,
        Constants::Km * Constants::ESun,
        Constants::Kr * 4.0f * DirectX::XM_PI,
        Constants::Km * 4.0f * DirectX::XM_PI,
        scale,
        scaleDepth,
        scale / scaleDepth,
        Constants::Af,
        Constants::Af * Constants::Af
    };

    return buffer;
}
