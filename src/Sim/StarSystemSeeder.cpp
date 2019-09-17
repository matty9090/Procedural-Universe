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
    star.Colour = DirectX::SimpleMath::Color(0.6f, 1.0f, 1.0f);
    star.OriginalColour = star.Colour;
    star.Mass = 1e30;
    star.Position = DirectX::SimpleMath::Vector3::Zero;
    star.Forces = Vec3d();
    star.Velocity = Vec3d();

    std::default_random_engine generator;
    std::uniform_real_distribution<double> dist_rad(4.0 * Phys::AU * Phys::M, 7.0 * Phys::AU * Phys::M);
    std::uniform_real_distribution<double> dist_vel(0.02 * Phys::AU * Phys::M, 0.08 * Phys::AU * Phys::M);
    std::uniform_real_distribution<double> dist_mass(1e10, 1e26);
    std::uniform_real_distribution<double> dist_angle(0, 360.0f);

    for(int i = 1; i < Particles.size(); ++i)
    {
        Particles[i].Mass = dist_mass(generator);
        Particles[i].Position.x = 0;
        Particles[i].Position.y = 0;
        Particles[i].Position.z = static_cast<float>(dist_rad(generator) / Phys::StarSystemScale);

        Particles[i].Velocity.x = dist_vel(generator);
        Particles[i].Velocity.y = 0;
        Particles[i].Velocity.z = 0;

        Particles[i].Colour = DirectX::Colors::White;
        Particles[i].OriginalColour = DirectX::Colors::White;
        Particles[i].Forces = Vec3d();
    }
}