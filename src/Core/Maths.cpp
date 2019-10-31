#include "Maths.hpp"

const Particle& Maths::ClosestParticle(const DirectX::SimpleMath::Vector3& pos, const std::vector<Particle>& particles)
{
    const Particle* closest = nullptr;
    float distance = (std::numeric_limits<float>::max)();

    for (const auto& p : particles)
    {
        float d = DirectX::SimpleMath::Vector3::DistanceSquared(pos, p.Position);

        if (d < distance)
        {
            closest = &p;
            distance = d;
        }
    }

    return *closest;
}