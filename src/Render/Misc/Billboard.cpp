#include "Billboard.hpp"
#include "Services/ResourceManager.hpp"

size_t CBillboard::NumInstances = 0;

CBillboard::CBillboard(ID3D11DeviceContext* context, std::wstring tex, bool fades, unsigned int reserve, std::vector<BillboardInstance> instances)
    : Context(context),
      Instances(instances),
      Fades(fades)
{
    ID3D11Device* device;
    Context->GetDevice(&device);

    Texture = FResourceManager::Get().GetTexture(tex);

    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT         , 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    Pipeline.LoadVertex(L"shaders/Misc/Billboard.vsh");
    Pipeline.LoadGeometry(L"shaders/Misc/Billboard.gsh");
    Pipeline.LoadPixel(L"shaders/Misc/Billboard.psh");
    Pipeline.CreateInputLayout(device, layout);
    Pipeline.CreateDepthState(device, EDepthState::Read);
    Pipeline.Topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

    D3D11_BUFFER_DESC buffer;
    buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer.Usage = D3D11_USAGE_DYNAMIC;
    buffer.ByteWidth = sizeof(BillboardInstance) * (instances.size() <= 0 ? reserve : static_cast<UINT>(instances.size()));
    buffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    buffer.MiscFlags = 0;

    if (instances.size() > 0)
    {
        D3D11_SUBRESOURCE_DATA init;
        init.pSysMem = &instances;
        device->CreateBuffer(&buffer, &init, InstanceBuffer.ReleaseAndGetAddressOf());
    }
    else
    {
        device->CreateBuffer(&buffer, nullptr, InstanceBuffer.ReleaseAndGetAddressOf());
    }    

    VertexCB = std::make_unique<ConstantBuffer<VSBuffer>>(device);
}

CBillboard::~CBillboard()
{

}

void CBillboard::Render(const ICamera& cam)
{
    NumInstances += Instances.size();

    auto inv = static_cast<Matrix>(cam.GetViewMatrix()).Invert();
    auto world = Matrix::CreateScale(RelativeScale) * Matrix::CreateTranslation(Position);
    
    Pipeline.SetState(Context, [&]() {
        unsigned int offset = 0;
        unsigned int stride = sizeof(BillboardInstance);

        VSBuffer buf = {
            world * cam.GetViewMatrix() * cam.GetProjectionMatrix(), inv, cam.GetPosition(), 0.0f,
            Vector3(Fades ? 1.0f : 0.0f, 120.0f, 6000.0f), 0.0f
        };

        Context->IASetVertexBuffers(0, 1, InstanceBuffer.GetAddressOf(), &stride, &offset);
        VertexCB->SetData(Context, buf);
        
        Context->GSSetConstantBuffers(0, 1, VertexCB->GetBuffer());
        Context->PSSetShaderResources(0, 1, Texture.GetAddressOf());

        Context->Draw(static_cast<UINT>(Instances.size()), 0);
    });

    Context->GSSetShader(nullptr, 0, 0);
}

void CBillboard::Render(const ICamera& cam, float scale, Vector3 offset)
{
    NumInstances += Instances.size();

    auto inv = static_cast<Matrix>(cam.GetViewMatrix()).Invert(); 
    auto world = Matrix::CreateScale(RelativeScale) * Matrix::CreateTranslation(Position + offset);

    inv *= Matrix::CreateScale(scale);

    Pipeline.SetState(Context, [&]() {
        unsigned int offset = 0;
        unsigned int stride = sizeof(BillboardInstance);

        VSBuffer buf = {
            world * cam.GetViewMatrix() * cam.GetProjectionMatrix(), inv, cam.GetPosition(), 0.0f,
            Vector3(Fades ? 1.0f : 0.0f, 1.2f * scale, 60.0f * scale), 0.0f
        };

        Context->IASetVertexBuffers(0, 1, InstanceBuffer.GetAddressOf(), &stride, &offset);
        VertexCB->SetData(Context, buf);

        Context->GSSetConstantBuffers(0, 1, VertexCB->GetBuffer());
        Context->PSSetShaderResources(0, 1, Texture.GetAddressOf());

        Context->Draw(static_cast<UINT>(Instances.size()), 0);
    });

    Context->GSSetShader(nullptr, 0, 0);
}

void CBillboard::SetPosition(Vector3 pos)
{
    Position = pos;
}

void CBillboard::UpdateInstances(std::vector<BillboardInstance> instances)
{
    Instances = instances;

    D3D11_MAPPED_SUBRESOURCE mapped;
    Context->Map(InstanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, Instances.data(), Instances.size() * sizeof(BillboardInstance));
    Context->Unmap(InstanceBuffer.Get(), 0);
}
