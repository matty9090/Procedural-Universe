#pragma once

#include "IParticleSeeder.hpp"

class RandomSeeder : public IParticleSeeder
{
    public:
        RandomSeeder(std::vector<Particle>& particles);

        void Seed(uint64_t seed);

    private:
        std::vector<Particle>& Particles;
};