#pragma once

#include <d3d11.h>
#include "App/DeviceResources.hpp"

class IState
{
public:
    virtual void Init(DX::DeviceResources* resources) = 0;
    virtual void Cleanup() = 0;
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;
};