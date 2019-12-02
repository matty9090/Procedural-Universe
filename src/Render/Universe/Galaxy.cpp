#include "Galaxy.hpp"

#include "Core/Maths.hpp"
#include "Sim/IParticleSeeder.hpp"

using namespace DirectX::SimpleMath;

Galaxy::Galaxy(ID3D11DeviceContext* context) : Context(context)
{
    context->GetDevice(&Device);

    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ParticlePipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    ParticlePipeline.LoadVertex(L"shaders/PassThruGS.vsh");
    ParticlePipeline.LoadPixel(L"shaders/PlainColour.psh");
    ParticlePipeline.LoadGeometry(L"shaders/GalaxyParticle.gsh");
    ParticlePipeline.CreateRasteriser(Device, ECullMode::None);
    ParticlePipeline.CreateInputLayout(Device, layout);

    CommonStates = std::make_unique<DirectX::CommonStates>(Device);
    GSBuffer = std::make_unique<ConstantBuffer<GSConstantBuffer>>(Device);
    LerpBuffer = std::make_unique<ConstantBuffer<LerpConstantBuffer>>(Device);
}

void Galaxy::Seed(uint64_t seed)
{
    Particles.resize(40000);
    CreateParticleBuffer(Device, ParticleBuffer.ReleaseAndGetAddressOf(), Particles);

    auto seeder = CreateParticleSeeder(Particles, EParticleSeeder::Galaxy);
    seeder->Seed(seed);

    Scale(0.001f);
}

void Galaxy::Move(Vector3 v)
{
    for (auto& particle : Particles)
        particle.Position += v;

    Position += v;

    RegenerateBuffer();
}

void Galaxy::Scale(float scale)
{
    for (auto& particle : Particles)
        particle.Position /= scale;

    RegenerateBuffer();
}

void Galaxy::Render(const ICamera& cam, float t, float scale, Vector3 voffset, bool single)
{
    Matrix view = cam.GetViewMatrix();
    Matrix viewProj = view * cam.GetProjectionMatrix();

    view = view.Invert();
    view *= Matrix::CreateScale(scale);

    ParticlePipeline.SetState(Context, [&]() {
        unsigned int offset = 0;
        unsigned int stride = sizeof(Particle);

        LerpBuffer->SetData(Context, LerpConstantBuffer { t });

        Context->IASetVertexBuffers(0, 1, ParticleBuffer.GetAddressOf(), &stride, &offset);
        GSBuffer->SetData(Context, GSConstantBuffer { viewProj, view, voffset });
        Context->GSSetConstantBuffers(0, 1, GSBuffer->GetBuffer());
        Context->GSSetConstantBuffers(1, 1, LerpBuffer->GetBuffer());
        Context->OMSetBlendState(CommonStates->Additive(), DirectX::Colors::Black, 0xFFFFFFFF);

        if (single)
        {
            auto pivot1 = static_cast<UINT>(CurrentClosestObjectID);
            auto pivot2 = static_cast<UINT>(Particles.size() - CurrentClosestObjectID - 1);

            Context->Draw(pivot1, 0);
            Context->Draw(pivot2, static_cast<UINT>(CurrentClosestObjectID + 1));

            LerpBuffer->SetData(Context, LerpConstantBuffer { 1.0f });
            Context->GSSetConstantBuffers(1, 1, LerpBuffer->GetBuffer());
            Context->Draw(1, static_cast<UINT>(CurrentClosestObjectID));
        }
        else
        {
            Context->Draw(static_cast<UINT>(Particles.size()), 0);
        }
    });

    Context->GSSetShader(nullptr, 0, 0);
}

Vector3 Galaxy::GetClosestObject(Vector3 pos)
{
    return Maths::ClosestParticle(pos, Particles, &CurrentClosestObjectID).Position;
}

void Galaxy::RegenerateBuffer()
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    Context->Map(ParticleBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, Particles.data(), Particles.size() * sizeof(Particle));
    Context->Unmap(ParticleBuffer.Get(), 0);
}