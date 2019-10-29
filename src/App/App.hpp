//
// App.h
//

#pragma once

#include "App/DeviceResources.hpp"
#include "App/StepTimer.hpp"

#include "UI/UI.hpp"
#include "Sim/INBodySim.hpp"
#include "Sim/IParticleSeeder.hpp"

#include "Render/Camera.hpp"
#include "Render/Particle.hpp"
#include "Render/Splatting.hpp"
#include "Render/PostProcess.hpp"
#include "Render/ConstantBuffer.hpp"

#include <d3d11.h>
#include <SimpleMath.h>
#include <CommonStates.h>

class UI;
class Splatting;

namespace Buffers
{
    struct GS
    {
        DirectX::SimpleMath::Matrix ViewProj;
        DirectX::SimpleMath::Matrix InvView;
    };
}

class App final : public DX::IDeviceNotify
{
public:

    App() noexcept(false);

    // Initialization and management
    void Initialize(HWND window, int width, int height);
    void Tick();

    static void RunSimulation(float dt, int time, int numparticles, std::string file);

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize(int& width, int& height) const;

private:
    void InitParticles();
    void RegisterEvents();

    void Update(float dt);
    void Render();
    void RenderParticles();
    void RunBenchmark();
    void CheckParticleSelected(DirectX::Mouse::State& ms);
    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    static bool InitParticlesFromFile(std::string fname, std::vector<Particle>& particles);

    std::unique_ptr<Camera>                         Camera;
    std::unique_ptr<DX::DeviceResources>            DeviceResources;
    std::unique_ptr<DirectX::Mouse>                 Mouse;
    DX::StepTimer                                   Timer;
    std::unique_ptr<UI>                             UI;
    std::unique_ptr<PostProcess>                    PostProcess;
    std::unique_ptr<Splatting>                      Splatting;
    std::unique_ptr<DirectX::CommonStates>          CommonStates;

    Microsoft::WRL::ComPtr<ID3D11VertexShader>      VertexShader;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader>    GeometryShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>       PixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>       InputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            ParticleBuffer;

    std::unique_ptr<ConstantBuffer<Buffers::GS>>    GSBuffer;
    std::vector<Particle>                           Particles;
    unsigned int                                    NumParticles = 10;
    Particle*                                       SelectedParticle = nullptr;

    std::unique_ptr<INBodySim>                      Sim;
    std::unique_ptr<IParticleSeeder>                Seeder;
    float                                           SimSpeed = 0.02f;
    bool                                            bIsPaused = false;
    bool                                            bDrawDebug = false;
    bool                                            bUseBloom = true;
    bool                                            bUseSplatting = false;
};
