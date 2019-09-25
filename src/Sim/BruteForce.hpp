#pragma once

#include "INBodySim.hpp"
#include <wrl/client.h>

class BruteForce : public INBodySim
{
    public:
        BruteForce(ID3D11DeviceContext* context);

        void Init(std::vector<Particle>& particles) final;
        void Update(float dt) final;

    private:
        void CreateShader();
        void RunShader();

        ID3D11Device* Device = nullptr;
        ID3D11DeviceContext* Context = nullptr;

        std::vector<Particle>* Particles;

        Microsoft::WRL::ComPtr<ID3D11Buffer> inBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> outBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> outResBuffer;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
        Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uav;
        Microsoft::WRL::ComPtr<ID3D11ComputeShader> ComputeShader;
};