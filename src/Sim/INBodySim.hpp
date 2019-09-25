#pragma once

#include <vector>
#include <memory>
#include <d3d11.h>

#include "Render/Particle.hpp"

enum class ENBodySim
{
    BruteForce,
    BarnesHut
};

class INBodySim
{
    public:
        virtual void Init(std::vector<Particle>& particles) = 0;
        virtual void Update(float dt) = 0;
};

std::unique_ptr<INBodySim> CreateNBodySim(ID3D11DeviceContext* context, ENBodySim type);