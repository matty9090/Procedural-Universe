#pragma once

#include "Core/State.hpp"

#include "UI/UI.hpp"
#include "Sim/INBodySim.hpp"
#include "Sim/IParticleSeeder.hpp"

#include "Render/ArcballCamera.hpp"
#include "Render/Particle.hpp"
#include "Render/Splatting.hpp"
#include "Render/PostProcess.hpp"
#include "Render/RenderCommon.hpp"
#include "Render/ConstantBuffer.hpp"

#include <Mouse.h>
#include <Keyboard.h>
#include <SimpleMath.h>
#include <CommonStates.h>

class SimulationState final : public IState
{
public:
    // IState override
    void Init(DX::DeviceResources* resources, DirectX::Mouse* mouse, DirectX::Keyboard* keyboard, StateData& data) override;
    void Cleanup() override;
    void Update(float dt) override;
    void Render() override;
    // End IState override

    std::vector<Particle> GetParticles() const { return Particles; }

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

    struct GSConstantBuffer
    {
        DirectX::SimpleMath::Matrix ViewProj;
        DirectX::SimpleMath::Matrix InvView;
    };

    ID3D11Device*                                     Device;
    ID3D11DeviceContext*                              Context;
    DX::DeviceResources*                              DeviceResources;
                                                      
    DirectX::Mouse*                                   Mouse;
    std::unique_ptr<CArcballCamera>                   Camera;
    std::unique_ptr<CUI>                              UI;
    std::unique_ptr<CPostProcess>                     PostProcess;
    std::unique_ptr<CSplatting>                       Splatting;
    std::unique_ptr<DirectX::CommonStates>            CommonStates;
                                                      
    RenderPipeline                                    ParticlePipeline;
    Microsoft::WRL::ComPtr<ID3D11Buffer>              ParticleBuffer;

    std::unique_ptr<ConstantBuffer<GSConstantBuffer>> GSBuffer;
    std::vector<Particle>                             Particles;
    unsigned int                                      NumParticles = 1000;
    Particle* SelectedParticle = nullptr;             
                                                      
    std::unique_ptr<INBodySim>                        Sim;
    std::unique_ptr<IParticleSeeder>                  Seeder;
    float                                             SimSpeed = 0.02f;
    bool                                              bIsPaused = true;
    bool                                              bDrawDebug = false;
    bool                                              bUseBloom = true;
    bool                                              bUseSplatting = false;
};