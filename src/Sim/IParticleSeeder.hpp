#pragma once

#include "Render/Particle.hpp"

#include <memory>
#include <vector>

enum class EParticleSeeder
{
    Random,
    Galaxy,
    StarSystem
};

class IParticleSeeder
{
    public:
        virtual void Seed() = 0;
};

std::unique_ptr<IParticleSeeder> CreateParticleSeeder(std::vector<Particle>& particles, EParticleSeeder type);