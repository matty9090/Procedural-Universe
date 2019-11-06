#include "StarTarget.hpp"

#include "Services/Log.hpp"
#include "Services/ResourceManager.hpp"

StarTarget::StarTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, CShipCamera* camera, const std::vector<Particle>& seedData)
    : SandboxTarget(context, "Stellar", resources, camera),
      Particles(seedData)
{
    Scale = 0.01f;
    BeginTransitionDist = 3000.0f;
    EndTransitionDist = 340.0f;

    auto vp = Resources->GetScreenViewport();
    unsigned int width = static_cast<size_t>(vp.Width);
    unsigned int height = static_cast<size_t>(vp.Height);

    PostProcess = std::make_unique<CPostProcess>(Device, Context, width, height);
    CommonStates = std::make_unique<DirectX::CommonStates>(Device);

    CreateStarPipeline();
    CreateParticlePipeline();
}

void StarTarget::Render()
{
    auto rtv = Resources->GetRenderTargetView();
    auto dsv = Resources->GetDepthStencilView();

    Context->OMSetRenderTargets(1, &rtv, dsv);
    Parent->GetSkyBox().Draw(Camera->GetViewMatrix() * Camera->GetProjectionMatrix());

    RenderLerp();
}

void StarTarget::RenderTransitionChild(float t)
{
    Star->Move(ParentLocationSpace);
    RenderLerp(Scale, ParentLocationSpace, t);
    Star->Move(-ParentLocationSpace);
}

void StarTarget::RenderTransitionParent(float t)
{
    RenderLerp(1.0f, Vector3::Zero, t);
}

void StarTarget::MoveObjects(Vector3 v)
{
    for (auto& particle : Particles)
        particle.Position += v;

    Star->Move(v);

    D3D11_MAPPED_SUBRESOURCE mapped;
    Context->Map(ParticleBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, Particles.data(), Particles.size() * sizeof(Particle));
    Context->Unmap(ParticleBuffer.Get(), 0);
}

void StarTarget::ScaleObjects(float scale)
{
    for (auto& particle : Particles)
        particle.Position /= scale;

    Star->Scale(1.0f / scale);

    D3D11_MAPPED_SUBRESOURCE mapped;
    Context->Map(ParticleBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, Particles.data(), Particles.size() * sizeof(Particle));
    Context->Unmap(ParticleBuffer.Get(), 0);
}

Vector3 StarTarget::GetClosestObject(Vector3 pos)
{
    return Maths::ClosestParticle(pos, Particles).Position;
}

Vector3 StarTarget::GetMainObject() const
{
    return Star->GetPosition();
}

void StarTarget::StateIdle()
{
    
}

void StarTarget::RenderLerp(float scale, Vector3 voffset, float t)
{
    auto rtv = Resources->GetRenderTargetView();
    auto dsv = Resources->GetDepthStencilView();

    Context->OMSetRenderTargets(1, &rtv, dsv);

    Matrix view = Camera->GetViewMatrix();
    Matrix viewProj = view * Camera->GetProjectionMatrix();
    view = view.Invert();
    view *= Matrix::CreateScale(scale);

    ParticlePipeline.SetState(Context, [&]() {
        unsigned int offset = 0;
        unsigned int stride = sizeof(Particle);

        Context->IASetVertexBuffers(0, 1, ParticleBuffer.GetAddressOf(), &stride, &offset);
        GSBuffer->SetData(Context, GSConstantBuffer { viewProj, view, voffset });
        Context->GSSetConstantBuffers(0, 1, GSBuffer->GetBuffer());
        Context->GSSetConstantBuffers(1, 1, LerpBuffer->GetBuffer());
        Context->RSSetState(CommonStates->CullNone());
        Context->OMSetBlendState(CommonStates->Additive(), DirectX::Colors::Black, 0xFFFFFFFF);
        Context->Draw(static_cast<unsigned int>(Particles.size()), 0);
    });

    LerpBuffer->SetData(Context, LerpConstantBuffer { 1.0f });
    Context->PSSetConstantBuffers(0, 1, LerpBuffer->GetBuffer());
    Context->GSSetShader(nullptr, 0, 0);
    Context->RSSetState(CommonStates->CullClockwise());
    Context->OMSetBlendState(CommonStates->NonPremultiplied(), DirectX::Colors::Black, 0xFFFFFFFF);
    Star->Draw(Context, viewProj, StarPipeline);
}

void StarTarget::ResetObjectPositions()
{
    MoveObjects(-Star->GetPosition());
    Star->SetPosition(Vector3::Zero);
}

void StarTarget::CreateStarPipeline()
{
    Star = std::make_unique<CModel>(Device, RESM.GetMesh("assets/Sphere.obj"));
    Star->SetPosition(Vector3::Zero);
    Star->Scale(300.0f);

    StarPipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    StarPipeline.LoadVertex(L"shaders/Position.vsh");
    StarPipeline.LoadPixel(L"shaders/Star.psh");
    StarPipeline.CreateInputLayout(Device, CreateInputLayoutPosition());

    LerpBuffer = std::make_unique<ConstantBuffer<LerpConstantBuffer>>(Device);
}

void StarTarget::CreateParticlePipeline()
{
    ParticlePipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    ParticlePipeline.LoadVertex(L"shaders/PassThruGS.vsh");
    ParticlePipeline.LoadPixel(L"shaders/PlainColour.psh");
    ParticlePipeline.LoadGeometry(L"shaders/SandboxParticleLerp.gsh");
    ParticlePipeline.CreateInputLayout(Device, CreateInputLayoutPositionColour());

    CreateParticleBuffer(Device, ParticleBuffer.ReleaseAndGetAddressOf(), Particles);
    GSBuffer = std::make_unique<ConstantBuffer<GSConstantBuffer>>(Device);

    auto vp = Resources->GetScreenViewport();
    ParticleRenderTarget = CreateTarget(Device, static_cast<int>(vp.Width), static_cast<int>(vp.Height));
}
