#include "BruteForce.hpp"
#include "Physics.hpp"
#include "Core/Vec3.hpp"

using namespace DirectX::SimpleMath;

void BruteForce::Update(float dt)
{
    for(auto& particle : Particles)
        particle.Forces = Vec3d();

    for(int i = 0; i < Particles.size(); ++i)
    {
        for(int j = 0; j < Particles.size(); ++j)
        {
            if(i == j) continue;
            
            auto& a = Particles[i];
            auto& b = Particles[j];

            auto diff = (b.Position - a.Position);
            auto len = diff.Length();

            double f = Phys::Gravity(a, b);

            b.Forces += Vec3d(f * diff.x / len, f * diff.y / len, f * diff.z / len);
        }
    }

    for(auto& particle : Particles)
    {
        auto a = particle.Forces / particle.Mass;
        particle.Velocity += a * dt;

        auto vel = (particle.Velocity * dt) / Phys::StarSystemScale;

        particle.Position += Vector3(static_cast<float>(vel.x),
                                     static_cast<float>(vel.y),
                                     static_cast<float>(vel.z));
    }
}