#include "BruteForce.hpp"
#include "Physics.hpp"
#include "Core/Vec3.hpp"
#include "Services/Log.hpp"
#include "Render/Shader.hpp"

#include <stdlib.h>

using namespace DirectX::SimpleMath;

BruteForce::BruteForce(ID3D11DeviceContext* context)
    : Context(context)
{
    Context->GetDevice(&Device);

    const D3D_SHADER_MACRO defines[] = {
        { "G", Phys::GStr },
        { "SCALE", Phys::StarSystemScaleStr },
        { "S", Phys::SStr },
        { nullptr, nullptr }
    };

    if(!LoadComputeShader(Device, L"shaders/Gravity.csh", ComputeShader.ReleaseAndGetAddressOf(), defines))
    {
        FLog::Get().Log("Could not load compute shader");
    }
}

void BruteForce::Init(std::vector<Particle>& particles)
{
    Particles = &particles;

    CreateShader();
}

void BruteForce::Update(float dt)
{
    for(unsigned int i = 0; i < Particles->size(); ++i)
        (*Particles)[i].Forces = Vec3d();
    
    Context->UpdateSubresource(inBuffer.Get(), 0, NULL, &(*Particles)[0], 0, 0);
    Context->UpdateSubresource(outBuffer.Get(), 0, NULL, &(*Particles)[0], 0, 0);

    unsigned int num = static_cast<unsigned int>(Particles->size());

    Context->CSSetShader(ComputeShader.Get(), 0, 0);
    Context->CSSetShaderResources(0, 1, srv.GetAddressOf());
    Context->CSSetUnorderedAccessViews(0, 1, uav.GetAddressOf(), 0);
    Context->Dispatch(num, num, 1);
    Context->CopyResource(outResBuffer.Get(), outBuffer.Get());

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Context->Map(outResBuffer.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
    memcpy_s(&(*Particles)[0], num * sizeof(Particle), mappedResource.pData, num * sizeof(Particle));
    Context->Unmap(outResBuffer.Get(), 0);

    for(size_t i = 0; i < Particles->size(); ++i)
    {
        auto a = (*Particles)[i].Forces / (*Particles)[i].Mass;
        (*Particles)[i].Velocity += a * dt;

        auto vel = ((*Particles)[i].Velocity * dt) / Phys::StarSystemScale;

        (*Particles)[i].Position += Vector3(static_cast<float>(vel.x),
                                             static_cast<float>(vel.y),
                                             static_cast<float>(vel.z));
    }
}

void BruteForce::CreateShader()
{
    unsigned int stride = static_cast<unsigned int>(sizeof(Particle));
    unsigned int totalSize = static_cast<unsigned int>(Particles->size() * sizeof(Particle));

    D3D11_BUFFER_DESC constantDataDesc;
    constantDataDesc.Usage                  = D3D11_USAGE_DEFAULT;
    constantDataDesc.ByteWidth              = totalSize;
    constantDataDesc.BindFlags              = D3D11_BIND_SHADER_RESOURCE;
    constantDataDesc.CPUAccessFlags         = 0;
    constantDataDesc.StructureByteStride    = stride;
    constantDataDesc.MiscFlags              = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    D3D11_SUBRESOURCE_DATA initialData;
    initialData.pSysMem = &(*Particles)[0];

    Device->CreateBuffer(&constantDataDesc, &initialData, inBuffer.ReleaseAndGetAddressOf());

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                  = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension           = D3D11_SRV_DIMENSION_BUFFEREX;
    srvDesc.BufferEx.FirstElement   = 0;
    srvDesc.BufferEx.Flags          = 0;
    srvDesc.BufferEx.NumElements    = static_cast<unsigned int>(Particles->size());

    Device->CreateShaderResourceView(inBuffer.Get(), &srvDesc, srv.ReleaseAndGetAddressOf() );

    D3D11_BUFFER_DESC outputDesc;
    outputDesc.Usage                = D3D11_USAGE_DEFAULT;
    outputDesc.ByteWidth            = totalSize;
    outputDesc.BindFlags            = D3D11_BIND_UNORDERED_ACCESS;
    outputDesc.CPUAccessFlags       = 0;
    outputDesc.StructureByteStride  = stride;
    outputDesc.MiscFlags            = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    Device->CreateBuffer(&outputDesc, 0, outBuffer.ReleaseAndGetAddressOf());

    outputDesc.Usage            = D3D11_USAGE_STAGING;
    outputDesc.BindFlags        = 0;
    outputDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_READ;

    Device->CreateBuffer(&outputDesc, 0, outResBuffer.ReleaseAndGetAddressOf());

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Buffer.FirstElement     = 0;
    uavDesc.Buffer.Flags            = 0;
    uavDesc.Buffer.NumElements      = static_cast<unsigned int>(Particles->size());
    uavDesc.Format                  = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension           = D3D11_UAV_DIMENSION_BUFFER;

    Device->CreateUnorderedAccessView(outBuffer.Get(), &uavDesc, uav.ReleaseAndGetAddressOf());
}