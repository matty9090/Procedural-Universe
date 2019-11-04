#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

class ICamera
{
public:
    virtual void SetPosition(DirectX::SimpleMath::Vector3 p) = 0;
    virtual DirectX::SimpleMath::Vector3 GetPosition() const = 0;

    virtual DirectX::XMMATRIX GetViewMatrix() const = 0;
    virtual DirectX::XMMATRIX GetProjectionMatrix() const = 0;
};