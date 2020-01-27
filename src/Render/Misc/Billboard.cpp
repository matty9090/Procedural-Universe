#include "Billboard.hpp"
#include "Services/ResourceManager.hpp"

CBillboard::CBillboard(ID3D11DeviceContext* context, std::wstring tex, Vector3 pos, float scale, Color tint)
    : Context(context),
      Scale(scale),
      Tint(tint)
{
    ID3D11Device* device;
    Context->GetDevice(&device);

    Texture = FResourceManager::Get().GetTexture(tex);

    Pipeline.LoadVertex(L"shaders/Misc/Billboard.vsh");
    Pipeline.LoadPixel(L"shaders/Misc/Billboard.psh");
    Pipeline.CreateInputLayout(device, CreateInputLayoutPositionTexture());
    Pipeline.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    Vertex vertices[] {
        {{  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f }},
        {{ -0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f }},
        {{  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f }},
        {{ -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f }}
    };

    uint16_t indices[] {
        0, 1, 2, 2, 1, 3
    };

    D3D11_BUFFER_DESC buffer;
    buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer.Usage = D3D11_USAGE_DEFAULT;
    buffer.ByteWidth = 4 * sizeof(Vertex);
    buffer.CPUAccessFlags = 0;
    buffer.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA init;
    init.pSysMem = vertices;

    device->CreateBuffer(&buffer, &init, VertexBuffer.ReleaseAndGetAddressOf());

    buffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
    buffer.ByteWidth = 6 * sizeof(uint16_t);

    init.pSysMem = indices;

    device->CreateBuffer(&buffer, &init, IndexBuffer.ReleaseAndGetAddressOf());

    VertexCB = std::make_unique<ConstantBuffer<VSBuffer>>(device);
    PixelCB = std::make_unique<ConstantBuffer<Color>>(device);
    PixelCB->SetData(Context, Tint);
}

CBillboard::~CBillboard()
{

}

void CBillboard::Render(const ICamera& cam)
{
    auto inv = static_cast<Matrix>(cam.GetViewMatrix());
    World = Matrix::CreateBillboard(Position, cam.GetPosition(), inv.Up(), &inv.Forward());

    Pipeline.SetState(Context, [&]() {
        unsigned int offset = 0;
        unsigned int stride = sizeof(Vertex);

        Context->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &stride, &offset);
        Context->IASetIndexBuffer(IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

        VertexCB->SetData(Context, { World * cam.GetViewMatrix() * cam.GetProjectionMatrix(), Scale });

        Context->VSSetConstantBuffers(0, 1, VertexCB->GetBuffer());
        Context->PSSetConstantBuffers(0, 1, PixelCB->GetBuffer());
        Context->PSSetShaderResources(0, 1, Texture.GetAddressOf());

        Context->DrawIndexed(6, 0, 0);
    });
}
