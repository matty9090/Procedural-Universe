#include "StarSystemSeeder.hpp"
#include "Physics.hpp"

#include <random>
#include <DirectXColors.h>

#define _USE_MATH_DEFINES
#include <math.h>

StarSystemSeeder::StarSystemSeeder(std::vector<Particle>& particles) : Particles(particles)
{

}

void StarSystemSeeder::Seed()
{
    Particle& star = Particles[0];
    star.Colour = Color(0.6f, 1.0f, 1.0f);
    star.OriginalColour = star.Colour;
    star.Mass = 1e10f;
    star.Position = Vector3::Zero;
    star.Accel = Vector3::Zero;
    star.Velocity = Vector3::Zero;

    std::default_random_engine generator;
    std::uniform_real_distribution<float> dist_rad(4.0f * Phys::AU * Phys::M, 7.0f * Phys::AU * Phys::M);
    std::uniform_real_distribution<float> dist_vel(0.02f * Phys::AU * Phys::M, 0.08f * Phys::AU * Phys::M);
    std::exponential_distribution<float> dist_mass(1e10);
    std::uniform_real_distribution<float> dist_angle(0, 360.0f);

    for(int i = 1; i < Particles.size(); ++i)
    {
        Particles[i].Mass = dist_mass(generator);
        Particles[i].Position.x = 0;
        Particles[i].Position.y = 0;
        Particles[i].Position.z = static_cast<float>(dist_rad(generator) / Phys::StarSystemScale);

        Particles[i].Velocity.x = dist_vel(generator);
        Particles[i].Velocity.y = dist_vel(generator) / 20.0f;
        Particles[i].Velocity.z = 0;

        Particles[i].Colour = DirectX::Colors::White;
        Particles[i].OriginalColour = DirectX::Colors::White;
        Particles[i].Accel = Vector3::Zero;
    }
}