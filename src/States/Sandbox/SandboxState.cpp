#include "SandboxState.hpp"
#include "Core/Maths.hpp"

#include "Services/Log.hpp"
#include "Services/ResourceManager.hpp"

#include "UniverseTarget.hpp"
#include "GalaxyTarget.hpp"
#include "StarTarget.hpp"
#include "PlanetTarget.hpp"

#include "Render/Planet/Components/TerrainComponent.hpp"

#include <DirectXColors.h>

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

    Camera = std::make_unique<CShipCamera>(width, height);
    Camera->SetPosition(Vector3(0.0f, 4.0f, -30.0f));

    CTerrainComponent::GeneratePermutations();

    CreateModelPipeline();
    SetupTargets();

    Ship = std::make_unique<CShip>(Device, RESM.GetMesh("assets/Ship.obj"));
    Ship->Scale(0.1f);
    Ship->Move(Vector3(0.0f, 0.0f, 5000.0f));

    Camera->Attach(Ship.get());

    CommonStates = std::make_unique<DirectX::CommonStates>(Device);
    ClosestObjCube = std::make_unique<Cube>(Context);

    PostProcess = std::make_unique<CPostProcess>(Device, Context, width, height);
    PostProcess->GaussianBlur = 5.0f;

    Planet = std::make_unique<CPlanet>(Context, Camera.get());
    //Planet->SetPosition(Vector3(0.0f, 0.0f, 20000.0f));
}

void SandboxState::Cleanup()
{
    PostProcess.reset();
    CommonStates.reset();
    Ship.reset();
    Camera.reset();
    
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
    Tracker.Update(Keyboard->GetState());

    if (Tracker.IsKeyReleased(DirectX::Keyboard::F1))
        bShowClosestObject = !bShowClosestObject;

    if (Tracker.IsKeyReleased(DirectX::Keyboard::F2))
        PostProcess->UseBloom = !PostProcess->UseBloom;

    FloatingOrigin();
    TransitionLogic();

    if (!CurrentTarget->IsTransitioning())
        Ship->VelocityScale = CurrentTarget->VelocityMultiplier;

    Camera->Events(Mouse, Mouse->GetState(), dt);
    CurrentTarget->Update(dt);

    Ship->Control(Mouse, Keyboard, dt);
    Ship->Update(dt);
    Camera->Update(dt);

    if (CurrentTarget->Parent)
        CurrentTarget->Parent->GetSkyBox().SetPosition(Camera->GetPosition());

    //Planet->Update(dt);
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

    //Planet->Render();

    auto rtv = DeviceResources->GetRenderTargetView();
    auto dsv = DeviceResources->GetDepthStencilView();

    PostProcess->Render(rtv, dsv, DeviceResources->GetSceneTexture());

    Matrix viewProj = Camera->GetViewMatrix() * Camera->GetProjectionMatrix();

    Context->OMSetRenderTargets(1, &rtv, dsv);

    auto sampler = CommonStates->AnisotropicWrap();
    Context->OMSetBlendState(CommonStates->Opaque(), DirectX::Colors::Black, 0xFFFFFFFF);
    Context->OMSetDepthStencilState(CommonStates->DepthDefault(), 0);
    Context->PSSetSamplers(0, 1, &sampler);

    Ship->Draw(Context, viewProj, ModelPipeline);

    if (bShowClosestObject)
    {
        auto closest = CurrentTarget->GetClosestObject(Ship->GetPosition());
        ClosestObjCube->Render(closest, 100.0f, Camera->GetViewMatrix() * Camera->GetProjectionMatrix(), false);
    }
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
    auto shipPos = Ship->GetPosition();

    if (!CurrentTarget->IsTransitioning() && shipPos.Length() > CamOriginSnapThreshold)
    {
        Ship->SetPosition(Vector3::Zero);
        Planet->Move(-shipPos);
        CurrentTarget->MoveObjects(-shipPos);
    }
}

void SandboxState::TransitionLogic()
{
    if (CurrentTarget->Parent && !CurrentTarget->IsTransitioning())
    {
        // Transition up calculations
        float parentDist = Vector3::Distance(Ship->GetPosition(), CurrentTarget->GetCentre()) * CurrentTarget->Scale;
        float scaledDistToParent = (parentDist - CurrentTarget->EndTransitionDist) / CurrentTarget->BeginTransitionDist;

        if (scaledDistToParent > 0.0f && CurrentTarget->Parent)
        {
            // Swap the child and parent for simpler implementation
            // Correct the ships posititon

            LOGM("Starting up transition from " + CurrentTarget->Name + " to " + CurrentTarget->Parent->Name)

            CurrentTarget = CurrentTarget->Parent;

            Ship->Move(-CurrentTarget->Child->GetCentre());
            Ship->SetPosition(Ship->GetPosition() * CurrentTarget->Child->Scale);
            Ship->Move(CurrentTarget->Child->ParentLocationSpace);

            CurrentTarget->StartTransitionUpParent();
            CurrentTarget->Child->StartTransitionUpChild();
        }
    }

    if (CurrentTarget->Child)
    {
        // Transition down calculations
        auto object = CurrentTarget->GetClosestObject(Ship->GetPosition());
        float objectDist = Vector3::Distance(Ship->GetPosition(), object);
        float scaledDistToObject = (objectDist - CurrentTarget->Child->EndTransitionDist) / CurrentTarget->Child->BeginTransitionDist;

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
                Ship->VelocityScale = 1.0f;

                LOGM("Ending up transition from " + CurrentTarget->Child->Name + " to " + CurrentTarget->Name)
            }
            // End transition down
            else if (scaledDistToObject < 0.0f)
            {
                // Parent stops rendering
                // Current target becomes child and starts rendering normally
                // Ship to child space

                CurrentTarget->EndTransitionDownParent(object);
                CurrentTarget->Child->EndTransitionDownChild();
                CurrentTarget = CurrentTarget->Child.get();

                CurrentTarget->Parent->GetSkyBox().SetPosition(Camera->GetPosition());

                Ship->Move(-CurrentTarget->ParentLocationSpace);
                Ship->SetPosition(Ship->GetPosition() / CurrentTarget->Scale);
                Ship->VelocityScale = 1.0f;

                LOGM("Ending down transition from " + CurrentTarget->Parent->Name + " to " + CurrentTarget->Name)
            }
            // In transition
            else
            {
                CurrentTransitionT = Maths::Lerp(CurrentTarget->Child->Scale, 1.0f, scaledDistToObject);
                Ship->VelocityScale = CurrentTransitionT;
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
    ModelPipeline.LoadVertex(L"shaders/PositionTexture.vsh");
    ModelPipeline.LoadPixel(L"shaders/Texture.psh");
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

    Star->Child   = std::move(Planet);
    Galaxy->Child = std::move(Star);
    Universe->Child = std::move(Galaxy);

    CurrentTarget = Universe.get();
    RootTarget = std::move(Universe);
}
