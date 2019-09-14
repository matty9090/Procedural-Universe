#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

struct Particle
{
    DirectX::SimpleMath::Vector3 Position;
    DirectX::SimpleMath::Vector3 Velocity;
    DirectX::SimpleMath::Vector3 Acceleration;
    
    double Mass;
};