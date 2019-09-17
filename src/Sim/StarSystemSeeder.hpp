#pragma once

#include "IParticleSeeder.hpp"

class StarSystemSeeder : public IParticleSeeder
{
    public:
        StarSystemSeeder(std::vector<Particle>& particles);

        void Seed();

    private:
        std::vector<Particle>& Particles;
};