#pragma once

#include <vector>
#include <d3d11.h>
#include <SimpleMath.h>

#include "Render/Particle.hpp"

namespace Maths
{
    template <class T>
    T Lerp(T a, T b, T t) { return a + t * (b - a); }

    const Particle& ClosestParticle(const DirectX::SimpleMath::Vector3& pos, const std::vector<Particle>& particles);
}