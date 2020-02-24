#pragma once

#include "Render/DX/ConstantBuffer.hpp"
#include "Render/DX/RenderCommon.hpp"

#include <SimpleMath.h>

struct ModelConstantBuffer
{
    DirectX::SimpleMath::Matrix WorldViewProj;
    DirectX::SimpleMath::Matrix World;
};

class CMesh;

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
    void SetPosition(DirectX::SimpleMath::Vector3 p);
    void Rotate(DirectX::SimpleMath::Vector3 r);
    void Scale(float s);
    void SetScale(float s);
    
    void SetTexture(ID3D11ShaderResourceView* tex) { Texture = tex; }
    void Draw(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix viewProj, const RenderPipeline& pipeline);
    void Draw(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix viewProj, DirectX::SimpleMath::Matrix parentWorld, const RenderPipeline& pipeline);

protected:
    void UpdateMatrices();

    DirectX::SimpleMath::Matrix World;
    DirectX::SimpleMath::Vector3 Position;
    DirectX::SimpleMath::Quaternion Rotation;

    float RelativeScale = 1.0f;

private:
    CMesh* Mesh;
    ConstantBuffer<ModelConstantBuffer> MatrixBuffer;
    ID3D11ShaderResourceView* Texture = nullptr;
};