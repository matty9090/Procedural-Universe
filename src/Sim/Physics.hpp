#pragma once

#include "Render/Particle.hpp"

using namespace DirectX::SimpleMath;

namespace Phys
{
    const double G = 0.1;
    const double S = 1;

    inline double Gravity(const Particle& a, const Particle& b)
    {
        double d = Vector3::DistanceSquared(a.Position, b.Position);
        return -(G * a.Mass * b.Mass) / (d + S * S);
    }
}