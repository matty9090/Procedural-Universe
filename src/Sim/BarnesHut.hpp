#pragma once

#include "INBodySim.hpp"

class BarnesHut : public INBodySim
{
    public:
        BarnesHut(std::vector<Particle>& particles) : Particles(particles) {}

        void Update(float dt) final {}

    private:
        std::vector<Particle>& Particles;
};