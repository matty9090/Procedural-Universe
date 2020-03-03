#pragma once

#include "IParticleSeeder.hpp"

template <class T>
class RandomSeeder : public IParticleSeeder
{
    public:
        RandomSeeder(std::vector<T>& particles, float scale);

        void Seed(uint64_t seed);

    private:
        float Scale = 1.0f;
        std::vector<T>& Particles;
};

#include "RandomSeeder.cpp"