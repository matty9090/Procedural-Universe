#include "IParticleSeeder.hpp"

#include "RandomSeeder.hpp"
#include "GalaxySeeder.hpp"
#include "StarSystemSeeder.hpp"

std::unique_ptr<IParticleSeeder> CreateParticleSeeder(std::vector<Particle>& particles, EParticleSeeder type)
{
    std::unique_ptr<IParticleSeeder> seeder;

    switch(type)
    {
        case EParticleSeeder::Random:
            seeder = std::make_unique<RandomSeeder>(particles);
            break;

        case EParticleSeeder::Galaxy:
            seeder = std::make_unique<GalaxySeeder>(particles);
            break;

        case EParticleSeeder::StarSystem:
            seeder = std::make_unique<StarSystemSeeder>(particles);
            break;
    }

    return std::move(seeder);
}