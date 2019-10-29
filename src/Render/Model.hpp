#pragma once

#include "Mesh.hpp"
#include "ConstantBuffer.hpp"

struct ModelConstantBuffer
{
    DirectX::SimpleMath::Matrix WorldViewProj;
};

class CModel
{
public:
    CModel(ID3D11Device* device, CMesh* mesh);

    DirectX::SimpleMath::Vector3 GetPosition() const { return Position; }

    void Move(DirectX::SimpleMath::Vector3 v);
    void Rotate(DirectX::SimpleMath::Vector3 r);
    void Scale(float s);
    
    void Draw(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix ViewProj, const RenderPipeline& Pipeline);

private:
    CMesh* Mesh;

    DirectX::SimpleMath::Matrix World;
    DirectX::SimpleMath::Vector3 Position;
    DirectX::SimpleMath::Vector3 Rotation;
    float RelativeScale = 1.0f;

    ConstantBuffer<ModelConstantBuffer> MatrixBuffer;
};