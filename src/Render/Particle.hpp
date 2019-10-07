#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

#include "Core/Vec3.hpp"

using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Color;

struct Particle
{
    // 16 bytes
    Vector3 Position;
    float Mass;

    // 16 bytes
    Color Colour;

    // 16 bytes
    Vector3 Velocity;
    float Custom1;

    // 16 bytes
    Vector3 Accel;
    float Custom2;

	// 16 bytes
	Color OriginalColour;
};