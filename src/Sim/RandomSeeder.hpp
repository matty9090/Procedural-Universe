#pragma once

#include "IParticleSeeder.hpp"

class RandomSeeder : public IParticleSeeder
{
    public:
        RandomSeeder(std::vector<Particle>& particles);

        void Seed();

    private:
        std::vector<Particle>& Particles;
};