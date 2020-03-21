#pragma once

#include <map>
#include <array>
#include <vector>
#include <memory>
#include <CommonStates.h>
#include <GeometricPrimitive.h>

#include "Render/DX/RenderCommon.hpp"
#include "Render/DX/ConstantBuffer.hpp"
#include "PlanetComponent.hpp"

class CPlanet;

class CRingComponent : public IPlanetComponent
{
public:
    CRingComponent(CPlanet* planet, uint64_t seed = 0);

    void Init() final;
    void Update(float dt) final;
    void Render(DirectX::SimpleMath::Matrix viewProj) final;
    void RenderUI() final;

    std::string GetName() const override { return "Rings"; }

private:
    void AddRing(float diameter, DirectX::SimpleMath::Color col);

    CPlanet* Planet;
    uint64_t Seed;
    int NumRings;
    float RingRadius;
    float Thickness = 0.4f;
    DirectX::SimpleMath::Color BaseColour;

    struct Vertex
    {
        DirectX::SimpleMath::Vector3 Position;
        DirectX::SimpleMath::Vector3 Normal;
        DirectX::SimpleMath::Vector2 UV;
    };

    struct VSBuffer
    {
        DirectX::SimpleMath::Matrix WorldViewProj;
        DirectX::SimpleMath::Matrix World;
    };

    struct PSBuffer
    {
        DirectX::SimpleMath::Color Colour;
    };

    struct Ring
    {
        Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
        DirectX::SimpleMath::Color Colour;
    };

    RenderPipeline Pipeline;
    UINT NumIndices;

    std::unique_ptr<DirectX::CommonStates> CommonStates;
    std::unique_ptr<ConstantBuffer<VSBuffer>> VertexCB;
    std::unique_ptr<ConstantBuffer<PSBuffer>> PixelCB;
    Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
    std::vector<Ring> Rings;
};