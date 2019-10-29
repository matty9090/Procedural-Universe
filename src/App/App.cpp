//
// App.cpp
//

#include "App/AppCore.hpp"
#include "App/App.hpp"

#include "Core/Event.hpp"
#include "Core/Except.hpp"

#include "Render/Shader.hpp"
#include "Services/Log.hpp"

#include "States/SimulationState.hpp"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

App::App() noexcept(false)
{
    DeviceResources = std::make_unique<DX::DeviceResources>();
    DeviceResources->RegisterDeviceNotify(this);

    States.push_back(std::make_unique<SimulationState>());
    CurrentState = States[0].get();
}

// Initialize the Direct3D resources required to run.
void App::Initialize(HWND window, int width, int height)
{
    FLog::Get().Log("Initializing...");

    DeviceResources->SetWindow(window, width, height);

    DeviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    DeviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    Timer.SetFixedTimeStep(true);
    Timer.SetTargetElapsedSeconds(1.0 / 60.0);

    CurrentState->Init(DeviceResources.get());

    FLog::Get().Log("Initialized");
}

#pragma region Frame Update
// Executes the basic game loop.
void App::Tick()
{
    Timer.Tick([&]()
    {
        Update(static_cast<float>(Timer.GetElapsedSeconds()));
    });

    Render();
}

// Updates the world.
void App::Update(float dt)
{
    CurrentState->Update(dt);
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void App::Render()
{
    if (Timer.GetFrameCount() == 0)
        return;

    DeviceResources->PIXBeginEvent(L"Render");
    
    CurrentState->Render();
    
    DeviceResources->PIXEndEvent();
    DeviceResources->Present();
}

// Helper method to clear the back buffers.
void App::Clear()
{
    
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
    Timer.ResetElapsedTime();

    // TODO: App is being power-resumed (or returning from minimize).
}

void App::OnWindowMoved()
{
    auto r = DeviceResources->GetOutputSize();
    DeviceResources->WindowSizeChanged(r.right, r.bottom);
}

void App::OnWindowSizeChanged(int width, int height)
{
    if (!DeviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

// Properties
void App::GetDefaultSize(int& width, int& height) const
{
    width = 1400;
    height = 900;
}
void App::RunSimulation(float dt, int time, int numparticles, std::string file)
{
    SimulationState::RunSimulation(dt, time, numparticles, file);
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void App::CreateDeviceDependentResources()
{
    
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
