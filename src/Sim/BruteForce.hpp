#pragma once

#include "INBodySim.hpp"
#include <wrl/client.h>

class BruteForce : public INBodySim
{
    private:
        struct TestData
        {
            int i;
        };

    public:
        BruteForce(ID3D11DeviceContext* context, std::vector<Particle>& particles);

        void Update(float dt) final;

    private:
        std::vector<Particle>& Particles;

        ID3D11DeviceContext* Context;
        Microsoft::WRL::ComPtr<ID3D11ComputeShader> ComputeShader;
};