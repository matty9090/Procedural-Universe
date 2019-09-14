//
// App.cpp
//

#include "App/AppCore.hpp"
#include "App/App.hpp"

#include "Core/Except.hpp"

#include "Render/Shader.hpp"
#include "Services/Log.hpp"

#include "SimpleMath.h"

#include <random>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

App::App() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void App::Initialize(HWND window, int width, int height)
{
    FLog::Get().Log("Initializing...");

    InitParticles();

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    FLog::Get().Log("Initialized");
}

void App::InitParticles()
{
    std::default_random_engine generator;
    std::uniform_real_distribution<double> dist(-500.0f, 500.0);

    m_particles.resize(m_numParticles);

    for(unsigned int i = 0; i < m_numParticles; ++i)
    {
        m_particles[i].Position.x = static_cast<float>(dist(generator));
        m_particles[i].Position.y = static_cast<float>(dist(generator));
        m_particles[i].Position.z = static_cast<float>(dist(generator));
    }
}

#pragma region Frame Update
// Executes the basic game loop.
void App::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void App::Update(DX::StepTimer const& timer)
{
    float dt = float(timer.GetElapsedSeconds());

    m_camera.Update(dt);
    m_ui->Update(dt);

    for(auto& particle : m_particles)
    {
        particle.Position.z -= 40.0f * dt;
    }

    auto context = m_deviceResources->GetD3DDeviceContext();
    context->UpdateSubresource(m_particleBuffer.Get(), 0, NULL, &m_particles[0], 0, 0);
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void App::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    auto context = m_deviceResources->GetD3DDeviceContext();

    m_deviceResources->PIXBeginEvent(L"Render");

    unsigned int offset = 0;
    unsigned int stride = sizeof(Particle);

    context->IASetInputLayout(m_inputLayout.Get());
    context->IASetVertexBuffers(0, 1, m_particleBuffer.GetAddressOf(), &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    
    DirectX::SimpleMath::Matrix view = m_camera.GetViewMatrix();
    DirectX::SimpleMath::Matrix proj = m_camera.GetProjectionMatrix();

    m_gsBuffer->SetData(context, Buffers::GS { view * proj, view.Invert() });
    m_psBuffer->SetData(context, Buffers::PS { DirectX::Colors::White });

    context->GSSetConstantBuffers(0, 1, m_gsBuffer->GetBuffer());
    context->PSSetConstantBuffers(0, 1, m_psBuffer->GetBuffer());

    context->Draw(m_numParticles, 0);

    m_ui->Render();

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void App::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::Black);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void App::OnActivated()
{
    // TODO: App is becoming active window.
}

void App::OnDeactivated()
{
    // TODO: App is becoming background window.
}

void App::OnSuspending()
{
    // TODO: App is being power-suspended (or minimized).
}

void App::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: App is being power-resumed (or returning from minimize).
}

void App::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void App::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

// Properties
void App::GetDefaultSize(int& width, int& height) const
{
    width = 1400;
    height = 900;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void App::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    m_ui = std::make_unique<UI>(context, m_deviceResources->GetWindow());

    ID3DBlob* VertexCode;

    bool bVertex = LoadVertexShader(device, L"shaders/PassThruGS.vsh", m_vertexShader.ReleaseAndGetAddressOf(), &VertexCode);
    bool bGeometry = LoadGeometryShader(device, L"shaders/DrawParticle.gsh", m_geometryShader.ReleaseAndGetAddressOf());
    bool bPixel = LoadPixelShader(device, L"shaders/PlainColour.psh", m_pixelShader.ReleaseAndGetAddressOf());

    if(!(bVertex && bGeometry && bPixel))
        throw std::exception("Failed to load shader(s)");

    D3D11_INPUT_ELEMENT_DESC Layout = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    
    DX::ThrowIfFailed(
        device->CreateInputLayout(&Layout, 1, VertexCode->GetBufferPointer(), VertexCode->GetBufferSize(), m_inputLayout.ReleaseAndGetAddressOf())
    );

    context->VSSetShader(m_vertexShader.Get(), 0, 0);
    context->GSSetShader(m_geometryShader.Get(), 0, 0);
    context->PSSetShader(m_pixelShader.Get(), 0, 0);

    m_gsBuffer = std::make_unique<ConstantBuffer<Buffers::GS>>(device);
    m_psBuffer = std::make_unique<ConstantBuffer<Buffers::PS>>(device);

    D3D11_BUFFER_DESC buffer;
    buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer.Usage = D3D11_USAGE_DEFAULT;
    buffer.ByteWidth = m_numParticles * sizeof(Particle);
    buffer.CPUAccessFlags = 0;
    buffer.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA init;
    init.pSysMem = &m_particles[0];

    DX::ThrowIfFailed(device->CreateBuffer(&buffer, &init, m_particleBuffer.ReleaseAndGetAddressOf()));
}

// Allocate all memory resources that change on a window SizeChanged event.
void App::CreateWindowSizeDependentResources()
{
    int width, height;
    GetDefaultSize(width, height);

    m_camera = Camera(width, height);
}

void App::OnDeviceLost()
{
    
}

void App::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}
#pragma endregion
