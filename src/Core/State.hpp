#pragma once

#include <d3d11.h>
#include "App/DeviceResources.hpp"

namespace DirectX
{
    class Mouse;
    class Keyboard;
}

struct StateData {};

class IState
{
public:
    virtual void Init(DX::DeviceResources* resources, DirectX::Mouse* mouse, DirectX::Keyboard* keyboard, StateData& data) = 0;
    virtual void Cleanup() = 0;
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;
};