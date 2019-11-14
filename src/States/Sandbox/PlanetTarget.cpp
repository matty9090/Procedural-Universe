#include "PlanetTarget.hpp"

#include "Services/Log.hpp"
#include "Services/ResourceManager.hpp"

PlanetTarget::PlanetTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, CShipCamera* camera, ID3D11RenderTargetView* rtv, const std::vector<Particle>& seedData)
    : SandboxTarget(context, "Planetary", resources, camera, rtv),
      Particles(seedData)
{
    Scale = 0.02f;
    BeginTransitionDist = 1200.0f;
    EndTransitionDist = 200.0f;

    auto vp = Resources->GetScreenViewport();
    unsigned int width = static_cast<size_t>(vp.Width);
    unsigned int height = static_cast<size_t>(vp.Height);

    PostProcess = std::make_unique<CPostProcess>(Device, Context, width, height);
    CommonStates = std::make_unique<DirectX::CommonStates>(Device);
    Planet = std::make_unique<CPlanet>(Context, Camera);

    CreatePlanetPipeline();
}

void PlanetTarget::Render()
{
    Matrix view = Camera->GetViewMatrix();
    Matrix viewProj = view * Camera->GetProjectionMatrix();

    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);
    Parent->GetSkyBox().Draw(viewProj);

    Vector3 lightDir = Parent->GetCentre() - Centre;
    PlanetBuffer->SetData(Context, PlanetConstantBuffer { lightDir, 1.0f });
    Context->PSSetConstantBuffers(0, 1, PlanetBuffer->GetBuffer());
    Context->OMSetBlendState(CommonStates->Opaque(), DirectX::Colors::Black, 0xFFFFFFFF);
    Planet->Render();
}

void PlanetTarget::RenderTransitionChild(float t)
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);

    Planet->Move(ParentLocationSpace);

    Matrix view = Camera->GetViewMatrix();
    Matrix viewProj = view * Camera->GetProjectionMatrix();

    Vector3 lightDir = Parent->GetCentre() - Centre;
    PlanetBuffer->SetData(Context, PlanetConstantBuffer { lightDir, t });
    Context->PSSetConstantBuffers(0, 1, PlanetBuffer->GetBuffer());
    Context->OMSetBlendState(CommonStates->Opaque(), DirectX::Colors::Black, 0xFFFFFFFF);
    Planet->Render();

    Planet->Move(-ParentLocationSpace);
}

void PlanetTarget::RenderTransitionParent(float t)
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);
    Parent->GetSkyBox().Draw(Camera->GetViewMatrix() * Camera->GetProjectionMatrix());

    Matrix view = Camera->GetViewMatrix();
    Matrix viewProj = view * Camera->GetProjectionMatrix();

    Vector3 lightDir = Parent->GetCentre() - Centre;
    PlanetBuffer->SetData(Context, PlanetConstantBuffer { lightDir, t });
    Context->PSSetConstantBuffers(0, 1, PlanetBuffer->GetBuffer());
    Context->OMSetBlendState(CommonStates->Opaque(), DirectX::Colors::Black, 0xFFFFFFFF);
    Planet->Render();
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

void PlanetTarget::CreatePlanetPipeline()
{
    StarPipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    StarPipeline.LoadVertex(L"shaders/Position.vsh");
    StarPipeline.LoadPixel(L"shaders/Star.psh");
    StarPipeline.CreateRasteriser(Device, ECullMode::Clockwise);
    StarPipeline.CreateInputLayout(Device, CreateInputLayoutPosition());

    PlanetBuffer = std::make_unique<ConstantBuffer<PlanetConstantBuffer>>(Device);
}

void PlanetTarget::StateIdle(float dt)
{
    Planet->Update(dt);

    float dist = Vector3::Distance(Camera->GetPosition(), Planet->GetPosition()) - Planet->Radius;
    float t = (dist - 100.0f) / (Planet->Radius * 3.0f);

    VelocityMultiplier = Maths::Clamp(Maths::Lerp(0.01f, 1.0f, t), 0.01f, 1.0f);
}
