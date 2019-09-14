#pragma once

#include <vector>
#include <memory>

#include "Render/Particle.hpp"

enum class ENBodySim
{
    BruteForce,
    BarnesHut
};

class INBodySim
{
    public:
        virtual void Update(float dt) = 0;
};

std::unique_ptr<INBodySim> CreateNBodySim(std::vector<Particle>& particles, ENBodySim type);