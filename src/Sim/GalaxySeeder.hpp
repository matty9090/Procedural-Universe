#pragma once

#include "IParticleSeeder.hpp"

class GalaxySeeder : public IParticleSeeder
{
    public:
        GalaxySeeder(std::vector<Particle>& particles);

        void Seed();

    private:
        std::vector<Particle>& Particles;
};