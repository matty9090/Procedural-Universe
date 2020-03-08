#include "GalaxyTarget.hpp"
#include "Sim/IParticleSeeder.hpp"

#include <random>
#include <imgui.h>

GalaxyTarget::GalaxyTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, ICamera* camera, ID3D11RenderTargetView* rtv)
    : SandboxTarget(context, "Galactic", "Star", resources, camera, rtv)
{
    Scale = 0.01f;
    BeginTransitionDist = 1600.0f;
    EndTransitionDist = 200.0f;

    auto vp = Resources->GetScreenViewport();
    unsigned int width = static_cast<size_t>(vp.Width);
    unsigned int height = static_cast<size_t>(vp.Height);

    Splatting = std::make_unique<CSplatting>(Context, width, height);
    PostProcess = std::make_unique<CPostProcess>(Device, Context, width, height);
    CommonStates = std::make_unique<DirectX::CommonStates>(Device);

    GalaxyRenderer = std::make_unique<Galaxy>(Context);
    GalaxyRenderer->SetFades(true);

    SeedParticles.resize(PARTICLES_PER_GALAXY);
}

void GalaxyTarget::Seed(uint64_t seed)
{
    GalaxyRenderer->InitialSeed(seed);
}

void GalaxyTarget::Render()
{
    //RenderParentSkybox();
    Parent->RenderInChildSpace();
    RenderLerp(1.0f, 1.0f / Scale);
}

void GalaxyTarget::RenderUI()
{
    
}

void GalaxyTarget::RenderTransitionChild(float t)
{
    
}

void GalaxyTarget::RenderTransitionParent(float t)
{
    RenderParentSkybox();
    RenderLerp(t, 1.0f / Scale, Vector3::Zero, true);
}

void GalaxyTarget::MoveObjects(Vector3 v)
{
    GalaxyRenderer->Move(v);
    Centre += v;
    Parent->MoveObjects(v);
}

void GalaxyTarget::ScaleObjects(float scale)
{
    GalaxyRenderer->Scale(scale);
}

void GalaxyTarget::ResetObjectPositions()
{
    GalaxyRenderer->Move(-Centre);
    Centre = Vector3::Zero;
}

Vector3 GalaxyTarget::GetClosestObject(Vector3 pos)
{
    return GalaxyRenderer->GetClosestObject(pos);
}

void GalaxyTarget::OnStartTransitionUpChild()
{
    Parent->ScaleObjects(1.0f / Scale);
}

void GalaxyTarget::OnStartTransitionDownChild(Vector3 location)
{
    auto seed = GalaxyRenderer->GetSeed();
    auto col = GalaxyRenderer->GetColour();

    GalaxyRenderer->Scale(50.0f);

    DispatchTask(EWorkerTask::Seed, [&]() {
        const float Variation = 0.12f;

        auto seeder = CreateParticleSeeder(SeedParticles, EParticleSeeder::Galaxy, 0.1f);
        seeder->SetRedDist(col.R() - Variation, col.R() + Variation);
        seeder->SetGreenDist(col.G() - Variation, col.G() + Variation);
        seeder->SetBlueDist(col.B() - Variation, col.B() + Variation);
        seeder->Seed(seed);
    });
}

void GalaxyTarget::OnEndTransitionDownChild()
{
    FinishTask(EWorkerTask::Seed);
    GalaxyRenderer->FinishSeed(SeedParticles);
    Tmp = Parent->GetClosestObject(Camera->GetPosition());
    Parent->ResetObjectPositions();
    Parent->ScaleObjects(Scale);
    Parent->MoveObjects(-Tmp / Scale);
}

void GalaxyTarget::RenderLerp(float t, float scale, Vector3 voffset, bool single)
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);

    GalaxyRenderer->Render(*Camera, t, scale, voffset, single);
}

void GalaxyTarget::BakeSkybox(Vector3 object)
{
    SkyboxGenerator->Render([&](const ICamera& cam) {
        Parent->GetSkyBox().Draw(cam.GetViewMatrix() * cam.GetProjectionMatrix());
        GalaxyRenderer->Render(cam, 0.0f, 1.0f / Scale, Vector3::Zero, true);
    });
}
