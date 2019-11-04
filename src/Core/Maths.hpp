#pragma once

#include <vector>
#include <d3d11.h>
#include <SimpleMath.h>

#include "Render/Particle.hpp"

namespace Maths
{
    template <class T>
    T Lerp(T a, T b, T t) { return a + t * (b - a); }

    template <class T>
    T Clamp(T v, T min, T max) {
        if (v < min) v = min;
        if (v > max) v = max;
        return v;
    }

    const Particle& ClosestParticle(const DirectX::SimpleMath::Vector3& pos, const std::vector<Particle>& particles);
}