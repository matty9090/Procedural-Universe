#include "UniverseTarget.hpp"
#include "Sim/IParticleSeeder.hpp"

UniverseTarget::UniverseTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, ICamera* camera, ID3D11RenderTargetView* rtv)
    : SandboxTarget(context, "Universal", "Galaxy", resources, camera, rtv)
{
    Scale = 0.006f;
    BeginTransitionDist = 3200.0f;
    EndTransitionDist = 300.0f;

    auto vp = Resources->GetScreenViewport();
    unsigned int width = static_cast<size_t>(vp.Width);
    unsigned int height = static_cast<size_t>(vp.Height);

    Splatting = std::make_unique<CSplatting>(Context, width, height);
    PostProcess = std::make_unique<CPostProcess>(Device, Context, width, height);
    CommonStates = std::make_unique<DirectX::CommonStates>(Device);

    Seed(0);
}

void UniverseTarget::Render()
{
    RenderLerp(1.0f);
}

void UniverseTarget::RenderInChildSpace()
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);
    
    for (size_t i = 0; i < Galaxies.size(); ++i)
    {
        if (i != CurrentClosestObjectID)
            Galaxies[i]->RenderImposter(*Camera, 1.0f / Child->Scale);
    }
}

void UniverseTarget::RenderTransitionParent(float t)
{
    RenderLerp(t, true);
}

void UniverseTarget::MoveObjects(Vector3 v)
{
    for (auto& galaxy : Galaxies)
        galaxy->Move(v);

    Centre += v;
}

void UniverseTarget::ScaleObjects(float scale)
{
    for (auto& galaxy : Galaxies)
        galaxy->Scale(scale);
}

void UniverseTarget::ResetObjectPositions()
{
    for (auto& galaxy : Galaxies)
        galaxy->Move(-Centre);

    Centre = Vector3::Zero;
}

Vector3 UniverseTarget::GetClosestObject(Vector3 pos)
{
    return Maths::ClosestObject(pos, Galaxies, &CurrentClosestObjectID)->GetPosition();
}

void UniverseTarget::RenderLerp(float t, bool single)
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);

    /*if (single)
    {
        for (size_t i = 0; i < Galaxies.size(); ++i)
        {
            if (i == CurrentClosestObjectID)
                Galaxies[i]->Render(*Camera, t);
            else
                Galaxies[i]->RenderImposter(*Camera);
        }
    }
    else*/
    {
        for (auto& galaxy : Galaxies)
            galaxy->RenderImposter(*Camera);
    }
}

void UniverseTarget::Seed(uint64_t seed)
{
    Galaxies.clear();

    std::vector<Particle> particles;
    particles.resize(200);

    auto seeder = CreateParticleSeeder(particles, EParticleSeeder::Random);
    seeder->Seed(seed);

    uint64_t i = 0;

    for (const auto& particle : particles)
    {
        Galaxies.push_back(std::make_unique<Galaxy>(Context, true));
        Galaxies.back()->InitialSeed(i++);
        Galaxies.back()->Scale(5000.0f);
        Galaxies.back()->Move(particle.Position / 0.02f);
        Galaxies.back()->SetFades(false);
    }
}

void UniverseTarget::BakeSkybox(Vector3 object)
{
    /*SkyboxGenerator->Render([&](const ICamera& cam) {
        for (size_t i = 0; i < Galaxies.size(); ++i)
        {
            if(i != CurrentClosestObjectID)
                Galaxies[i]->RenderImposter(cam);
        }
    });*/
}