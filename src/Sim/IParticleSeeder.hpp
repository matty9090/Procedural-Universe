#pragma once

#include "Render/Misc/Particle.hpp"

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
        virtual void Seed(uint64_t seed = 0) = 0;

        virtual void SetRedDist(float low, float hi) {}
        virtual void SetGreenDist(float low, float hi) {}
        virtual void SetBlueDist(float low, float hi) {}
};

std::unique_ptr<IParticleSeeder> CreateParticleSeeder(std::vector<Particle>& particles, EParticleSeeder type);