#include "SandboxState.hpp"
#include <DirectXColors.h>

void SandboxState::Init(DX::DeviceResources* resources, DirectX::Mouse* mouse, DirectX::Keyboard* keyboard, StateData& data)
{
    Device = resources->GetD3DDevice();
    Context = resources->GetD3DDeviceContext();
    DeviceResources = resources;
    Mouse = mouse;

    CommonStates = std::make_unique<DirectX::CommonStates>(Device);
    
    auto vp = DeviceResources->GetScreenViewport();
    Camera = std::make_unique<CCamera>(vp.Width, vp.Height);

    auto sandboxData = static_cast<SandboxStateData&>(data);
    Particles = sandboxData.Particles;

    CreateModelPipeline();
    CreateParticlePipeline();

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
    
    Matrix view = Camera->GetViewMatrix();
    Matrix viewProj = view * Camera->GetProjectionMatrix();

    ParticlePipeline.SetState(Context, [&]() {
        unsigned int offset = 0;
        unsigned int stride = sizeof(Particle);

        Context->IASetVertexBuffers(0, 1, ParticleBuffer.GetAddressOf(), &stride, &offset);
        GSBuffer->SetData(Context, GSConstantBuffer { viewProj, view.Invert() });
        Context->GSSetConstantBuffers(0, 1, GSBuffer->GetBuffer());
        Context->RSSetState(DeviceResources->GetRasterizerState());
        Context->OMSetBlendState(CommonStates->Additive(), DirectX::Colors::Black, 0xFFFFFFFF);
        Context->Draw(Particles.size(), 0);
    });
    
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

void SandboxState::CreateParticlePipeline()
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ParticlePipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    ParticlePipeline.LoadVertex(Device, L"shaders/PassThruGS.vsh");
    ParticlePipeline.LoadPixel(Device, L"shaders/PlainColour.psh");
    ParticlePipeline.LoadGeometry(Device, L"shaders/DrawParticle.gsh");
    ParticlePipeline.CreateInputLayout(Device, layout);

    CreateParticleBuffer(Device, ParticleBuffer.ReleaseAndGetAddressOf(), Particles);
    GSBuffer = std::make_unique<ConstantBuffer<GSConstantBuffer>>(Device);
}

