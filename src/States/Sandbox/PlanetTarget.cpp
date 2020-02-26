#include "PlanetTarget.hpp"
#include "StarTarget.hpp"

#include "Services/Log.hpp"
#include "Services/ResourceManager.hpp"

PlanetTarget::PlanetTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, ICamera* camera, ID3D11RenderTargetView* rtv)
    : SandboxTarget(context, "Planetary", "N/A", resources, camera, rtv)
{
    Scale = 0.001f;
    BeginTransitionDist = 10.0f;
    EndTransitionDist = 0.3f;

    auto vp = Resources->GetScreenViewport();
    unsigned int width = static_cast<size_t>(vp.Width);
    unsigned int height = static_cast<size_t>(vp.Height);

    PostProcess = std::make_unique<CPostProcess>(Device, Context, width, height);
    CommonStates = std::make_unique<DirectX::CommonStates>(Device);
    Planet = std::make_unique<CPlanet>(Context, *Camera);

    CreatePlanetPipeline();
}

void PlanetTarget::Render()
{
    RenderParentSkybox();
    Planet->Render();
}

void PlanetTarget::RenderUI()
{
    Planet->RenderUI();
}

void PlanetTarget::RenderTransitionChild(float t)
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);

    //PlanetBuffer->SetData(Context, PlanetConstantBuffer { Planet->LightSource, t });
    //Context->PSSetConstantBuffers(0, 1, PlanetBuffer->GetBuffer());

    Planet->Move(ParentLocationSpace);
    Planet->Render(Scale);
    Planet->Move(-ParentLocationSpace);
}

void PlanetTarget::RenderTransitionParent(float t)
{
    /*RenderParentSkybox();

    PlanetBuffer->SetData(Context, PlanetConstantBuffer{ Planet->LightSource, t });
    Context->PSSetConstantBuffers(0, 1, PlanetBuffer->GetBuffer());

    Planet->Render();*/
}

void PlanetTarget::MoveObjects(Vector3 v)
{
    Planet->Move(v);
    Centre += v;
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
    MoveObjects(-Planet->GetPosition());
    Planet->SetPosition(Vector3::Zero);
}

void PlanetTarget::OnStartTransitionDownChild(Vector3 location)
{
    Planet->LightSource = -static_cast<StarTarget*>(Parent)->GetLightDirection();
    //Planet->SetScale(Scale);
}

void PlanetTarget::CreatePlanetPipeline()
{
    StarPipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    StarPipeline.LoadVertex(L"shaders/Standard/Position.vsh");
    StarPipeline.LoadPixel(L"shaders/Particles/Star.psh");
    StarPipeline.CreateRasteriser(Device, ECullMode::Clockwise);
    StarPipeline.CreateInputLayout(Device, CreateInputLayoutPosition());

    PlanetBuffer = std::make_unique<ConstantBuffer<PlanetConstantBuffer>>(Device);
}

void PlanetTarget::StateIdle(float dt)
{
    Planet->Update(dt);

    float dist = Vector3::Distance(Camera->GetPosition(), Planet->GetPosition()) - Planet->Radius;
    float t = (dist - Planet->Radius * 0.5f) / (Planet->Radius * 3.0f);

    VelocityMultiplier = Maths::Clamp(Maths::Lerp(0.02f, 1.0f, t), 0.02f, 1.0f);
}
