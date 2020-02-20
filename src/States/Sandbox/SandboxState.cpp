#include "SandboxState.hpp"
#include "Core/Maths.hpp"

#include "Services/Log.hpp"
#include "Services/ResourceManager.hpp"

#include "UniverseTarget.hpp"
#include "GalaxyTarget.hpp"
#include "StarTarget.hpp"
#include "PlanetTarget.hpp"

#include "Render/Planet/Components/TerrainComponent.hpp"

#include <sstream>
#include <DirectXColors.h>

#include <imgui.h>
#include <UI/ImGuiDx11.h>
#include <UI/ImGuiWin32.h>

void SandboxState::Init(DX::DeviceResources* resources, DirectX::Mouse* mouse, DirectX::Keyboard* keyboard, StateData& data)
{
    Device = resources->GetD3DDevice();
    Context = resources->GetD3DDeviceContext();
    DeviceResources = resources;
    Mouse = mouse;
    Keyboard = keyboard;
    
    auto vp = DeviceResources->GetScreenViewport();
    unsigned int width = static_cast<size_t>(vp.Width);
    unsigned int height = static_cast<size_t>(vp.Height);

    Camera = std::make_unique<CSandboxCamera>(width, height);
    Camera->SetPosition(Vector3(0.0f, 0.0f, 5000.0f));

    CTerrainComponent<TerrainHeightFunc>::GeneratePermutations();
    CTerrainComponent<WaterHeightFunc>::GeneratePermutations();

    CreateParticleBuffer(Device, Galaxy::ParticleBuffer.ReleaseAndGetAddressOf(), PARTICLES_PER_GALAXY);

    CreateModelPipeline();
    SetupTargets();

    CommonStates = std::make_unique<DirectX::CommonStates>(Device);
    ClosestObjCube = std::make_unique<Cube>(Context);
    
    Font = std::make_unique<DirectX::SpriteFont>(Device, L"assets/SegoeUI.font");
    SpriteBatch = std::make_unique<DirectX::SpriteBatch>(Context);

    PostProcess = std::make_unique<CPostProcess>(Device, Context, width, height);
    PostProcess->GaussianBlur = 5.0f;

    Planet = std::make_unique<CPlanet>(Context, *Camera);
    Planet->SetPosition(Vector3(0.0f, 0.0f, 20000.0f));

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGui_ImplWin32_Init(resources->GetWindow());
    ImGui_ImplDX11_Init(Device, Context);
}

void SandboxState::Cleanup()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    Font.reset();
    PostProcess.reset();
    CommonStates.reset();
    Camera.reset();
    SpriteBatch.reset();
    
    auto t = &RootTarget;

    while (*t)
    {
        t = &t->get()->Child;

        if (*t)
            t->reset();
    }
}

void SandboxState::Update(float dt)
{
    ++Frames;
    FrameTimer += dt;

    if (FrameTimer >= 1.0f)
    {
        std::string fps = std::to_string(static_cast<int>((1.0f * Frames) / FrameTimer));
        SetWindowTextA(DeviceResources->GetWindow(), (std::string("NBody Simulator [FPS: ") + fps + "]").c_str());
        Frames = 0, FrameTimer = 0.0f;
    }

    Tracker.Update(Keyboard->GetState());

    if (Tracker.IsKeyReleased(DirectX::Keyboard::F1))
        bShowClosestObject = !bShowClosestObject;

    if (Tracker.IsKeyReleased(DirectX::Keyboard::F2))
        PostProcess->UseBloom = !PostProcess->UseBloom;

    FloatingOrigin();
    TransitionLogic();

    if (!CurrentTarget->IsTransitioning())
        Camera->VelocityScale = CurrentTarget->VelocityMultiplier;

    Camera->Events(Mouse, Mouse->GetState(), Keyboard->GetState(), dt);
    CurrentTarget->Update(dt);
    Camera->Update(dt);

    if (CurrentTarget->Parent)
        CurrentTarget->Parent->GetSkyBox().SetPosition(Camera->GetPosition());

    Planet->Update(dt);
}

void SandboxState::Render()
{
    Clear();

    if (CurrentTarget->IsTransitioning())
    {
        CurrentTarget->RenderTransitionParent(CurrentTransitionT);
        CurrentTarget->Child->RenderTransitionChild(1.0f - CurrentTransitionT);
    }
    else
    {
        CurrentTarget->Render();
    }

    Planet->Render();

    auto rtv = DeviceResources->GetRenderTargetView();
    auto dsv = DeviceResources->GetDepthStencilView();

    PostProcess->Render(rtv, dsv, DeviceResources->GetSceneTexture());

    Matrix viewProj = Camera->GetViewMatrix() * Camera->GetProjectionMatrix();

    Context->OMSetRenderTargets(1, &rtv, dsv);

    auto sampler = CommonStates->AnisotropicWrap();
    Context->OMSetBlendState(CommonStates->Opaque(), DirectX::Colors::Black, 0xFFFFFFFF);
    Context->OMSetDepthStencilState(CommonStates->DepthDefault(), 0);
    Context->PSSetSamplers(0, 1, &sampler);

    if (bShowClosestObject && CurrentTarget->Child != nullptr)
    {
        auto closest = CurrentTarget->GetClosestObject(Camera->GetPosition());
        ClosestObjCube->Render(closest, 2.0f, Camera->GetViewMatrix() * Camera->GetProjectionMatrix(), false);

        int x, y;
        Camera->PixelFromWorldPoint(closest, x, y);

        const auto vp = DeviceResources->GetScreenViewport();
        const int w = static_cast<int>(vp.Width);
        const int h = static_cast<int>(vp.Height);

        // Don't show closest if behind the camera
        //if (Camera->GetForward().Dot(closest) > 0.0f)
        {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(static_cast<float>(Maths::Clamp(x - 200, 10, w - 170)), static_cast<float>(Maths::Clamp(y - 160, 16, h - 110))));
            ImGui::SetNextWindowSize(ImVec2(160, 100));
            ImGui::SetNextWindowBgAlpha(0.5f);

            ImGui::Begin(CurrentTarget->ObjName.c_str(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
            ImGui::Text("Index: %i", CurrentTarget->GetClosestObjectIndex());

            if (CurrentTarget->Parent)
                ImGui::Text("Parent index: %i", CurrentTarget->Parent->GetClosestObjectIndex());

            ImGui::Text("Distance: %i", static_cast<int>(Vector3::Distance(Camera->GetPosition(), closest)));

            if (CurrentTarget->IsTransitioning())
                ImGui::Text("Transition: %i%%", static_cast<int>(CurrentTransitionT * 100.0f));

            ImGui::End();

            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }
    }

    auto speed = GetSpeedStr(static_cast<double>(Camera->GetSpeed()) / (CurrentTarget->GlobalScale));

    SpriteBatch->Begin();
    Font->DrawString(SpriteBatch.get(), (std::string("Speed: ") + speed).c_str(), Vector3(2.0f, 2.0, 0.0f));
    SpriteBatch->End();
}

void SandboxState::Clear()
{
    DeviceResources->PIXBeginEvent(L"Clear");

    auto depthStencil = DeviceResources->GetDepthStencilView();
    auto renderTarget = DeviceResources->GetRenderTargetView();
    auto sceneTarget  = DeviceResources->GetSceneRenderTargetView();

    Context->ClearRenderTargetView(renderTarget, DirectX::Colors::Black);
    Context->ClearRenderTargetView(sceneTarget , DirectX::Colors::Black);
    Context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    auto viewport = DeviceResources->GetScreenViewport();
    Context->RSSetViewports(1, &viewport);

    DeviceResources->PIXEndEvent();
}

void SandboxState::FloatingOrigin()
{
    auto camPos = Camera->GetPosition();

    if (!CurrentTarget->IsTransitioning() && camPos.Length() > CamOriginSnapThreshold)
    {
        Camera->SetPosition(Vector3::Zero);
        CurrentTarget->MoveObjects(-camPos);
        Planet->Move(-camPos);
    }
}

void SandboxState::TransitionLogic()
{
    if (CurrentTarget->Parent && !CurrentTarget->IsTransitioning())
    {
        // Transition up calculations
        float parentDist = Vector3::Distance(Camera->GetPosition(), CurrentTarget->GetCentre()) * CurrentTarget->Scale;
        float scaledDistToParent = (parentDist - CurrentTarget->EndTransitionDist) / CurrentTarget->BeginTransitionDist;

        if (scaledDistToParent > 0.0f && CurrentTarget->Parent)
        {
            // Swap the child and parent for simpler implementation
            // Correct the ships posititon

            LOGM("Starting up transition from " + CurrentTarget->Name + " to " + CurrentTarget->Parent->Name)

            CurrentTarget = CurrentTarget->Parent;

            Camera->Move(-CurrentTarget->Child->GetCentre());
            Camera->SetPosition(Camera->GetPosition() * CurrentTarget->Child->Scale);
            Camera->Move(CurrentTarget->Child->ParentLocationSpace);

            CurrentTarget->StartTransitionUpParent();
            CurrentTarget->Child->StartTransitionUpChild();
        }
    }

    if (CurrentTarget->Child)
    {
        // Transition down calculations
        auto object = CurrentTarget->GetClosestObject(Camera->GetPosition());
        auto newIndex = CurrentTarget->GetClosestObjectIndex();

        float objectDist = Vector3::Distance(Camera->GetPosition(), object);
        float scaledDistToObject = (objectDist - CurrentTarget->Child->EndTransitionDist) / CurrentTarget->Child->BeginTransitionDist;

        // Cancel transition if there's another close object (TODO: Re-enable this if needed)
        /*if (CurrentTarget->IsTransitioning() && newIndex != ClosestObjIndex)
        {
            ClosestObjIndex = newIndex;
            LOGM("There's another close object, switching to new one")

            CurrentTarget->EndTransitionUpParent();
            CurrentTarget->Child->EndTransitionUpChild();

            CurrentTransitionT = Maths::Lerp(CurrentTarget->Child->Scale, 1.0f, scaledDistToObject);
            Camera->VelocityScale = CurrentTransitionT;
        }*/

        if (!CurrentTarget->IsTransitioning())
        {
            // Down transition
            if (scaledDistToObject < 1.0f && CurrentTarget->Child)
            {
                // Parent starts rendering and lerping any visuals
                // Child starts rendering in the parent's world space
                // Correct the ships posititon

                LOGM("Starting down transition from " + CurrentTarget->Name + " to " + CurrentTarget->Child->Name)
                
                CurrentTarget->Child->StartTransitionDownChild(object, CurrentTarget->GetClosestObjectIndex());
                CurrentTarget->StartTransitionDownParent(object);
            }
        }
        else
        {
            // End transition up
            if (scaledDistToObject > 1.0f)
            {
                // Child stops rendering
                // Parent stops lerping visuals

                CurrentTarget->EndTransitionUpParent();
                CurrentTarget->Child->EndTransitionUpChild();
                Camera->VelocityScale = 1.0f;

                LOGM("Ending up transition from " + CurrentTarget->Child->Name + " to " + CurrentTarget->Name)
            }
            // End transition down
            else if (scaledDistToObject < 0.0f)
            {
                // Parent stops rendering
                // Current target becomes child and starts rendering normally
                // Convert to child space

                CurrentTarget->EndTransitionDownParent(object);
                CurrentTarget->Child->EndTransitionDownChild();
                CurrentTarget = CurrentTarget->Child.get();

                CurrentTarget->Parent->GetSkyBox().SetPosition(Camera->GetPosition());

                Camera->Move(-CurrentTarget->ParentLocationSpace);
                Camera->SetPosition(Camera->GetPosition() / CurrentTarget->Scale);
                Camera->VelocityScale = 1.0f;

                LOGM("Ending down transition from " + CurrentTarget->Parent->Name + " to " + CurrentTarget->Name)
            }
            // In transition
            else
            {
                CurrentTransitionT = Maths::Lerp(CurrentTarget->Child->Scale, 1.0f, scaledDistToObject);
                Camera->VelocityScale = CurrentTransitionT;
            }
        }
    }
}

void SandboxState::CreateModelPipeline()
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ModelPipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    ModelPipeline.LoadVertex(L"shaders/Standard/PositionTexture.vsh");
    ModelPipeline.LoadPixel(L"shaders/Standard/Texture.psh");
    ModelPipeline.CreateRasteriser(Device, ECullMode::Clockwise);
    ModelPipeline.CreateInputLayout(Device, layout);
}

void SandboxState::SetupTargets()
{
    auto rtv = DeviceResources->GetSceneRenderTargetView();

    std::unique_ptr<SandboxTarget> Universe = std::make_unique<UniverseTarget>(Context, DeviceResources, Camera.get(), rtv);
    std::unique_ptr<SandboxTarget> Galaxy   = std::make_unique<GalaxyTarget>  (Context, DeviceResources, Camera.get(), rtv);
    std::unique_ptr<SandboxTarget> Star     = std::make_unique<StarTarget>    (Context, DeviceResources, Camera.get(), rtv);
    std::unique_ptr<SandboxTarget> Planet   = std::make_unique<PlanetTarget>  (Context, DeviceResources, Camera.get(), rtv);

    Galaxy->Parent = Universe.get();
    Star->Parent = Galaxy.get();
    Planet->Parent = Star.get();

    Universe->GlobalScale = Universe->Scale * Galaxy->Scale * Star->Scale * Planet->Scale;
    Galaxy->GlobalScale = Galaxy->Scale * Star->Scale * Planet->Scale;
    Star->GlobalScale = Star->Scale * Planet->Scale;
    Planet->GlobalScale = Planet->Scale;

    Star->Child   = std::move(Planet);
    Galaxy->Child = std::move(Star);
    Universe->Child = std::move(Galaxy);

    CurrentTarget = Universe.get();
    RootTarget = std::move(Universe);
}

std::string SandboxState::GetSpeedStr(double speed)
{
    int power = static_cast<int>(log10(speed));
    std::ostringstream str;

    if (power <= 3)
    {
        str << static_cast<int>(speed);
        str << " m/s";
    }
    else if (power <= 6)
    {
        speed /= 3;
        str << static_cast<int>(speed);
        str << " km/s";
    }
    else
    {
        speed /= 299792458.0f;
        str.precision(2);
        str << std::fixed << speed;
        str << " c";
    }

    return str.str();
}
