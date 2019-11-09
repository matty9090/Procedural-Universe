#pragma once

#include <array>
#include <d3d11.h>
#include <functional>
#include <SimpleMath.h>

#include "Render/DX/RenderCommon.hpp"
#include "Render/Cameras/Camera.hpp"

class CSkyboxCamera : public ICamera
{
public:
    CSkyboxCamera();

    void SetPosition(DirectX::SimpleMath::Vector3 p) override { Position = p; }
    DirectX::SimpleMath::Vector3 GetPosition() const override { return Position; }

    DirectX::XMMATRIX GetViewMatrix() const override { return View; }
    DirectX::XMMATRIX GetProjectionMatrix() const override { return Proj; }

    void MakeLookAt(DirectX::SimpleMath::Vector3 target, DirectX::SimpleMath::Vector3 up);

private:
    DirectX::SimpleMath::Matrix View, Proj;
    DirectX::SimpleMath::Vector3 Position;
};

class CSkyboxGenerator
{
public:
    CSkyboxGenerator(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height);

    void SetPosition(DirectX::SimpleMath::Vector3 pos) { Camera.SetPosition(pos); }
    void Render(std::function<void(const ICamera&)> renderFunc);
    
    ID3D11ShaderResourceView* GetTextureTakeOwnership() { return Srv.Detach(); }

private:
    void Generate();

    ID3D11Device* Device;
    ID3D11DeviceContext* Context;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Srv;

    CSkyboxCamera Camera;

    static bool bMadeViews;
    static D3D11_TEXTURE2D_DESC TexArrayDesc;
    static std::array<RenderView, 6> Views;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> TexArray;

    std::array<DirectX::SimpleMath::Vector3, 6> ForwardVectors = {
        DirectX::SimpleMath::Vector3 { -1.0f,  0.0f,  0.0f }, // Face 0
        DirectX::SimpleMath::Vector3 {  1.0f,  0.0f,  0.0f }, // Face 1
        DirectX::SimpleMath::Vector3 {  0.0f,  1.0f,  0.0f }, // Face 2
        DirectX::SimpleMath::Vector3 {  0.0f, -1.0f,  0.0f }, // Face 3
        DirectX::SimpleMath::Vector3 {  0.0f,  0.0f,  1.0f }, // Face 4
        DirectX::SimpleMath::Vector3 {  0.0f,  0.0f, -1.0f }  // Face 5
    };

    std::array<DirectX::SimpleMath::Vector3, 6> UpVectors = {
        DirectX::SimpleMath::Vector3 {  0.0f,  1.0f,  0.0f }, // Face 0 
        DirectX::SimpleMath::Vector3 {  0.0f,  1.0f,  0.0f }, // Face 1
        DirectX::SimpleMath::Vector3 {  0.0f,  0.0f, -1.0f }, // Face 2
        DirectX::SimpleMath::Vector3 {  0.0f,  0.0f,  1.0f }, // Face 3
        DirectX::SimpleMath::Vector3 {  0.0f,  1.0f,  0.0f }, // Face 4
        DirectX::SimpleMath::Vector3 {  0.0f,  1.0f,  0.0f }  // Face 5
    };
};