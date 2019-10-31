//
// App.cpp
//

#include "App/AppCore.hpp"
#include "App/App.hpp"

#include "Core/Event.hpp"
#include "Core/Except.hpp"

#include "Render/Shader.hpp"
#include "Services/Log.hpp"

#include "States/Simulation/SimulationState.hpp"
#include "States/Sandbox/SandboxState.hpp"

using Microsoft::WRL::ComPtr;

App::App() noexcept(false)
{
    DeviceResources = std::make_unique<DX::DeviceResources>();
    DeviceResources->RegisterDeviceNotify(this);

    States.push_back(std::make_unique<SimulationState>());
    States.push_back(std::make_unique<SandboxState>());
}

// Initialize the Direct3D resources required to run.
void App::Initialize(HWND window, int width, int height)
{
    FLog::Get().Log("Initializing...");

    DeviceResources->SetWindow(window, width, height);

    DeviceResources->CreateDeviceResources();
    DeviceResources->CreateWindowSizeDependentResources();

    //Timer.SetFixedTimeStep(true);
    //Timer.SetTargetElapsedSeconds(1.0 / 60.0);

    Mouse = std::make_unique<DirectX::Mouse>();
    Keyboard = std::make_unique<DirectX::Keyboard>();

    Mouse->SetWindow(DeviceResources->GetWindow());

    SwitchState(EState::Simulation);

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
    auto state = Keyboard->GetState();
    Tracker.Update(state);

    if (Tracker.IsKeyReleased(DirectX::Keyboard::X))
    {
        if (CurrentStateID == EState::Simulation)
        {
            auto Sim = static_cast<SimulationState*>(CurrentState);
            SwitchState(EState::Sandbox, SandboxStateData(Sim->GetParticles()));
        }
        else
        {
            SwitchState(EState::Simulation);
        }
    }

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

void App::SwitchState(EState state, StateData& data)
{
    FLog::Get().Log("Switching to state " + std::to_string(static_cast<int>(state)));

    if (CurrentState)
        CurrentState->Cleanup();

    CurrentStateID = state;
    CurrentState = States[static_cast<int>(state)].get();
    CurrentState->Init(DeviceResources.get(), Mouse.get(), Keyboard.get(), data);
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
}

// Properties
void App::GetDefaultSize(int& width, int& height) const
{
    width = 1400;
    height = 900;
}
void App::RunSimulation(float dt, int time, int numparticles, std::string file)
{
    // TODO: Move to own class?
    SimulationState::RunSimulation(dt, time, numparticles, file);
}
#pragma endregion

#pragma region Direct3D Resources
void App::OnDeviceLost()
{
    
}

void App::OnDeviceRestored()
{

}
#pragma endregion
