#include "Cube.hpp"
#include "Core/Except.hpp"
#include "Render/Shader.hpp"

#include <DirectXColors.h>

Cube::Cube(ID3D11DeviceContext* context) : Context(context)
{
    ID3D11Device* device;
    Context->GetDevice(&device);

    Pipeline.LoadVertex(L"shaders/Position.vsh");
    Pipeline.LoadPixel(L"shaders/Tint.psh");
    Pipeline.CreateInputLayout(device, CreateInputLayoutPosition());
    Pipeline.Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

    Vertex vertices[] {
        {{ -1.0f,  1.0f, -1.0f }},
        {{ -1.0f,  1.0f,  1.0f }},
        {{  1.0f,  1.0f,  1.0f }},
        {{  1.0f,  1.0f, -1.0f }},
        {{ -1.0f, -1.0f, -1.0f }},
        {{ -1.0f, -1.0f,  1.0f }},
        {{  1.0f, -1.0f,  1.0f }},
        {{  1.0f, -1.0f, -1.0f }}
    };

    uint16_t indices[] {
        0, 1, 1, 2, 2, 3, 3, 0, // Top
        4, 5, 5, 6, 6, 7, 7, 4, // Bottom
        0, 4, 1, 5, 2, 6, 3, 7  // Middle
    };

    D3D11_BUFFER_DESC buffer;
    buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer.Usage = D3D11_USAGE_DEFAULT;
    buffer.ByteWidth = 8 * sizeof(Vertex);
    buffer.CPUAccessFlags = 0;
    buffer.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA init;
    init.pSysMem = vertices;

    device->CreateBuffer(&buffer, &init, VertexBuffer.ReleaseAndGetAddressOf());

    buffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
    buffer.ByteWidth = 24 * sizeof(uint16_t);

    init.pSysMem = indices;

    device->CreateBuffer(&buffer, &init, IndexBuffer.ReleaseAndGetAddressOf());

    VertexCB = std::make_unique<ConstantBuffer<VSBuffer>>(device);
    PixelCB  = std::make_unique<ConstantBuffer<PSBuffer>>(device);
}

void Cube::Render(DirectX::SimpleMath::Vector3 position, float scale, DirectX::SimpleMath::Matrix viewProj, bool dynamicColour)
{
    Pipeline.SetState(Context, [&]() {
        unsigned int offset = 0;
        unsigned int stride = sizeof(Vertex);

        Context->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &stride, &offset);
        Context->IASetIndexBuffer(IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
        
        DirectX::SimpleMath::Vector4 col(position);

        if (dynamicColour)
        {
            col.Normalize();

            if (col.x < 0.3f && col.y < 0.3f && col.z < 0.3f)
                col = DirectX::Colors::LightSkyBlue;

            col.w = 1.0f;
        }
        else
            col = DirectX::Colors::DarkCyan;

        DirectX::SimpleMath::Matrix world = DirectX::SimpleMath::Matrix::CreateScale(scale / 2.0f) *
            DirectX::SimpleMath::Matrix::CreateTranslation(position);

        VertexCB->SetData(Context, { world * viewProj });
        PixelCB->SetData(Context, { col });

        Context->VSSetConstantBuffers(0, 1, VertexCB->GetBuffer());
        Context->PSSetConstantBuffers(0, 1, PixelCB->GetBuffer());

        Context->DrawIndexed(24, 0, 0);
    });
}