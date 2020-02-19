#pragma once

#include "IParticleSeeder.hpp"

class RandomSeeder : public IParticleSeeder
{
    public:
        RandomSeeder(std::vector<Particle>& particles, float scale);

        void Seed(uint64_t seed);

    private:
        float Scale = 1.0f;
        std::vector<Particle>& Particles;
};