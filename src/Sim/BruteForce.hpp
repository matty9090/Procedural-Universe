#pragma once

#include "INBodySim.hpp"

class BruteForce : public INBodySim
{
    public:
        BruteForce(std::vector<Particle>& particles) : Particles(particles) {}

        void Update(float dt) final;

    private:
        std::vector<Particle>& Particles;
};