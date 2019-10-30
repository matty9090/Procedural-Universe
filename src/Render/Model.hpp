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

    float GetScale() const { return RelativeScale; }

    DirectX::SimpleMath::Vector3 GetUp() const { return World.Up(); }
    DirectX::SimpleMath::Vector3 GetRight() const { return World.Right(); }
    DirectX::SimpleMath::Vector3 GetForward() const { return World.Forward(); }
    DirectX::SimpleMath::Vector3 GetPosition() const { return Position; }
    DirectX::SimpleMath::Matrix  GetMatrix() const { return World; }

    void Move(DirectX::SimpleMath::Vector3 v);
    void Rotate(DirectX::SimpleMath::Vector3 r);
    void Scale(float s);
    
    void Draw(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix ViewProj, const RenderPipeline& Pipeline);

protected:
    DirectX::SimpleMath::Matrix World;
    DirectX::SimpleMath::Vector3 Position;
    DirectX::SimpleMath::Vector3 Rotation;

    float RelativeScale = 1.0f;

private:
    CMesh* Mesh;

    ConstantBuffer<ModelConstantBuffer> MatrixBuffer;
};