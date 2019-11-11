#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

class IPlanetComponent
{
public:
    virtual void Update(float dt) = 0;
    virtual void Render(DirectX::SimpleMath::Matrix viewProj) = 0;
};