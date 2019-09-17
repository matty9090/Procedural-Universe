#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

#include "Core/Vec3.hpp"

struct Particle
{
    DirectX::SimpleMath::Vector3 Position;
    DirectX::SimpleMath::Color Colour;
    DirectX::SimpleMath::Color OriginalColour;

    Vec3d Velocity;
    Vec3d Forces;
    
    double Mass;
};