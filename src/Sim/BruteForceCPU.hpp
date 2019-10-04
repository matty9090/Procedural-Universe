#pragma once

#include "INBodySim.hpp"
#include "Render/ConstantBuffer.hpp"

#include <wrl/client.h>

class BruteForceCPU : public INBodySim
{
    public:
        BruteForceCPU(ID3D11DeviceContext* context);

        void Init(std::vector<Particle>& particles) final;
        void Update(float dt) final;

    private:
        ID3D11DeviceContext* Context = nullptr;

        std::vector<Particle>* Particles;

        void Exec(size_t index, size_t loops);
};