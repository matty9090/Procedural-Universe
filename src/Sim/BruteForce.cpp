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
            diff /= diff.Length();

            Vec3d diff_d(static_cast<double>(diff.x),
                         static_cast<double>(diff.y),
                         static_cast<double>(diff.z));

            double f = -Phys::Gravity(a, b);

            b.Forces += Vec3d(f / diff.x, f / diff.y, f / diff.z);
        }
    }

    for(auto& particle : Particles)
    {
        auto a = particle.Forces * particle.Mass;
        particle.Velocity += a * dt;

        auto vel = particle.Velocity * dt;

        particle.Position += Vector3(static_cast<float>(vel.x),
                                     static_cast<float>(vel.y),
                                     static_cast<float>(vel.z));
    }
}