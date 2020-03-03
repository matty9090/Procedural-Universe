#pragma once

#include "Render/Misc/Particle.hpp"

#include <memory>
#include <vector>

#include "RandomSeeder.hpp"
#include "GalaxySeeder.hpp"
#include "StarSystemSeeder.hpp"

enum class EParticleSeeder
{
    Random,
    Galaxy,
    StarSystem
};

class IParticleSeeder
{
    public:
        virtual void Seed(uint64_t seed = 0) = 0;

        virtual void SetRedDist(float low, float hi) {}
        virtual void SetGreenDist(float low, float hi) {}
        virtual void SetBlueDist(float low, float hi) {}
};

template <class T = Particle>
std::unique_ptr<IParticleSeeder> CreateParticleSeeder(std::vector<T>& particles, EParticleSeeder type, float scale = 1.0f)
{
    std::unique_ptr<IParticleSeeder> seeder;

    switch(type)
    {
        case EParticleSeeder::Random:
            seeder = std::make_unique<RandomSeeder<T>>(particles, scale);
            break;

        case EParticleSeeder::Galaxy:
            seeder = std::make_unique<GalaxySeeder<T>>(particles, scale);
            break;

        case EParticleSeeder::StarSystem:
            seeder = std::make_unique<StarSystemSeeder<T>>(particles, scale);
            break;
    }

    return std::move(seeder);
}