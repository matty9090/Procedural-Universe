#include "DebugSandboxState.hpp"
#include "Core/Maths.hpp"

#include "Services/Log.hpp"
#include "Services/ResourceManager.hpp"

#include "States/Sandbox/UniverseTarget.hpp"
#include "States/Sandbox/GalaxyTarget.hpp"
#include "States/Sandbox/StarTarget.hpp"
#include "States/Sandbox/PlanetTarget.hpp"

#include <DirectXColors.h>

#include <imgui.h>
#include <UI/ImGuiDx11.h>
#include <UI/ImGuiWin32.h>

void DebugSandboxState::Init(DX::DeviceResources* resources, DirectX::Mouse* mouse, DirectX::Keyboard* keyboard, StateData& data)
{
    Device = resources->GetD3DDevice();
    Context = resources->GetD3DDeviceContext();
    DeviceResources = resources;
    Mouse = mouse;
    Keyboard = keyboard;

    Camera = std::make_unique<CShipCamera>();

    SetupTargets();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGui_ImplWin32_Init(resources->GetWindow());
    ImGui_ImplDX11_Init(Device, Context);
}

void DebugSandboxState::Cleanup()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    auto t = &RootTarget;

    while(*t)
    {
        t = &t->get()->Child;

        if(*t)
            t->reset();
    }
}

void DebugSandboxState::Update(float dt)
{
    
}

void DebugSandboxState::Render()
{
    Clear();

    auto rtv = DeviceResources->GetRenderTargetView();
    auto dsv = DeviceResources->GetDepthStencilView();

    Context->OMSetRenderTargets(1, &rtv, dsv);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (ImGui::Button("Test transitions"))
        TestTransitions();

    if (ImGui::Button("Test universe -> galaxy"))
    {
        for (int i = 0; i < 8; ++i)
        {
            RootTarget->Child->StartTransitionDownChild(Vector3::Zero, 0);
            RootTarget->StartTransitionDownParent(Vector3::Zero);
            RootTarget->EndTransitionDownParent(Vector3::Zero);
            RootTarget->Child->EndTransitionDownChild();
            CurrentTarget = RootTarget->Child.get();
        }
    }

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void DebugSandboxState::Clear()
{
    DeviceResources->PIXBeginEvent(L"Clear");

    auto depthStencil = DeviceResources->GetDepthStencilView();
    auto renderTarget = DeviceResources->GetRenderTargetView();

    Context->ClearRenderTargetView(renderTarget, DirectX::Colors::Black);
    Context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    auto viewport = DeviceResources->GetScreenViewport();
    Context->RSSetViewports(1, &viewport);

    DeviceResources->PIXEndEvent();
}

void DebugSandboxState::RenderUI()
{

}

void DebugSandboxState::SetupTargets()
{
    auto rtv = DeviceResources->GetSceneRenderTargetView();

    std::unique_ptr<SandboxTarget> Universe = std::make_unique<UniverseTarget>(Context, DeviceResources, Camera.get(), rtv);
    std::unique_ptr<SandboxTarget> Galaxy = std::make_unique<GalaxyTarget>(Context, DeviceResources, Camera.get(), rtv);
    std::unique_ptr<SandboxTarget> Star = std::make_unique<StarTarget>(Context, DeviceResources, Camera.get(), rtv);
    std::unique_ptr<SandboxTarget> Planet = std::make_unique<PlanetTarget>(Context, DeviceResources, Camera.get(), rtv);

    Galaxy->Parent = Universe.get();
    Star->Parent = Galaxy.get();
    Planet->Parent = Star.get();

    Universe->GlobalScale = Universe->Scale * Galaxy->Scale * Star->Scale * Planet->Scale;
    Galaxy->GlobalScale = Galaxy->Scale * Star->Scale * Planet->Scale;
    Star->GlobalScale = Star->Scale * Planet->Scale;
    Planet->GlobalScale = Planet->Scale;

    Star->Child = std::move(Planet);
    Galaxy->Child = std::move(Star);
    Universe->Child = std::move(Galaxy);

    CurrentTarget = Universe.get();
    RootTarget = std::move(Universe);
}

void DebugSandboxState::TestTransitions()
{
    auto TransitionDown = [&]() {
        if (CurrentTarget->Child)
        {
            auto object = CurrentTarget->GetClosestObject(Vector3::Zero);
            float objectDist = Vector3::Distance(Vector3::Zero, object);
            float scaledDistToObject = (objectDist - CurrentTarget->Child->EndTransitionDist) / CurrentTarget->Child->BeginTransitionDist;

            if (!CurrentTarget->IsTransitioning())
            {
                if (CurrentTarget->Child)
                {
                    CurrentTarget->Child->StartTransitionDownChild(object, 0);
                    CurrentTarget->StartTransitionDownParent(object);
                    CurrentTarget->EndTransitionDownParent(object);
                    CurrentTarget->Child->EndTransitionDownChild();
                    CurrentTarget = CurrentTarget->Child.get();
                }
            }
        }
    };

    auto TransitionUp = [&]() {
        if (CurrentTarget->Parent && !CurrentTarget->IsTransitioning())
        {
            float parentDist = Vector3::Distance(Vector3::Zero, CurrentTarget->GetCentre()) * CurrentTarget->Scale;
            float scaledDistToParent = (parentDist - CurrentTarget->EndTransitionDist) / CurrentTarget->BeginTransitionDist;

            if (CurrentTarget->Parent)
            {
                CurrentTarget = CurrentTarget->Parent;
                CurrentTarget->StartTransitionUpParent();
                CurrentTarget->Child->StartTransitionUpChild();
                CurrentTarget->EndTransitionUpParent();
                CurrentTarget->Child->EndTransitionUpChild();
            }
        }
    };
    
    for (int i = 0; i < 4; ++i)
    {
        TransitionDown();
        TransitionUp();
    }
}
