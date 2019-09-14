//
// App.cpp
//

#include "App/AppCore.hpp"
#include "App/App.hpp"

#include "Services/Log.hpp"

#include "SimpleMath.h"

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

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    FLog::Get().Log("Initialized");
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
    float elapsedTime = float(timer.GetElapsedSeconds());

    m_ui->Update(elapsedTime);
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

    m_deviceResources->PIXBeginEvent(L"Render");

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
}

// Allocate all memory resources that change on a window SizeChanged event.
void App::CreateWindowSizeDependentResources()
{
    
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
