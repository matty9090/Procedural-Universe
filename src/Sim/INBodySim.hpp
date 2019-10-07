#pragma once

#include <vector>
#include <memory>
#include <string>
#include <d3d11.h>
#include <SimpleMath.h>

#include "Render/Particle.hpp"

enum class ENBodySim
{
    BruteForceCPU,
    BruteForceGPU,
    BarnesHut,
    NumSims
};

class INBodySim
{
    public:
        virtual void Init(std::vector<Particle>& particles) = 0;
        virtual void Update(float dt) = 0;
        virtual void RenderDebug(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj) {}
};

std::unique_ptr<INBodySim> CreateNBodySim(ID3D11DeviceContext* context, ENBodySim type);
std::string NBodySimGetName(ENBodySim type);