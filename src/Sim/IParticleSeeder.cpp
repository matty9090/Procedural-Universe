#include "IParticleSeeder.hpp"

#include "RandomSeeder.hpp"
#include "GalaxySeeder.hpp"
#include "StarSystemSeeder.hpp"

std::unique_ptr<IParticleSeeder> CreateParticleSeeder(std::vector<Particle>& particles, EParticleSeeder type, float scale)
{
    std::unique_ptr<IParticleSeeder> seeder;

    switch(type)
    {
        case EParticleSeeder::Random:
            seeder = std::make_unique<RandomSeeder>(particles, scale);
            break;

        case EParticleSeeder::Galaxy:
            seeder = std::make_unique<GalaxySeeder>(particles, scale);
            break;

        case EParticleSeeder::StarSystem:
            seeder = std::make_unique<StarSystemSeeder>(particles, scale);
            break;
    }

    return std::move(seeder);
}