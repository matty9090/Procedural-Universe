#include "Galaxy.hpp"
#include "Core/Maths.hpp"
#include "Services/Log.hpp"
#include "Services/ResourceManager.hpp"
#include "Sim/IParticleSeeder.hpp"

#include <random>

using namespace DirectX::SimpleMath;

float Galaxy::ImposterThreshold = 8000.0f;
float Galaxy::ImposterFadeDist = 600.0f;
float Galaxy::ImposterOffsetPercent = 0.4f;

Galaxy::Galaxy(ID3D11DeviceContext* context) : Context(context)
{
    context->GetDevice(&Device);

    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ParticlePipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    ParticlePipeline.LoadVertex(L"shaders/PassThruGS.vsh");
    ParticlePipeline.LoadPixel(L"shaders/Particles/StarParticle.psh");
    ParticlePipeline.LoadGeometry(L"shaders/Particles/GalaxyParticle.gsh");
    ParticlePipeline.CreateRasteriser(Device, ECullMode::Anticlockwise);
    ParticlePipeline.CreateInputLayout(Device, layout);

    CommonStates = std::make_unique<DirectX::CommonStates>(Device);
    GSBuffer = std::make_unique<ConstantBuffer<GSConstantBuffer>>(Device);
    LerpBuffer = std::make_unique<ConstantBuffer<LerpConstantBuffer>>(Device);
    Imposter = std::make_unique<CBillboard>(Context, L"assets/GalaxyImposter.png", Position, 500.0f, Colour);
    StarTexture = RESM.GetTexture(L"assets/StarImposter.png");
}

void Galaxy::InitialSeed(uint64_t seed)
{
    Seed = seed;

    std::default_random_engine gen { static_cast<unsigned int>(Seed) };
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    Colour = Color(dist(gen), dist(gen), dist(gen));

    Imposter->SetTint(Colour);

    Scale(0.001f);
}

void Galaxy::FullSeed()
{
    Particles.resize(1000000);
    CreateParticleBuffer(Device, ParticleBuffer.ReleaseAndGetAddressOf(), Particles);

    const float Variation = 0.12f;

    auto seeder = CreateParticleSeeder(Particles, EParticleSeeder::Galaxy);
    seeder->SetRedDist(Colour.R() - Variation, Colour.R() + Variation);
    seeder->SetGreenDist(Colour.G() - Variation, Colour.G() + Variation);
    seeder->SetBlueDist(Colour.B() - Variation, Colour.B() + Variation);
    seeder->Seed(Seed);
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

void Galaxy::Render(const ICamera& cam, float t, float scale, Vector3 voffset, bool single, bool forceStars)
{
    Context->OMSetDepthStencilState(CommonStates->DepthRead(), 0);

    Matrix view = cam.GetViewMatrix();
    Matrix viewProj = view * cam.GetProjectionMatrix();

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
        Context->PSSetShaderResources(0, 1, StarTexture.GetAddressOf());

        auto sampler = CommonStates->AnisotropicClamp();
        Context->PSSetSamplers(0, 1, &sampler);

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
    Context->OMSetDepthStencilState(CommonStates->DepthDefault(), 0);
}

void Galaxy::RenderImposter(const ICamera& cam)
{
    float dist = ((Vector3::Distance(cam.GetPosition(), Position) - ImposterFadeDist) / ImposterThreshold) - 1.0f;
    float imposterT = Maths::Clamp(dist + ImposterOffsetPercent, 0.0f, 1.0f);

    if (imposterT > 0.0f)
    {
        Context->OMSetDepthStencilState(CommonStates->DepthRead(), 0);
        Context->OMSetBlendState(CommonStates->Additive(), DirectX::Colors::Black, 0xFFFFFFFF);
        Imposter->SetTint(Color(Colour.R(), Colour.G(), Colour.B(), imposterT));
        Imposter->Render(cam);
        Context->OMSetDepthStencilState(CommonStates->DepthDefault(), 0);
    }
}

Vector3 Galaxy::GetClosestObject(Vector3 pos)
{
    return Particles.size() > 0 ? Maths::ClosestParticle(pos, Particles, &CurrentClosestObjectID).Position : Vector3::Zero;
}

void Galaxy::RegenerateBuffer()
{
    if (Particles.size() <= 0)
        return;

    D3D11_MAPPED_SUBRESOURCE mapped;
    Context->Map(ParticleBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, Particles.data(), Particles.size() * sizeof(Particle));
    Context->Unmap(ParticleBuffer.Get(), 0);
}