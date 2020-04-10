#include "RingComponent.hpp"
#include "Render/Planet/Planet.hpp"
#include "Services/Log.hpp"
#include "Misc/Shapes.hpp"
#include "Misc/ProcUtils.hpp"

#include <random>
#include <imgui.h>

RenderPipeline CRingComponent::Pipeline;

CRingComponent::CRingComponent(CPlanet* planet, uint64_t seed) : Planet(planet), Seed(seed)
{
    CommonStates = std::make_unique<DirectX::CommonStates>(Planet->GetDevice());
    VertexCB = std::make_unique<ConstantBuffer<VSBuffer>>(Planet->GetDevice());
    PixelCB = std::make_unique<ConstantBuffer<PSBuffer>>(Planet->GetDevice());

    std::default_random_engine gen{ static_cast<unsigned int>(Seed) };
    std::uniform_int_distribution<int> numDist(8, 200);
    std::uniform_real_distribution<float> radDist(1.4f, 1.6f);

    NumRings = numDist(gen);
    RingRadius = radDist(gen);
    BaseColour = ProcUtils::RandomColour(gen);
    BaseColour.A(0.1f);
}

void CRingComponent::LoadCache(ID3D11Device* device)
{
    Pipeline.LoadVertex(L"shaders/Standard/PositionNormalTexture.vsh");
    Pipeline.LoadPixel(L"shaders/Planet/Ring.psh");
    Pipeline.CreateInputLayout(device, CreateInputLayoutPositionNormalTexture());
    Pipeline.CreateDepthState(device, EDepthState::Normal);
    Pipeline.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

void CRingComponent::Init()
{
    Rings.clear();

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    
    Shapes::ComputeTorus(vertices, indices, 2.0f, 0.06f, 160);

    CreateVertexBuffer(Planet->GetDevice(), vertices, VertexBuffer.ReleaseAndGetAddressOf());
    CreateIndexBuffer(Planet->GetDevice(), indices, IndexBuffer.ReleaseAndGetAddressOf());

    NumIndices = static_cast<UINT>(indices.size());

    std::default_random_engine gen { static_cast<unsigned int>(Seed) };
    std::uniform_real_distribution<float> radSep(0.54f, 0.72f);
    std::uniform_real_distribution<float> colDist(-0.1f, 0.1f);
    std::uniform_int_distribution<int> gapDist(0, 60);
    std::uniform_real_distribution<float> gapSizeDist(7.0f, 12.0f);

    float r = Planet->Radius * RingRadius;

    for (int i = 0; i < NumRings; ++i)
    {
        auto col = BaseColour;
        col.x += colDist(gen);
        col.y += colDist(gen);
        col.z += colDist(gen);

        Ring ring = { r + Thickness, col };
        r += radSep(gen);

        if (gapDist(gen) == 0)
        {
            r += gapSizeDist(gen);
        }

        Rings.push_back(ring);
    }
}

void CRingComponent::Update(float dt)
{

}

void CRingComponent::Render(DirectX::SimpleMath::Matrix viewProj, float t)
{
    Pipeline.SetState(Planet->GetContext(), [&]() {
        unsigned int offset = 0;
        unsigned int stride = sizeof(Vertex);

        Planet->GetContext()->OMSetBlendState(CommonStates->NonPremultiplied(), DirectX::Colors::Black, 0xFFFFFFFF);
        Planet->GetContext()->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &stride, &offset);
        Planet->GetContext()->IASetIndexBuffer(IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
        Planet->GetContext()->VSSetConstantBuffers(0, 1, VertexCB->GetBuffer());

        for (auto& ring : Rings)
        {
            auto world = DirectX::SimpleMath::Matrix::CreateScale(ring.Radius, Thickness, ring.Radius) * Planet->World;
            VertexCB->SetData(Planet->GetContext(), { world * viewProj, world });
            PixelCB->SetData(Planet->GetContext(), { ring.Colour });
            Planet->GetContext()->PSSetConstantBuffers(0, 1, PixelCB->GetBuffer());
            Planet->GetContext()->DrawIndexed(NumIndices, 0, 0);
        }
    });
}

void CRingComponent::RenderUI()
{
    if (ImGui::CollapsingHeader("Rings"))
    {
        if (ImGui::SliderInt("Num", &NumRings, 1, 200)) Init();
        if (ImGui::SliderFloat("Radius", &RingRadius, 1.4f, 2.2f)) Init();
        if (ImGui::SliderFloat("Thickness", &Thickness, 0.2f, 6.0f)) Init();
        if (ImGui::ColorPicker4("Colour", &BaseColour.x)) Init();
    }
}