#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include <memory>
#include <wrl/client.h>
#include <Render/ConstantBuffer.hpp>

class Cube
{
public:
    Cube(ID3D11DeviceContext* context);

    void Render(DirectX::SimpleMath::Vector3 position, float scale, DirectX::SimpleMath::Matrix viewProj);

private:
    struct Vertex
    {
        DirectX::SimpleMath::Vector3 Position;
        DirectX::SimpleMath::Color Colour;
    };

    struct VSBuffer
    {
        DirectX::SimpleMath::Matrix ViewProj;
    };

    struct PSBuffer
    {
        DirectX::SimpleMath::Color Colour;
    };

    ID3D11DeviceContext* Context;
    
    Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> Layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;

    std::unique_ptr<ConstantBuffer<VSBuffer>> VertexCB;
    std::unique_ptr<ConstantBuffer<PSBuffer>> PixelCB;
};