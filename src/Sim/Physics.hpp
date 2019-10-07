#pragma once

#include "Render/Particle.hpp"

using namespace DirectX::SimpleMath;

namespace Phys
{
    const float G = 0.1f; // Gravitational constant
    const float S = 10.0f; // Softener
    const float M = 1000; // Meter (in this programs's unit)

    const float AU = 1.15e12f; // Astronomical units
    const float LY = 9.46e15f; // Light year

    const float StarSystemScale = 20.0f * AU;
    const float GalaxyScale = 1e6f * LY;

    const char GStr[] = "6.674e-11";
    const char SStr[] = "1e4";

    const char StarSystemScaleStr[] = "2.3e13";
    const char GalaxyScaleStr[] = "9.46e21";

    __forceinline float Gravity(const Particle& a, const Particle& b)
    {
        float d = Vector3::DistanceSquared(a.Position, b.Position);
        return -(G * a.Mass * b.Mass) / (d + S);
    }
}