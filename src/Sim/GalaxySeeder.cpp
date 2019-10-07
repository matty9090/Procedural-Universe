#include "GalaxySeeder.hpp"

#include <random>
#include <DirectXColors.h>

GalaxySeeder::GalaxySeeder(std::vector<Particle>& particles) : Particles(particles)
{

}

void GalaxySeeder::Seed()
{
    std::default_random_engine generator;
    std::uniform_real_distribution<float> dist(-320.0f, 320.0);
    std::uniform_real_distribution<float> dist_vel(0.0f, 3.0f);
    std::uniform_real_distribution<float> dist_mass(1e28, 1e30);

    unsigned int i = 0;

    for(i; i < Particles.size() / 2; ++i)
    {
        Particles[i].Position.x = static_cast<float>(dist(generator)) - 600.0f;
        Particles[i].Position.y = static_cast<float>(dist(generator));
        Particles[i].Position.z = static_cast<float>(dist(generator));
        Particles[i].Velocity.x = static_cast<float>(dist_vel(generator));
        Particles[i].Velocity.y = static_cast<float>(0.0);
        Particles[i].Velocity.z = static_cast<float>(0.0);
        Particles[i].Mass   = dist_mass(generator);
        Particles[i].Accel = Vector3::Zero;
        Particles[i].Colour = DirectX::Colors::White;
        Particles[i].OriginalColour = Particles[i].Colour;
    }

    for(i; i < Particles.size(); ++i)
    {
        Particles[i].Position.x = static_cast<float>(dist(generator)) + 600.0f;
        Particles[i].Position.y = static_cast<float>(dist(generator));
        Particles[i].Position.z = static_cast<float>(dist(generator));
        Particles[i].Velocity.x = static_cast<float>(dist_vel(generator));
        Particles[i].Velocity.y = static_cast<float>(dist_vel(generator));
        Particles[i].Velocity.z = static_cast<float>(dist_vel(generator));
        Particles[i].Mass   = dist_mass(generator);
        Particles[i].Accel = Vector3::Zero;
        Particles[i].Colour = DirectX::Colors::White;
        Particles[i].OriginalColour = Particles[i].Colour;
    }

    Particles[0].Mass = 100000.0f;
    Particles[0].Position = { -300, 0, 0 };

    Particles[Particles.size() / 2].Mass = 1000000.0f;
    Particles[Particles.size() / 2].Position = { 300, 0, 0 };
}