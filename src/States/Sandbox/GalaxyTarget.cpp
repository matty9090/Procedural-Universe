#include "GalaxyTarget.hpp"
#include "Sim/IParticleSeeder.hpp"

#include <random>

GalaxyTarget::GalaxyTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, ICamera* camera, ID3D11RenderTargetView* rtv)
    : SandboxTarget(context, "Galactic", "Star", resources, camera, rtv)
{
    Scale = 0.01f;
    BeginTransitionDist = 4000.0f;
    EndTransitionDist = 400.0f;

    auto vp = Resources->GetScreenViewport();
    unsigned int width = static_cast<size_t>(vp.Width);
    unsigned int height = static_cast<size_t>(vp.Height);

    Splatting = std::make_unique<CSplatting>(Context, width, height);
    PostProcess = std::make_unique<CPostProcess>(Device, Context, width, height);
    CommonStates = std::make_unique<DirectX::CommonStates>(Device);

    GalaxyRenderer = std::make_unique<Galaxy>(Context);
    SeedParticles.resize(1000000);
}

void GalaxyTarget::Seed(uint64_t seed)
{
    GalaxyRenderer->InitialSeed(seed);
}

void GalaxyTarget::Render()
{
    RenderParentSkybox();
    RenderLerp(1.0f, 1.0f / Scale);
}

void GalaxyTarget::RenderTransitionChild(float t)
{
    RenderLerp(t, 1.0f, ParentLocationSpace, true);
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

void GalaxyTarget::OnStartTransitionDownChild(Vector3 location)
{
    auto seed = GalaxyRenderer->GetSeed();
    auto col = GalaxyRenderer->GetColour();

    DispatchTask(EWorkerTask::Seed, [&]() {
        const float Variation = 0.12f;

        SeedDustClouds.clear();

        auto seeder = CreateParticleSeeder(SeedParticles, EParticleSeeder::Galaxy);
        seeder->SetRedDist(col.R() - Variation, col.R() + Variation);
        seeder->SetGreenDist(col.G() - Variation, col.G() + Variation);
        seeder->SetBlueDist(col.B() - Variation, col.B() + Variation);
        seeder->Seed(seed);

        std::default_random_engine gen { static_cast<unsigned int>(seed) };
        std::uniform_real_distribution<double> dist(0, static_cast<double>(SeedParticles.size()));
        std::uniform_real_distribution<float> distScale(4.0f, 18.0f);
        std::uniform_real_distribution<float> distAlpha(0.04f, 0.16f);

        for (int i = 0; i < SeedParticles.size() && i < GalaxyRenderer->NumDustClouds; ++i)
        {
            SeedDustClouds.push_back(BillboardInstance {
                SeedParticles[static_cast<int>(dist(gen))].Position,
                distScale(gen),
                Color(1.0f, 1.0f, 1.0f, distAlpha(gen))
            });
        }
    });
}

void GalaxyTarget::OnEndTransitionDownChild()
{
    FinishTask(EWorkerTask::Seed);
    
    GalaxyRenderer->FinishSeed(SeedParticles, SeedDustClouds);
    GalaxyRenderer->Scale(5.0f * Scale);
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
