#pragma once

#include <vector>
#include <d3d11.h>
#include <SimpleMath.h>

#include "Render/Misc/Particle.hpp"

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

    float RandFloat();
    float RandFloat(float min, float max);

    const Particle& ClosestParticle(const DirectX::SimpleMath::Vector3& pos, const std::vector<Particle>& particles, size_t* outID = nullptr);

    template <class T>
    const T& ClosestObject(const DirectX::SimpleMath::Vector3& pos, const std::vector<T>& particles, size_t* outID)
    {
        const T* closest = nullptr;
        size_t id = 0;
        float distance = (std::numeric_limits<float>::max)();

        for (size_t i = 0; i < particles.size(); ++i)
        {
            float d = DirectX::SimpleMath::Vector3::DistanceSquared(pos, particles[i]->GetPosition());

            if (d < distance)
            {
                closest = &particles[i];
                id = i;
                distance = d;
            }
        }

        if (outID)
            *outID = id;

        return *closest;
    }
}