#include "Galaxy.hpp"
#include "Core/Maths.hpp"
#include "Services/Log.hpp"
#include "Sim/IParticleSeeder.hpp"

using namespace DirectX::SimpleMath;

float Galaxy::ImposterThreshold = 12000.0f;
float Galaxy::ImposterFadeDist = 600.0f;
float Galaxy::ImposterOffsetPercent = 0.4f;

Galaxy::Galaxy(ID3D11DeviceContext* context)
    : Context(context),
      Colour(Maths::RandFloat(), Maths::RandFloat(), Maths::RandFloat(), 1.0f)
{
    context->GetDevice(&Device);

    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ParticlePipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    ParticlePipeline.LoadVertex(L"shaders/PassThruGS.vsh");
    ParticlePipeline.LoadPixel(L"shaders/Standard/PlainColour.psh");
    ParticlePipeline.LoadGeometry(L"shaders/Particles/GalaxyParticle.gsh");
    ParticlePipeline.CreateRasteriser(Device, ECullMode::None);
    ParticlePipeline.CreateInputLayout(Device, layout);

    CommonStates = std::make_unique<DirectX::CommonStates>(Device);
    GSBuffer = std::make_unique<ConstantBuffer<GSConstantBuffer>>(Device);
    LerpBuffer = std::make_unique<ConstantBuffer<LerpConstantBuffer>>(Device);
    Imposter = std::make_unique<CBillboard>(Context, L"assets/GalaxyImposter.png", Position, 500.0f, Colour);
}

void Galaxy::Seed(uint64_t seed)
{
    Particles.resize(1000);
    CreateParticleBuffer(Device, ParticleBuffer.ReleaseAndGetAddressOf(), Particles);

    const float Variation = 0.26f;

    auto seeder = CreateParticleSeeder(Particles, EParticleSeeder::Galaxy);
    seeder->SetRedDist(Colour.R() - Variation, Colour.R() + Variation);
    seeder->SetGreenDist(Colour.G() - Variation, Colour.G() + Variation);
    seeder->SetBlueDist(Colour.B() - Variation, Colour.B() + Variation);
    seeder->Seed(seed);

    Scale(0.001f);
}

void Galaxy::Move(Vector3 v)
{
    for (auto& particle : Particles)
        particle.Position += v;

    Position += v;
    Imposter->SetPosition(Position);
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

    float dist = ((Vector3::Distance(cam.GetPosition(), Position) - ImposterFadeDist) / ImposterThreshold) - 1.0f;
    float imposterT = Maths::Clamp(dist + ImposterOffsetPercent, 0.0f, 1.0f);
    float galaxyT = Maths::Clamp(1.0f - dist, 0.0f, 1.0f);

    if (galaxyT > 0.0f)
    {
        view = view.Invert();
        view *= Matrix::CreateScale(scale);

        ParticlePipeline.SetState(Context, [&]() {
            unsigned int offset = 0;
            unsigned int stride = sizeof(Particle);

            LerpBuffer->SetData(Context, LerpConstantBuffer { galaxyT });

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

                LerpBuffer->SetData(Context, LerpConstantBuffer { t });
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
    
    if (imposterT > 0.0f)
    {
        Context->OMSetBlendState(CommonStates->NonPremultiplied(), DirectX::Colors::Black, 0xFFFFFFFF);
        Imposter->SetTint(Color(Colour.R(), Colour.G(), Colour.B(), imposterT));
        Imposter->Render(cam);
    }
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