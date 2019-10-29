#include "SandboxState.hpp"
#include <DirectXColors.h>

void SandboxState::Init(DX::DeviceResources* resources, DirectX::Mouse* mouse, DirectX::Keyboard* keyboard, StateData& data)
{
    Device = resources->GetD3DDevice();
    Context = resources->GetD3DDeviceContext();
    DeviceResources = resources;
    Mouse = mouse;

    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ModelPipeline.LoadVertex(Device, L"shaders/PositionTexture.vsh");
    ModelPipeline.LoadPixel(Device, L"shaders/Texture.psh");
    ModelPipeline.CreateInputLayout(Device, layout);
    
    auto vp = DeviceResources->GetScreenViewport();
    Camera = std::make_unique<CCamera>(vp.Width, vp.Height);

    auto sandboxData = static_cast<SandboxStateData&>(data);
    sandboxData.Particles;

    auto ShipMesh = CMesh::Load(Device, "resources/Ship.obj");
    Ship = std::make_unique<CModel>(Device, ShipMesh.get());

    Meshes.push_back(std::move(ShipMesh));
}

void SandboxState::Cleanup()
{

}

void SandboxState::Update(float dt)
{
    Camera->Update(dt);
    Camera->Events(Mouse, Mouse->GetState(), dt);
}

void SandboxState::Render()
{
    Clear();

    auto rtv = DeviceResources->GetRenderTargetView();
    auto dsv = DeviceResources->GetDepthStencilView();

    Context->OMSetRenderTargets(1, &rtv, dsv);

    auto viewProj = Camera->GetViewMatrix() * Camera->GetProjectionMatrix();
    
    Ship->Draw(Context, viewProj, ModelPipeline);
}

void SandboxState::Clear()
{
    DeviceResources->PIXBeginEvent(L"Clear");

    auto context = DeviceResources->GetD3DDeviceContext();

    auto depthStencil = DeviceResources->GetDepthStencilView();
    auto renderTarget = DeviceResources->GetRenderTargetView();

    context->ClearRenderTargetView(renderTarget, DirectX::Colors::Black);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    auto viewport = DeviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    DeviceResources->PIXEndEvent();
}