//
// App.h
//

#pragma once

#include "App/DeviceResources.hpp"
#include "App/StepTimer.hpp"

#include "UI/UI.hpp"

#include "Render/Camera.hpp"
#include "Render/Particle.hpp"
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

    struct PS
    {
        DirectX::SimpleMath::Color Colour;
    };
}

class App final : public DX::IDeviceNotify
{
public:

    App() noexcept(false);

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

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

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    Camera                                          m_camera;

    std::unique_ptr<DX::DeviceResources>            m_deviceResources;
    DX::StepTimer                                   m_timer;
    std::unique_ptr<UI>                             m_ui;

    Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader>    m_geometryShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_particleBuffer;

    std::unique_ptr<ConstantBuffer<Buffers::GS>>    m_gsBuffer;
    std::unique_ptr<ConstantBuffer<Buffers::PS>>    m_psBuffer;

    std::vector<Particle>                           m_particles;
    unsigned int                                    m_numParticles = 10000;
};
