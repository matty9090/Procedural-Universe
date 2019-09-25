#pragma once

#include "INBodySim.hpp"

class BarnesHut : public INBodySim
{
    public:
        BarnesHut() {}

        void Init(std::vector<Particle>& particles) final {}
        void Update(float dt) final {}

    private:
        std::vector<Particle>* Particles;
};