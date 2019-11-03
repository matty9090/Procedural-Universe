#include "SandboxState.hpp"
#include "Core/Maths.hpp"

#include "Services/Log.hpp"
#include "Services/ResourceManager.hpp"

#include "GalaxyTarget.hpp"
#include "StarTarget.hpp"

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

    auto sandboxData = static_cast<SandboxStateData&>(data);

    for (auto& p : sandboxData.Particles)
        p.Position *= 340.0f;

    CreateModelPipeline();
    SetupTargets(sandboxData.Particles);

    Ship = std::make_unique<CShip>(Device, RESM.GetMesh("assets/Ship.obj"));
    Ship->Scale(0.1f);
    Ship->Move(Vector3(0.0f, 0.0f, 5000.0f));

    Camera->Attach(Ship.get());

    CommonStates = std::make_unique<DirectX::CommonStates>(Device);
    PostProcess = std::make_unique<CPostProcess>(Device, Context, width, height);
}

void SandboxState::Cleanup()
{
    PostProcess.reset();
    CommonStates.reset();
    Ship.reset();
    Camera.reset();
    
    auto t = &RootTarget;

    while(*t)
    {
        t = &t->get()->Child;

        if(*t)
            t->reset();
    }
}

void SandboxState::Update(float dt)
{
    auto shipPos = Ship->GetPosition();

    if (!CurrentTarget->IsTransitioning() && shipPos.Length() > CamOriginSnapThreshold)
    {
        Ship->SetPosition(Vector3::Zero);
        CurrentTarget->MoveObjects(-shipPos);

        LOGM("Moving to origin")
    }
    
    if (CurrentTarget)
    {
        // Transition down calculations
        auto object = CurrentTarget->GetClosestObject(Ship->GetPosition());
        float objectDist = Vector3::Distance(Ship->GetPosition(), object);
        float scaledDistToObject = (objectDist - CurrentTarget->EndTransitionDist) / CurrentTarget->BeginTransitionDist;

        // Transition up calculations
        float parentDist = Vector3::Distance(Ship->GetPosition(), CurrentTarget->GetMainObject()) * CurrentTarget->Scale;
        float scaledDistToParent = (parentDist - CurrentTarget->EndTransitionDist) / CurrentTarget->BeginTransitionDist;

        if (!CurrentTarget->IsTransitioning())
        {
            bool isTransitioningUp = scaledDistToParent > 0.0f && CurrentTarget->Parent;
            bool isTransitioningDown = scaledDistToObject < 1.0f && CurrentTarget->Child;

            // Begin transition
            if (isTransitioningUp || isTransitioningDown)
            {
                // Parent starts rendering and lerping any visuals
                // Child starts rendering in the parent's world space
                // Correct the ships posititon

                if (isTransitioningUp)
                {
                    LOGM("Starting transition from " + CurrentTarget->Name + " to " + CurrentTarget->Parent->Name)
                    
                    CurrentTarget = CurrentTarget->Parent;
                    scaledDistToObject = scaledDistToParent;

                    Ship->Move(-CurrentTarget->Child->GetMainObject());
                    Ship->SetPosition(Ship->GetPosition() * CurrentTarget->Child->Scale);
                    Ship->Move(CurrentTarget->Child->ParentLocationSpace);

                    CurrentTarget->Child->StartTransitionUpChild();
                }
                else
                {
                    LOGM("Starting transition from " + CurrentTarget->Name + " to " + CurrentTarget->Child->Name)

                    CurrentTarget->Child->StartTransitionDownChild(object);
                }

                CurrentTarget->StartTransitionParent();
            }
        }
        else
        {
            // End transition up
            if (scaledDistToObject > 1.0f)
            {
                // Child stops rendering
                // Parent stops lerping visuals

                CurrentTarget->EndTransitionParent();
                CurrentTarget->Child->EndTransitionUpChild();
                Ship->VelocityScale = 1.0f;

                LOGM("Ending transition from " + CurrentTarget->Child->Name + " to " + CurrentTarget->Name)
            }
            // End transition down
            else if (scaledDistToObject < 0.0f)
            {
                // Parent stops rendering
                // Current target becomes child and starts rendering normally
                // Ship to child space

                CurrentTarget->EndTransitionParent();
                CurrentTarget->Child->EndTransitionDownChild();
                CurrentTarget = CurrentTarget->Child.get();

                Ship->Move(-CurrentTarget->ParentLocationSpace);
                Ship->SetPosition(Ship->GetPosition() / CurrentTarget->Scale);
                Ship->VelocityScale = 1.0f;

                LOGM("Ending transition from " + CurrentTarget->Parent->Name + " to " + CurrentTarget->Name)
            }
            // In transition
            else
            {
                CurrentTransitionT = Maths::Lerp(CurrentTarget->Child->Scale, CurrentTarget->Scale, scaledDistToObject);
                Ship->VelocityScale = CurrentTransitionT;
            }
        }
    }

    Camera->Events(Mouse, Mouse->GetState(), dt);
    CurrentTarget->Update(dt);

    Ship->Control(Mouse, Keyboard, dt);
    Ship->Update(dt);
    Camera->Update(dt);
}

void SandboxState::Render()
{
    Clear();

    if (CurrentTarget->IsTransitioning())
    {
        CurrentTarget->RenderTransition(1.0f - CurrentTransitionT);
        CurrentTarget->Child->RenderTransition(CurrentTransitionT);
    }
    else
    {
        CurrentTarget->Render();
    }

    auto rtv = DeviceResources->GetRenderTargetView();
    auto dsv = DeviceResources->GetDepthStencilView(); 

    Context->OMSetRenderTargets(1, &rtv, dsv);
    
    Matrix viewProj = Camera->GetViewMatrix() * Camera->GetProjectionMatrix();

    auto sampler = CommonStates->AnisotropicWrap();
    Context->OMSetBlendState(CommonStates->Opaque(), DirectX::Colors::Black, 0xFFFFFFFF);
    Context->OMSetDepthStencilState(CommonStates->DepthDefault(), 0);
    Context->RSSetState(CommonStates->CullClockwise());
    Context->PSSetSamplers(0, 1, &sampler);

    Ship->Draw(Context, viewProj, ModelPipeline);
}

void SandboxState::Clear()
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

void SandboxState::CreateModelPipeline()
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ModelPipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    ModelPipeline.LoadVertex(L"shaders/PositionTexture.vsh");
    ModelPipeline.LoadPixel(L"shaders/Texture.psh");
    ModelPipeline.CreateInputLayout(Device, layout);
}

void SandboxState::SetupTargets(const std::vector<Particle>& seedData)
{
    std::vector<Particle> seedData2 = seedData;
    seedData2.erase(seedData2.end() - seedData2.size() + 60, seedData2.end());

    for (auto& p : seedData2)
        p.Position /= 50;

    std::unique_ptr<SandboxTarget> Galaxy = std::make_unique<GalaxyTarget>(Context, DeviceResources, Camera.get(), seedData);
    std::unique_ptr<SandboxTarget> Star   = std::make_unique<StarTarget>  (Context, DeviceResources, Camera.get(), seedData2);

    Star->Parent = Galaxy.get();
    Galaxy->Child = std::move(Star);

    CurrentTarget = Galaxy.get();
    RootTarget = std::move(Galaxy);
}
