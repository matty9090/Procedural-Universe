#pragma once

#include "RenderCommon.hpp"
#include "Model.hpp"

#include <CommonStates.h>

class CSkyBox
{
public:
    CSkyBox(ID3D11DeviceContext* context);

    void Draw(DirectX::SimpleMath::Matrix viewProj);
    void SetTextureReceiveOwnership(ID3D11ShaderResourceView* tex);
    void SetPosition(DirectX::SimpleMath::Vector3 position) { Sphere->SetPosition(position); }
    void Move(DirectX::SimpleMath::Vector3 v) { Sphere->Move(v); }

private:
    struct Vertex
    {
        DirectX::SimpleMath::Vector3 Position;
        DirectX::SimpleMath::Vector2 TexCoord;
    };

    ID3D11DeviceContext* Context;

    RenderPipeline Pipeline;

    std::unique_ptr<CModel> Sphere;
    std::unique_ptr<DirectX::CommonStates> CommonStates;

    Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer, IndexBuffer;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> Sampler;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Texture;
};