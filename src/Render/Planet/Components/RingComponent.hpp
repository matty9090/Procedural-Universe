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
    CPlanet* Planet;
    float RingRadius;
    float Thickness = 10.0f;

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

    RenderPipeline Pipeline;
    UINT NumIndices;

    Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;

    std::unique_ptr<ConstantBuffer<VSBuffer>> VertexCB;
    std::unique_ptr<ConstantBuffer<PSBuffer>> PixelCB;
};