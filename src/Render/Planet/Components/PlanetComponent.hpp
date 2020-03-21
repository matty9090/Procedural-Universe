#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

class IPlanetComponent
{
public:
    virtual void Init() = 0;
    virtual void Update(float dt) = 0;
    virtual void Render(DirectX::SimpleMath::Matrix viewProj, float t) = 0;
    virtual void RenderUI() {}
    virtual std::string GetName() const = 0;
};