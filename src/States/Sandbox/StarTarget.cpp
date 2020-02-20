#include "StarTarget.hpp"

#include "Sim/IParticleSeeder.hpp"

#include "Services/Log.hpp"
#include "Services/ResourceManager.hpp"

StarTarget::StarTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, ICamera* camera, ID3D11RenderTargetView* rtv)
    : SandboxTarget(context, "Stellar", "Planet", resources, camera, rtv)
{
    Scale = 0.0005f;
    BeginTransitionDist = 10.0f;
    EndTransitionDist = 0.2f;

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
    RenderParentSkybox();
    RenderLerp();
}

void StarTarget::RenderTransitionChild(float t)
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);

    Star->Move(ParentLocationSpace);
    RenderLerp(Scale, ParentLocationSpace, t, true);
    Star->Move(-ParentLocationSpace);
}

void StarTarget::RenderTransitionParent(float t)
{
    RenderParentSkybox();
    RenderLerp(1.0f, Vector3::Zero, t, true);
}

void StarTarget::MoveObjects(Vector3 v)
{
    for (auto& particle : Particles)
        particle.Position += v;

    Star->Move(v);
    UpdateParticleBuffer();
    Centre += v;
}

void StarTarget::ScaleObjects(float scale)
{
    for (auto& particle : Particles)
        particle.Position /= scale;

    UpdateParticleBuffer();
}

Vector3 StarTarget::GetClosestObject(Vector3 pos)
{
    return Maths::ClosestParticle(pos, Particles, &CurrentClosestObjectID).Position;
}

void StarTarget::RenderLerp(float scale, Vector3 voffset, float t, bool single)
{
    Matrix view = Camera->GetViewMatrix();
    Matrix viewProj = view * Camera->GetProjectionMatrix();

    view = view.Invert();
    view *= Matrix::CreateScale(scale);

    ParticlePipeline.SetState(Context, [&]() {
        unsigned int offset = 0;
        unsigned int stride = sizeof(Particle);

        LerpBuffer->SetData(Context, LerpConstantBuffer { 1.0f });

        Context->IASetVertexBuffers(0, 1, ParticleBuffer.GetAddressOf(), &stride, &offset);
        GSBuffer->SetData(Context, GSConstantBuffer { viewProj, view, voffset });
        Context->GSSetConstantBuffers(0, 1, GSBuffer->GetBuffer());
        Context->GSSetConstantBuffers(1, 1, LerpBuffer->GetBuffer());
        Context->OMSetBlendState(CommonStates->Additive(), DirectX::Colors::Black, 0xFFFFFFFF);

        if (single && CurrentClosestObjectID < Particles.size())
        {
            auto pivot1 = static_cast<UINT>(CurrentClosestObjectID);
            auto pivot2 = static_cast<UINT>(Particles.size() - CurrentClosestObjectID - 1);

            Context->Draw(pivot1, 0);
            Context->Draw(pivot2, static_cast<UINT>(CurrentClosestObjectID + 1));

            // Draw object of interest separately to lerp it's size
            LerpBuffer->SetData(Context, LerpConstantBuffer { t });
            Context->GSSetConstantBuffers(1, 1, LerpBuffer->GetBuffer());
            Context->Draw(1, static_cast<UINT>(CurrentClosestObjectID));
        }
        else
        {
            Context->Draw(static_cast<UINT>(Particles.size()), 0);
        }
    });

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
        
        Context->OMSetBlendState(CommonStates->Opaque(), DirectX::Colors::Black, 0xFFFFFFFF);
        Parent->GetSkyBox().Draw(viewProj);

        ParticlePipeline.SetState(Context, [&]() {
            unsigned int offset = 0;
            unsigned int stride = sizeof(Particle);

            Context->IASetVertexBuffers(0, 1, ParticleBuffer.GetAddressOf(), &stride, &offset);
            GSBuffer->SetData(Context, GSConstantBuffer { viewProj, view.Invert(), Vector3::Zero });
            Context->GSSetConstantBuffers(0, 1, GSBuffer->GetBuffer());
            Context->GSSetConstantBuffers(1, 1, LerpBuffer->GetBuffer());
            Context->RSSetState(CommonStates->CullNone());
            Context->OMSetBlendState(CommonStates->Additive(), DirectX::Colors::Black, 0xFFFFFFFF);

            auto pivot1 = static_cast<unsigned int>(CurrentClosestObjectID);
            auto pivot2 = static_cast<unsigned int>(Particles.size() - CurrentClosestObjectID - 1);

            Context->Draw(pivot1, 0);
            Context->Draw(pivot2, static_cast<unsigned int>(CurrentClosestObjectID + 1));
        });

        Context->GSSetShader(nullptr, 0, 0);
        LerpBuffer->SetData(Context, LerpConstantBuffer { 1.0f });
        Context->PSSetConstantBuffers(0, 1, LerpBuffer->GetBuffer());
        Context->GSSetShader(nullptr, 0, 0);
        Context->OMSetBlendState(CommonStates->NonPremultiplied(), DirectX::Colors::Black, 0xFFFFFFFF);
        Star->Draw(Context, viewProj, StarPipeline);
    });
}

void StarTarget::Seed(uint64_t seed)
{
    Particles.resize(12);
    CreateParticleBuffer(Device, ParticleBuffer.ReleaseAndGetAddressOf(), Particles);

    auto seeder = CreateParticleSeeder(Particles, EParticleSeeder::Random, 2.0f);
    seeder->Seed(seed);
}

void StarTarget::UpdateParticleBuffer()
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    Context->Map(ParticleBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, Particles.data(), Particles.size() * sizeof(Particle));
    Context->Unmap(ParticleBuffer.Get(), 0);
}

void StarTarget::ResetObjectPositions()
{
    MoveObjects(-Star->GetPosition());
    Star->SetPosition(Vector3::Zero);
    Centre = Vector3::Zero;
}

void StarTarget::CreateStarPipeline()
{
    Star = std::make_unique<CModel>(Device, RESM.GetMesh("assets/Sphere.obj"));
    Star->Scale(0.003f);

    StarPipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    StarPipeline.LoadVertex(L"shaders/Standard/Position.vsh");
    StarPipeline.LoadPixel(L"shaders/Particles/Star.psh");
    StarPipeline.CreateRasteriser(Device, ECullMode::Clockwise);
    StarPipeline.CreateInputLayout(Device, CreateInputLayoutPosition());

    LerpBuffer = std::make_unique<ConstantBuffer<LerpConstantBuffer>>(Device);
}

void StarTarget::CreateParticlePipeline()
{
    ParticlePipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    ParticlePipeline.LoadVertex(L"shaders/PassThruGS.vsh");
    ParticlePipeline.LoadPixel(L"shaders/Standard/PlainColour.psh");
    ParticlePipeline.LoadGeometry(L"shaders/Particles/PlanetParticle.gsh");
    ParticlePipeline.CreateRasteriser(Device, ECullMode::None);
    ParticlePipeline.CreateInputLayout(Device, CreateInputLayoutPositionColour());

    GSBuffer = std::make_unique<ConstantBuffer<GSConstantBuffer>>(Device);

    auto vp = Resources->GetScreenViewport();
    ParticleRenderTarget = CreateTarget(Device, static_cast<int>(vp.Width), static_cast<int>(vp.Height));
}
