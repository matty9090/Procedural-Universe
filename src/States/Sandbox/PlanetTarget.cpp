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
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);
    Parent->GetSkyBox().Draw(Camera->GetViewMatrix() * Camera->GetProjectionMatrix());

    Matrix viewProj = Camera->GetViewMatrix() * Camera->GetProjectionMatrix();

    RenderLerp();
}

void PlanetTarget::RenderTransitionChild(float t)
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);

    Planet->Move(ParentLocationSpace);
    RenderLerp(Scale, ParentLocationSpace, t);
    Planet->Move(-ParentLocationSpace);
}

void PlanetTarget::RenderTransitionParent(float t)
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);
    Parent->GetSkyBox().Draw(Camera->GetViewMatrix() * Camera->GetProjectionMatrix());

    RenderLerp(1.0f, Vector3::Zero, t, true);
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

void PlanetTarget::RenderLerp(float scale, Vector3 voffset, float t, bool single)
{
    Matrix view = Camera->GetViewMatrix();
    Matrix viewProj = view * Camera->GetProjectionMatrix();

    /*LerpBuffer->SetData(Context, LerpConstantBuffer { t });
    Context->PSSetConstantBuffers(0, 1, LerpBuffer->GetBuffer());
    Context->GSSetShader(nullptr, 0, 0);
    Context->OMSetBlendState(CommonStates->NonPremultiplied(), DirectX::Colors::Black, 0xFFFFFFFF);*/
    Planet->Render();
}

void PlanetTarget::BakeSkybox(Vector3 object)
{
    SkyboxGenerator->Render([&](const ICamera& cam) {
        LerpBuffer->SetData(Context, LerpConstantBuffer{ 1.0f });

        Matrix view = cam.GetViewMatrix();
        Matrix viewProj = view * cam.GetProjectionMatrix();
        
        Context->OMSetBlendState(CommonStates->Opaque(), DirectX::Colors::Black, 0xFFFFFFFF);
        Parent->GetSkyBox().Draw(viewProj);
    });
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

    LerpBuffer = std::make_unique<ConstantBuffer<LerpConstantBuffer>>(Device);
}

void PlanetTarget::StateIdle(float dt)
{
    Planet->Update(dt);

    float dist = Vector3::Distance(Camera->GetPosition(), Planet->GetPosition()) - Planet->Radius;
    float t = (dist - 100.0f) / (Planet->Radius * 3.0f);

    VelocityMultiplier = Maths::Clamp(Maths::Lerp(0.01f, 1.0f, t), 0.01f, 1.0f);
}
