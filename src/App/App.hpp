//
// App.h
//

#pragma once

#include "App/DeviceResources.hpp"
#include "App/StepTimer.hpp"

#include "Core/State.hpp"

#include <string>
#include <vector>
#include <d3d11.h>
#include <SimpleMath.h>

class CUI;
class CSplatting;

class App final : public DX::IDeviceNotify
{
public:
    App() noexcept(false);

    // Initialization and management
    void Initialize(HWND window, int width, int height);
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
    void GetDefaultSize(int& width, int& height) const;

    static void RunSimulation(float dt, int time, int numparticles, std::string file);

private:
    void Update(float dt);
    void Render();
    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    std::vector<std::unique_ptr<IState>>  States;
    IState*                               CurrentState;

    DX::StepTimer                         Timer;
    std::unique_ptr<DX::DeviceResources>  DeviceResources;
};
