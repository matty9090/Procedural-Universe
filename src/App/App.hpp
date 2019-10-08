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
#include "Render/PostProcess.hpp"
#include "Render/ConstantBuffer.hpp"

#include <d3d11.h>
#include <SimpleMath.h>

class UI;

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

    static void RunSimulation(float dt, int time, int numparticles);

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
    void GetDefaultSize( int& width, int& height ) const;

private:
    void InitParticles();
    void InitParticlesFromFile(std::string fname);
    void RegisterEvents();

    void Update(float dt);
    void Render();
    void RenderParticles();
    void RunBenchmark();
    void CheckParticleSelected(DirectX::Mouse::State& ms);
    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    Camera                                          m_camera;

    std::unique_ptr<DX::DeviceResources>            m_deviceResources;
    std::unique_ptr<DirectX::Mouse>                 m_mouse;
    DX::StepTimer                                   m_timer;
    std::unique_ptr<UI>                             m_ui;
    std::unique_ptr<PostProcess>                    m_postProcess;

    Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader>    m_geometryShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_particleBuffer;

    std::unique_ptr<ConstantBuffer<Buffers::GS>>    m_gsBuffer;
    std::vector<Particle>                           m_particles;
    unsigned int                                    m_numParticles = 10;
    Particle*                                       m_selectedParticle = nullptr;

    std::unique_ptr<INBodySim>                      m_sim;
    std::unique_ptr<IParticleSeeder>                m_seeder;
    bool                                            m_isPaused = false;
    float                                           m_simSpeed = 1.0f;
    bool                                            m_drawDebug = false;
};
