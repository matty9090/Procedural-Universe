#pragma once

#include "INBodySim.hpp"
#include "Core/ThreadPool.hpp"
#include "Render/DX/ConstantBuffer.hpp"

#include <wrl/client.h>

class BruteForceCPU : public INBodySim
{
    public:
        BruteForceCPU(ID3D11DeviceContext* context);

        void Init(std::vector<Particle>& particles) final;
        void Update(float dt) final;

    private:
        ID3D11DeviceContext* Context = nullptr;

        struct ParticleInfo
        {
            size_t Index;
            size_t Loops;
        };

        std::vector<Particle>* Particles;
        CThreadPool<ParticleInfo> Pool;

        void Exec(const ParticleInfo& info);
};