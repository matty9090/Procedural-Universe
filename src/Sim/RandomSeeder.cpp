#include "RandomSeeder.hpp"

#include <random>
#include <DirectXColors.h>

RandomSeeder::RandomSeeder(std::vector<Particle>& particles) : Particles(particles)
{

}

void RandomSeeder::Seed()
{
    std::default_random_engine generator;
    std::uniform_real_distribution<float> dist(-500.0f, 500.0);
    std::uniform_real_distribution<float> dist_vel(-1.0, 1.0);
    std::uniform_real_distribution<float> dist_mass(1e3, 1e5);

    for(unsigned int i = 0; i < Particles.size(); ++i)
    {
        Particles[i].Position.x = static_cast<float>(dist(generator));
        Particles[i].Position.y = static_cast<float>(dist(generator));
        Particles[i].Position.z = static_cast<float>(dist(generator));
        Particles[i].Velocity.x = static_cast<float>(dist_vel(generator));
        Particles[i].Velocity.y = static_cast<float>(dist_vel(generator));
        Particles[i].Velocity.z = static_cast<float>(dist_vel(generator));
        Particles[i].Mass = dist_mass(generator);
        Particles[i].Accel = Vector3::Zero;
        Particles[i].Colour = DirectX::Colors::White;
        Particles[i].OriginalColour = Particles[i].Colour;
    }
}