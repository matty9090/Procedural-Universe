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
    void Render(DirectX::SimpleMath::Matrix viewProj, float t) final;
    void RenderUI() final;

    std::string GetName() const override { return "Rings"; }

private:
    CPlanet* Planet;
    uint64_t Seed;
    int NumRings;
    float RingRadius;
    float Thickness = 1.6f;
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
        float Radius;
        DirectX::SimpleMath::Color Colour;
    };

    RenderPipeline Pipeline;
    UINT NumIndices;

    std::unique_ptr<DirectX::CommonStates> CommonStates;
    std::unique_ptr<ConstantBuffer<VSBuffer>> VertexCB;
    std::unique_ptr<ConstantBuffer<PSBuffer>> PixelCB;
    Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
    std::vector<Ring> Rings;
};