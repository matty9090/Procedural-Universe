#include "Maths.hpp"

float Maths::RandFloat()
{
    return static_cast<float>(rand()) / RAND_MAX;
}

float Maths::RandFloat(float min, float max)
{
    return (static_cast<float>(rand()) / RAND_MAX)* (max - min) + min;
}

const Particle& Maths::ClosestParticle(const DirectX::SimpleMath::Vector3& pos, const std::vector<Particle>& particles, size_t* outID)
{
    const Particle* closest;
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