#include "RandomSeeder.hpp"

#include <random>
#include <DirectXColors.h>

RandomSeeder::RandomSeeder(std::vector<Particle>& particles, float scale) : Particles(particles)
{

}

void RandomSeeder::Seed(uint64_t seed)
{
    std::default_random_engine generator;
    std::uniform_real_distribution<double> dist(-500.0f, 500.0);
    std::uniform_real_distribution<float>  dist_col(0.2f, 1.0f);
    std::uniform_real_distribution<double> dist_vel(0.0f, 0.2f);
    std::uniform_real_distribution<double> dist_mass(1e20, 1e30);

    for(unsigned int i = 0; i < Particles.size(); ++i)
    {
        Particles[i].Position.x = static_cast<float>(dist(generator));
        Particles[i].Position.y = static_cast<float>(dist(generator));
        Particles[i].Position.z = static_cast<float>(dist(generator));

        auto normal = Particles[i].Position;
        normal.Normalize();

        Particles[i].Velocity.x = normal.x * 10000000000000000.0f;
        Particles[i].Velocity.y = normal.y * 10000000000000000.0f;
        Particles[i].Velocity.z = normal.z * 10000000000000000.0f;
        Particles[i].Mass   = dist_mass(generator);
        Particles[i].Forces = Vec3d();
        Particles[i].Colour = DirectX::SimpleMath::Color(dist_col(generator), dist_col(generator), dist_col(generator), 1.0f);
        Particles[i].OriginalColour = Particles[i].Colour;
    }
}