#include "SandboxState.hpp"
#include "Services/Log.hpp"
#include "Core/Maths.hpp"

#include "GalaxyTarget.hpp"

#include <DirectXColors.h>

void SandboxState::Init(DX::DeviceResources* resources, DirectX::Mouse* mouse, DirectX::Keyboard* keyboard, StateData& data)
{
    Device = resources->GetD3DDevice();
    Context = resources->GetD3DDeviceContext();
    DeviceResources = resources;
    Mouse = mouse;
    Keyboard = keyboard;

    CommonStates = std::make_unique<DirectX::CommonStates>(Device);
    
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

    auto ShipMesh = CMesh::Load(Device, "resources/Ship.obj");
    Ship = std::make_unique<CShip>(Device, ShipMesh.get());
    Ship->Scale(1.0f);
    Meshes.push_back(std::move(ShipMesh));

    Camera->Attach(Ship.get());
}

void SandboxState::Cleanup()
{

}

void SandboxState::Update(float dt)
{
    auto shipPos = Ship->GetPosition();

    if (shipPos.Length() > CamLengthThreshold)
    {
        Ship->SetPosition(Vector3::Zero);
        CurrentTarget->MoveObjects(-shipPos);
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
    CurrentTarget->Render();

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
    ModelPipeline.LoadVertex(Device, L"shaders/PositionTexture.vsh");
    ModelPipeline.LoadPixel(Device, L"shaders/Texture.psh");
    ModelPipeline.CreateInputLayout(Device, layout);
}

void SandboxState::SetupTargets(const std::vector<Particle>& seedData)
{
    std::unique_ptr<SandboxTarget> Galaxy = std::make_unique<GalaxyTarget>(Context, "Galactic", DeviceResources, Camera.get(), seedData);

    CurrentTarget = Galaxy.get();
    RootTarget = std::move(Galaxy);
}
