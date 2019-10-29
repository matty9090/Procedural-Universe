#include "SandboxState.hpp"
#include <DirectXColors.h>

void SandboxState::Init(DX::DeviceResources* resources, DirectX::Mouse* mouse, DirectX::Keyboard* keyboard, StateData& data)
{
    Device = resources->GetD3DDevice();
    Context = resources->GetD3DDeviceContext();
    DeviceResources = resources;
}

void SandboxState::Cleanup()
{

}

void SandboxState::Update(float dt)
{

}

void SandboxState::Render()
{
    Clear();
}

void SandboxState::Clear()
{
    DeviceResources->PIXBeginEvent(L"Clear");

    auto context = DeviceResources->GetD3DDeviceContext();

    auto depthStencil = DeviceResources->GetDepthStencilView();
    auto renderTarget = DeviceResources->GetSceneRenderTargetView();

    context->ClearRenderTargetView(renderTarget, DirectX::Colors::Black);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    auto viewport = DeviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    DeviceResources->PIXEndEvent();
}