#include "StarTarget.hpp"

#include <imgui.h>

#include "Sim/IParticleSeeder.hpp"

#include "Services/Log.hpp"
#include "Services/ResourceManager.hpp"

StarTarget::StarTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, ICamera* camera, ID3D11RenderTargetView* rtv)
    : SandboxTarget(context, "Stellar", "Planet", resources, camera, rtv)
{
    Scale = 0.0005f;
    BeginTransitionDist = 10.0f;
    EndTransitionDist = 0.2f;
    RenderParentInChildSpace = true;

    auto vp = Resources->GetScreenViewport();
    unsigned int width = static_cast<size_t>(vp.Width);
    unsigned int height = static_cast<size_t>(vp.Height);

    PostProcess = std::make_unique<CPostProcess>(Device, Context, width, height);
    CommonStates = std::make_unique<DirectX::CommonStates>(Device);

    CreateStarPipeline();
}

void StarTarget::Render()
{
    RenderParentSkybox();
    Parent->RenderInChildSpace(*Camera, 100.0f / Scale);
    RenderLerp();

    for (auto& planet : Planets)
    {
        planet->Render();
    }
}

void StarTarget::RenderObjectUI()
{
    ImGui::Text("Seed: %i", static_cast<int>(ParticleInfo[CurrentClosestObjectID].Seed));
    ImGui::Text("Radius: %i", static_cast<int>(ParticleInfo[CurrentClosestObjectID].Radius));
}

void StarTarget::RenderInChildSpace(const ICamera& cam, float scale)
{
    RenderParentSkybox();
    Parent->RenderInChildSpace(cam, scale);
    RenderLerp(Child->Scale, 0.0f, true);
}

void StarTarget::RenderTransitionChild(float t)
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);

    Star->Move(ParentLocationSpace);
    RenderLerp(Scale, t, true);
    Star->Move(-ParentLocationSpace);

    for (auto& planet : Planets)
    {
        planet->Move(ParentLocationSpace);
        planet->Render(Scale, t);
        planet->Move(-ParentLocationSpace);
    }
}

void StarTarget::RenderTransitionParent(float t)
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);

    RenderParentSkybox();
    Parent->RenderInChildSpace(*Camera, 100.0f / Scale);
    RenderLerp(1.0f, t, true);
}

void StarTarget::MoveObjects(Vector3 v)
{
    for (auto& planet : Planets)
        planet->Move(v);

    Star->Move(v);
    Centre += v;

    ParentOffset += v;
    Parent->MoveObjects(v);
}

void StarTarget::ScaleObjects(float scale)
{
    for (auto& planet : Planets)
        planet->Scale(1.0f / scale);
}

Vector3 StarTarget::GetClosestObject(Vector3 pos)
{
    return Maths::ClosestParticle(pos, Particles, &CurrentClosestObjectID).Position;
}

Vector3 StarTarget::GetLightDirection() const
{
    auto pos = Particles[CurrentClosestObjectID].Position;
    pos.Normalize();
    return pos;
}

void StarTarget::OnStartTransitionDownParent(Vector3 object)
{
    GenerateSkybox(object);
}

void StarTarget::OnStartTransitionDownChild(Vector3 object)
{

}

void StarTarget::OnEndTransitionDownChild()
{

}

void StarTarget::RenderLerp(float scale, float t, bool single)
{
    Matrix view = Camera->GetViewMatrix();
    Matrix viewProj = view * Camera->GetProjectionMatrix();

    if (single)
        LerpBuffer->SetData(Context, LerpConstantBuffer { 1.0f });
    else
        LerpBuffer->SetData(Context, LerpConstantBuffer { t });
    
    Context->PSSetConstantBuffers(0, 1, LerpBuffer->GetBuffer());
    Context->GSSetShader(nullptr, 0, 0);
    Context->OMSetBlendState(CommonStates->NonPremultiplied(), DirectX::Colors::Black, 0xFFFFFFFF);
    Star->SetScale(scale);
    Star->Draw(Context, viewProj, StarPipeline);
}

void StarTarget::BakeSkybox(Vector3 object)
{
    SkyboxGenerator->Render([&](const ICamera& cam) {
        LerpBuffer->SetData(Context, LerpConstantBuffer { 1.0f });

        Matrix view = cam.GetViewMatrix();
        Matrix viewProj = view * cam.GetProjectionMatrix();
        
        Parent->RenderInChildSpace(cam, 100.0f / Scale);

        for (auto& planet : Planets)
        {
            planet->Render();
        }

        Context->GSSetShader(nullptr, 0, 0);
        LerpBuffer->SetData(Context, LerpConstantBuffer { 1.0f });
        Context->PSSetConstantBuffers(0, 1, LerpBuffer->GetBuffer());
        Context->GSSetShader(nullptr, 0, 0);
        Context->OMSetBlendState(CommonStates->AlphaBlend(), DirectX::Colors::Black, 0xFFFFFFFF);
        Star->Draw(Context, viewProj, StarPipeline);
    });
}

void StarTarget::Seed(uint64_t seed)
{
    auto gen = std::default_random_engine { static_cast<unsigned int>(seed) };
    std::uniform_int_distribution<> dist(4, 15);

    Particles.resize(dist(gen));
    ParticleInfo.resize(Particles.size());

    auto seeder = CreateParticleSeeder(Particles, EParticleSeeder::Random, 2.0f);
    seeder->Seed(seed);

    Planets.resize(Particles.size());

    for (size_t i = 0; i < Planets.size(); ++i)
    {
        Planets[i] = std::make_unique<CPlanet>(Context, *Camera);
        CPlanetSeeder seeder((seed << 5) + i);
        seeder.SeedPlanet(Planets[i].get());
        ParticleInfo[i] = seeder;
        Planets[i]->SetPosition(Particles[i].Position);
        Planets[i]->Scale(Scale * 2.0f);
        Planets[i]->LightSource = -Particles[i].Position;
        Planets[i]->LightSource.Normalize();
    }
}

void StarTarget::ResetObjectPositions()
{
    MoveObjects(-Centre);
    Star->SetPosition(Vector3::Zero);
    Centre = Vector3::Zero;
}

void StarTarget::CreateStarPipeline()
{
    Star = std::make_unique<CModel>(Device, RESM.GetMesh("assets/Sphere.obj"));
    Star->Scale(0.004f);

    StarPipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    StarPipeline.LoadVertex(L"shaders/Standard/Position.vsh");
    StarPipeline.LoadPixel(L"shaders/Particles/Star.psh");
    StarPipeline.CreateDepthState(Device, EDepthState::Normal);
    StarPipeline.CreateRasteriser(Device, ECullMode::Clockwise);
    StarPipeline.CreateInputLayout(Device, CreateInputLayoutPosition());

    LerpBuffer = std::make_unique<ConstantBuffer<LerpConstantBuffer>>(Device);
}