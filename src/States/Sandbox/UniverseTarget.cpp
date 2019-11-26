#include "UniverseTarget.hpp"
#include "Sim/IParticleSeeder.hpp"

UniverseTarget::UniverseTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, CShipCamera* camera, ID3D11RenderTargetView* rtv)
    : SandboxTarget(context, "Universal", resources, camera, rtv)
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
    CreateParticlePipeline();
}

void UniverseTarget::Render()
{
    RenderLerp(1.0f);
}

void UniverseTarget::RenderTransitionParent(float t)
{
    RenderLerp(t, true);
}

void UniverseTarget::MoveObjects(Vector3 v)
{
    for (auto& particle : Particles)
        particle.Position += v;

    Centre += v;
    RegenerateBuffer();
}

void UniverseTarget::ScaleObjects(float scale)
{
    for (auto& particle : Particles)
        particle.Position /= scale;

    RegenerateBuffer();
}

Vector3 UniverseTarget::GetClosestObject(Vector3 pos)
{
    return Maths::ClosestParticle(pos, Particles, &CurrentClosestObjectID).Position;
}

void UniverseTarget::RenderLerp(float t, bool single)
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);

    Matrix view = Camera->GetViewMatrix();
    Matrix viewProj = view * Camera->GetProjectionMatrix();

    ParticlePipeline.SetState(Context, [&]() {
        unsigned int offset = 0;
        unsigned int stride = sizeof(Particle);

        LerpBuffer->SetData(Context, LerpConstantBuffer { 1.0f });

        Context->IASetVertexBuffers(0, 1, ParticleBuffer.GetAddressOf(), &stride, &offset);
        GSBuffer->SetData(Context, GSConstantBuffer { viewProj, view.Invert(), Vector3::Zero });
        Context->GSSetConstantBuffers(0, 1, GSBuffer->GetBuffer());
        Context->GSSetConstantBuffers(1, 1, LerpBuffer->GetBuffer());
        Context->OMSetBlendState(CommonStates->Additive(), DirectX::Colors::Black, 0xFFFFFFFF);

        if (single)
        {
            auto pivot1 = static_cast<UINT>(CurrentClosestObjectID);
            auto pivot2 = static_cast<UINT>(Particles.size() - CurrentClosestObjectID - 1);

            Context->Draw(pivot1, 0);
            Context->Draw(pivot2, static_cast<UINT>(CurrentClosestObjectID + 1));

            // Draw object of interest separately to lerp it's size
            LerpBuffer->SetData(Context, LerpConstantBuffer{ t });
            Context->Draw(1, static_cast<UINT>(CurrentClosestObjectID));
        }
        else
        {
            Context->Draw(static_cast<UINT>(Particles.size()), 0);
        }
    });

    Context->GSSetShader(nullptr, 0, 0);
    //Splatting->Render(static_cast<UINT>(Particles.size()), Camera->GetPosition());
}

void UniverseTarget::Seed(uint64_t seed)
{
    Particles.resize(50000);
    CreateParticleBuffer(Device, ParticleBuffer.ReleaseAndGetAddressOf(), Particles);

    auto seeder = CreateParticleSeeder(Particles, EParticleSeeder::Random);
    seeder->Seed(seed);

    ScaleObjects(0.001f);
}

void UniverseTarget::BakeSkybox(Vector3 object)
{
    SkyboxGenerator->Render([&](const ICamera& cam) {
        LerpBuffer->SetData(Context, LerpConstantBuffer { 1.0f });

        Matrix view = cam.GetViewMatrix();
        Matrix viewProj = view * cam.GetProjectionMatrix();

        ParticlePipeline.SetState(Context, [&]() {
            unsigned int offset = 0;
            unsigned int stride = sizeof(Particle);

            Context->IASetVertexBuffers(0, 1, ParticleBuffer.GetAddressOf(), &stride, &offset);
            GSBuffer->SetData(Context, GSConstantBuffer { viewProj, view.Invert(), Vector3::Zero });
            Context->GSSetConstantBuffers(0, 1, GSBuffer->GetBuffer());
            Context->GSSetConstantBuffers(1, 1, LerpBuffer->GetBuffer());
            Context->OMSetBlendState(CommonStates->Additive(), DirectX::Colors::Black, 0xFFFFFFFF);

            auto pivot1 = static_cast<unsigned int>(CurrentClosestObjectID);
            auto pivot2 = static_cast<unsigned int>(Particles.size() - CurrentClosestObjectID - 1);

            Context->Draw(pivot1, 0);
            Context->Draw(pivot2, static_cast<unsigned int>(CurrentClosestObjectID + 1));
        });

        Context->GSSetShader(nullptr, 0, 0);
        //Splatting->Render(static_cast<UINT>(Particles.size()), Camera->GetPosition());
    });
}

void UniverseTarget::CreateParticlePipeline()
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ParticlePipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    ParticlePipeline.LoadVertex(L"shaders/PassThruGS.vsh");
    ParticlePipeline.LoadPixel(L"shaders/PlainColour.psh");
    ParticlePipeline.LoadGeometry(L"shaders/SandboxParticleLerp.gsh");
    ParticlePipeline.CreateRasteriser(Device, ECullMode::None);
    ParticlePipeline.CreateInputLayout(Device, layout);

    GSBuffer = std::make_unique<ConstantBuffer<GSConstantBuffer>>(Device);

    auto vp = Resources->GetScreenViewport();
    ParticleRenderTarget = CreateTarget(Device, static_cast<int>(vp.Width), static_cast<int>(vp.Height));

    LerpBuffer = std::make_unique<ConstantBuffer<LerpConstantBuffer>>(Device);
}

void UniverseTarget::RegenerateBuffer()
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    Context->Map(ParticleBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, Particles.data(), Particles.size() * sizeof(Particle));
    Context->Unmap(ParticleBuffer.Get(), 0);
}
