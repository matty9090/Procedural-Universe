#pragma once

#include "INBodySim.hpp"
#include "Render/DX/ConstantBuffer.hpp"

#include <wrl/client.h>

class BruteForceGPU : public INBodySim
{
    public:
        BruteForceGPU(ID3D11DeviceContext* context);

        void Init(std::vector<Particle>& particles) final;
        void Update(float dt) final;

    private:
        void CreateShader();
        void RunShader();

        struct FrameBuffer
        {
            float FrameTime;
            double Scale;
        };

        ID3D11Device* Device = nullptr;
        ID3D11DeviceContext* Context = nullptr;

        std::vector<Particle>* Particles;
        std::unique_ptr<ConstantBuffer<FrameBuffer>> UpdateBuffer;

        Microsoft::WRL::ComPtr<ID3D11Buffer> InBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> OutBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> OutResBuffer;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SrvIn;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SrvOut;
        Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> UavOut;
        Microsoft::WRL::ComPtr<ID3D11ComputeShader> GravityShader;
        Microsoft::WRL::ComPtr<ID3D11ComputeShader> UpdateShader;
};