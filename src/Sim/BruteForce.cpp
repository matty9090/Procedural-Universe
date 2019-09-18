#include "BruteForce.hpp"
#include "Physics.hpp"
#include "Core/Vec3.hpp"
#include "Services/Log.hpp"

#include "Render/Shader.hpp"
#include "Render/ConstantBuffer.hpp"

using namespace DirectX::SimpleMath;

BruteForce::BruteForce(ID3D11DeviceContext* context, std::vector<Particle>& particles)
    : Context(context), Particles(particles)
{
    ID3D11Device* device = nullptr;
    context->GetDevice(&device);

    const int num = 20;

    std::vector<TestData> data;
    data.resize(num);

    for(int x = 0; x < num; ++x)
    {
        data[x].i = 200;
    }

    Microsoft::WRL::ComPtr<ID3D11Buffer> inBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> outBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> outResBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uav;

    D3D11_BUFFER_DESC constantDataDesc;
    constantDataDesc.Usage                  = D3D11_USAGE_DYNAMIC;
    constantDataDesc.ByteWidth              = sizeof(TestData) * num;
    constantDataDesc.BindFlags              = D3D11_BIND_SHADER_RESOURCE;
    constantDataDesc.CPUAccessFlags         = D3D11_CPU_ACCESS_WRITE;
    constantDataDesc.StructureByteStride    = sizeof(TestData);
    constantDataDesc.MiscFlags              = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    D3D11_SUBRESOURCE_DATA initialData;
    initialData.pSysMem = &data[0];

    device->CreateBuffer(&constantDataDesc, &initialData, inBuffer.ReleaseAndGetAddressOf());

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                  = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension           = D3D11_SRV_DIMENSION_BUFFEREX;
    srvDesc.BufferEx.FirstElement   = 0;
    srvDesc.BufferEx.Flags          = 0;
    srvDesc.BufferEx.NumElements    = num;

    device->CreateShaderResourceView(inBuffer.Get(), &srvDesc, srv.ReleaseAndGetAddressOf() );

    D3D11_BUFFER_DESC outputDesc;
    outputDesc.Usage                = D3D11_USAGE_DEFAULT;
    outputDesc.ByteWidth            = sizeof(TestData) * num;
    outputDesc.BindFlags            = D3D11_BIND_UNORDERED_ACCESS;
    outputDesc.CPUAccessFlags       = 0;
    outputDesc.StructureByteStride  = sizeof(TestData);
    outputDesc.MiscFlags            = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    device->CreateBuffer(&outputDesc, 0, outBuffer.ReleaseAndGetAddressOf());

    outputDesc.Usage            = D3D11_USAGE_STAGING;
    outputDesc.BindFlags        = 0;
    outputDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_READ;

    device->CreateBuffer(&outputDesc, 0, outResBuffer.ReleaseAndGetAddressOf());

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Buffer.FirstElement     = 0;
    uavDesc.Buffer.Flags            = 0;
    uavDesc.Buffer.NumElements      = num;
    uavDesc.Format                  = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension           = D3D11_UAV_DIMENSION_BUFFER;

    device->CreateUnorderedAccessView(outBuffer.Get(), &uavDesc, uav.ReleaseAndGetAddressOf());

    if(!LoadComputeShader(device, L"shaders/Test.csh", ComputeShader.ReleaseAndGetAddressOf()))
        FLog::Get().Log("Could not load compute shader");

    context->CSSetShader(ComputeShader.Get(), 0, 0);
    context->CSSetShaderResources(0, 1, srv.GetAddressOf());
    context->CSSetUnorderedAccessViews(0, 1, uav.GetAddressOf(), 0);
    context->Dispatch(1, 1, 1);
    context->CopyResource(outResBuffer.Get(), outBuffer.Get());

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    context->Map(outResBuffer.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);

    TestData* d = reinterpret_cast<TestData*>(mappedResource.pData);

    for(int i = 0; i < num; ++i)
        FLog::Get().Log(d->i);

    context->Unmap(outResBuffer.Get(), 0);
}

void BruteForce::Update(float dt)
{
    for(auto& particle : Particles)
        particle.Forces = Vec3d();

    for(int i = 0; i < Particles.size(); ++i)
    {
        for(int j = 0; j < Particles.size(); ++j)
        {
            if(i == j) continue;
            
            auto& a = Particles[i];
            auto& b = Particles[j];

            auto diff = (b.Position - a.Position);
            auto len = diff.Length();

            double f = Phys::Gravity(a, b);

            b.Forces += Vec3d(f * diff.x / len, f * diff.y / len, f * diff.z / len);
        }
    }

    for(auto& particle : Particles)
    {
        auto a = particle.Forces / particle.Mass;
        particle.Velocity += a * dt;

        auto vel = (particle.Velocity * dt) / Phys::StarSystemScale;

        particle.Position += Vector3(static_cast<float>(vel.x),
                                     static_cast<float>(vel.y),
                                     static_cast<float>(vel.z));
    }
}