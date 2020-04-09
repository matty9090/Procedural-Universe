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

    template <class T>
    T Map(T x, T in_min, T in_max, T out_min, T out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    template <class T>
    T EaseInOutQuad(T t) {
        return t < 0.5f ? 2 * t * t : t * (4 - 2 * t) - 1;
    }

    template <class T>
    T EaseInOutCubic(T t)
    {
        if (t < 0.5f)
            return 4 * t * t * t;
        else
        {
            t = ((2 * t) - 2);
            return 0.5f * t * t * t + 1;
        }
    }

    template <class T>
    T EaseInOutQuint(T t)
    {
        if (t < 0.5f) {
            t *= t;
            return 8 * t * t;
        }
        else
        {
            t = (--t) * t;
            return 1 - 8 * t * t;
        }
    }

    int   RandInt(int min, int max);
    float RandFloat();
    float RandFloat(float min, float max);

    template <class T>
    const T& ClosestParticle(const DirectX::SimpleMath::Vector3& pos, const std::vector<T>& particles, size_t* outID = nullptr)
    {
        const T* closest;
        size_t id = 0;
        float distance = (std::numeric_limits<float>::max)();

        for (size_t i = 0; i < particles.size(); ++i)
        {
            float d = DirectX::SimpleMath::Vector3::DistanceSquared(pos, particles[i].Position);

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