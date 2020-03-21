#include "RingComponent.hpp"
#include "Render/Planet/Planet.hpp"
#include "Services/Log.hpp"
#include "Misc/Shapes.hpp"
#include "Misc/ProcUtils.hpp"

#include <random>
#include <imgui.h>

CRingComponent::CRingComponent(CPlanet* planet, uint64_t seed) : Planet(planet), Seed(seed)
{
    Pipeline.LoadVertex(L"shaders/Standard/PositionNormalTexture.vsh");
    Pipeline.LoadPixel(L"shaders/Planet/Ring.psh");
    Pipeline.CreateInputLayout(Planet->GetDevice(), CreateInputLayoutPositionNormalTexture());
    Pipeline.CreateDepthState(Planet->GetDevice(), EDepthState::Normal);
    Pipeline.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    CommonStates = std::make_unique<DirectX::CommonStates>(Planet->GetDevice());
    VertexCB = std::make_unique<ConstantBuffer<VSBuffer>>(Planet->GetDevice());
    PixelCB = std::make_unique<ConstantBuffer<PSBuffer>>(Planet->GetDevice());

    std::default_random_engine gen{ static_cast<unsigned int>(Seed) };
    std::uniform_int_distribution<int> numDist(60, 900);
    std::uniform_real_distribution<float> radDist(1.4f, 1.6f);

    NumRings = numDist(gen);
    RingRadius = radDist(gen);
    BaseColour = ProcUtils::RandomColour(gen);
    BaseColour.A(0.1f);
}

void CRingComponent::Init()
{
    Rings.clear();

    std::default_random_engine gen { static_cast<unsigned int>(Seed) };
    std::uniform_real_distribution<float> radSep(0.24f, 0.32f);
    std::uniform_real_distribution<float> colDist(-0.1f, 0.1f);
    std::uniform_int_distribution<int> gapDist(0, 60);
    std::uniform_real_distribution<float> gapSizeDist(7.0f, 12.0f);

    float r = 2.0f * Planet->Radius * RingRadius;    

    for (int i = 0; i < NumRings; ++i)
    {
        auto col = BaseColour;
        col.x += colDist(gen);
        col.y += colDist(gen);
        col.z += colDist(gen);

        AddRing(r, col);
        r += radSep(gen);

        if (gapDist(gen) == 0)
        {
            r += gapSizeDist(gen);
        }
    }
}

void CRingComponent::AddRing(float diameter, DirectX::SimpleMath::Color col)
{
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    Ring ring;

    Shapes::ComputeTorus(vertices, indices, diameter * RingRadius, Thickness, 128);
    NumIndices = static_cast<UINT>(indices.size());

    D3D11_BUFFER_DESC buffer;
    buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer.Usage = D3D11_USAGE_DEFAULT;
    buffer.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(Vertex));
    buffer.CPUAccessFlags = 0;
    buffer.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA init;
    init.pSysMem = vertices.data();

    Planet->GetDevice()->CreateBuffer(&buffer, &init, ring.VertexBuffer.ReleaseAndGetAddressOf());

    // Index buffer is shared among all rings, only create once
    if (!IndexBuffer.Get())
    {
        buffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
        buffer.ByteWidth = NumIndices * sizeof(uint16_t);

        init.pSysMem = indices.data();

        Planet->GetDevice()->CreateBuffer(&buffer, &init, IndexBuffer.ReleaseAndGetAddressOf());
    }

    ring.Colour = col;
    Rings.push_back(ring);
}

void CRingComponent::Update(float dt)
{

}

void CRingComponent::Render(DirectX::SimpleMath::Matrix viewProj)
{
    Pipeline.SetState(Planet->GetContext(), [&]() {
        unsigned int offset = 0;
        unsigned int stride = sizeof(Vertex);

        Planet->GetContext()->OMSetBlendState(CommonStates->NonPremultiplied(), DirectX::Colors::Black, 0xFFFFFFFF);
        Planet->GetContext()->IASetIndexBuffer(IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
        VertexCB->SetData(Planet->GetContext(), { Planet->World * viewProj, Planet->World });
        Planet->GetContext()->VSSetConstantBuffers(0, 1, VertexCB->GetBuffer());

        for (auto& ring : Rings)
        {
            Planet->GetContext()->IASetVertexBuffers(0, 1, ring.VertexBuffer.GetAddressOf(), &stride, &offset);
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
        if (ImGui::SliderInt("Num", &NumRings, 1, 1000)) Init();
        if (ImGui::SliderFloat("Radius", &RingRadius, 1.4f, 2.2f)) Init();
        if (ImGui::SliderFloat("Thickness", &Thickness, 0.2f, 1.6f)) Init();
        if (ImGui::ColorPicker4("Colour", &BaseColour.x)) Init();
    }
}