#include "RandomSeeder.hpp"

#include <random>
#include <DirectXColors.h>

RandomSeeder::RandomSeeder(std::vector<Particle>& particles) : Particles(particles)
{

}

void RandomSeeder::Seed()
{
    std::default_random_engine generator;
    std::uniform_real_distribution<double> dist(-500.0f, 500.0);
    std::uniform_real_distribution<double> dist_vel(0.0f, 0.2f);
    std::uniform_real_distribution<double> dist_mass(0.1f, 10.0f);

    for(unsigned int i = 0; i < Particles.size(); ++i)
    {
        Particles[i].Position.x = static_cast<float>(dist(generator));
        Particles[i].Position.y = static_cast<float>(dist(generator));
        Particles[i].Position.z = static_cast<float>(dist(generator));
        Particles[i].Velocity.x = static_cast<float>(dist_vel(generator));
        Particles[i].Velocity.y = static_cast<float>(dist_vel(generator));
        Particles[i].Velocity.z = static_cast<float>(dist_vel(generator));
        Particles[i].Mass   = dist_mass(generator);
        Particles[i].Forces = Vec3d();
        Particles[i].Colour = DirectX::Colors::White;
    }
}