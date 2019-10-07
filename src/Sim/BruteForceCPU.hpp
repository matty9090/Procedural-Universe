#pragma once

#include "INBodySim.hpp"
#include "Render/ConstantBuffer.hpp"

#include <list>
#include <thread>
#include <wrl/client.h>

class BruteForceCPU : public INBodySim
{
    public:
        BruteForceCPU(ID3D11DeviceContext* context);

        void Init(std::vector<Particle>& particles) final;
        void Update(float dt) final;

    private:
        ID3D11DeviceContext* Context = nullptr;

        std::vector<Vector3> Accel;
        std::vector<Particle>* Particles;
        std::list<std::thread> Threads;

        void Exec(float dt, size_t index, size_t loops);
};