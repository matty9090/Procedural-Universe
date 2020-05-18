#include "GalaxyTarget.hpp"

#include "Misc/ProcUtils.hpp"
#include "Sim/IParticleSeeder.hpp"

#include <random>
#include <imgui.h>

GalaxyTarget::GalaxyTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, ICamera* camera, ID3D11RenderTargetView* rtv)
    : SandboxTarget(context, "Galactic", "Star", resources, camera, rtv)
{
    Scale = 0.01f;
    ObjectScale = 0.05f;
    BeginTransitionDist = 1600.0f;
    EndTransitionDist = 200.0f;
    RenderParentInChildSpace = true;

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
    GalaxyRenderer->InitialSeed((Parent->GetSeed() << 21) + seed);
}

void GalaxyTarget::Render()
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);

    Parent->RenderInChildSpace(*Camera, 1.0f / Scale);
    RenderLerp(1.0f, 1.0f / Scale);
}

void GalaxyTarget::RenderUI()
{
    
}

void GalaxyTarget::RenderInChildSpace(const ICamera& cam, float scale)
{
    GalaxyRenderer->Render(cam, 0.0f, scale, Vector3::Zero, true);
}

void GalaxyTarget::RenderTransitionChild(float t)
{
    // Universe target renders imposters, don't need to render anything here
}

void GalaxyTarget::RenderTransitionParent(float t)
{
    Parent->RenderInChildSpace(*Camera, 1.0f / Scale);
    RenderLerp(t, 1.0f / Scale, Vector3::Zero, true);
}

void GalaxyTarget::MoveObjects(Vector3 v)
{
    GalaxyRenderer->Move(v);
    Centre += v;
    ParentOffset += v;
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

std::string GalaxyTarget::GetObjectName() const
{
    uint64_t seed = GalaxyRenderer->GetSeed() + static_cast<uint64_t>(GalaxyRenderer->GetClosestObjectIndex());
    std::default_random_engine gen { static_cast<unsigned int>(seed) };

    return ProcUtils::RandomStarName(gen);
}

Vector3 GalaxyTarget::GetClosestObject(Vector3 pos)
{
    return GalaxyRenderer->GetClosestObject(pos);
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
        Parent->RenderInChildSpace(cam, 1.2f / Scale);
    });
}
