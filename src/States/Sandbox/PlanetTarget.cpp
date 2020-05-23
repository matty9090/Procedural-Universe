#include "PlanetTarget.hpp"
#include "StarTarget.hpp"

#include "Core/Event.hpp"

#include "Services/Log.hpp"
#include "Services/ResourceManager.hpp"

PlanetTarget::PlanetTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, ICamera* camera, ID3D11RenderTargetView* rtv)
    : SandboxTarget(context, "Planetary", "N/A", resources, camera, rtv)
{
    Scale = 0.001f;
    ObjectScale = 0.0f;
    BeginTransitionDist = 12.0f;
    EndTransitionDist = 0.6f;
    Enable = false;
    RenderParentInChildSpace = true;

    auto vp = Resources->GetScreenViewport();
    unsigned int width = static_cast<size_t>(vp.Width);
    unsigned int height = static_cast<size_t>(vp.Height);

    Planet = std::make_unique<CPlanet>(Context, *Camera);
}

void PlanetTarget::Render()
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);

    Parent->RenderInChildSpace(*Camera, 1.0f / (Parent->Scale * Scale));
    Planet->Render();
}

void PlanetTarget::RenderUI()
{
    Planet->RenderUI();
}

void PlanetTarget::RenderTransitionChild(float t)
{
    EventStream::Report(EEvent::SandboxBloomBaseChanged, FloatEventData { 1.0f - t });
}

void PlanetTarget::RenderTransitionParent(float t)
{
    
}

void PlanetTarget::Seed(uint64_t seed)
{
    CPlanetSeeder seeder((Parent->GetSeed() << 5) + seed);
    seeder.SeedPlanet(Planet.get());
}

void PlanetTarget::MoveObjects(Vector3 v)
{
    Planet->Move(v);
    Centre += v;
    ParentOffset += v;
    Parent->MoveObjects(v);
}

void PlanetTarget::ScaleObjects(float scale)
{
    Planet->Scale(1.0f / scale);
}

Vector3 PlanetTarget::GetClosestObject(Vector3 pos)
{
    return Vector3::Zero;
}

void PlanetTarget::ResetObjectPositions()
{
    MoveObjects(-Centre);
    Planet->SetPosition(Vector3::Zero);
    Centre = Vector3::Zero;
}

void PlanetTarget::OnStartTransitionDownChild(Vector3 location)
{
    Planet->LightSource = -static_cast<StarTarget*>(Parent)->GetLightDirection();
}

void PlanetTarget::OnEndTransitionUpChild()
{
    EventStream::Report(EEvent::SandboxBloomBaseChanged, FloatEventData { 1.0f });
}

void PlanetTarget::StateIdle(float dt)
{
    Planet->Update(dt);

    float dist = Vector3::Distance(Camera->GetPosition(), Planet->GetPosition()) - Planet->Radius;
    float t = (dist - Planet->Radius * 0.5f) / (Planet->Radius * 3.0f);

    VelocityMultiplier = Maths::Clamp(Maths::Lerp(0.02f, 1.0f, t), 0.02f, 1.0f);
}
