#include "RingComponent.hpp"
#include "Render/Planet/Planet.hpp"
#include "Services/Log.hpp"
#include "Misc/Shapes.hpp"

#include <random>
#include <imgui.h>

CRingComponent::CRingComponent(CPlanet* planet, uint64_t seed) : Planet(planet)
{
    Pipeline.LoadVertex(L"shaders/Standard/PositionNormalTexture.vsh");
    Pipeline.LoadPixel(L"shaders/Planet/Ring.psh");
    Pipeline.CreateInputLayout(Planet->GetDevice(), CreateInputLayoutPositionNormalTexture());
    Pipeline.CreateDepthState(Planet->GetDevice(), EDepthState::Normal);
    Pipeline.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    VertexCB = std::make_unique<ConstantBuffer<VSBuffer>>(Planet->GetDevice());
    PixelCB = std::make_unique<ConstantBuffer<PSBuffer>>(Planet->GetDevice());

    RingRadius = 1.2f;
}

void CRingComponent::Init()
{
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    Shapes::ComputeTorus(vertices, indices, 2.0f * Planet->Radius * RingRadius, Thickness, 64);
    NumIndices = static_cast<UINT>(indices.size());

    D3D11_BUFFER_DESC buffer;
    buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer.Usage = D3D11_USAGE_DEFAULT;
    buffer.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(Vertex));
    buffer.CPUAccessFlags = 0;
    buffer.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA init;
    init.pSysMem = vertices.data();

    Planet->GetDevice()->CreateBuffer(&buffer, &init, VertexBuffer.ReleaseAndGetAddressOf());

    buffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
    buffer.ByteWidth = NumIndices * sizeof(uint16_t);

    init.pSysMem = indices.data();

    Planet->GetDevice()->CreateBuffer(&buffer, &init, IndexBuffer.ReleaseAndGetAddressOf());
}

void CRingComponent::Update(float dt)
{

}

void CRingComponent::Render(DirectX::SimpleMath::Matrix viewProj)
{
    Pipeline.SetState(Planet->GetContext(), [&]() {
        unsigned int offset = 0;
        unsigned int stride = sizeof(Vertex);

        Planet->GetContext()->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &stride, &offset);
        Planet->GetContext()->IASetIndexBuffer(IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

        VertexCB->SetData(Planet->GetContext(), { Planet->World * viewProj, Planet->World });
        PixelCB->SetData(Planet->GetContext(), {});

        Planet->GetContext()->VSSetConstantBuffers(0, 1, VertexCB->GetBuffer());
        Planet->GetContext()->PSSetConstantBuffers(0, 1, PixelCB->GetBuffer());

        Planet->GetContext()->DrawIndexed(NumIndices, 0, 0);
    });
}

void CRingComponent::RenderUI()
{
    if (ImGui::CollapsingHeader("Rings"))
    {
        if (ImGui::SliderFloat("Radius", &RingRadius, 1.4f, 2.2f)) Init();
        if (ImGui::SliderFloat("Thickness", &Thickness, 1.5f, 5.0f)) Init();
    }
}