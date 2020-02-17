#pragma once

#include "IParticleSeeder.hpp"

class StarSystemSeeder : public IParticleSeeder
{
    public:
        StarSystemSeeder(std::vector<Particle>& particles, float scale);

        void Seed(uint64_t seed);

    private:
        std::vector<Particle>& Particles;
};