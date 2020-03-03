#pragma once

#include "IParticleSeeder.hpp"

template <class T>
class StarSystemSeeder : public IParticleSeeder
{
    public:
        StarSystemSeeder(std::vector<T>& particles, float scale);

        void Seed(uint64_t seed);

    private:
        std::vector<T>& Particles;
};

#include "StarSystemSeeder.cpp"