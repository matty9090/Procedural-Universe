#pragma once

#include "Core/State.hpp"

#include "UI/UI.hpp"
#include "Sim/INBodySim.hpp"
#include "Sim/IParticleSeeder.hpp"

#include "Render/Camera.hpp"
#include "Render/Particle.hpp"
#include "Render/Splatting.hpp"
#include "Render/PostProcess.hpp"
#include "Render/ConstantBuffer.hpp"

#include <Mouse.h>
#include <Keyboard.h>
#include <SimpleMath.h>
#include <CommonStates.h>

namespace Buffers
{
    struct GS
    {
        DirectX::SimpleMath::Matrix ViewProj;
        DirectX::SimpleMath::Matrix InvView;
    };
}

class SimulationState final : public IState
{
public:
    // IState override
    void Init(DX::DeviceResources* resources, DirectX::Mouse* mouse, DirectX::Keyboard* keyboard, StateData& data) override;
    void Cleanup() override;
    void Update(float dt) override;
    void Render() override;
    // End IState override

    static bool InitParticlesFromFile(std::string fname, std::vector<Particle>& particles);
    static void RunSimulation(float dt, int time, int numparticles, std::string file);

private:
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    void Clear();
    void RegisterEvents();
    void InitParticles();
    void RenderParticles();
    void RunBenchmark();
    void CheckParticleSelected(DirectX::Mouse::State& ms);

    ID3D11Device*                                   Device;
    ID3D11DeviceContext*                            Context;
    DX::DeviceResources*                            DeviceResources;

    DirectX::Mouse*                                 Mouse;
    std::unique_ptr<CCamera>                        Camera;
    std::unique_ptr<CUI>                            UI;
    std::unique_ptr<CPostProcess>                   PostProcess;
    std::unique_ptr<CSplatting>                     Splatting;
    std::unique_ptr<DirectX::CommonStates>          CommonStates;

    Microsoft::WRL::ComPtr<ID3D11VertexShader>      VertexShader;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader>    GeometryShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>       PixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>       InputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            ParticleBuffer;

    std::unique_ptr<ConstantBuffer<Buffers::GS>>    GSBuffer;
    std::vector<Particle>                           Particles;
    unsigned int                                    NumParticles = 10;
    Particle* SelectedParticle = nullptr;

    std::unique_ptr<INBodySim>                      Sim;
    std::unique_ptr<IParticleSeeder>                Seeder;
    float                                           SimSpeed = 0.02f;
    bool                                            bIsPaused = false;
    bool                                            bDrawDebug = false;
    bool                                            bUseBloom = true;
    bool                                            bUseSplatting = false;
};