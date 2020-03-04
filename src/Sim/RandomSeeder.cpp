#include "RandomSeeder.hpp"

#include <random>
#include <DirectXColors.h>

template <class T>
RandomSeeder<T>::RandomSeeder(std::vector<T>& particles, float scale) : Particles(particles), Scale(scale)
{

}

template <class T>
void RandomSeeder<T>::Seed(uint64_t seed)
{
    std::default_random_engine generator;
    std::uniform_real_distribution<double> dist(-500.0f, 500.0);
    std::uniform_real_distribution<float>  dist_col(0.2f, 1.0f);
    std::uniform_real_distribution<double> dist_vel(0.0f, 0.2f);
    std::uniform_real_distribution<double> dist_mass(1e20, 1e30);

    for (unsigned int i = 0; i < Particles.size(); ++i)
    {
        Particles[i].Position.x = static_cast<float>(dist(generator)) / Scale;
        Particles[i].Position.y = static_cast<float>(dist(generator)) / Scale;
        Particles[i].Position.z = static_cast<float>(dist(generator)) / Scale;

        auto normal = Particles[i].Position;
        normal.Normalize();

        Vec3d vel(normal.x, normal.y, normal.z);
        vel *= 10000000000000000.0f;

        AddParticleVelocity(Particles[i], vel);
        AddParticleMass(Particles[i], dist_mass(generator));
        AddParticleColour(Particles[i], DirectX::SimpleMath::Color(dist_col(generator), dist_col(generator), dist_col(generator), 1.0f));
        AddParticleOriginalColour(Particles[i], Particles[i].Colour);
        AddParticleForces(Particles[i], Vec3d());
        AddParticleScale(Particles[i], 1.0f);
    }
}