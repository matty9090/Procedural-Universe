#include "AtmosphereComponent.hpp"
#include "Render/Planet/Planet.hpp"
#include "Services/ResourceManager.hpp"
#include "Services/Log.hpp"
#include "Misc/ProcUtils.hpp"

#include <random>
#include <imgui.h>

RenderPipeline CAtmosphereComponent::PipelineSky;
RenderPipeline CAtmosphereComponent::PipelineSpace;

CAtmosphereComponent::CAtmosphereComponent(CPlanet* planet, uint64_t seed)
    : Planet(planet),
      Buffer(planet->GetDevice())
{
    CommonStates = std::make_unique<DirectX::CommonStates>(Planet->GetDevice());
    Sphere = std::make_unique<CModel>(Planet->GetDevice(), RESM.GetMesh("assets/Sphere.obj"));

    std::default_random_engine gen { static_cast<unsigned int>(seed) };

    std::uniform_real_distribution<float> KrDist(KrMin, KrMax);
    std::uniform_real_distribution<float> KmDist(KmMin, KmMax);
    std::uniform_real_distribution<float> ESunDist(ESunMin, ESunMax);
    std::uniform_real_distribution<float> HueDist(0.0f, 360.0f);

    Kr = KrDist(gen);
    Km = KmDist(gen);
    ESun = ESunDist(gen);

    int H = 0, S = 0, L = 0;
    auto col = DirectX::SimpleMath::Color(0.65f, 0.57f, 0.475f);

    ProcUtils::RGBToHSL(col, H, S, L);
    ProcUtils::HSLToRGB(HueDist(gen), static_cast<float>(S), static_cast<float>(L), col);

    Colour = { col.R(), col.G(), col.B() };
}

void CAtmosphereComponent::LoadCache(ID3D11Device* device)
{
    PipelineSky.LoadVertex(L"shaders/Volumetric/SkyFromAtmosphere.vsh");
    PipelineSky.LoadPixel(L"shaders/Volumetric/SkyFromAtmosphere.psh");
    PipelineSky.CreateRasteriser(device, ECullMode::Anticlockwise);
    PipelineSky.CreateInputLayout(device, CreateInputLayoutPosition());
    PipelineSky.Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    PipelineSpace.LoadVertex(L"shaders/Volumetric/SkyFromSpace.vsh");
    PipelineSpace.LoadPixel(L"shaders/Volumetric/SkyFromSpace.psh");
    PipelineSpace.CreateRasteriser(device, ECullMode::Anticlockwise);
    PipelineSpace.CreateInputLayout(device, CreateInputLayoutPosition());
    PipelineSpace.Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

void CAtmosphereComponent::Init()
{
    Sphere->SetScale(Planet->Radius * 1.025f);
}

void CAtmosphereComponent::Update(float dt)
{

}

void CAtmosphereComponent::Render(DirectX::SimpleMath::Matrix viewProj, float t)
{
    Buffer.SetData(Planet->GetContext(), GetScatterBuffer());

    Planet->GetContext()->OMSetBlendState(CommonStates->AlphaBlend(), DirectX::Colors::Black, 0xFFFFFFFF);
    Planet->GetContext()->OMSetDepthStencilState(CommonStates->DepthRead(), 0);
    Planet->GetContext()->VSSetConstantBuffers(1, 1, Buffer.GetBuffer());
    Planet->GetContext()->PSSetConstantBuffers(1, 1, Buffer.GetBuffer());
    
    auto camHeight = (Planet->Camera.GetPosition() - Planet->GetPosition()).Length();
    auto atmRadius = GetRadius();

    Sphere->Draw(Planet->GetContext(), viewProj, Planet->World, camHeight < atmRadius ? PipelineSky : PipelineSpace);
    //Sphere->Draw(Planet->GetContext(), viewProj, camHeight < atmRadius ? PipelineSky : PipelineSpace);
}

void CAtmosphereComponent::RenderUI()
{
    if (ImGui::CollapsingHeader("Atmosphere"))
    {
        ImGui::SliderFloat("Scale depth", &ScaleDepth, 0.02f, 1.0f);
        
        if(ImGui::SliderFloat("Height", &Height, 1.0f, 1.4f))
            Sphere->SetScale(Planet->Radius * Height);

        ImGui::ColorPicker3("Colour", &Colour.x);
        ImGui::SliderFloat("Kr", &Kr, KrMin, KrMax, "%.5f");
        ImGui::SliderFloat("Km", &Km, KmMin, KmMax, "%.5f");
        ImGui::SliderFloat("Af", &Af, -0.5f, -1.2f);
        ImGui::SliderFloat("ESun", &ESun, ESunMin, ESunMax);
    }
}

float CAtmosphereComponent::GetRadius() const
{
    return Planet->Radius * Height * Planet->GetScale();
}

ScatterBuffer CAtmosphereComponent::GetScatterBuffer()
{
    auto camPos = Planet->Camera.GetPosition();

    float radius = Planet->Radius * Planet->GetScale();
    float atmradius = radius * Height;
    float camHeight = (camPos - Planet->GetPosition()).Length();

    float scale = 1.0f / (atmradius - radius);

    auto wavelength = Colour;
    wavelength.x = 1.0f / powf(wavelength.x, 4.0f);
    wavelength.y = 1.0f / powf(wavelength.y, 4.0f);
    wavelength.z = 1.0f / powf(wavelength.z, 4.0f);

    ScatterBuffer buffer;
    buffer.fInnerRadius = radius;
    buffer.fOuterRadius = atmradius;
    buffer.fInnerRadius2 = radius * radius;
    buffer.fOuterRadius2 = atmradius * atmradius;
    buffer.fScale = scale;
    buffer.fScaleDepth = ScaleDepth;
    buffer.fScaleOverScaleDepth = scale / ScaleDepth;
    buffer.v3CameraPos = camPos;
    buffer.v3LightPos = Planet->LightSource;
    buffer.fCameraHeight = camHeight;
    buffer.fCameraHeight2 = camHeight * camHeight;
    buffer.v3InvWavelength = wavelength;
    buffer.fKrESun = Kr * ESun;
    buffer.fKmESun = Km * ESun;
    buffer.fKr4PI = Kr * 4.0f * DirectX::XM_PI;
    buffer.fKm4PI = Km * 4.0f * DirectX::XM_PI;
    buffer.g = Af;
    buffer.g2 = Af * Af;

    return buffer;
}
