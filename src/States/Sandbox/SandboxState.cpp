#include "SandboxState.hpp"
#include "Services/Log.hpp"
#include "Core/Maths.hpp"

#include <DirectXColors.h>

void SandboxState::Init(DX::DeviceResources* resources, DirectX::Mouse* mouse, DirectX::Keyboard* keyboard, StateData& data)
{
    Device = resources->GetD3DDevice();
    Context = resources->GetD3DDeviceContext();
    DeviceResources = resources;
    Mouse = mouse;
    Keyboard = keyboard;

    CommonStates = std::make_unique<DirectX::CommonStates>(Device);
    DualPostProcess = std::make_unique<DirectX::DualPostProcess>(Device);
    BasicPostProcess = std::make_unique<DirectX::BasicPostProcess>(Device);
    
    auto vp = DeviceResources->GetScreenViewport();
    unsigned int width = static_cast<size_t>(vp.Width);
    unsigned int height = static_cast<size_t>(vp.Height);

    Camera = std::make_unique<CShipCamera>(width, height);
    Camera->SetPosition(Vector3(0.0f, 8.0f, -30.0f));

    PostProcess = std::make_unique<CPostProcess>(Device, Context, width, height);

    auto sandboxData = static_cast<SandboxStateData&>(data);
    Particles = sandboxData.Particles;

    for (auto& p : Particles)
        p.Position *= 340.0f;

    CreateModelPipeline();
    CreateParticlePipeline();

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
    Camera->Update(dt);
    Camera->Events(Mouse, Mouse->GetState(), dt);

    Ship->Control(Mouse, Keyboard, dt);
    Ship->Update(dt);

    auto closest = Maths::ClosestParticle(Ship->GetPosition(), Particles);
    float d = Vector3::Distance(Ship->GetPosition(), closest.Position);

    float scaledDist = Maths::Clamp((d - 400.0f) / 1800.0f, 0.0f, 1.0f);
    Ship->VelocityScale = Maths::Lerp(StellarSpeed, GalacticSpeed, scaledDist);
}

void SandboxState::Render()
{
    Clear();

    auto rtv = DeviceResources->GetRenderTargetView();
    auto dsv = DeviceResources->GetDepthStencilView();

    SetRenderTarget(Context, ParticleRenderTarget);
    
    Matrix view = Camera->GetViewMatrix();
    Matrix viewProj = view * Camera->GetProjectionMatrix();

    ParticlePipeline.SetState(Context, [&]() {
        unsigned int offset = 0;
        unsigned int stride = sizeof(Particle);

        Context->IASetVertexBuffers(0, 1, ParticleBuffer.GetAddressOf(), &stride, &offset);
        GSBuffer->SetData(Context, GSConstantBuffer { viewProj, view.Invert() });
        Context->GSSetConstantBuffers(0, 1, GSBuffer->GetBuffer());
        Context->RSSetState(CommonStates->CullNone());
        Context->OMSetBlendState(CommonStates->Additive(), DirectX::Colors::Black, 0xFFFFFFFF);
        Context->Draw(static_cast<unsigned int>(Particles.size()), 0);
    });

    Context->GSSetShader(nullptr, 0, 0);

    PostProcess->Render(rtv, dsv, ParticleRenderTarget.Srv.Get());

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
    Context->ClearRenderTargetView(DeviceResources->GetSceneRenderTargetView(), DirectX::Colors::Black);

    ParticleRenderTarget.Clear(Context);

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

void SandboxState::CreateParticlePipeline()
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ParticlePipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    ParticlePipeline.LoadVertex(Device, L"shaders/PassThruGS.vsh");
    ParticlePipeline.LoadPixel(Device, L"shaders/PlainColour.psh");
    ParticlePipeline.LoadGeometry(Device, L"shaders/SandboxParticle.gsh");
    ParticlePipeline.CreateInputLayout(Device, layout);

    CreateParticleBuffer(Device, ParticleBuffer.ReleaseAndGetAddressOf(), Particles);
    GSBuffer = std::make_unique<ConstantBuffer<GSConstantBuffer>>(Device);

    auto vp = DeviceResources->GetScreenViewport();
    ParticleRenderTarget = CreateTarget(Device, static_cast<int>(vp.Width), static_cast<int>(vp.Height));
}

